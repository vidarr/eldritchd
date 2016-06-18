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
#include "minunit.h"
#include "contenttype.h"
#include <string.h>
#include <stdlib.h>
/*----------------------------------------------------------------------------*/
static char buffer[256];
/*----------------------------------------------------------------------------*/
char* toBuffer(char* string)
{
  strncpy(buffer, string, 255);
  return buffer;
}
/*----------------------------------------------------------------------------*/
char* test_initContentDb()
{
  mu_assert("ContentType db failed to initialize",
             (0 == initializeContentTypeDb()));
  return 0;
}
/*----------------------------------------------------------------------------*/
char* test_closeContentDb()
{
  mu_assert("ContentType db failed to initialize",
             (0 == closeContentTypeDb()));
  return 0;
}
/*----------------------------------------------------------------------------*/
char* checkSingleContentType(char* path,
    char* typeString, ContentEncoding encoding)
{
  ContentType* type = getContentType(toBuffer(path), 255);
  mu_assert("Wrong encoding for .h file", encoding == type->encoding);
  mu_assert("Wrong contentType",
   strncmp(type->typeString, typeString, 255));
  return 0;
}
/*----------------------------------------------------------------------------*/
char* test_contentTypeMapping()
{
  mu_assert("abc.",
      (0 == checkSingleContentType("abc.", "text/html", None)) );
  mu_assert("abc.c",
      (0 == checkSingleContentType("abc.c", "text/html", None)));
  mu_assert("/abc.c",
      (0 == checkSingleContentType("/abc.c",
                                   "text/html", None)));
  mu_assert("/abc/def.h",
      (0 == checkSingleContentType("/abc/def.h",
                                   "text/html", None)) );
  mu_assert("/abc/def/ghi.h",
      (0 == checkSingleContentType("/abc/def/ghi.h",
                                   "text/html", None)) );
  mu_assert("ghi.jpg",
      (0 == checkSingleContentType("ghi.jpg",
                                   "image/jpeg", None)) );
  mu_assert("/def/ghi.jpg",
      (0 == checkSingleContentType("/def/ghi.jpg",
                                   "image/jpeg", None)) );
  mu_assert("/abc/def/ghi.jpg",
      (0 == checkSingleContentType("/abc/def/ghi.jpg",
                                   "image/jpeg", None)) );
  return 0;
}
/*----------------------------------------------------------------------------*/
char* all_tests()
{
  mu_run_test(test_initContentDb);
  mu_run_test(test_contentTypeMapping);
  mu_run_test(test_closeContentDb);
  return 0;
}
/*----------------------------------------------------------------------------*/
TEST_MAIN(all_tests);
/*----------------------------------------------------------------------------*/
