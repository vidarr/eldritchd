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
#include <fcntl.h>
#include "forker.h"
/*----------------------------------------------------------------------------*/
static int keepListening = 1;
static int keepAlive = 1;
static int timeoutSecs = DEFAULT_TIMEOUT_SECS;
/*----------------------------------------------------------------------------*/
void forker_stop(int signal) {
    LOG(INFO, "Caught signal...");
    keepListening = 0;
}
/*----------------------------------------------------------------------------*/
/* List of file descriptors opened with their expiration unix epoc */
static int descriptors[MAX_DESCRIPTORS];
static int timeoutEpocs[MAX_DESCRIPTORS];
static int acceptSocket;
void (* acceptor)(int socketFd, int timeoutSecs, int keepAlive) = NULL;
/*----------------------------------------------------------------------------*/
/**
 * Walks through the list of open file descriptors, clears those timed out
 * and sets appropriate bits in descriptor set for those still valid.
 */
int initDescriptorSet(fd_set **descriptorSet) {
    int i;
    int maxDescriptor;
    time_t epoc = time(NULL);
    FD_ZERO(*descriptorSet);
    // First set acceptor socket
    FD_SET(acceptSocket, *descriptorSet);
    maxDescriptor = acceptSocket;
    for(i = 0; i < MAX_DESCRIPTORS; i++) {
        if(0 <= descriptors[i]) {
            if(timeoutEpocs[i] < epoc) {
                close(descriptors[i]);
                descriptors[i] = -1;
                timeoutEpocs[i] = -1;
            } else {
                FD_SET(descriptors[i], *descriptorSet);
                if(descriptors[i] > maxDescriptor) {
                    maxDescriptor = descriptors[i];
                }
            }
        }
    }
    return maxDescriptor;
}
/*----------------------------------------------------------------------------*/
int insertDescriptor(int sock) {
    int i;
    for(i =0; i < MAX_DESCRIPTORS; i++) {
        if(0 > descriptors[i]) {
            descriptors[i] = sock;
            timeoutEpocs[i] = time(NULL) + timeoutSecs;
            return 0;
        }
    }
    return -1;
}
/*----------------------------------------------------------------------------*/
void processFileDescriptors(fd_set *descriptorSet) {
    int i;
    struct timeval timeout;
    time_t epoc = time(NULL);
    for(i = 0; i < MAX_DESCRIPTORS; i++) {
        if( (0 <= descriptors[i]) && (timeoutEpocs[i] < epoc) ) {
            close(descriptors[i]);
            descriptors[i] = -1;
            timeoutEpocs[i] = -1;
        } else if(FD_ISSET(descriptors[i], descriptorSet)) {
            pid_t pid;
            int readySocket = descriptors[i];
            descriptors[i] = -1;
            timeoutEpocs[i] = -1;
            pid = fork();
            if(0 == pid) {
                /* I am the child */
                close(acceptSocket);
                acceptor(readySocket, timeoutSecs, keepAlive);
                exit(0);
            }
            /* I am still the parent */
            close(readySocket);
            if(0 > pid) {
                LOG(ERROR, strerror(errno));
                LOG(ERROR, "Could not fork");
            }
        }
    }
    if(FD_ISSET(acceptSocket, descriptorSet)) {
        struct sockaddr incomingAddress;
        socklen_t incomingAddressLength;
        int incomingSocket;
        incomingAddressLength = sizeof(incomingAddressLength);
        LOG(INFO, "Accepting new connection...");
        incomingSocket = accept(acceptSocket,
                &incomingAddress, &incomingAddressLength);
        if(0 > incomingSocket) {
            LOG(ERROR, strerror(errno));
        } else {
            snprintf(buffer, BUFFER_LENGTH, "Incoming connection from %s",
                    sockaddrToString(&incomingAddress));
            LOG_CON(INFO, incomingSocket, buffer);
            /* set timeout on socket */
            timeout.tv_sec = timeoutSecs;
            timeout.tv_usec = 1;
            if(0 != setsockopt(incomingSocket, SOL_SOCKET, SO_RCVTIMEO,
                        &timeout, sizeof(struct timeval)) )
            {
                close(incomingSocket);
                LOG(ERROR, strerror(errno));
            }
            if(0 != insertDescriptor(incomingSocket)) {
                LOG(ERROR,
                        "Reached max. amount of open descriptors - closing");
                close(incomingSocket);
            }
        }
    }
}
/*----------------------------------------------------------------------------*/
void closeDescriptors(void) {
    int i;
    close(acceptSocket);
    for(i = 0; i < MAX_DESCRIPTORS; i++) {
        if(descriptors[i] > -1) {
            close(descriptors[i]);
        }
        descriptors[i] = -1;
        timeoutEpocs[i] = -1;
    }
}
/*----------------------------------------------------------------------------*/
void forker_loopRead(void) {
    fd_set fdToReadFrom;
    int maxDescriptor;
    fd_set *fdSetPointer = &fdToReadFrom;
    memset(descriptors, -1, sizeof(int) * MAX_DESCRIPTORS);
    memset(timeoutEpocs, -1, sizeof(int) * MAX_DESCRIPTORS);
    /* Make accept(2) non-blocking */
    if(0 != fcntl(acceptSocket, F_SETFD, O_NONBLOCK)) {
        close(acceptSocket);
        perror(strerror(errno));
        PANIC("Could not set listen socket Non-Blocking");
    }
    while(keepListening) {
        maxDescriptor = initDescriptorSet(&fdSetPointer);
        /* Will only be interrupted by signals */
        maxDescriptor = select(maxDescriptor + 1, &fdToReadFrom, NULL, NULL,
                               NULL);
        if( 0 > maxDescriptor) {
            if((EBADF == errno) || (EINVAL == errno) || (ENOMEM == errno)) {
                LOG(ERROR, strerror(errno));
                closeDescriptors();
                PANIC("Error while waiting on incoming requests");
            }
            else if(EINTR == errno) {
                keepListening = 0;
            } else {
                perror(strerror(errno));
            }
        } else {
            processFileDescriptors(&fdToReadFrom);
        }
    }
    closeDescriptors();
}
/*----------------------------------------------------------------------------*/
void forker_listen(int socketFd, void (*acceptorFunc)(int, int, int)) {
    struct sigaction sigchildNoWait = {
        .sa_handler = SIG_DFL,
        .sa_flags = SA_NOCLDWAIT
    };
    sigaction(SIGCHLD, &sigchildNoWait, NULL);
    if(0 != listen(socketFd, MAX_PENDING_REQUESTS)) {
        perror(strerror(errno));
        close(socketFd);
        PANIC("Cannot start listening");
    }
    acceptSocket = socketFd;
    acceptor = acceptorFunc;
    forker_loopRead();
}
/*----------------------------------------------------------------------------*/
