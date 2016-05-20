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
#include <fcntl.h>
#include "forker.h"
/*----------------------------------------------------------------------------*/
static int keepListening = 1;
static int timeoutSecs = DEFAULT_TIMEOUT_SECS;
/*----------------------------------------------------------------------------*/
void forker_stop(int signal) {
    printf("Caught signal %i ...\n", signal);
    keepListening = 0;
}
/*----------------------------------------------------------------------------*/
/* List of file descriptors opened with their expiration unix epoc */
static int descriptors[MAX_DESCRIPTORS];
static int timeoutEpocs[MAX_DESCRIPTORS];
static int acceptSocket;
void (* acceptor)(int socketFd, int timeoutSecs) = NULL;
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
        if(0 < descriptors[i]) {
            if(timeoutEpocs[i] < epoc) {
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
            printf("Inserted new socket at pos %i\n", i);
            return 0;
        }
    }
    return -1;
}
/*----------------------------------------------------------------------------*/
void processFileDescriptors(fd_set *descriptorSet) {
    int i;
    printf("processFileDescriptors\n");
    time_t epoc = time(NULL);
    for(i = 0; i < MAX_DESCRIPTORS; i++) {
        if(timeoutEpocs[i] < epoc) {
            descriptors[i] = -1;
            timeoutEpocs[i] = -1;
        } else if(FD_ISSET(descriptors[i], descriptorSet)) {
            printf("found ready descriptor\n");
            pid_t pid;
            int readySocket = descriptors[i];
            descriptors[i] = -1;
            timeoutEpocs[i] = -1;
            pid = fork();
            if(0 == pid) {
                /* I am the child */
                acceptor(readySocket, timeoutSecs);
                exit(0);
            }
            /* I am still the parent */
            if(0 > pid) {
                perror(strerror(errno));
                fprintf(stderr, "Could not fork\n");
                close(readySocket);
            }
        }
    }
    if(FD_ISSET(acceptSocket, descriptorSet)) {
        struct sockaddr incomingAddress;
        socklen_t incomingAddressLength;
        char hostAddress[INET6_ADDRSTRLEN + 1];
        int incomingSocket;
        incomingAddressLength = 0;
        printf("Accepting new connection...\n");
        incomingSocket = accept(acceptSocket,
                &incomingAddress, &incomingAddressLength);
        if(0 > incomingSocket) {
            perror(strerror(errno));
        } else {
            CHECK_STRING_FUNC(hostAddress, INET6_ADDRSTRLEN, \
                sockaddrToString(&incomingAddress,           \
                    hostAddress, INET6_ADDRSTRLEN),          \
                NOP);
            printf("Incoming connection from %s\n", hostAddress);
            if(0 != insertDescriptor(incomingSocket)) {
                fprintf(stderr,
                        "Reached max. amount of open descriptors - closing\n");
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
        maxDescriptor = select(maxDescriptor + 1, &fdToReadFrom, NULL, NULL, NULL);
        printf("select returned with %i\n", maxDescriptor);
        if( 0 > maxDescriptor) {
            if((EBADF == errno) || (EINVAL == errno) || (ENOMEM == errno)) {
                perror(strerror(errno));
                closeDescriptors();
                PANIC("Error while waiting on incoming requests");
            }
            if(EINTR != errno) {
                /* The signal callback takes care of setting keepRunning = 0 */
            } else {
                perror(strerror(errno));
            }
        } else {
            printf("select returned with %i\n", maxDescriptor);
            processFileDescriptors(&fdToReadFrom);
        }
    }
    closeDescriptors();
}
/*----------------------------------------------------------------------------*/
void forker_listen(int socketFd, void (*acceptorFunc)(int, int)) {
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
