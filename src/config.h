/*
 * (C) 2016 Michael J. Beer
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA.
 */
#ifndef __CONFIG_H__
#define __CONFIG_H__
/*----------------------------------------------------------------------------*/
#define MAX_OPT_STR_LEN       255
#define MAX_PENDING_REQUESTS  25
#define DEFAULT_DOCUMENT_ROOT "/home/httpd/htdocs"
#define DEFAULT_INTERFACE     ANY_INTERFACE
#define DEFAULT_USER_DB       "/home/httpd/users"
#define DEFAULT_RULES_DB      "/home/httpd/rules"
#define DEFAULT_PORT          "531"
#define DEFAULT_TIMEOUT_SECS  10
#define PORT_MAX              ((1 << 16) -1)
/*----------------------------------------------------------------------------*/
#define MAX_DESCRIPTORS       25
/*----------------------------------------------------------------------------*/
#define ANY_INTERFACE         "*"
/*----------------------------------------------------------------------------*/
#define SOCKET_READ_BUFFER_LEN 255
/*----------------------------------------------------------------------------*/
#define MIN_TOKEN_LENGTH 16
#define MAX_TOKEN_LENGTH (64 * 1024)
/*----------------------------------------------------------------------------*/
#endif
