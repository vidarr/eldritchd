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
#include "configuration.h"
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
#define MAX_LEN 255
/*----------------------------------------------------------------------------*/
static char** expectedEndings;
static char** expectedTypes;
static char** expectedEncodings;
static size_t currentExpectedIndex;
static size_t maxIndex;
/*----------------------------------------------------------------------------*/
struct  EldritchConfig config;
/*----------------------------------------------------------------------------*/
void freeExpectedArrays()
{
  free(expectedEndings);
  free(expectedTypes);
  free(expectedEncodings);
}
/*----------------------------------------------------------------------------*/
void initializeExpectedArrays(size_t numEntries)
{
    maxIndex = numEntries - 1;
    currentExpectedIndex = 0;
    expectedEndings   = malloc(sizeof(char*) * numEntries);
    expectedTypes     = malloc(sizeof(char*) * numEntries);
    expectedEncodings = malloc(sizeof(char*) * numEntries);
}
/*----------------------------------------------------------------------------*/
char* writeToTempFile(char* buffer, size_t bufferLength)
{
  static char* filename = "/tmp/test_test_configuration.temp";
  int fileno = creat(filename, O_RDWR);
  assert(0 <= fileno);
  assert(bufferLength == write(fileno, buffer, bufferLength));
  close(fileno);
  return filename;
}
/*----------------------------------------------------------------------------*/
void deleteFile(char* filename)
{
  unlink(filename);
}
/*----------------------------------------------------------------------------*/
int contenttypeSetCallback(char* ending, char* type, char* encoding)
{
    assert(maxIndex < currentExpectedIndex);
    assert(0 == strncmp(expectedEndings[currentExpectedIndex], ending, MAX_LEN));
    assert(0 == strncmp(expectedTypes[currentExpectedIndex], type, MAX_LEN));
    assert(0 == strncmp(expectedTypes[currentExpectedIndex], encoding, MAX_LEN));
    currentExpectedIndex++;
    return 0;
}
/*----------------------------------------------------------------------------*/
char* test_emptyConfig()
{
  char* filename = writeToTempFile("", 0);
  mu_assert("readFile reported error",
      0 == configuration_readFile(filename, &config));
  deleteFile(filename);
}

/*----------------------------------------------------------------------------*/
char* all_tests()
{
  config.contenttype_set = contenttypeSetCallback;
  mu_run_test(test_emptyConfig);
  return 0;
}
/*----------------------------------------------------------------------------*/
TEST_MAIN(all_tests);
/*----------------------------------------------------------------------------*/
