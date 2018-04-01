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
#include "io.h"
#include "url.h"
#include "contenttype.h"
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
#define HEADER_CONTENT_TYPE   "Content-Type"
#define HEADER_CONNECTION     "Connection"
/*----------------------------------------------------------------------------*/
#define CONTENT_TYPE_DEFAULT  "text/html; charset=UTF-8"
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
#define SEND(data, length)                       \
    do {                                         \
        if(length != send(socketFd, data, length, 0)) \
        {                                        \
            LOG_CON(ERROR, socketFd, strerror(errno)); \
            return -1;                           \
        }                                        \
    } while(0)
/*----------------------------------------------------------------------------*/
int http_sendResponseStatus(int statusCode)
{
    int   numPrinted;
    numPrinted = snprintf(readBuffer, BUF_SIZE, "HTTP/1.1 %3i %s" CRLF,
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
int http_sendChunked(char* buffer, size_t length)
{
    /* EVEN BETTER: Check bytes available to write, and send as much
     * as possible ? */
    size_t bytesRemaining = length;
    size_t nextChunkSize = 0;
    char* nextChunk = buffer;
    size_t sentBytes = 0;
    while(0 < bytesRemaining)
    {
        nextChunkSize = bytesRemaining;
        if(nextChunkSize > SEND_CHUNK_SIZE_BYTES)
        {
            nextChunkSize = SEND_CHUNK_SIZE_BYTES;
        }
        sentBytes = send(socketFd, nextChunk, nextChunkSize, 0);
        bytesRemaining -= sentBytes;
        nextChunk += sentBytes;
        if(0 > sentBytes)
        {
            LOG_CON(ERROR, socketFd, strerror(errno));
            return -1;
        }
    }
    return 0;
}
/*----------------------------------------------------------------------------*/
#define CONVERT_BUFFER_LENGTH (sizeof(size_t) * 3 + 1)
/*----------------------------------------------------------------------------*/
int http_sendBuffer(int statusCode,
                    char* contentType,
                    char* body, size_t bodyLength)
{
    static char convertBuffer[CONVERT_BUFFER_LENGTH];
    if(0 > http_sendResponseStatus(statusCode))
    {
        return -1;
    }
    if(0 > snprintf(convertBuffer, CONVERT_BUFFER_LENGTH, "%zu",bodyLength))
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
    if(0 > http_sendHeader(HEADER_CONTENT_TYPE, contentType))
    {
        LOG_CON(ERROR, socketFd, "Could not send Content-Type");
        return -1;
    }
    if(0 > http_terminateRequest())
    {
        LOG_CON(ERROR, socketFd, "Could not terminate header");
        return -1;
    }
    if(0 != body)
    {
        return http_sendChunked(body, bodyLength);
    }
    return 0;
}
/*----------------------------------------------------------------------------*/
#undef CONVERT_BUFFER_LENGTH
/*----------------------------------------------------------------------------*/
#define SEND_BUF_LENGTH 1024
/*----------------------------------------------------------------------------*/
void http_sendDefaultResponse(int statusCode)
{
    static char sendBuffer[SEND_BUF_LENGTH + 1];
    size_t messageLength = 0;
    char* message = http_message(statusCode);
    if(0 != message) {
        messageLength = snprintf(sendBuffer, SEND_BUF_LENGTH + 1,
                "<html>"
                "<a href=\"" ELDRITCH_URL "\">"
                ELDRITCH_NAME " %u.%u.%u-%u</a>"
                "   reporting:<br><br>"
                "<b>%u - %s</b></html>",
                VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH,
                BUILD_NUM, statusCode, message);
        sendBuffer[SEND_BUF_LENGTH] = 0;
        if(0 > http_sendBuffer(statusCode,
                    CONTENT_TYPE_DEFAULT,
                    sendBuffer, messageLength))
        {
            LOG_CON(ERROR, socketFd, "Could not send reply");
        }
    }
}
/*----------------------------------------------------------------------------*/
#undef SEND_BUF_LENGTH
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
    enum {None, Cr1, Lf1, Cr2, Lf2} crLfReadingState;
    int numCrLf = 0;
    signed char c = 0;
    signed char c2 = 0;
    enum { BEFORE, READING, DONE }  readingState = BEFORE;
    /* Read method */
    while(DONE != readingState)
    {
        NEXT_CHAR(c);
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
    EXPECT(LF, c);
    crLfReadingState = Lf1;
    /* Read until end of header */
    while(Lf2 != crLfReadingState)
    {
        NEXT_CHAR(c);
        if(CR == c)
        {
            if(Lf1 == crLfReadingState)
            {
                crLfReadingState = Cr2;
            }
            else
            {
                crLfReadingState = Cr1;
            }
        }
        else if(LF == c)
        {
            if(Cr1 == crLfReadingState)
            {
                crLfReadingState = Lf1;
            }
            else if(Cr2 == crLfReadingState)
            {
                crLfReadingState = Lf2;
            }
            else
            {
                crLfReadingState = None;
            }
        }
        else
        {
            crLfReadingState = None;
        }
    }
    /* Line should be terminated by CRLF, but LF might be missing */
    return 0;
}
/*----------------------------------------------------------------------------*/
#undef NEXT_CHAR
#undef EXPECT
/*----------------------------------------------------------------------------*/
int http_processGetHead(HttpRequest* request)
{
    size_t fileLength = 0;
    char* path = 0;
    size_t pathLength = 0;
    ContentType* contentType = 0;
    char* contentTypeString = CONTENT_TYPE_DEFAULT;
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
    if( ('/' == path[0]) && (0 == path[1]) )
    {
        path = DEFAULT_FILE_NAME;
    }
    snprintf(buffer, BUFFER_LENGTH, "Requested '%s'\n", path);
    LOG_CON(INFO, socketFd, buffer);
    memset(&fileState, 0, sizeof(&fileState));
    if(0 != stat(path, &fileState))
    {
        http_sendDefaultResponse(404);
        close(socketFd);
        PANIC("Requested resource not found");
    }
    fileLength = fileState.st_size;
    /* Get content type */
    contentType = contenttype_get(path, pathLength);
    if(0 == contentType)
    {
        LOG_CON(ERROR, socketFd, "Something went severly wrong");
        return -1;
    }
    else
    {
        contentTypeString = contentType->typeString;
    }

    int fd = -1;
    char* mmapped = 0;

    /* Open desired file to ensure it is still there and readable */
    if(GET == request->type)
    {
        if(0 != io_mmapFileRO(path, fileLength, &fd, (void**) &mmapped))
        {
            close(socketFd);
            PANIC("Could not map requested file into memory");
        }

    }

    /* Send the HTTP Request */
    int retval = http_sendBuffer(200, contentTypeString, mmapped, fileLength);
    if(0 != io_unmapFile(fd, mmapped, fileLength))
    {
        close(socketFd);
        PANIC("Could not unmap file");
    }

    if(0 != retval)
    {
        close(socketFd);
        PANIC("Could not send HTTP response");
    }

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
