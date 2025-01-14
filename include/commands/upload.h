/**
 * @file upload.h
 * @brief FTP Upload (PUT) 명령어를 제어하기 위한 헤더파일
 * @details 사용자 로컬 PC 에서 입력된 경로의 파일을 서버로 전송한다.
 *          연결 모드 (PASSIVE / ACTIVE)에 따라 구분하여 파일을 전송한다.
 */

#ifndef UPLOAD_H
#define UPLOAD_H

#include "ftp_client.h"

/**
 * @brief FTP Upload (PUT)
 *
 * @param spFtpClient 클라이언트, 연결소켓정보가 포함된 구조체
 * @param sLocalPath 업로드 할 파일의 로컬경로
 * @details 로컬파일을 서버로 업로드합니다.
 * @return int 결과성공시 0 반환, 결과실패 시 -1 반환
 */
int upload(FTPClient *spFtpClient, const char *sLocalPath);

#endif  // UPLOAD_H