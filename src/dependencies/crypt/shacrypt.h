#ifndef SHACRYPT_H
#define SHACRYPT_H

#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/types.h>

#ifdef __APPLE__
#include <machine/endian.h>
#else
#ifdef _WIN32
#define	__LITTLE_ENDIAN	1234
#define __BYTE_ORDER __LITTLE_ENDIAN
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#else
#include <endian.h>
#endif
#endif

/* stpncpy and mempcpy are not available everywhere: mempcpy is a glibc
   extension, and on macOS <string.h> defines stpncpy as a fortifying macro,
   so a local definition of that name does not compile. Use private helpers
   instead of taking either name from libc. */
static inline char *crypt_stpncpy(char *dest, const char *src, size_t n)
{
    size_t size = strnlen(src, n);
    memcpy(dest, src, size);
    dest += size;
    if (size != n)
        dest[0] = '\0';

    return dest;
}

static inline void *crypt_mempcpy(void *dest, const void *src, size_t n)
{
    memcpy(dest, src, n);
    return (char *)dest + n;
}

#endif // SHACRYPT_H
