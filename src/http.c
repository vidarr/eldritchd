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
static int socketFd = 0;
/*----------------------------------------------------------------------------*/
#define HEADER_CONTENT_LENGTH "Content-Length"
#define HEADER_MIME_TYPE      "Content-Type"
#define HEADER_CONNECTION     "Connection"
/*----------------------------------------------------------------------------*/
#define MIME_TYPE_DEFAULT     "text/html; charset=UTF-8"
#define MIME_TYPE_DEFAULT_LENGTH strlen(MIME_TYPE_DEFAULT)
/*----------------------------------------------------------------------------*/
#define CONNECTION_KEEP_ALIVE "Keep-alive"
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
int readFileIntoBuffer(char* fileBuffer, size_t fileLength, char* path)
{
    ssize_t readBytes = 0;
    int fd = open(path, O_RDONLY);
    if(0 > fd)
    {
        LOG_CON(ERROR, socketFd, strerror(errno));
        return -1;
    }
    readBytes = read(fd, fileBuffer, fileLength);
    close(fd);
    if(fileLength > readBytes)
    {
        if(0 > readBytes)
        {
            LOG_CON(ERROR, socketFd, strerror(errno));
        } else {
            LOG_CON(ERROR, socketFd, "File size unexpected ?");
        }
        return -1;
    }
    return 0;
}
/*----------------------------------------------------------------------------*/
#define SEND(data, length)                       \
    do {                                         \
        if(BUF_SIZE <= length)                   \
        {                                        \
            LOG_CON(ERROR, socketFd, "While sending: line exceeding limit\n"); \
            return -1;                           \
        }                                        \
        if(length != send(socketFd, data, length, 0)) \
        {                                        \
            return -1;                           \
        }                                        \
    } while(0)
/*----------------------------------------------------------------------------*/
int http_sendResponseStatus(int statusCode)
{
    int   numPrinted;
    numPrinted = snprintf(readBuffer, BUF_SIZE, "HTTP/1.0 %3i %s" CRLF,
            statusCode, http_message(statusCode));
    readBuffer[BUF_SIZE] = 0;
    SEND(readBuffer, numPrinted);
    return 0;
}
/*----------------------------------------------------------------------------*/
int http_sendHeader(char* key, char* value)
{
    int   numPrinted;
    numPrinted = snprintf(readBuffer, BUF_SIZE, "%s: %s" CRLF,
            key, value);
    readBuffer[BUF_SIZE] = 0;
    SEND(readBuffer, numPrinted);
    return 0;
}
/*----------------------------------------------------------------------------*/
int http_terminateRequest()
{
    static char* crlf = CRLF;
    return 2 != send(socketFd, crlf, 2, 0);
}
/*----------------------------------------------------------------------------*/
#define CONVERT_BUFFER_LENGTH (sizeof(size_t) + 1)
/*----------------------------------------------------------------------------*/
int http_sendBuffer(int statusCode,
                    char* mimeType, size_t mimeTypeLength,
                    char* body, size_t bodyLength)
{
    static char convertBuffer[CONVERT_BUFFER_LENGTH];
    if(0 > http_sendResponseStatus(statusCode))
    {
        return -1;
    }
    if(0 > snprintf(convertBuffer, CONVERT_BUFFER_LENGTH, "%i",bodyLength))
    {
        LOG_CON(ERROR, socketFd, "Could not convert body length");
        return -1;
    }
    if(0 > http_sendHeader(HEADER_CONTENT_LENGTH, convertBuffer))
    {
        LOG_CON(ERROR, socketFd, "Could not send Content-Length");
        return -1;
    }
    if(0 > http_sendHeader(HEADER_CONNECTION, CONNECTION_KEEP_ALIVE))
    {
        LOG_CON(ERROR, socketFd, "Could not send Connnection-parameters");
        return -1;
    }
    if(0 > http_sendHeader(HEADER_MIME_TYPE, mimeType))
    {
        LOG_CON(ERROR, socketFd, "Could not send Mime-Type");
        return -1;
    }
    if(0 != body)
    {
        if(0 > http_terminateRequest())
        {
            LOG_CON(ERROR, socketFd, "Error during transmission");
            return -1;
        }
        SEND(body, bodyLength);
    }
    if(0 > http_terminateRequest())
    {
        LOG_CON(ERROR, socketFd, "Could not terminate header");
        return -1;
    }
    return 0;
}
/*----------------------------------------------------------------------------*/
#undef CONVERT_BUFFER_LENGTH
/*----------------------------------------------------------------------------*/
void http_sendDefaultResponse(int statusCode)
{
    char* message = http_message(statusCode);
    if(0 != message) {
        if(0 > http_sendBuffer(statusCode,
                               MIME_TYPE_DEFAULT, MIME_TYPE_DEFAULT_LENGTH,
                               message, strlen(message)) )
        {
            LOG_CON(ERROR, socketFd, "Could not send reply");
        }
    }
}
/*----------------------------------------------------------------------------*/
#define NEXT_CHAR(c)                                     \
    do {                                                 \
        errno = 0;                                       \
        if(readBufferDataBeginIndex == readBufferDataEndIndex) \
        {                                                \
            readBufferDataEndIndex = read(socketFd, readBuffer, BUF_SIZE);   \
            if(-1 == readBufferDataEndIndex)             \
            {                                            \
                LOG_CON(ERROR, socketFd, strerror(errno)); \
                return -1;                               \
            }                                            \
            readBufferDataBeginIndex = 0;                \
        }                                                \
        c = readBuffer[readBufferDataBeginIndex++];      \
    } while(0)
/*----------------------------------------------------------------------------*/
#define EXPECT(expectedChar, c)                          \
    do {                                                 \
        NEXT_CHAR(c);                          \
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
static char getToken(char** buffer, size_t* bufferLength)
{
    size_t writeIndex = 0;
    char c;
    NEXT_CHAR(c);
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
        NEXT_CHAR(c);
    }
    (*buffer)[writeIndex] = 0;
    return c;
}
/*----------------------------------------------------------------------------*/
int http_readRequest(HttpRequest* request)
{
        /*
         * Request constitutes of
         * request-lineCRLF
         * Header1: *Value1CRLF
         * ...CRLF
         * CRLF
         * Body
         */
    signed char c = 0;
    enum { BEFORE, READING, DONE }   readingState = BEFORE;
    /* Read method */
    while(DONE != readingState)
    {
        NEXT_CHAR(c);
        printf("FIRST CHAR  %x  %c\n", c, c);
        printf("'%128s'\n", readBuffer);
        switch( toupper(c) )
        {
            case LF:
            case CR:
                if(BEFORE != readingState)
                {
                    LOG_CON(ERROR, socketFd, "Premature end of HTTP request\n");
                    return -1;
                }
                break;
            case 'G':
                request->type = GET;
                EXPECT('E', c);
                EXPECT('T', c);
                EXPECT(SPACE, c);
                readingState = DONE;
                LOG_CON(INFO, socketFd, "Got GET request");
                break;
            case 'H':
                request->type = HEAD;
                EXPECT('E', c);
                EXPECT('A', c);
                EXPECT('D', c);
                EXPECT(SPACE, c);
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
    if(SPACE != getToken(request->url, &request->urlMaxLength) )
    {
        LOG_CON(ERROR, socketFd, "Could not read URL for HTTP requst\n");
        return -1;
    }
    LOG_CON(INFO, socketFd, "Read URL");
    EXPECT('H', c);
    EXPECT('T', c);
    EXPECT('T', c);
    EXPECT('P', c);
    EXPECT('/', c);
    NEXT_CHAR(request->majVersion);
    EXPECT('.', c);
    NEXT_CHAR(request->minVersion);
    EXPECT(CR, c);
    /* Line should be terminated by CRLF, but LF might be missing */
    return 0;
}
/*----------------------------------------------------------------------------*/
#undef NEXT_CHAR
#undef EXPECT
/*----------------------------------------------------------------------------*/
int http_processGetHead(HttpRequest* request)
{
    char* fileBuffer = 0;
    size_t fileLength = 0;
    char* path = 0;
    size_t pathLength = 0;
    if( (0 != url_getPath(*(request->url), request->urlMaxLength,
                          &path, &pathLength)) ||
            (1 > pathLength) )
    {
        snprintf(buffer, BUFFER_LENGTH,
                 "Requested url '%s' malformed\n", request->url);
        LOG_CON(ERROR, socketFd, buffer);
        close(socketFd);
        PANIC("Requested url malformed");
    }
    path[pathLength] = 0;
    memset(&fileState, 0, sizeof(&fileState));
    if(0 != stat(path, &fileState))
    {
        http_sendDefaultResponse(404);
        close(socketFd);
        PANIC("Requested resource not found");
    }
    fileLength = fileState.st_size;
    if(GET == request->type)
    {
        fileBuffer = malloc(fileLength + 1);
        if(0 == fileBuffer)
        {
            LOG_CON(ERROR, socketFd, "Could not allocate memory");
            close(socketFd);
            PANIC("Could not allocate memory");
        }
        if( 0 > readFileIntoBuffer(fileBuffer, fileLength, path))
        {
            http_sendDefaultResponse(404);
            free(fileBuffer);
            close(socketFd);
            PANIC("Requested resource not found");
        }
        fileBuffer[fileLength] = 0;
        fileLength = strnlen(fileBuffer, fileLength) + 1;
    }
    if(0 != http_sendBuffer(200,
                            MIME_TYPE_DEFAULT,  MIME_TYPE_DEFAULT_LENGTH,
                            fileBuffer, fileLength))
    {
        free(fileBuffer);
        close(socketFd);
        PANIC("Could not send HTTP response");
    }
    free(fileBuffer);
    return 0;
}
/*----------------------------------------------------------------------------*/
void http_accept(int conSocket, int timeoutSecs, int keepAlive)
{
    /* Init request structure */
    HttpRequest* request = malloc(sizeof(HttpRequest));
    socketFd = conSocket;
    memset((void *)request, 0, sizeof(HttpRequest));
    char* urlBuffer = malloc(128);
    request->url = &urlBuffer;
    request->urlMaxLength = 128;
    LOG_CON(INFO, socketFd, "New HTTP request incoming");
    /* Set timeout on socket */
    if(0 > setSocketTimeout(socketFd, timeoutSecs))
    {
        LOG_CON(ERROR, socketFd, "Could not set timeout on client socket");
        close(socketFd);
        PANIC("Could not timeout on socket");
    }
    do
    {
        if( 0 != http_readRequest(request))
        {
            LOG_CON(socketFd, ERROR, strerror(errno));
            close(socketFd);
            PANIC("Something wrong with the HTTP header");
        }
        switch(request->type)
        {
            case GET:
            case HEAD:
                http_processGetHead(request);
                break;
            case OTHER:
                http_sendDefaultResponse(405);
                close(socketFd);
                LOG_CON(WARN, socketFd, "Unsupported request type");
                PANIC("Bad request");
            default:
                assert(! "SHOULD NEVER HAPPEND");
        }
    }while(keepAlive);
    close(socketFd);
}
/*----------------------------------------------------------------------------*/
#undef BUF_SIZE
