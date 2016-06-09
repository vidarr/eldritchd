/*
 * (C) 2016 Michael J. Beer
 * All rights reserved.
 *
 * Redistribution  and use in source and binary forms, with or with‐
 * out modification, are permitted provided that the following  con‐
 * ditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above  copy‐
 * right  notice,  this  list  of  conditions and the following dis‐
 * claimer in the documentation and/or other materials provided with
 * the distribution.
 *
 * 3.  Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote  products  derived
 * from this software without specific prior written permission.
 *
 * THIS  SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBU‐
 * TORS "AS IS" AND ANY EXPRESS OR  IMPLIED  WARRANTIES,  INCLUDING,
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE  ARE  DISCLAIMED.  IN  NO  EVENT
 * SHALL  THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DI‐
 * RECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR  CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS IN‐
 * TERRUPTION)  HOWEVER  CAUSED  AND  ON  ANY  THEORY  OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING  NEGLI‐
 * GENCE  OR  OTHERWISE)  ARISING  IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
void http_accept      (int socketFd, int timeoutSecs, int keepAlive);
/*----------------------------------------------------------------------------*/
#endif

