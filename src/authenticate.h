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
#ifndef __AUTHENTICATE_H__
#define __AUTHENTICATE_H__

#include <stdbool.h>
#include <stdint.h>

/*----------------------------------------------------------------------------*/

struct AuthDB;

struct AuthConfig {

    char const *auth_socket;
    uint32_t token_lifetime_secs;

};

typedef char AuthToken[16];
typedef struct AuthDB AuthDB;
typedef struct AuthConfig AuthConfig;

/*----------------------------------------------------------------------------*/

AuthDB *auth_loadDBs(char *u, char *r) {
    return 0;
}

/*----------------------------------------------------------------------------*/

/**
 * Starts authenticator process
 */
void auth_start(AuthDB *db, AuthConfig cfg);

/**
 * Stops auth process
 */
void auth_stop();

/*----------------------------------------------------------------------------*/

/**
 * Checks whether user can be authenticated using password pw.
 * If successful, a new token is generated and written into `token`
 */
bool auth_authenticate(char const *user, char const *pw, AuthToken token);

/*----------------------------------------------------------------------------*/

/**
 * Checks whether `token` is associated with a user that is able to
 * access resource path `path`
 */
bool auth_isAuthorized(char const *path, AuthToken token);

/*----------------------------------------------------------------------------*/
#endif

