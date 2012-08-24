/* -*- Mode: C; tab-width: 2; c-basic-offset: 2 -*- */
/* vim:set softtabstop=2 shiftwidth=2: */
/* 
 * Copyright (c) 2012, Jonas Pommerening <jonas.pommerening@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met: 
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef _GLOB_H_
#define _GLOB_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <unistd.h>
#include <stddef.h>

#include <fnmatch.h>

#define GLOB_EXTERN extern

typedef enum {
  GLOB_NOMATCH,
  GLOB_MATCH,
  GLOB_PUSHDIR,
  GLOB_READDIR = GLOB_PUSHDIR,
  GLOB_POPDIR,
  GLOB_CONTINUE,
  GLOB_STOP,
  GLOB_ERROR,
} glob_state_t;

typedef enum {
  GLOB_END  = 0,
  GLOB_DIR  = (1 << 0),
  GLOB_FILE = (1 << 1),
  GLOB_LINK = (1 << 2)
} glob_filetype_t;

typedef struct glob_context_s glob_context_t;

struct glob_context_s {
  fnmatch_context_t fnmatch;
  glob_filetype_t type;
  char*  path;
  size_t plen;
  size_t palloc;
  
  char*  buffer;
  size_t buflen;
  size_t bufalloc;
};

/* MARK: - Resumeable API *//**
 * @name Resumeable API
 */
GLOB_EXTERN void glob_context_init( glob_context_t* context, fnmatch_pattern_t* pattern, const char* path );
GLOB_EXTERN void glob_context_destroy( glob_context_t* context );
GLOB_EXTERN void glob_context_dirent( glob_context_t* context, glob_filetype_t type, const char* dirent );
GLOB_EXTERN glob_state_t glob_context_match( glob_context_t* context );
/** @} */

/* MARK: - POSIX.2 API *//**
 * @name POSIX.2 API
 * The POSIX implementation is rather simple.
 * @{
 */

/* The similarity to Linux is coincidental ;) */
#define	GLOB_ERR      (1 << 0)
#define	GLOB_MARK     (1 << 1)
#define	GLOB_NOSORT   (1 << 2)
#define	GLOB_DOOFFS   (1 << 3)
#define	GLOB_NOCHECK  (1 << 4)
#define	GLOB_APPEND   (1 << 5)
#define	GLOB_NOESCAPE (1 << 6)
#define	GLOB_PERIOD   (1 << 7)

#define GLOB_NOMATCH 1
#define GLOB_NOSPACE 2
#define GLOB_ABORTED 3
#define GLOB_NOSYS   4

typedef struct {
  size_t gl_pathc;
  char** gl_pathv;
  size_t gl_offs;
} glob_t;

GLOB_EXTERN int glob( const char* pattern, int flags,
  int (*errfunc) (const char*, int), glob_t *glob );
GLOB_EXTERN void globfree( glob_t *glob );

/** @} */

#ifdef __cplusplus
}
#endif

#endif
