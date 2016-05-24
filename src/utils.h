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
#ifndef __UTILS_H__
#define __UTILS_H__
/*----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
/*----------------------------------------------------------------------------*/
#define PANIC(msg)                    \
    do {                              \
        fprintf(stderr, "%s\n", msg); \
        exit(1);                      \
    }while(0)
/*----------------------------------------------------------------------------*/
#define NOP do{}while(0)
/*----------------------------------------------------------------------------*/
#define CHECK_STRING_FUNC(STR, LEN, CODE, EXCEPT_CODE) \
    do {                                  \
        STR[LEN] = 0;                     \
        CODE;                             \
        if(0 != STR[LEN]) {               \
            STR[LEN] = 0;                 \
            fprintf(stderr, "Detected string overflow: %s\n", STR);  \
            EXCEPT_CODE;                  \
        }                                 \
    }while(0)
/*----------------------------------------------------------------------------*/
void sockaddrToString(struct sockaddr *addr, char *buffer, size_t buflen);
/*----------------------------------------------------------------------------*/
/*                                LOGGING                                     */
/*----------------------------------------------------------------------------*/
#define INFO  0
#define WARN  1
#define ERROR 2
/*----------------------------------------------------------------------------*/
#define LOG(PRIO, MSG) do {           \
    char __LOG_BUF__[255];            \
    snprintf(__LOG_BUF__, 254, MSG);  \
    __LOG_BUF__[254] = 0;             \
    logMsg(PRIO, __LOG_BUF__, 254);   \
}while(0)
/*----------------------------------------------------------------------------*/
void logMsg(int priority, char *message, size_t length);
/*----------------------------------------------------------------------------*/
#endif
