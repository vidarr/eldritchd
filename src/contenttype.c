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
#include "contenttype.h"
#include "url.h"
#include "hash.h"
#include <stdlib.h>
/*----------------------------------------------------------------------------*/
static struct HashTable* contentTypeHash;
static ContentType* defaultContentType;
/*----------------------------------------------------------------------------*/
ContentType* createContentType(
        char* contentTypeString, ContentEncoding encoding)
{
  char* copyOfTypeString = 0;
  ContentType* newType = malloc(sizeof(ContentType));
  memset(newType, 0, sizeof(ContentType));
  copyOfTypeString = malloc(
          strnlen(contentTypeString, MAX_TYPE_STRING_LENGTH) * sizeof(char));
  newType->typeString = copyOfTypeString;
  newType->encoding = encoding;
  return newType;
}
/*----------------------------------------------------------------------------*/
void disposeContentType(ContentType* toDispose)
{
  if(0 != toDispose)
  {
    if(0 != toDispose->typeString)
    {
        free(toDispose->typeString);
    }
    free(toDispose);
  }
}
/*----------------------------------------------------------------------------*/
int initializeContentTypeDb()
{
    contentTypeHash = hashTableCreate(15, stdHashFunction);
    hashTableSet(contentTypeHash, "jpg",
                 createContentType("image/jpeg", None));
    defaultContentType = createContentType("text/html", None);
    return 0;
}
/*----------------------------------------------------------------------------*/
int closeContentTypeDb()
{
  size_t i = 0;
  char** keys = hashTableKeys(contentTypeHash);
  disposeContentType(defaultContentType);
  for(i = 0; keys[i] != 0; i++)
  {
    disposeContentType(hashTableGet(contentTypeHash, keys[i]));
  }
  hashTableDispose(contentTypeHash);
  return 0;
}
/*----------------------------------------------------------------------------*/
ContentType* getContentType(char* filePath, size_t maxPathLength)
{
  size_t index;
  size_t pathLength = strnlen(filePath, maxPathLength);
  if(pathLength >= maxPathLength)
  {
    LOG(ERROR, "File path too long");
    return 0;
  }
  for(index = pathLength - 1; index >= 0; --index)
  {
    if(FILE_EXTENSION_SEPARATOR == filePath[index])
    {
      char* extension = filePath + index + 1;
      printf("checking '%s'\n", extension);
      ContentType* contentType = hashTableGet(contentTypeHash, extension);
      if(0 == contentType)
      {
        contentType = defaultContentType;
      }
      return contentType;
    }
  }
  return defaultContentType;
}
/*----------------------------------------------------------------------------*/
