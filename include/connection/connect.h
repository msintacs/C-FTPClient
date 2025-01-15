/**
 * @file connect.h
 * @brief 서버와의 연결을 제어하기 위한 헤더파일
 * @date 2025-01-08
 * @details 서버정보, 로그인정보 등을 입력 받고 서버로 연결을 제어합니다.
 */

#ifndef CONNECT_H
#define CONNECT_H

#include "ftp_client.h"

/**
 * @brief PORT (ACTIVE) 모드 사용시 고정 포트 할당
 */
#define REUSE_PORT 30000

/**
 * @brief 연결대상서버의 IP 주소 입력받아 구조체에 저장
 *
 * @param spFtpClient 클라이언트, 연결소켓정보가 포함된 구조체 (초기 입력 상태)
 * @details 사용자로부터 연결할 FTP 서버의 IP 주소를 입력받아 검증하고,
 *          유효한 경우 클라이언트 구조체에 저장합니다.
 * @return int 결과성공시 0 반환, 결과실패 시 -1 반환
 */
int getServerAddress(FTPClient *spFtpClient);

/**
 * @brief 로그인 정보 입력 받아 구조체에 저장
 *
 * @param spFtpClient  클라이언트, 연결소켓정보가 포함된 구조체 (초기 입력 상태)
 * @details 사용자로부터 FTP 서버 접속에 필요한 아이디와 비밀번호를 입력받아
 *          클라이언트 구조체에 저장합니다.
 * @return int 결과성공시 0 반환, 결과실패 시 -1 반환
 */
int getLoginInfo(FTPClient *spFtpClient);

/**
 * @brief FTP 서버 연결
 *
 * @param spFtpClient  클라이언트, 연결소켓정보가 포함된 구조체 (정보입력 완료)
 * @details 저장된 서버 주소와 포트를 사용하여 TCP 소켓을 생성하고
 *          FTP 서버와의 연결을 수립합니다.
 * @return int 결과성공 응답코드를 반환, 실패시 -1 반환
 */
int connection(FTPClient *spFtpClient);

/**
 * @brief FTP 서버 연결 프로세스 실행
 *
 * @param spFtpClient 클라이언트, 연결소켓정보가 포함된 구조체 (정보입력 완료)
 * @details 서버 주소입력, 로그인 정보 입력을 순차적으로 수행하고
 *          최종적으로 FTP 서버와의 연결을 수립합니다.
 * @return int 결과성공시 0 반환, 결과실패 시 -1 반환
 */
int connectFTP(FTPClient *spFtpClient);

/**
 * @brief PASV 모드의 FTP 연결
 *
 * @param spFtpClient 클라이언트, 연결소켓정보가 포함된 구조체 (정보입력 완료)
 * @details 서버로 PASV 연결요청을하고 전달 받는 IP, PORT 값을 파싱하여
 *          데이터 소켓을 생성하고 연결을 수행합니다.
 * @return int 결과성공시 0 반환, 결과실패 시 -1 반환
 */
int connectPASV(FTPClient *spFtpClient);

/**
 * @brief PORT 모드의 FTP 연결
 *
 * @param spFtpClient 클라이언트, 연결소켓정보가 포함된 구조체 (정보입력 완료)
 * @details 로컬의 IP와 Port 를 할당하고 파싱한후 서버로 PORT 요청시 함께 전달하여
 *          데이터 소켓을 생성하고 연결을 수행합니다.
 * @return int 결과성공시 0 반환, 결과실패 시 -1 반환
 */
int connectPORT(FTPClient *spFtpClient);

/**
 * @brief PORT 모드 연결시 accept 를 수행
 *
 * @param spFtpClient 클라이언트, 연결소켓정보가 포함된 구조체 (정보입력 완료)
 * @return int 결과성공시 0 반환, 결과실패 시 -1 반환
 */
int acceptDataConnection(FTPClient *spFtpClient);

/**
 * @brief 실제 명령어 수행 전 MODE 에 따라 제어
 *
 * @param spFtpClient 클라이언트, 연결소켓정보가 포함된 구조체 (정보입력 완료)
 * @details 구조체에 저장된 모드정보를 확인하여 PASV, PORT 데이터소켓 연결을 제어합니다.
 * @return int 결과성공시 0 반환, 결과실패 시 -1 반환
 */
int setupDataConnection(FTPClient *spFtpClient);

#endif