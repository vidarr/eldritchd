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
#include <errno.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
#define MAX_LINE_LENGTH MAX_OPT_STR_LEN
/*----------------------------------------------------------------------------*/
char* configuration_error;
/*----------------------------------------------------------------------------*/
static struct EldritchConfig* config;
/*----------------------------------------------------------------------------*/
/**
 * Places startOfWord at beginning of next word, endOfWord onto first position
 * after word.
 */
static void cutNextToken(char* buffer, char** startOfWord, char** endOfWord)
{
  size_t index = 0;
  *startOfWord = buffer;
  while( (' ' == **startOfWord) || ('\t' == **startOfWord) )
  {
    (*startOfWord)++;
    assert(index < MAX_LINE_LENGTH);
    index++;
  }
  *endOfWord = *startOfWord;
  if(0 == *startOfWord) return;
  while( (' ' != **endOfWord) &&
      ('\t' != **endOfWord) &&
      (0 != **endOfWord) &&
      ('\n' != **endOfWord) )
  {
    (*endOfWord)++;
  }
}
/*----------------------------------------------------------------------------*/
static int readContentType(char* line)
{
  char* fileEnding = 0;
  char* endOfToken = 0;
  char* type = 0;
  char* encoding = 0;
  printf("Found contentype ...\n");
  cutNextToken(line, &fileEnding, &endOfToken);
  if(0 != endOfToken)
  {
    *endOfToken = 0;
    endOfToken++;
  }
  else
  {
    return -1;
  }
  cutNextToken(endOfToken, &type, &endOfToken);
  if(0 != endOfToken)
  {
    *endOfToken = 0;
    endOfToken++;
  }
  else
  {
    /* IGNORED for now */
  }
  config->contenttype_set(fileEnding, type, "");
  return 0;
}
/*----------------------------------------------------------------------------*/
static int readListen(char* line)
{
  size_t length;
  char* interface = 0;
  char* endOfToken = 0;
  char*  port = 0;
  printf("Found listen ...\n");
  cutNextToken(line, &interface, &endOfToken);
  if(0 != endOfToken)
  {
    *endOfToken = 0;
    endOfToken++;
  }
  else
  {
    return -1;
  }
  cutNextToken(endOfToken, &port, &endOfToken);
  if(0 != endOfToken)
  {
    *endOfToken = 0;
    endOfToken++;
  }
  else
  {
    return -1;
  }
  length = strnlen(interface, config->maxStringLength);
  memcpy(config->interface, interface, length);
  config->interface[length] = 0;
  length = strnlen(port, config->maxStringLength);
  memcpy(config->port, port, config->maxStringLength);
  config->port[length] = 0;
  return 0;
}
/*----------------------------------------------------------------------------*/
static int readDocumentRoot(char* line)
{
  char* path = 0;
  char* endOfToken = 0;
  printf("Found documentroot ...\n");
  cutNextToken(line, &path, &endOfToken);
  if(0 != endOfToken)
  {
    *endOfToken = 0;
    endOfToken++;
  }
  else
  {
    return -1;
  }
  size_t length = strnlen(path, config->maxStringLength);
  memcpy(config->documentRoot, path, length);
  config->documentRoot[length] = 0;
  return 0;
}
/*----------------------------------------------------------------------------*/
static int interpretLine(char* line)
{
  char* beginning = 0;
  char* end = 0;
  printf("I got '%s'\n", line);
  cutNextToken(line, &beginning, &end);
  if(0 != end)
  {
    *end= 0;
    end++;
  }
  printf("got '%s'\n", beginning);
  /* Note:
     beginning points to a string containing the first word only.
     end points to the 'beginning of the remainder'.
     If *end == 0, we are at the end of line */
  if(0 == strncmp(beginning, CONFIG_KEY_CONTENTTYPE, MAX_LINE_LENGTH))
  {
    return readContentType(end);
  }
  if(0 == strncmp(beginning, CONFIG_KEY_LISTEN, MAX_LINE_LENGTH))
  {
    return readListen(end);
  }
  if(0 == strncmp(beginning, CONFIG_KEY_DOCUMENTROOT, MAX_LINE_LENGTH))
  {
    return readDocumentRoot(end);
  }
  return -1;
}
/*----------------------------------------------------------------------------*/
static int readStream(FILE* inStream, struct EldritchConfig* conf)
{
  int index;
  char* buffer = malloc(MAX_LINE_LENGTH);
  config = conf;
  while(NULL != fgets(buffer, MAX_LINE_LENGTH, inStream))
  {
    printf("Got line '%s'\n", buffer);
    if(0 != interpretLine(buffer))
    {
      configuration_error = "Malformed configuration file";
      return -1;
    }
  }
  printf("Done reading...\n");
  free(buffer);
  if(0 == feof(inStream))
  {
    configuration_error = strerror(errno);
    printf("%s\n", configuration_error);
    return -1;
  }
  return 0;
}
/*----------------------------------------------------------------------------*/
int configuration_readFile(char* filePath, struct EldritchConfig* conf)
{
  int returnValue = 0;
  configuration_error = "OK";
  FILE* configFile = fopen(filePath, "r");
  if(NULL == configFile)
  {
    configuration_error = "Could not open file";
    printf("Could not open file %s\n", filePath);
    return -1;
  }
  returnValue = readStream(configFile, conf);
  fclose(configFile);
  return returnValue;
}
/*----------------------------------------------------------------------------*/
void configuration_initializeConfigStructure(struct EldritchConfig* conf,
                                             size_t maxStringLength)
{
  conf->maxStringLength = maxStringLength;
  conf->interface = malloc((maxStringLength + 1) * sizeof(char));
  conf->port = malloc((maxStringLength + 1) * sizeof(char));
  conf->documentRoot = malloc((maxStringLength + 1) * sizeof(char));
}
/*----------------------------------------------------------------------------*/
void configuration_clearConfigStructure(struct EldritchConfig * conf)
{
  free(conf->interface);
  free(conf->port);
  free(conf->documentRoot);
}
#undef MAX_LINE_LENGTH
