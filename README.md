Hyport Command Line Tool
========================

## 1. Introduction

Hyport is a fuse application to allow cloud storage drives to be mounted as your local file system. And you can access/store files on your remote drives as the conduct on your local device.  Currently, the following list of cloud services are supported right now:

- OneDrive
- IPFS

## 2. Build From Souce

Before building,  certain requirement of utilities for compilation should be installed on your machine.  Otherwise, try to run the commands below to install them at

```shell
$ sudo apt-get update
$ sudo apt-get install -f build-essential autoconf automake libtool cmake
```

After that, use the following commands to conduct building:

```shell
$ git clone https://github.com/elastos/Elastos.NET.Hive.App.Native.HyPort HyPort
$ cd HyPort/build
$ cmake -DCMAKE_INSTALL_PREFIX=dist ..
$ make -j4
$ make install
$ cd dist/bin
```

You will find **hyport** cli command under directory **dist/bin**.

## 3. Requirement for running

Currently, the **hyport** command line tool is only supported for **Linux system** (**Ubuntu-16.04** is recommended) and **x86/64** architectures.

And the dependency **libfuse-dev** should be installed before using **hyport**. Run the following command to install it:

```shell
$sudo apt-get install libfuse-dev=2.9.4-1ubuntu3.1
```

or with simple way like:

```shell
$sudo apt-get install libfuse-dev
```

## 4. Usage

Run the following command to get usage to **hyport** cli:


```shell  
$ ./hyport -h
usage: hyport [options] <mountpoint>
File-system specific options:
    --config=<s>    Path of config file
                    (default: hyport.conf)
    --type=<s>      Backend type(onedrive, ipfs)
                    (default: ipfs)
    --debug         Wait for debugger attach after start
```

## 5. Examples

### 5.1 OneDrive

Run the commands to create a mount point for OneDrive:

```shell
$ cd $YOUR-TRY-DIR
$ mdkir onedrive
$ ./hyport --type=onedrive onedrive
```

When you run the command, there will pop up a browser window to conduct **authorisation** to get permission to access your **OneDrive**. Just enter the account and password to get authorisation.

After mounting on oneDrive, then you can **cd** to **onedrive** directory to have a try with posix commands as below:

```shell
$ cd onedrive
$ echo "hello onedrive" > onedrive.txt
$ ls 
$ cat onedrive.txt
$ mkdir foo
$ cd foo
$ echo "onedrive" > bar.txt
```

### 5.2  Example for Hive IPFS

Run the commands to create a mount point for **Hive IPFS**:

```shell
$ cd $YOUR-TRY-DIR
$ mdkir ipfs
$ ./hyport --type=ipfs ipfs
```

After mounting on **Hive IPFS**, then you can **cd** to **ipfs** directory to have a try with posix commands as below:

```shell
$ cd ipfs
$ echo "hello ipfs" > ipfs.txt
$ ls 
$ cat ipfs.txt
$ mkdir foo
$ cd foo
$ echo "ipfs" > bar.txt
```


### 5.3. Example to copy file from OneDrive to Hive IPFS

Here is a special case we would want to demonstrate that the files in **OneDrive** can be copied to **Hive IPFS** with the following command. 

At first, the following directory structure should be set up as below:

```shell
|--$YOUR-TRY-DIR
   |--onedrive
   |--ipfs
```

where the directories **onedrive** and **ipfs** are both the mount points to **OneDrive** and **Hive IPFS** respectively as created with samples before.

Then run the command to conduct the **copy** file in **onedrive** to **ipfs**:

```shell
$ cd onedrive
$ cp onedrive.txt ../ipfs/.
$ cd ../ipfs/ipfs.txt .
$ ls 
```

After that, you can see file **onedirve.txt** has been copied to **Hive IPFS** network, and **ipfs.txt** has been copied to **OneDrive** cloud drive.

## 6. Issue notices

For now, **hyport** command is still being under development, and there are lots of improvement ahead of us. Any feedback would be appreciated.

## 7. License

MIT

