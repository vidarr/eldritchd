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
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
/*----------------------------------------------------------------------------*/
#define MAX_LEN 255
#define OK_MARKER  "-"
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
  maxIndex = numEntries;
  currentExpectedIndex = 0;
  expectedEndings   = malloc(sizeof(char*) * numEntries);
  expectedTypes     = malloc(sizeof(char*) * numEntries);
  expectedEncodings = malloc(sizeof(char*) * numEntries);
}
/*----------------------------------------------------------------------------*/
int checkAllExpectedArrays()
{
  size_t index = 0;
  for(index = 0; index < maxIndex; index++)
  {
    if(0 != strncmp(expectedEndings[index], OK_MARKER, MAX_LEN))
    {
      fprintf(stderr, "Expected '%s' but not found\n", expectedEndings[index]);
      return -1;
    }
    if(0 != strncmp(expectedTypes[index], OK_MARKER, MAX_LEN))
    {
      fprintf(stderr, "Expected '%s' but not found\n", expectedTypes[index]);
      return -1;
    }
  }
  return 0;
}
/*----------------------------------------------------------------------------*/
char* writeToTempFile(char* buffer, long bufferLength)
{
  static char* filename = "/tmp/test_test_configuration.temp";
  unlink(filename);
  int fileno = open(filename, O_CREAT | O_RDWR, S_IRWXU);
  printf("%i: %s\n", fileno, strerror(errno));
  assert(0 <= fileno);
  if(0 > bufferLength)
  {
    bufferLength = strnlen(buffer, MAX_LEN);
  }
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
int contentTypeSetCallback(char* ending, char* type, char* encoding)
{
  printf("contentTypeSetCallback: At pos %i called with '%s' '%s' '%s'\n",
      currentExpectedIndex, ending, type, encoding);
  printf("expectedEndings: '%s'\n", expectedEndings[currentExpectedIndex]);
  printf("expectedTypes: '%s'\n", expectedTypes[currentExpectedIndex]);
  assert(maxIndex > currentExpectedIndex);
  assert(0 == strncmp(expectedEndings[currentExpectedIndex], ending, MAX_LEN));
  expectedEndings[currentExpectedIndex] = OK_MARKER;
  assert(0 == strncmp(expectedTypes[currentExpectedIndex], type, MAX_LEN));
  expectedTypes[currentExpectedIndex] = OK_MARKER;
  /* assert(0 == strncmp(expectedEncodings[currentExpectedIndex],
   *                     encoding, MAX_LEN)); */
  currentExpectedIndex++;
  return 0;
}
/*----------------------------------------------------------------------------*/
char* test_emptyConfig()
{
  char* filename = writeToTempFile("", 0);
  configuration_initializeConfigStructure(
      &config, contentTypeSetCallback, MAX_LEN);
  mu_assert("readFile reported error",
      0 == configuration_readFile(filename, &config));
  deleteFile(filename);
  configuration_clearConfigStructure(&config);
  return 0;
}
/*----------------------------------------------------------------------------*/
char* test_singleValidLine()
{
  char* filename = writeToTempFile(CONFIG_KEY_CONTENTTYPE " abc rheto/rik none",
      -1);
  initializeExpectedArrays(1);
  expectedEndings[0] = "abc";
  expectedTypes[0] = "rheto/rik";
  configuration_initializeConfigStructure(
      &config, contentTypeSetCallback, MAX_LEN);
  mu_assert("Reading single valid config line",
      0 == configuration_readFile(filename, &config));
  mu_assert("Not all expected contentypes read in",
      (0 == checkAllExpectedArrays()) );
  deleteFile(filename);
  freeExpectedArrays();
  configuration_clearConfigStructure(&config);
  return 0;
}
/*----------------------------------------------------------------------------*/
char* test_MultipleValidLines()
{
  char* filename = writeToTempFile(
      CONFIG_KEY_CONTENTTYPE " abc rheto/rik none\n"
      CONFIG_KEY_CONTENTTYPE " c noting/rik none\n"
      CONFIG_KEY_CONTENTTYPE " vorbis disc/omnia\n"
      CONFIG_KEY_CONTENTTYPE " Nanny_Ogg disc/lancre\n"
      CONFIG_KEY_CONTENTTYPE " Hub disc/noman none\n",
      -1);
  initializeExpectedArrays(5);
  expectedEndings[0] = "abc";
  expectedEndings[1] = "c";
  expectedEndings[2] = "vorbis";
  expectedEndings[3] = "Nanny_Ogg";
  expectedEndings[4] = "Hub";
  expectedTypes[0] = "rheto/rik";
  expectedTypes[1] = "noting/rik";
  expectedTypes[2] = "disc/omnia";
  expectedTypes[3] = "disc/lancre";
  expectedTypes[4] = "disc/noman";
  configuration_initializeConfigStructure(
      &config, contentTypeSetCallback, MAX_LEN);
  mu_assert("Reading multiple valid config line",
      0 == configuration_readFile(filename, &config));
  mu_assert("Not all expected contentypes read in",
      (0 == checkAllExpectedArrays()) );
  deleteFile(filename);
  freeExpectedArrays();
  configuration_clearConfigStructure(&config);
  return 0;
}
/*----------------------------------------------------------------------------*/
char* test_singleInvalidLine()
{
  char* filename = writeToTempFile("THIS is TotAlly Invalid!",
      -1);
  initializeExpectedArrays(0);
  configuration_initializeConfigStructure(
      &config, contentTypeSetCallback, MAX_LEN);
  mu_assert("Reading single invalid config line",
      0 != configuration_readFile(filename, &config));
  deleteFile(filename);
  freeExpectedArrays();
  configuration_clearConfigStructure(&config);
  return 0;
}
/*----------------------------------------------------------------------------*/
char* test_listen()
{
  #define TEST_INTERFACE "my-interface"
  #define TEST_PORT "12345"
  char* filename = writeToTempFile(
      CONFIG_KEY_LISTEN " " TEST_INTERFACE " " TEST_PORT, -1);
  initializeExpectedArrays(0);
  configuration_initializeConfigStructure(
      &config, contentTypeSetCallback, MAX_LEN);
  mu_assert("Reading listen directive",
      0 == configuration_readFile(filename, &config));
  mu_assert("Reading listen directive - interface",
      0 == strncmp(config.interface, TEST_INTERFACE, MAX_LEN));
  mu_assert("Reading listen directive - port",
      0 == strncmp(config.port, TEST_PORT, MAX_LEN));
  deleteFile(filename);
  freeExpectedArrays();
  configuration_clearConfigStructure(&config);
  return 0;
  #undef TEST_INTERFACE
  #undef TEST_PORT
}
/*----------------------------------------------------------------------------*/
char* test_documentRoot()
{
  #define TEST_DOCUMENTROOT "/my/doc/root"
  char* filename = writeToTempFile(
      CONFIG_KEY_DOCUMENTROOT " " TEST_DOCUMENTROOT, -1);
  initializeExpectedArrays(0);
  configuration_initializeConfigStructure(
      &config, contentTypeSetCallback, MAX_LEN);
  mu_assert("Reading documentroot directive",
      0 == configuration_readFile(filename, &config));
  mu_assert("Reading documentroot directive",
      0 == strncmp(config.documentRoot, TEST_DOCUMENTROOT, MAX_LEN));
  deleteFile(filename);
  freeExpectedArrays();
  configuration_clearConfigStructure(&config);
  return 0;
  #undef TEST_DOCUMENTROOT
}
/*----------------------------------------------------------------------------*/
char* test_logFile()
{
  #define TEST_LOGFILE "/my/logfile"
  char* filename = writeToTempFile(
      CONFIG_KEY_LOGFILE " " TEST_LOGFILE, -1);
  initializeExpectedArrays(0);
  configuration_initializeConfigStructure(
      &config, contentTypeSetCallback, MAX_LEN);
  mu_assert("Reading logfile directive",
      0 == configuration_readFile(filename, &config));
  mu_assert("Reading logfile directive",
      0 == strncmp(config.logFile, TEST_LOGFILE, MAX_LEN));
  deleteFile(filename);
  freeExpectedArrays();
  configuration_clearConfigStructure(&config);
  return 0;
  #undef TEST_DOCUMENTROOT
}
/*----------------------------------------------------------------------------*/
char* all_tests()
{
  mu_run_test(test_emptyConfig);
  mu_run_test(test_singleValidLine);
  mu_run_test(test_MultipleValidLines);
  mu_run_test(test_singleInvalidLine);
  mu_run_test(test_listen);
  mu_run_test(test_documentRoot);
  mu_run_test(test_logFile);
  return 0;
}
/*----------------------------------------------------------------------------*/
TEST_MAIN(all_tests);
/*----------------------------------------------------------------------------*/
