# C-FTPClient
C 언어를 활용한 FTP Client 프로그램

Windows OS, Linux OS 두개의 플랫폼에서 사용이 가능한 FTP Client 프로그램
Socket TCP 통신을 사용하며 FTP Protocol 명령어를 사용한다.

```
include/
├─── commands/
│   ├── cmd_control.h
│   ├── download.h
│   ├── list.h
│   └── upload.h
│
├─── connection/
│   └── connect.h
│
├─── defines/
│   ├── ftp_client.h
│   ├── response_code.h
│   └── socket.h
│
├─── utils/
│   └── socket_utils.h
│
src/
├─── commands/
│   ├── cmd_control.c
│   ├── download.c
│   ├── list.c
│   └── upload.c
│
├─── connection/
│   └── connect.c
│
├─── utils/
│   └── socket_utils.h
│
main.c
Makefile
compile.sh
```
