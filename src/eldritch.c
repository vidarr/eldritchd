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
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include "config.h"
#include "utils.h"
#include "authenticate.h"
#include "forker.h"
#include "http.h"
#include "contenttype.h"
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
void daemonize()
{
  int result = fork();
  if(0 > result)
  {
    PANIC("Could not fork for daemonization");
  }
  else if(0 != result)
  {
    /* We are the parent - exit */
    exit(0);
  }
  /* if(0 > setsid())
  {
    PANIC("Could not create process session");
  } */
  /* We will change our directory to documentroot as soon as we chroot */
  umask(0);
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
    if(0 < socketFd)
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
    struct addrinfo *addrInfo;
    struct addrinfo *res;
    retrieveAddrInfo(interface, port, &addrInfo);
    for (res = addrInfo; res != NULL; res = res->ai_next)
    {
        socketFd = tryBindTo(res);
        if(0 < socketFd)
        {
            snprintf(buffer, BUFFER_LENGTH,
                    "Bound to %s", sockaddrToString(res->ai_addr));
            LOG(INFO, buffer);
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
    char* port;
    int listenSocket;
    char* interface;
    char* documentRoot;
    char*  logFile;
    port = DEFAULT_PORT;
    opterr = 0;
    gid = 0;
    uid = 0;
    checkUid(&uid, &gid);
    port         = initializeString(DEFAULT_PORT);
    interface    = initializeString(DEFAULT_INTERFACE);
    documentRoot = initializeString(DEFAULT_DOCUMENT_ROOT);
    logFile      = initializeString(DEFAULT_LOG_FILE);
    while((c = getopt(argc, argv, "i:d:p:o:")) != EOF)
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
            case 'o':
                sanitizeString(logFile, optarg, MAX_OPT_STR_LEN);
                break;
            default:
                PANIC("Unknown option");
        };
    };
    daemonize();
    log_open(logFile);
    snprintf(buffer, BUFFER_LENGTH,
    "Starting Version %u.%u.%u Build %05u",
        VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, BUILD_NUM);
    LOG(INFO, buffer);
    if(0 != contenttype_initialize())
    {
      PANIC("Could not initialize ContentTypeDb");
    }
    LOG(INFO,"Initialized ContentType database");
    changeRoot(documentRoot);
    snprintf(buffer, BUFFER_LENGTH,
             "Chrooted to %s", documentRoot);
    LOG(INFO, buffer);
    memset(documentRoot, 0, MAX_OPT_STR_LEN);
    free(documentRoot);
    listenSocket = bindTo(interface, port);
    memset(interface, 0, MAX_OPT_STR_LEN);
    free(interface);
    dropPriviledges(uid, gid);
    registerSignals();
    LOG(INFO, "Dropped priviledges");
    snprintf(buffer, BUFFER_LENGTH,
             "uid=%i gid=%i   euid=%i   egid=%i",
             getuid(), getgid(), geteuid(), getegid());
    LOG(INFO, buffer);
    forker_listen(listenSocket, http_accept);
    LOG(INFO, "Closing listening socket");
    close(listenSocket);
    contenttype_close();
    log_close();
    return 0;
}
