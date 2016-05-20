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
#include "http.h"
/*----------------------------------------------------------------------------*/
void sanitizeString(char *dest, char *src, size_t maxLen)
{
    dest[maxLen] = '1';
    strncpy(dest, src, maxLen + 1);
    if(dest[maxLen] != 0)
    {
        PANIC("String too long");
    }
}
/*----------------------------------------------------------------------------*/
void checkUid(uid_t *uid, gid_t *gid)
{
    *uid = getuid();
    *gid = getgid();
    if( (0 == *uid) || (0 == *gid) )
    {
        PANIC("Refusing to run as root");
    }
    if((0 != geteuid()) || (0 != getegid()))
    {
        PANIC("Must run setuid root");
    }
}
/*----------------------------------------------------------------------------*/
void dropPriviledges(uid_t uid, gid_t gid)
{
    if((0 != setregid(gid, gid)) || (0 != setreuid(uid, uid)) )
    {
        perror(strerror(errno));
        PANIC("Could not drop priviledges");
    }
    /* Just to be sure ... */
    if( (0 == getuid()) || (0 == geteuid()) ||
        (0 == getgid()) || (0 == getegid()) )
    {
        PANIC("Could not drop priviledges");
    }
}
/*----------------------------------------------------------------------------*/
void changeRoot(char *root)
{
    if(0 != chroot(root))
    {
        perror(strerror(errno));
        PANIC("Could not chroot");
    }
    if(0 != chdir("/"))
    {
        perror(strerror(errno));
        PANIC("Could not chroot\n");
    }
    return;
}
/*----------------------------------------------------------------------------*/
int isAnyInterface(char *interface)
{
    return strcmp(interface, ANY_INTERFACE) == 0;
}
/*----------------------------------------------------------------------------*/
void retrieveAddrInfo(char *addrString, char *port, struct addrinfo **result)
{
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_canonname = NULL;
    int error = getaddrinfo(addrString, port, &hints, result);
    if (error != 0)
    {
        if (error != EAI_SYSTEM)
        {
            perror(gai_strerror(error));
        }
        PANIC("Could not retrieve address info");
    }
}
/*----------------------------------------------------------------------------*/
int tryBindTo(struct addrinfo *addr)
{
    int socketFd = -1;
    socketFd = socket(addr->ai_family, SOCK_STREAM, 0);
    if(0 <= socketFd)
    {
        if(0 == bind(socketFd, addr->ai_addr, addr->ai_addrlen))
        {
            return socketFd;
        }
        close(socketFd);
    }
    return -1;
}
/*----------------------------------------------------------------------------*/
int bindTo(char *interface, char *port)
{
    int socketFd;
    char buffer[MAX_OPT_STR_LEN];
    struct addrinfo *addrInfo;
    struct addrinfo *res;
    retrieveAddrInfo(interface, port, &addrInfo);
    for (res = addrInfo; res != NULL; res = res->ai_next)
    {
        socketFd = tryBindTo(res);
        if(0 <= socketFd)
        {
            sockaddrToString(res->ai_addr, buffer, MAX_OPT_STR_LEN);
            buffer[MAX_OPT_STR_LEN - 1] = 0;
            printf("Bound to %s\n", buffer);
            freeaddrinfo(addrInfo);
            return socketFd;
        }
    }
    PANIC("Could not find interface to bind to\n");
}
/*----------------------------------------------------------------------------*/
char *initializeString(char initValue[MAX_OPT_STR_LEN])
{
    char *buffer = malloc(sizeof(char) * (MAX_OPT_STR_LEN + 1));
    if(buffer == NULL)
    {
        PANIC("No memory left");
    }
    sanitizeString(buffer, initValue, MAX_OPT_STR_LEN);
    return buffer;
}
/*----------------------------------------------------------------------------*/
void registerSignals(void)
{
    struct sigaction signalAction;
    signalAction.sa_handler = &forker_stop;
    signalAction.sa_flags = SA_RESTART;
    sigfillset(&signalAction.sa_mask);
    if(0 != sigaction(SIGHUP, &signalAction, NULL))
    {
        PANIC("Cannot register signal handler for SIGHUP");
    }
    if(0 != sigaction(SIGINT, &signalAction, NULL))
    {
        PANIC("Cannot register signal handler for SIGINT");
    }
}
/*----------------------------------------------------------------------------*/
int main(int argc, char** argv)
{
    uid_t uid;
    gid_t gid;
    int c;
    char *port;
    int listenSocket;
    char *interface;
    char *documentRoot;
    char *userDb;
    char *rulesDb;
    port = DEFAULT_PORT;
    opterr = 0;
    /* checkUid(&uid, &gid); */
    port         = initializeString(DEFAULT_PORT);
    interface    = initializeString(DEFAULT_INTERFACE);
    documentRoot = initializeString(DEFAULT_DOCUMENT_ROOT);
    userDb       = initializeString(DEFAULT_USER_DB);
    rulesDb      = initializeString(DEFAULT_RULES_DB);
    while((c = getopt(argc, argv, "i:d:p:u:r:")) != EOF)
    {
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
    /* changeRoot(documentRoot); */
    printf("Chrooted to %s\n", documentRoot);
    memset(documentRoot, 0, MAX_OPT_STR_LEN);
    free(documentRoot);
    listenSocket = bindTo(interface, port);
    memset(interface, 0, MAX_OPT_STR_LEN);
    free(interface);
    /* dropPriviledges(uid, gid); */
    registerSignals();
    printf("Dropped priviledges\n");
    printf("uid=%i gid=%i   euid=%i   egid=%i\n",
            getuid(), getgid(), geteuid(), getegid());
    forker_listen(listenSocket, http_accept);
    printf("Closing listening socket\n");
    close(listenSocket);
    return 0;
}
