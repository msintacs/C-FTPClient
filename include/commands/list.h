/**
 * @file list.h
 * @brief FTP List (ls) 명령어 제어를 위한 헤더파일
 * @details 사용자 명령 요청시 서버의 현재 디렉토리 리스트를 조회하고 응답한다.
 */

#ifndef LIST_H
#define LIST_H

#include "ftp_client.h"

/**
 * @brief FTL File List (ls)
 *
 * @param spFtpClient
 * @details 연결 된 서버의 디렉토리 리스트를 조회한다.
 * @return int 결과성공시 0 반환, 결과실패 시 -1 반환
 */
int list(FTPClient *spFtpClient);

#endif  // LIST_H