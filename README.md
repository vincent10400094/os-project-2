# OS 2020 spring project 2

kernel version: 4.15.0.106

## Members

- 資工二 b07902006 林則仰
- 資工二 b07902013 陳健豪
- 資工二 b07902016 林義閔
- 資工二 b07902021 簡捷
- 資工二 b07902060 趙雋同
- 資工二 b07902114 陳柏衡

## Usage

### Compile

```
$ sudo make
```

or

```
$ bash compile.sh
```

### Execute

```
$ sudo ./user_program/master 1 file1_in mmap
$ sudo ./user_program/slave 1 file1_out fcntl 127.0.0.1
```

### Clean

```
$ sudo make clean
```

or

```
$ bash clean.sh
```