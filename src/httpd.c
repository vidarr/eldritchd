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
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include "config.h"
#include "version.h"
#include "utils.h"
#include "authenticate.h"
#include "forker.h"

void sanitizeString(char *dest, char *src, size_t maxLen) {
    dest[maxLen] = '1';
    strncpy(dest, src, maxLen + 1);
    if(dest[maxLen] != 0) {
        PANIC("String too long");
    }
}

void checkUid(uid_t *uid, gid_t *gid) {
    *uid = getuid();
    *gid = getgid();
    printf("uid=%i gid=%i   euid=%i   egid=%i\n",
            getuid(), getgid(), geteuid(), getegid());
    if( (0 == *uid) || (0 == *gid) ) {
        PANIC("Refusing to run as root");
    }
    if((0 != geteuid()) || (0 != getegid())) {
        PANIC("Must run setuid root");
    }
}

void dropPriviledges(uid_t uid, gid_t gid) {
    if((0 != setregid(gid, gid)) || (0 != setreuid(uid, uid)) ) {
        perror(strerror(errno));
        PANIC("Could not drop priviledges");
    }
    /* Just to be sure ... */
    if( (0 == getuid()) || (0 == geteuid()) ||
        (0 == getgid()) || (0 == getegid()) ) {
        PANIC("Could not drop priviledges");
    }
}

void changeRoot(char *root) {
    if(0 != chroot(root)) {
        perror(strerror(errno));
        PANIC("Could not chroot");
    }
    if(0 != chdir("/")) {
        perror(strerror(errno));
        PANIC("Could not chroot\n");
    }
    return;
}

int isAnyInterface(char *interface) {
    return strcmp(interface, ANY_INTERFACE) == 0;
}

void retrieveAddrInfo(char *addrString, char *port, struct addrinfo **result) {
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    int error = getaddrinfo(addrString, port, &hints, result);
    if (error != 0) {
        if (error != EAI_SYSTEM) {
            perror(gai_strerror(error));
        }
        PANIC("Could not retrieve address info");
    }
}

void bindToAddresses(int soc, char *interface, char *port) {
    struct addrinfo *addrInfo;
    struct addrinfo *res;
    retrieveAddrInfo(interface, port, &addrInfo);
    for (res = addrInfo; res != NULL; res = res->ai_next) {
        // set port !
        if(0 != bind(soc, res->ai_addr, res->ai_addrlen)) {
            PANIC("Could not bind to socket");
        }
    }
    freeaddrinfo(addrInfo);
}


int bindTo(char *interface, char *port, unsigned ipVersion) {
    struct sockaddr *addr;
    socklen_t len;
    int socketFd;
    int domain = AF_INET;
    socketFd = -1;
    switch(ipVersion) {
        case 4:
            break;
        case 6:
            domain = AF_INET6;
            break;
        default:
            PANIC("Could not create socket - illegal ip version requested");
    };
    socketFd = socket(domain, SOCK_STREAM, 0);
    printf("%i\n", socketFd);
    if(0 > socketFd) {
        perror(strerror(errno));
        PANIC("Could not create socket");
    }
    bindToAddresses(socketFd, interface, port);
    return socketFd;
}

char *initializeString(char initValue[MAX_OPT_STR_LEN]) {
    char *buffer = malloc(sizeof(char) * (MAX_OPT_STR_LEN + 1));
    if(buffer == NULL) {
        PANIC("No memory left");
    }
    sanitizeString(buffer, initValue, MAX_OPT_STR_LEN);
    return buffer;
}

void registerSignals() {
    struct sigaction signalAction;
    signalAction.sa_handler = &forker_stop;
    signalAction.sa_flags = SA_RESTART;
    sigfillset(&signalAction.sa_mask);
    if(0 != sigaction(SIGHUP, &signalAction, NULL)) {
        PANIC("Cannot register signal handler for SIGHUP");
    }
    if(0 != sigaction(SIGINT, &signalAction, NULL)) {
        PANIC("Cannot register signal handler for SIGINT");
    }
    if(0 != sigaction(SIGKILL, &signalAction, NULL)) {
        fprintf(stderr, "WARNING: Cannot register signal handler for SIGKILL");
    }
}

int main(int argc, char** argv)
{
    uid_t uid;
    gid_t gid;
    int c;
    long longPort;
    char *port;
    int listenSocket;
    char *buffer;
    char *endPtr;
    char *interface;
    char *documentRoot;
    char *userDb;
    char *rulesDb;
    port = DEFAULT_PORT;
    opterr = 0;
    buffer = 0;
    endPtr = 0;
    unsigned ipVersion = 4;
    checkUid(&uid, &gid);
    interface    = initializeString(DEFAULT_INTERFACE);
    documentRoot = initializeString(DEFAULT_DOCUMENT_ROOT);
    userDb       = initializeString(DEFAULT_USER_DB);
    rulesDb      = initializeString(DEFAULT_RULES_DB);
    while((c = getopt(argc, argv, "i:d:p:u:r:")) != EOF) {
        switch(c) {
            case 'i':
                sanitizeString(interface, optarg, MAX_OPT_STR_LEN);
                break;
            case 'd':
                sanitizeString(documentRoot, optarg, MAX_OPT_STR_LEN);
                break;
            case 'p':
                sanitizeString(port, optarg, 5);
                break;
            case 'u':
                sanitizeString(userDb, optarg, MAX_OPT_STR_LEN);
                break;
            case 'r':
                sanitizeString(rulesDb, optarg, MAX_OPT_STR_LEN);
                break;
            default:
                PANIC("Unknown option");
        };
    };
    printf("Starting Version %u.%u.%u\n",
            MAJOR_VERSION, MINOR_VERSION, PATCH_VERSION);
    authenticate_loadDbs(userDb, rulesDb);
    printf("Loaded user database from %s\n", userDb);
    printf("Loaded rules database from %s\n", rulesDb);
    memset(userDb, 0, MAX_OPT_STR_LEN);
    memset(rulesDb, 0, MAX_OPT_STR_LEN);
    free(userDb);
    free(rulesDb);
    changeRoot(documentRoot);
    printf("Chrooted to %s\n", documentRoot);
    memset(documentRoot, 0, MAX_OPT_STR_LEN);
    free(documentRoot);
    listenSocket = bindTo(interface, port, ipVersion);
    printf("Bound to %s:%s\n", interface, port);
    memset(interface, 0, MAX_OPT_STR_LEN);
    free(interface);
    registerSignals();
    dropPriviledges(uid, gid);
    printf("Dropped priviledges\n");
    printf("uid=%i gid=%i   euid=%i   egid=%i\n",
            getuid(), getgid(), geteuid(), getegid());
    forker_listen(listenSocket, forker_accept);
    printf("Closing listening socket\n");
    close(listenSocket);
    return 0;
}
