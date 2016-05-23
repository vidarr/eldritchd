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
