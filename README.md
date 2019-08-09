Elastos.NET.Hive.App.Native.HyPort  
==================================

## Introduction  

Hyport is an FUSE application that allows you to mount your cloud drives under your local file system. It supports OneDrive and IPFS at the moment. 

## Prerequisites

- x86-64 machine
- Ubuntu 16.04 64-bit
- libfuse-dev (ver 2.9.4)

```shell  
$ sudo apt install libfuse-dev=2.9.4-1ubuntu3.1
```
## Build

#### 1. Install Dependencies
  
```shell  
$ sudo apt update  
$ sudo apt install -f build-essential autoconf automake libtool cmake  
```  
  
#### 2. Build

```shell  
$ git clone https://github.com/elastos/Elastos.NET.Hive.App.Native.HyPort  
$ cd Elastos.NET.Hive.App.Native.HyPort/build
$ cmake [-DCMAKE_INSTALL_PREFIX=YOUR-INSTALL-PATH] ..
$ make -j4
$ make install
```  
  
## Usage

```shell  
usage: ./hyport [options] <mountpoint>
File-system specific options:
    --config=<s>    Path of config file
                    (default: hyport.conf)
    --type=<s>      Backend type(onedrive, ipfs)
                    (default: ipfs)
    --debug         Wait for debugger attach after start
```


