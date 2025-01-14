/**
 * @file response_code.h
 * @brief FTP 서버 응답코드 정의 헤더
 * @details 클라이언트-FTP서버간 응답을 주고 받을때 사용되는 코드 정의
 */
#ifndef RESPONSE_CODE_H
#define RESPONSE_CODE_H

// Connection Response
#define CONN_SUCCESS 220

// Authentication Response
#define LOGIN_ID_OK 331
#define LOGIN_SUCCESS 230
#define LOGIN_FAIL 530

#define ENTER_PASSIVE 227
#define ENTER_ACTIVE 200

#define FILE_STATUS_OK 150
#define FILE_TRANSFER_SUCCESS 226

#endif  // RESPONSE_CODE_h
