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
#include "utils.h"
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
/*----------------------------------------------------------------------------*/
char* sockaddrToString(struct sockaddr* addr)
{
    static char addressStr[128];
    static char portStr[7];
    void *addrIn = 0;
    struct sockaddr_in* sin;
    struct sockaddr_in6* sin6;
    if(addr) {
        switch(addr->sa_family)
        {
            case AF_INET:
                sin = (struct sockaddr_in*)addr;
                addrIn = &(sin->sin_addr);
                snprintf(portStr, 7, ":%d", ntohs(sin->sin_port));
                break;
            case AF_INET6:
                sin6 = (struct sockaddr_in6*)addr;
                snprintf(portStr, 7, ":%i", ntohs(sin6->sin6_port));
                addrIn = &(sin->sin_addr);
                break;
            default:
                fprintf(stderr,
                        "Could not print network address: "
                        "family not supported %d\n", addr->sa_family);
                return "UNKNOWN";
        };
        if(NULL == inet_ntop(addr->sa_family, addrIn, addressStr, 128))
        {
            fprintf(stderr, "Could not print network address:%s\n",
                    strerror(errno));
            return "INVALID";
        }
        strncat(addressStr, portStr, 128);
        addressStr[127] = 0;
    } else {
        addressStr[0] = '-';
        addressStr[1] = 0;
    }
    return addressStr;
}
/*----------------------------------------------------------------------------*/
char buffer[BUFFER_LENGTH];
static FILE* logFileDescriptor = 0;
/*----------------------------------------------------------------------------*/
void log_open(char* fileName)
{
    if(0 == fileName)
    {
        fprintf(stderr, "No log file name given\n");
        exit(1);
    }
    logFileDescriptor = fopen(fileName, "w");
    if(0 == logFileDescriptor)
    {
        fprintf(stderr, "Could not open log file '%s': %s\n",
            fileName, strerror(errno));
        exit(1);
    }
    int fd = fileno(logFileDescriptor);
    /* set permissions of log file to be accessable by userId only */
    /* if( (0 != fchown(fd, getuid(), getgid())) ||
        (0 != fchmod(fd, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) )
    {
        PANIC("Could not set appropriate file permissions on log file");
    } */
    fprintf(stderr, "Logging to %s\n", fileName);
}
/*----------------------------------------------------------------------------*/
void log_close()
{
  if((stderr != logFileDescriptor) && (0 != logFileDescriptor))
  {
    fclose(logFileDescriptor);
  }
}
/*----------------------------------------------------------------------------*/
void logMsg(int priority, int sockfd, char* message, size_t length) {
    static char strTime[45];
    time_t now;
    struct tm * localTime;
    char *priorityString;
    /** Allow logging before log_open() called */
    if(0 == logFileDescriptor)
    {
      logFileDescriptor = stderr;
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
    localTime = localtime(&now);
    strftime(strTime, 45, "%c", localTime);
    message[length] = 0;
    if(0 > sockfd)
    {
        fprintf(logFileDescriptor,
                " [%30s] %5s - %50s\n", strTime, priorityString, message);
    }
    else
    {
        struct sockaddr addr;
        socklen_t addrLength = sizeof(addr);
        memset(&addr, addrLength, 0);
        if(0 != getpeername(sockfd, &addr, &addrLength))
        {
            fprintf(logFileDescriptor,
                    " [%30s] %5s - %50s\n", strTime, priorityString, message);
            return;
        }
        fprintf(logFileDescriptor,
                " [%30s] %5s - %50s - %50s\n",
                strTime, priorityString,
                message, sockaddrToString(&addr));
    }
}
/*----------------------------------------------------------------------------*/
int setSocketTimeout(int fd, int timeoutSecs)
{
    int opts = 0;
    struct timeval tv;
    tv.tv_sec = timeoutSecs;
    tv.tv_usec = 0;
    opts = fcntl(fd, F_GETFL);
    if(0 > opts)
    {
        return -1;
    }
    opts = opts & (~ O_NONBLOCK);
    if(0 > fcntl(fd, F_SETFL, opts))
    {
        return -1;
    }
    if( (0 > setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) ) ||
        (0 > setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) ) )
    {
        return -1;
    }
    return 0;
}
/*----------------------------------------------------------------------------*/
