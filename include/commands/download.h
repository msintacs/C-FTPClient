/**
 * @file download.h
 * @brief FTP Download (get) 명렁어 제어를 위한 헤더파일
 * @details 사용자로부터 파일명을 입력받아 서버에 파일이 존재할 경우 사용자에게
 *          해당 파일을 전송한다.
 */

#ifndef DOWNLOAD_H
#define DOWNLOAD_H

#include "ftp_client.h"

/**
 * @brief FTP Download (get)
 *
 * @param spFtpClient 클라이언트, 연결소켓정보가 포함된 구조체
 * @param sRemotePath 다운로드 대상 파일 경로
 * @param sLocalPath 파일을 저장할 로컬 경로
 * @details 다운로드 받을 파일의 경로와 저장할 경로값을 입력하여
 *          서버로부터 파일을 다운로드 받는다.
 * @return int 결과성공시 0 반환, 결과실패 시 -1 반환
 */
int download(FTPClient *spFtpClient, const char *sRemotePath, const char *sLocalPath);

#endif  // DOWNLOAD_H