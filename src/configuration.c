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
#include "configuration.h"
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "contenttype.h"
/*----------------------------------------------------------------------------*/
#define MAX_LINE_LENGTH MAX_OPT_STR_LEN
/*----------------------------------------------------------------------------*/
/**
 * Places startOfWord at beginning of next word, endOfWord onto first position
 * after word.
 */
static void cutNextToken(char* buffer, char* startOfWord, char* endOfWord)
{
    startOfWord = buffer;
    while( (' ' == *startOfWord) || ('\t' == *startOfWord) )
    {
        startOfWord++;
    }
    endOfWord = startOfWord;
    if(0 == startOfWord) return;
    while( (' ' != *endOfWord) &&
           ('\t' != *endOfWord) &&
           (0 != *endOfWord) )
    {
        endOfWord++;
    }
}
/*----------------------------------------------------------------------------*/
static int readContentType(char* line)
{
    char* fileEnding = 0;
    char* endOfToken = 0;
    char* type = 0;
    char* encoding = 0;
    cutNextToken(line, fileEnding, endOfToken);
    if(0 != endOfToken)
    {
        *endOfToken = 0;
        endOfToken++;
    }
    else
    {
        return -1;
    }
    cutNextToken(endOfToken, type, endOfToken);
    if(0 != endOfToken)
    {
        *endOfToken = 0;
        endOfToken++;
    }
    else
    {
        return -1;
    }
    contenttype_set(fileEnding, type, None);
    return 0;
}
/*----------------------------------------------------------------------------*/
static void interpretLine(char* line)
{
    char* beginning = 0;
    char* end = 0;
    cutNextToken(line, beginning, end);
    if(0 != end)
    {
        *end= 0;
        end++;
    }
    /* Note:
       beginning points to a string containing the first word only.
       end points to the 'beginning of the remainder'.
       If *end == 0, we are at the end of line */
    if(0 == strncmp(beginning, CONFIG_KEY_CONTENTTYPE, MAX_LINE_LENGTH))
    {
        readContentType(end);
    }
}
/*----------------------------------------------------------------------------*/
static int readStream(FILE* inStream, struct EldritchConfig* conf)
{
    int index;
    char* buffer = malloc(MAX_LINE_LENGTH);
    while(NULL != fgets(buffer, MAX_LINE_LENGTH, inStream))
    {
        interpretLine(buffer);
    }
    free(buffer);
    if(0 == feof(inStream))
    {
        return -1;
    }
    return 0;
}
/*----------------------------------------------------------------------------*/
int configuration_readFile(char* filePath, struct EldritchConfig* conf)
{
    int returnValue = 0;
    FILE* configFile = fopen(filePath, "r");
    if(NULL == configFile)
    {
        return -1;
    }
    returnValue = readStream(configFile, conf);
    fclose(configFile);
    return returnValue;
}
/*----------------------------------------------------------------------------*/
#undef MAX_LINE_LENGTH
