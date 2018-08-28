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
/*----------------------------------------------------------------------------*/
#include "io.h"
#include "config.h"
#include "utils.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
/*----------------------------------------------------------------------------*/
int io_mmapFileRO(char* path, size_t size, int* targetFd, void** targetPointer)
{

        int fd = open(path, O_RDONLY);
        if(-1 == fd)
        {
            return -1;
        }

        char* mmapped = 0;
        mmapped = (char*)mmap(0, size, PROT_READ, MAP_SHARED, fd, 0);

        if(0 == mmapped)
        {
            close(fd);
            return -1;
        }

        *targetFd = fd;
        *targetPointer = mmapped;

        return 0;

}
/*----------------------------------------------------------------------------*/
int io_unmapFile(int fd, void* mmapped, size_t length)
{
    int retval = 0;
    if(0 != mmapped)
    {
        retval = munmap(mmapped, length);

    }
    if(-1 < fd) close(fd);
    return retval;
}
/*----------------------------------------------------------------------------*/
int io_writeChunked(int fd, char* buffer, size_t length)
{
    if(0 > fd)
    {
        LOG(ERROR, "Tried to send to invalid FD\n");
        return -1;
    }

    /* EVEN BETTER: Check bytes available to write, and send as much
     * as possible ? */
    size_t bytesRemaining = length;
    size_t nextChunkSize = 0;
    char* nextChunk = buffer;
    /* Setting this to > MIN_CHUNK_SIZE prevents waiting before first write */
    size_t sentBytes = MIN_CHUNK_SIZE_BYTES + 1;
    while(0 < bytesRemaining)
    {
        if(MIN_CHUNK_SIZE_BYTES > sentBytes)
        {
            /* If we wrote less than a certain amount,
             * the IP-stack buffers seem to have been filled up.
             * lets give the kernel time to send some data and relieve ...
             */
            io_waitMicroSecs(WAIT_FOR_WRITE_USECS);
        }
        nextChunkSize = bytesRemaining;
        if(nextChunkSize > SEND_CHUNK_SIZE_BYTES)
        {
            nextChunkSize = SEND_CHUNK_SIZE_BYTES;
        }
        sentBytes = send(fd, nextChunk, nextChunkSize, 0);
        bytesRemaining -= sentBytes;
        nextChunk += sentBytes;
        if(0 > sentBytes)
        {
            LOG_CON(ERROR, fd, strerror(errno));
            return -1;
        }
    }
    return 0;
}
/*----------------------------------------------------------------------------*/
int io_waitMicroSecs(long usec)
{

    struct timespec timeToWait =
    {
        .tv_sec = 0,
        .tv_nsec = 1000 * usec,
    };

    if(1000 * 1000 < usec)
    {
        timeToWait.tv_sec = usec / 1000 * 1000;
        timeToWait.tv_nsec -= timeToWait.tv_sec * 1000 * 1000;
    }

    return nanosleep(&timeToWait, 0);
}
/*----------------------------------------------------------------------------*/
