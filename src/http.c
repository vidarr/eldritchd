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
#include "http.h"
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
/*----------------------------------------------------------------------------*/
char *http_message(int statusCode)
{
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
    } while(0)
/*----------------------------------------------------------------------------*/
int http_sendResponseStatus(int socketFd, int statusCode)
{
    int   numPrinted;
    numPrinted = snprintf(readBuffer, BUF_SIZE, "HTTP/1.1 %3i %s" CRLF,
            statusCode, http_message(statusCode));
    readBuffer[BUF_SIZE] = 0;
    SEND(socketFd, readBuffer, numPrinted);
    return 0;
}
/*----------------------------------------------------------------------------*/
int http_sendHeader(char* key, char* value)
{
    int   numPrinted;
    numPrinted = snprintf(readBuffer, BUF_SIZE, "%s: %s" CRLF,
            key, value);
    readBuffer[BUF_SIZE] = 0;
    SEND(socketFd, readBuffer, numPrinted);
    return 0;
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
            fprintf(stderr,                              \
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
        if(writeIndex >= *bufferLength - 1)
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
        writeIndex++;
        NEXT_CHAR(c, socketFd);
    }
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
        printf("Reading...\n");
        NEXT_CHAR(c, socketFd);
        switch( toupper(c) )
        {
            case LF:
            case CR:
                if(BEFORE != readingState)
                {
                    fprintf(stderr, "Premature end of HTTP request\n");
                    return -1;
                }
                break;
            case 'G':
                request->type = GET;
                EXPECT('E', c, socketFd);
                EXPECT('T', c, socketFd);
                EXPECT(SPACE, c, socketFd);
                readingState = DONE;
                break;
            case 'H':
                request->type = HEAD;
                EXPECT('E', c, socketFd);
                EXPECT('A', c, socketFd);
                EXPECT('D', c, socketFd);
                EXPECT(SPACE, c, socketFd);
                readingState = DONE;
                break;
            default:
                fprintf(stderr,
                        "Could not parse HTTP request - "
                        " Unsupported HTTP method?\n");
                return -1;
        };
    };
    if(SPACE != getToken(socketFd, request->url, &request->urlMaxLength) )
    {
        fprintf(stderr, "Could not read URL for HTTP requst\n");
        return -1;
    }
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
void http_accept(int socketFd, int timeoutSecs)
{
    HttpRequest* request = malloc(sizeof(HttpRequest));
    memset((void *)request, 0, sizeof(HttpRequest));
    printf("New HTTP request incoming\n");
    if( 0 != http_readRequest(socketFd, request))
    {
        close(socketFd);
        PANIC("Something wrong with the HTTP header");
    }
    fprintf(stderr, "Got Http Request\n");
    if(GET != request->type)
    {
        http_sendResponseStatus(socketFd, 405);
        close(socketFd);
        fprintf(stderr, "Got unsupported request type %i\n", request->type);
        PANIC("Bad request");
    }
    /* TODO: Check availability of file */
    if(0 != http_sendResponseStatus(socketFd, 200))
    {
        close(socketFd);
        PANIC("Could not send HTTP response");
    }
    if(0 != http_sendHeader("Conent-Type", "text/html; charset=UTF-8"))
    {
        close(socketFd);
        PANIC("Could not send HTTP response");
    }
    /* TODO: Send  */
    close(socketFd);
}
/*----------------------------------------------------------------------------*/
#undef BUF_SIZE
