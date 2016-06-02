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
#include <assert.h>
#include "url.h"
/*----------------------------------------------------------------------------*/
#define RETURN_FAIL do{*path = 0; *pathLength = 0; return -1;}while(0)
/*----------------------------------------------------------------------------*/
#define RETURN_OK do{ \
     *pathLength = readIndex - startIndex; \
     printf("Found:   '%s'\nlength: %i\n", *path, *pathLength);     \
    return 0;}while(0)
/*----------------------------------------------------------------------------*/
int url_getPath (char* url, size_t urlMaxLength,
                 char** path, size_t* pathLength)
{
    size_t readIndex;
    size_t startIndex;
    enum {PATH_OR_SCHEME, SCHEME_OR_PORT, PORT,
          FIRST_SLASH, SECOND_SLASH,
          SERVER, PATH}
         state;
    *path = url;
    readIndex = 0;
    startIndex = 0;
    state = PATH_OR_SCHEME;
    char readChar = *url;
    while( (readIndex < urlMaxLength) &&
           (0 != readChar) )
    {
        readChar = url[readIndex];
        printf("At position %i ( '%c' )\n", readIndex, readChar);
        switch(state)
        {
            case PATH_OR_SCHEME:
                printf("PATH_OR_SCHEME\n");
                switch(readChar)
                {
                    case ':':
                        *path = 0;
                        state = SCHEME_OR_PORT;
                        break;
                    case '#':
                    case '?':
                        RETURN_OK;
                        break;
                    default:
                        NOP;
                };
                break;
            case SCHEME_OR_PORT:
                printf("SCHEME_OR_PORT\n");
                switch(readChar)
                {
                    case '/':
                        state = FIRST_SLASH;
                        break;
                    default:
                        state = PORT;
                        break;
                }
                break;
            case PORT:
                printf("PORT\n");
                switch(readChar)
                {
                    case '/':
                        state = PATH;
                        startIndex = readIndex + 1;
                        *path = url + startIndex;
                        break;
                    default:
                        NOP; // In theory, only numbers are allowed ...
                };
                break;
            case FIRST_SLASH:
                printf("FIRST_SLASH\n");
                switch(readChar)
                {
                    case '/':
                        state = SECOND_SLASH;
                        break;
                    default:
                        RETURN_FAIL;
                };
                break;
            case SECOND_SLASH:
                printf("SECOND_SLASH\n");
                state = SERVER;
                break;
            case SERVER:
                printf("SERVER\n");
                if('/' == readChar)
                {
                    state = PATH;
                    startIndex = readIndex;
                    *path = url + startIndex;
                }
                break;
            case PATH:
                printf("PATH\n");
                switch(readChar)
                {
                    case '#':
                    case '?':
                        RETURN_OK;
                    default:
                        NOP;
                };
                break;
            default:
                assert(! "SHOULD NEVER HAPPEN!");
        };
        readIndex++;
    };
    if( (PATH == state) ||
        (PATH_OR_SCHEME == state) )
    {
        RETURN_OK;
    }
    printf("Did not find\n");
    RETURN_FAIL;
}
/*----------------------------------------------------------------------------*/
#undef RETURN_FAIL
#undef RETURN_OK
/*----------------------------------------------------------------------------*/
