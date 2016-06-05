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
#include <assert.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include "http.h"
#include "url.h"
/*----------------------------------------------------------------------------*/
#define CRLF "\r\n"
#define CR    '\r'
#define LF    '\n'
#define SPACE ' '
/*----------------------------------------------------------------------------*/
#define BUF_SIZE  SOCKET_READ_BUFFER_LEN
/*----------------------------------------------------------------------------*/
static char buffer1[BUF_SIZE + 1];
static char* readBuffer = buffer1;
static size_t readBufferDataEndIndex = 0;
static size_t readBufferDataBeginIndex = 0;
static struct stat fileState;
/*----------------------------------------------------------------------------*/
char *http_message(int statusCode)
{
    switch(statusCode)
    {
        case 404:
            return "Not Found";
        default:
            if( (199 < statusCode) && (statusCode < 300) )
            {
                return OK_STR;
            }
            if( (299 < statusCode) && (statusCode < 400) )
            {
                return REDIRECT_STR;
            }
            if( (399 < statusCode) && (statusCode < 500) )
            {
                return CLIENT_ERROR_STR;
            }
    };
    return UNKNOWN_ERROR_STR;
}
/*----------------------------------------------------------------------------*/
#define SEND(socket, data, length)               \
    do {                                         \
        if(BUF_SIZE <= length)                   \
        {                                        \
            perror("While sending: line exceeding limit\n"); \
            return -1;                           \
        }                                        \
        if(length != send(socket, data, length, 0)) \
        {                                        \
            return -1;                           \
        }                                        \
    } while(0)
/*----------------------------------------------------------------------------*/
int http_sendResponseStatus(int socketFd, int statusCode)
{
    int   numPrinted;
    numPrinted = snprintf(readBuffer, BUF_SIZE, "HTTP/1.0 %3i %s" CRLF,
            statusCode, http_message(statusCode));
    readBuffer[BUF_SIZE] = 0;
    printf("%i    - '%s'\n", numPrinted, readBuffer);
    SEND(socketFd, readBuffer, numPrinted);
    return 0;
}
/*----------------------------------------------------------------------------*/
int http_sendHeader(int socketFd, char* key, char* value)
{
    int   numPrinted;
    numPrinted = snprintf(readBuffer, BUF_SIZE, "%s: %s" CRLF,
            key, value);
    readBuffer[BUF_SIZE] = 0;
    SEND(socketFd, readBuffer, numPrinted);
    return 0;
}
/*----------------------------------------------------------------------------*/
int http_terminateRequest(socketFd)
{
    static char* crlf = CRLF;
    return 2 != send(socketFd, crlf, 2, 0);
}
/*----------------------------------------------------------------------------*/
int http_processGet(int socketFd, ssize_t readBytes,
                 char** headerKeys,char** headerValues,
                 char** body, size_t* bodyLen)
{
    return -1;
}
/*----------------------------------------------------------------------------*/
#define NEXT_CHAR(c, socketFd)                           \
    do {                                                 \
        errno = 0;                                       \
        if(readBufferDataBeginIndex == readBufferDataEndIndex) \
        {                                                \
            readBufferDataEndIndex = read(socketFd, readBuffer, BUF_SIZE);   \
            if(-1 == readBufferDataEndIndex)             \
            {                                            \
                perror(strerror(errno));                 \
                return -1;                               \
            }                                            \
            readBufferDataBeginIndex = 0;                \
        }                                                \
        c = readBuffer[readBufferDataBeginIndex++];      \
    } while(0)
/*----------------------------------------------------------------------------*/
#define EXPECT(expectedChar, c, socketFd)                \
    do {                                                 \
        NEXT_CHAR(c, socketFd);                          \
        if(toupper(c) != expectedChar)                   \
        {                                                \
            LOG(WARN,                                    \
                    "Could not interpret request - "     \
                    "unsupported HTTP method?\n");       \
            return -1;                                   \
        }                                                \
    } while(0)
/*----------------------------------------------------------------------------*/
/**
 * Reads until encounter of a SPACE, CR or LF.
 * Writes read bytes int *buffer, without terminating SPACE, CR and LF.
 * Reallocs buffer if necessary.
 * Returns the last char read (i.e. either SPACE, CR, LF) or 0 on error.
 */
static char getToken(int socketFd, char** buffer, size_t* bufferLength)
{
    size_t writeIndex = 0;
    char c;
    NEXT_CHAR(c, socketFd);
    while( (SPACE != c) && (CR != c) && (LF != c) )
    {
        if(writeIndex >= *bufferLength - 2)
        {
            if(MAX_TOKEN_LENGTH <= *bufferLength)
            {
                return 0;
            }
            if(0 == *bufferLength)
            {
                *bufferLength = MIN_TOKEN_LENGTH;
            }
            *bufferLength *= 2;
            *buffer = realloc(*buffer, *bufferLength);
        }
        (*buffer)[writeIndex] = c;
        writeIndex++;
        NEXT_CHAR(c, socketFd);
    }
    (*buffer)[writeIndex] = 0;
    return c;
}
/*----------------------------------------------------------------------------*/
int http_readRequest(int socketFd, HttpRequest* request)
{
        /*
         * Request constitutes of
         * request-lineCRLF
         * Header1: *Value1CRLF
         * ...CRLF
         * CRLF
         * Body
         */
    signed char c;
    enum { BEFORE, READING, DONE }   readingState = BEFORE;
    /* Read method */
    while(DONE != readingState)
    {
        NEXT_CHAR(c, socketFd);
        switch( toupper(c) )
        {
            case LF:
            case CR:
                if(BEFORE != readingState)
                {
                    LOG(ERROR, "Premature end of HTTP request\n");
                    return -1;
                }
                break;
            case 'G':
                request->type = GET;
                EXPECT('E', c, socketFd);
                EXPECT('T', c, socketFd);
                EXPECT(SPACE, c, socketFd);
                readingState = DONE;
                LOG_CON(INFO, socketFd, "Got GET request");
                break;
            case 'H':
                request->type = HEAD;
                EXPECT('E', c, socketFd);
                EXPECT('A', c, socketFd);
                EXPECT('D', c, socketFd);
                EXPECT(SPACE, c, socketFd);
                readingState = DONE;
                LOG_CON(INFO, socketFd, "Got HEAD request");
                break;
            default:
                LOG_CON(ERROR, socketFd,
                        "Could not parse HTTP request - "
                        " Unsupported HTTP method?\n");
                return -1;
        };
    };
    if(SPACE != getToken(socketFd, request->url, &request->urlMaxLength) )
    {
        LOG_CON(ERROR, socketFd, "Could not read URL for HTTP requst\n");
        return -1;
    }
    LOG_CON(INFO, socketFd, "Read URL");
    EXPECT('H', c, socketFd);
    EXPECT('T', c, socketFd);
    EXPECT('T', c, socketFd);
    EXPECT('P', c, socketFd);
    EXPECT('/', c, socketFd);
    NEXT_CHAR(request->majVersion, socketFd);
    EXPECT('.', c, socketFd);
    NEXT_CHAR(request->minVersion, socketFd);
    EXPECT(CR, c, socketFd);
    /* Line should be terminated by CRLF, but LF might be missing */
    return 0;
}
/*----------------------------------------------------------------------------*/
#undef NEXT_CHAR
#undef EXPECT
/*----------------------------------------------------------------------------*/
int http_processGetHead(int socketFd, HttpRequest* request)
{
    char* path = 0;
    size_t pathLength = 0;
    if( (0 != url_getPath(*(request->url), request->urlMaxLength,
                          &path, &pathLength)) ||
            (1 > pathLength) )
    {
        snprintf(buffer, BUFFER_LENGTH,
                 "Requested url '%s' malformed\n", request->url);
        LOG_CON(ERROR, socketFd, buffer);
        if(0 != http_sendResponseStatus(socketFd, 200))
        {
            LOG_CON(ERROR, socketFd, strerror(errno));
        }
        close(socketFd);
        PANIC("Requested url malformed");
    }
    path[pathLength] = 0;
    memset(&fileState, 0, sizeof(&fileState));
    if(0 != stat(path, &fileState))
    {
        http_sendResponseStatus(socketFd, 404);
        http_terminateRequest(socketFd);
        close(socketFd);
        PANIC("Requested resource not found");
    }
    if(0 != http_sendResponseStatus(socketFd, 200))
    {
        close(socketFd);
        PANIC("Could not send HTTP response");
    }
    if(0 !=
         http_sendHeader(socketFd, "Content-Type", "text/html; charset=UTF-8"))
    {
        close(socketFd);
        PANIC("Could not send HTTP response");
    }
    return 0;
}
/*----------------------------------------------------------------------------*/
void http_accept(int socketFd, int timeoutSecs)
{
    /* Init request structure */
    HttpRequest* request = malloc(sizeof(HttpRequest));
    memset((void *)request, 0, sizeof(HttpRequest));
    char* urlBuffer = malloc(128);
    request->url = &urlBuffer;
    request->urlMaxLength = 128;
    LOG_CON(INFO, socketFd, "New HTTP request incoming");
    if( 0 != http_readRequest(socketFd, request))
    {
        close(socketFd);
        PANIC("Something wrong with the HTTP header");
    }
    switch(request->type)
    {
        case GET:
        case HEAD:
            http_processGetHead(socketFd, request);
            break;
        case OTHER:
            http_sendResponseStatus(socketFd, 405);
            close(socketFd);
            LOG_CON(WARN, socketFd, "Unsupported request type");
            PANIC("Bad request");
        default:
            assert(! "SHOULD NEVER HAPPEND");
    }
    close(socketFd);
}
/*----------------------------------------------------------------------------*/
#undef BUF_SIZE
