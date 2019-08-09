/*
 * Copyright (c) 2019 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#include <fuse.h>
#include <crystal.h>

#include "config.h"

#define HIVETEST_REDIRECT_URL "http://localhost:12345"
#define HIVETEST_SCOPE "User.Read Files.ReadWrite.All offline_access"
#define HIVETEST_ONEDRIVE_CLIENT_ID "afd3d647-a8b7-4723-bf9d-1b832f43b881"

typedef struct {
    hash_entry_t he;
    HiveFile *file;
} hive_file;

static struct options {
    const char *config_path;
    const char *backend_type;
    int debug;
    int show_help;
    config *conf;
    struct fuse_args *args;
} options;

static struct {
    HiveClient *client;
    HiveDrive *drive;
    hashtable_t *files;
} context;

#define OPTION(t, p) \
    { t, offsetof(struct options, p), 1 }
static const struct fuse_opt option_spec[] = {
	OPTION("--config=%s", config_path),
	OPTION("--type=%s", backend_type),
    OPTION("--debug", debug),
	OPTION("-h", show_help),
	OPTION("--help", show_help),
	FUSE_OPT_END
};

static void file_destructor(void *obj)
{
    hive_file *f = (hive_file *)obj;

    hashtable_remove(context.files, f->he.key, f->he.keylen);
    free((void *)f->he.key);
    hive_file_commit(f->file);
    hive_file_close(f->file);
}

static hive_file *file_create(const char *path, const char *mode)
{
    hive_file *f;
    HiveFile *file;
    int rc;

    file = hive_file_open(context.drive, path, mode);
    if (!file)
        return NULL;

    rc = hive_file_commit(file);
    if (rc < 0 && hive_get_error() != HIVE_GENERAL_ERROR(HIVEERR_NOT_SUPPORTED)) {
        hive_file_close(file);
        return NULL;
    }

    f = rc_zalloc(sizeof(hive_file), file_destructor);
    if (!f) {
        hive_file_close(file);
        return NULL;
    }

    f->file = file;
    f->he.key = strdup(path);
    f->he.keylen = strlen(path);
    f->he.data = f;

    hashtable_put(context.files, &f->he);
    deref(f);

    return f;
}

static int hive_getattr(const char *path, struct stat *stbuf)
{
    int ret;
    HiveFileInfo info;

    ret = hive_drive_file_stat(context.drive, path, &info);
    if (ret < 0)
        return -ENOENT;

    memset(stbuf, 0, sizeof(struct stat));
    stbuf->st_nlink = 1;
    stbuf->st_mode = !strcmp(info.type, "file") ? (S_IFREG | 0644) : (S_IFDIR | 0755);
    stbuf->st_size = !strcmp(info.type, "file") ? info.size : 0;

    return 0;
}

static bool list_callback(const KeyValue *info, size_t size, void *user_data)
{
    size_t i;
    fuse_fill_dir_t filler = (fuse_fill_dir_t)((void **)user_data)[0];
    void *buf = ((void **)user_data)[1];

    for (i = 0; i < size; ++i) {
        if (!strcmp(info[i].key, "name")) {
           filler(buf, info[i].value, NULL, 0);
           break;
        }
    }

    return true;
}

static int hive_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                        off_t offset, struct fuse_file_info *fi)
{
    int ret;

    (void)offset;
    (void)fi;

    void *args[] = {filler, buf};
    ret = hive_drive_list_files(context.drive, path, list_callback, args);
    if (ret < 0)
        return -ENONET;

    filler(buf, ".", NULL, 0);
    if (strcmp(path, "/"))
        filler(buf, "..", NULL, 0);

    return 0;
}

static int hive_mkdir(const char *path, mode_t mode)
{
    int ret;

    (void)mode;

    ret = hive_drive_mkdir(context.drive, path);
    if (ret < 0)
        return -EBADE;

    return 0;
}

static int hive_unlink(const char *path)
{
    int ret;

    if (hashtable_exist(context.files, path, strlen(path)))
        return -EBADE;

    ret = hive_drive_delete_file(context.drive, path);
    if (ret < 0)
        return -EBADE;

    return 0;
}

static int hive_rename(const char *from, const char *to)
{
    int ret;

    if (hashtable_exist(context.files, from, strlen(from)) ||
        hashtable_exist(context.files, to, strlen(to)))
        return -EBADE;

    ret = hive_drive_move_file(context.drive, from, to);
    if (ret < 0)
        return -EBADE;

    return 0;
}

static int hive_open(const char *path, struct fuse_file_info *fi)
{
    hive_file *f;

    f = hashtable_get(context.files, path, strlen(path));
    if (!f) {
        f = file_create(path, "a+");
        if (!f)
            return -EBADE;
    }

    fi->fh = (uint64_t)f;

    return 0;
}

static int hive_read(const char *path, char *buf, size_t size, off_t offset,
                     struct fuse_file_info *fi)
{
    hive_file *f = (hive_file *)fi->fh;
    HiveFile *file = f->file;
    ssize_t ret;

    (void)path;

    if ((fi->flags & O_ACCMODE) == O_WRONLY)
        return -EBADE;

    ret = hive_file_seek(file, offset, HiveSeek_Set);
    if (ret < 0)
        return -EBADE;

    ret = hive_file_read(file, buf, size);
    if (ret < 0)
        return -EBADE;

    return ret;
}

static int hive_write(const char *path, const char *buf, size_t size,
                      off_t offset, struct fuse_file_info *fi)
{
    hive_file *f = (hive_file *)fi->fh;
    HiveFile *file = f->file;
    ssize_t ret;

    (void)path;

    if ((fi->flags & O_ACCMODE) == O_RDONLY)
        return -EBADE;

    ret = hive_file_seek(file, offset, HiveSeek_Set);
    if (ret < 0)
        return -EBADE;

    ret = hive_file_write(file, buf, size);
    if (ret < 0)
        return -EBADE;

    return ret;
}

static int hive_release(const char *path, struct fuse_file_info *fi)
{
    hive_file *f = (hive_file *)fi->fh;

    (void)path;

    deref(f);

    return 0;
}

static int hive_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
    hive_file *f;

    f = hashtable_get(context.files, path, strlen(path));
    if (!f) {
        f = file_create(path, "a+");
        if (!f)
            return -EBADE;
    }

    fi->fh = (uint64_t)f;

    return 0;
}

static int hive_utimens(const char *path, const struct timespec tv[2])
{
    return 0;
}

static int hive_truncate(const char *path, off_t off)
{
    HiveFile *file;
    hive_file *f;
    int rc;

    if (off)
        return -EBADE;

    file = hive_file_open(context.drive, path, "w");
    if (!file)
        return -EBADE;

    f = hashtable_get(context.files, path, strlen(path));
    if (f) {
        hive_file_close(f->file);
        f->file = file;
        deref(f);
    }

    rc = hive_file_commit(file);
    if (rc < 0 && hive_get_error() != HIVE_GENERAL_ERROR(HIVEERR_NOT_SUPPORTED)) {
        if (!f)
            hive_file_close(file);
        return -EBADE;
    }

    return 0;
}

static int hive_rmdir(const char *path)
{
    return hive_unlink(path);
}

static struct fuse_operations hive_ops = {
    .getattr  = hive_getattr,
    .readdir  = hive_readdir,
    .mkdir    = hive_mkdir,
    .unlink   = hive_unlink,
    .rename   = hive_rename,
    .open     = hive_open,
    .read     = hive_read,
    .write    = hive_write,
    .release  = hive_release,
    .create   = hive_create,
    .utimens  = hive_utimens,
    .truncate = hive_truncate,
    .rmdir    = hive_rmdir
};

static void show_help(const char *progname)
{
    printf("usage: %s [options] <mountpoint>\n\n", progname);
    printf("File-system specific options:\n"
           "    --config=<s>    Path of config file\n"
           "                    (default: hyport.conf)\n"
           "    --type=<s>      Backend type(onedrive, ipfs)\n"
           "                    (default: ipfs)\n"
           "    --debug         Wait for debugger attach after start\n"
           "\n");
}

void deinit()
{
    if (options.backend_type)
        free((char *)options.backend_type);

    if (options.config_path)
        free((char *)options.config_path);

    if (options.conf)
        deref(options.conf);

    fuse_opt_free_args(options.args);

    if (context.drive)
        hive_drive_close(context.drive);

    if (context.client)
        hive_client_close(context.client);

    if (context.files)
        deref(context.files);
}

static int open_url(const char *url, void *ctx)
{
    (void)ctx;

#if defined(_WIN32) || defined(_WIN64)
    ShellExecute(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL);
    return 0;
#elif defined(__linux__)
    char cmd[strlen("xdg-open ") + strlen(url) + 3];
    sprintf(cmd, "xdg-open '%s'", url);
    system(cmd);
    return 0;
#elif defined(__APPLE__)
    char cmd[strlen("open ") + strlen(url) + 3];
    sprintf(cmd, "open '%s'", url);
    system(cmd);
    return 0;
#else
#   error "Unsupported Os."
#endif
}

static int setup_context()
{
    if (!strcmp(options.backend_type, "onedrive")) {
        OneDriveOptions opts = {
            .base.drive_type          = HiveDriveType_OneDrive,
            .base.persistent_location = options.conf->persistent_location,
            .redirect_url             = HIVETEST_REDIRECT_URL,
            .scope                    = HIVETEST_SCOPE,
            .client_id                = HIVETEST_ONEDRIVE_CLIENT_ID
        };

        context.client = hive_client_new((HiveOptions *)&opts);
        if (!context.client)
            return -1;
    } else if (!strcmp(options.backend_type, "ipfs")) {
        config *cfg = options.conf;
        int i;

        HiveRpcNode *nodes = calloc(1, sizeof(HiveRpcNode) * cfg->ipfs_rpc_nodes_sz);
        if (!nodes)
            return -1;

        for (i = 0; i < cfg->ipfs_rpc_nodes_sz; ++i) {
            HiveRpcNode *node = nodes + i;

            node->ipv4 = cfg->ipfs_rpc_nodes[i]->ipv4;
            node->ipv6 = cfg->ipfs_rpc_nodes[i]->ipv6;
            node->port = cfg->ipfs_rpc_nodes[i]->port;
        }

        IPFSOptions opts = {
            .base.persistent_location = cfg->persistent_location,
            .base.drive_type = HiveDriveType_IPFS,
            .uid = cfg->uid,
            .rpc_node_count = cfg->ipfs_rpc_nodes_sz,
            .rpcNodes = nodes
        };

        context.client = hive_client_new((HiveOptions *)&opts);
        free(nodes);
        if (!context.client)
            return -1;
    }

    if (hive_client_login(context.client, open_url, NULL)) {
        hive_client_close(context.client);
        context.client = NULL;
        return -1;
    }

    context.drive = hive_drive_open(context.client);
    if (!context.drive) {
        hive_client_close(context.client);
        context.client = NULL;
        return -1;
    }

    context.files = hashtable_create(4, 0, NULL, NULL);
    if (!context.files) {
        hive_drive_close(context.drive);
        context.drive = NULL;
        hive_client_close(context.client);
        context.client = NULL;
        return -1;
    }

    return 0;
}

static void logging(const char *fmt, va_list args)
{
    //DO NOTHING.
}

int main(int argc, char *argv[])
{
    int ret;
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

    options.config_path = strdup("hyport.conf");
    options.backend_type = strdup("ipfs");
    options.args = &args;

    if (fuse_opt_parse(&args, &options, option_spec, NULL) == -1) {
        deinit();
        return 1;
    }

    if (options.debug) {
        printf("Wait for debugger attaching, process id is: %d.\n", getpid());
        printf("After debugger attached, press any key to continue......\n");
        getchar();
    }

    if (options.show_help) {
        show_help(argv[0]);
        return 0;
    }

    if (strcmp(options.backend_type, "onedrive") &&
        strcmp(options.backend_type, "ipfs")) {
        deinit();
        return 1;
    }

    options.conf = load_config(options.config_path);
    if (!options.conf) {
        deinit();
        return 1;
    }

    ela_log_init(options.conf->loglevel, options.conf->logfile, logging);

    if (setup_context()) {
        deinit();
        return 1;
    }

    ret = fuse_main(args.argc, args.argv, &hive_ops, NULL);
    deinit();
    return ret;
}
