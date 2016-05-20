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
#include "utils.h"
/*----------------------------------------------------------------------------*/
void sockaddrToString(struct sockaddr *addr, char *buffer, size_t buflen)
{
    void *addrIn = 0;
    if(addr) {
    if (addr->sa_family == AF_INET) {
        addrIn = &(((struct sockaddr_in*)addr)->sin_addr);
    } else {
        addrIn = &(((struct sockaddr_in6*)addr)->sin6_addr);
    }
    inet_ntop(addr->sa_family, addrIn, buffer, buflen);
    buffer[buflen - 1] = 0;
    } else {
        buffer[0] = '-';
        buffer[1] = 0;
    }
}
/*----------------------------------------------------------------------------*/
void logMsg(int priority, char *message, size_t length) {
    char *strTime;
    time_t now;
    char *priorityString;
    FILE *out = stdout;
    if(priority != INFO) {
        out = stderr;
    }
    switch(priority) {
        case 0:
            priorityString = "INFO";
            break;
        case 1:
            priorityString = "WARN";
            break;
        default:
            priorityString = "ERROR";
    };
    now = time(NULL);
    strTime = ctime(&now);
    message[length] = 0;
    fprintf(out, " [%45s] %5s - %100s", strTime, priorityString, message);
}
/*----------------------------------------------------------------------------*/
