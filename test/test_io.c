/*
 * (C) 2018 Michael J. Beer
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
#include "io.h"
#include <sys/time.h>
/*----------------------------------------------------------------------------*/
#define MIN_ACCURACY_USEC 200
/*----------------------------------------------------------------------------*/
static long measureTimeWaited(long usecsToWait)
{
    struct timeval before;
    struct timeval after;
    gettimeofday(&before, 0);
    io_waitMicroSecs(usecsToWait);
    gettimeofday(&after, 0);
    long usecsPassed = after.tv_sec - before.tv_sec;
    usecsPassed *= 1000 * 1000;
    usecsPassed += after.tv_usec - before.tv_usec;
    return usecsPassed;
}
/*----------------------------------------------------------------------------*/
char* test_waitMicroSecs()
{
    long timeWaited = measureTimeWaited(24);
    mu_assert( "24 ms",
            (24 + MIN_ACCURACY_USEC > timeWaited) && (24 < timeWaited));

    timeWaited = measureTimeWaited(80);
    mu_assert( "80 ms",
            (80 + MIN_ACCURACY_USEC > timeWaited) && (80 < timeWaited));

    timeWaited = measureTimeWaited(517);
    mu_assert( "517 ms",
            (517 + MIN_ACCURACY_USEC > timeWaited) && (517 < timeWaited));

    timeWaited = measureTimeWaited(2013);
    mu_assert( "2013 ms",
            (2013 + MIN_ACCURACY_USEC > timeWaited) && (2013 < timeWaited));

    timeWaited = measureTimeWaited(24611);
    mu_assert( "24611 ms",
            (24611 + MIN_ACCURACY_USEC > timeWaited) && (24611 < timeWaited));

    return 0;
}
/*----------------------------------------------------------------------------*/
char* all_tests()
{
    mu_run_test(test_waitMicroSecs);
    return 0;
}
/*----------------------------------------------------------------------------*/
TEST_MAIN(all_tests);
/*----------------------------------------------------------------------------*/
