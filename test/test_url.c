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
#include "url.h"
#include <string.h>
/*----------------------------------------------------------------------------*/
char* checkUrl(char* url, char* path)
{
    char* buffer;
    size_t pathLength = 0;
    char* foundPath = 0;
    size_t urlLength = strlen(url);
    buffer = (char*)malloc(sizeof(char) * urlLength + 1);
    strncpy(buffer, url, urlLength + 1);
    buffer[urlLength] = 0;
    if(0 == path)
    {
        if(-1 == url_getPath(buffer, urlLength, &foundPath, &pathLength) )
        {
            return 0;
        }
        return "Path component detected despite there should not be any";
    }
    if(0 != url_getPath(buffer, urlLength, &foundPath, &pathLength) )
    {
        return "No path component found";
    }
    foundPath[pathLength] = 0;
    printf("%s   |   %s(length: %i)\n", url, foundPath, pathLength);
    if( (pathLength == strlen(path)) && (0 != strcmp(path, foundPath)) )
    {
        printf("Expected %s   Got %s\n", path, foundPath);
        free(buffer);
        return "Paths do not match";
    }
    free(buffer);
    return 0;
}
/*----------------------------------------------------------------------------*/
char* test_url()
{
    char* path = 0;
    size_t pathLenth = 0;
    mu_assert( "Do not fit", 0 == checkUrl("abc.t", "abc.t"));
    mu_assert( "Path returned when none expected",
         0 == checkUrl("http://abc.t", 0));
    mu_assert( "Path returned when none expected",
         0 == checkUrl("http://abc.t?", 0));
    mu_assert( "Path returned when none expected",
         0 == checkUrl("http://abc.t?s", 0));
    mu_assert( "Path returned when none expected",
         0 == checkUrl("http://abc.t?asdf", 0));
    mu_assert( "Path returned when none expected",
         0 == checkUrl("http://abc.t#", 0));
    mu_assert( "Path returned when none expected",
         0 == checkUrl("http://abc.t#f", 0));
    mu_assert( "Path returned when none expected",
         0 == checkUrl("http://abc.t#fsaf", 0));
    mu_assert( "Path returned when none expected",
         0 == checkUrl("http://abc.t?asdf#adfs", 0));
    mu_assert( "Do not fit",
         0 == checkUrl("http://tbc/abc.t", "/abc.t"));
    mu_assert( "Do not fit",
         0 == checkUrl("http://tbc.de/abc.t", "/abc.t"));
    mu_assert( "Do not fit",
         0 == checkUrl("http://www.tbc.de/abc.t", "/abc.t"));
    mu_assert( "Do not fit",
         0 == checkUrl("http://www.tbc.de/abc.t?", "/abc.t"));
    mu_assert( "Do not fit",
         0 == checkUrl("http://www.tbc.de/abc.t?d", "/abc.t"));
    mu_assert( "Do not fit",
         0 == checkUrl("http://www.tbc.de/abc.t?dts", "/abc.t"));
    mu_assert( "Do not fit",
         0 == checkUrl("http://www.tbc.de/abc.t#", "/abc.t"));
    mu_assert( "Do not fit",
         0 == checkUrl("http://www.tbc.de/abc.t#d", "/abc.t"));
    mu_assert( "Do not fit",
         0 == checkUrl("http://www.tbc.de/abc.t#dts", "/abc.t"));
    mu_assert( "Do not fit",
         0 == checkUrl("http://www.tbc.de/abc.t?dts#1fs", "/abc.t"));
    return 0;
}
/*----------------------------------------------------------------------------*/
char* all_tests()
{
    mu_run_test(test_url);
    return 0;
}
/*----------------------------------------------------------------------------*/
TEST_MAIN(all_tests);
/*----------------------------------------------------------------------------*/
