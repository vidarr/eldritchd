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
#include "forker.h"

static int maxPendingRequests = DEFAULT_MAX_PENDING_REQUESTS;
static int keepListening = 1;

void forker_stop(int signal) {
    printf("Caught signal %i ...\n", signal);
    // Todo: interrupt accept(2)
    keepListening = 0;
}

void forker_listen(int socketFd, void (* acceptor)(int socketFd)) {
    struct sockaddr incomingAddress;
    socklen_t incomingAddressLength;
    char hostAddress[INET6_ADDRSTRLEN];
    int incomingSocket;
    incomingAddressLength = 0;
    if(0 != listen(socketFd, maxPendingRequests)) {
        perror(strerror(errno));
        PANIC("Cannot start listening");
    }
    while(keepListening) {
        incomingSocket = accept(socketFd,
                &incomingAddress, &incomingAddressLength);
        if(0 > incomingSocket) {
            perror(strerror(errno));
        } else {
            sockaddrToSring(&incomingAddress, hostAddress, INET6_ADDRSTRLEN);
            printf("Incoming connection from %s\n", hostAddress);
        }
    }
}

void forker_accept(int socketFd) {
}
