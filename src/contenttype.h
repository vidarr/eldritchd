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
#ifndef __CONTENTTYPE_H__
#define __CONTENTTYPE_H__
/*----------------------------------------------------------------------------*/
#include <string.h>
/*----------------------------------------------------------------------------*/
#define MAX_TYPE_STRING_LENGTH 255
/*----------------------------------------------------------------------------*/
typedef enum {
    None, Base64
} ContentEncoding;
/*----------------------------------------------------------------------------*/
typedef struct {
  char* typeString;
  ContentEncoding encoding;
} ContentType;
/*----------------------------------------------------------------------------*/
/**
 * Initialize contenttype database
 * @return 0 on success, -1 otherwise
 */
int contenttype_initialize();
/*----------------------------------------------------------------------------*/
int contenttype_close();
/*----------------------------------------------------------------------------*/
int contenttype_set(char* fileEnding, char* type, char* encodingString);
/*----------------------------------------------------------------------------*/
/**
 * Find appropriate ContentType for file filePath
 * Do not free the pointer returned!
 * @param filePath path to file
 * @param maxPathLength max length of filePath
 * @return Pointer to a ContentType struct describing the contenttype assoc. with the file
 */
ContentType* contenttype_get(char* filePath, size_t maxPathLength);
/*----------------------------------------------------------------------------*/
#endif
