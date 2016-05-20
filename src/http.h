/*
 * (C) 2016 Michael J. Beer
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA.
 */
#ifndef __HTTP_H__
#define __HTTP_H__
/*----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include "config.h"
#include "utils.h"
/*----------------------------------------------------------------------------*/
/*                             HTTP Status Codes                              */
/*----------------------------------------------------------------------------*/
#define OK                200
#define REDIRECT          300
#define CLIENT_ERROR      400
/*----------------------------------------------------------------------------*/
/*                            HTTP Status Messages                            */
/*----------------------------------------------------------------------------*/
#define OK_STR            "OK"
#define REDIRECT_STR      "REDIRECT"
#define CLIENT_ERROR_STR  "CLIENT ERROR"
#define UNKNOWN_ERROR_STR "UNKNOWN ERROR"
/*----------------------------------------------------------------------------*/
typedef struct
{
    enum {GET, HEAD, OTHER} type;
    char** url;
    size_t urlMaxLength;
    char   minVersion;
    char   majVersion;
    char** body;
    size_t maxBodyLength;
    char** headerKeys;
    char** headerValues;
} HttpRequest;
/*----------------------------------------------------------------------------*/
char *http_message    (int statusCode);
/*----------------------------------------------------------------------------*/
int http_sendResponseStatus (int socketFd, int statusCode);
/*----------------------------------------------------------------------------*/
int http_sendHeader   (char* key, char* value);
/*----------------------------------------------------------------------------*/
int http_processGet   (int socketFd, ssize_t readBytes,
                       char** headerKeys,char** headerValues,
                       char** body, size_t* bodyLen);
/*----------------------------------------------------------------------------*/
int http_readRequest  (int socketFd,
                       HttpRequest* request);
/*----------------------------------------------------------------------------*/
void http_accept      (int socketFd, int timeoutSecs);
/*----------------------------------------------------------------------------*/
#endif

