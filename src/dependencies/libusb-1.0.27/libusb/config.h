/* config.h - configuration for libusb-1.0.27 built as a bundled dependency.
 *
 * This file is a manually crafted replacement for the autoconf-generated
 * config.h. It covers the platforms gem-imager builds libusb from source for:
 * Windows/MinGW and macOS. Linux keeps using the system libusb through
 * pkg-config, so it never reaches this file.
 */

#pragma once

/* Enable message logging */
#ifndef ENABLE_LOGGING
#define ENABLE_LOGGING 1
#endif

/* Default symbol visibility (empty for static library) */
#ifndef DEFAULT_VISIBILITY
#define DEFAULT_VISIBILITY
#endif

/* printf format attribute for GCC/Clang */
#if defined(__GNUC__) && (__GNUC__ >= 3)
#define PRINTF_FORMAT(fmt_idx, arg_idx) \
    __attribute__((__format__(__printf__, fmt_idx, arg_idx)))
#else
#define PRINTF_FORMAT(fmt_idx, arg_idx)
#endif

#if defined(_WIN32)

/* Define to 1 if the system has `struct timespec' */
#ifndef HAVE_STRUCT_TIMESPEC
#define HAVE_STRUCT_TIMESPEC 1
#endif

/* Windows platform */
#ifndef PLATFORM_WINDOWS
#define PLATFORM_WINDOWS 1
#endif

/* No POSIX sys/time.h on Windows */
/* #undef HAVE_SYS_TIME_H */

/* No POSIX poll on Windows */
/* #undef HAVE_POLL_H */
/* #undef HAVE_NFDS_T */

/* No Linux-specific features */
/* #undef HAVE_EVENTFD */
/* #undef HAVE_TIMERFD */

/* No clock_gettime on older Windows */
/* #undef HAVE_CLOCK_GETTIME */

/* No pthreads on Windows */
/* #undef HAVE_PTHREAD */
/* #undef HAVE_PTHREAD_CONDATTR_SETCLOCK */

#elif defined(__APPLE__)

/* macOS uses the POSIX event/thread abstractions with the darwin backend */
#ifndef PLATFORM_POSIX
#define PLATFORM_POSIX 1
#endif

#ifndef HAVE_SYS_TIME_H
#define HAVE_SYS_TIME_H 1
#endif

#ifndef HAVE_POLL_H
#define HAVE_POLL_H 1
#endif

/* <poll.h> declares nfds_t */
#ifndef HAVE_NFDS_T
#define HAVE_NFDS_T 1
#endif

/* pthread_threadid_np() is the darwin way to get a thread id */
#ifndef HAVE_PTHREAD_THREADID_NP
#define HAVE_PTHREAD_THREADID_NP 1
#endif

/* HAVE_CLOCK_GETTIME is deliberately left undefined: libusbi.h ignores it on
   Apple anyway (`#if defined(HAVE_CLOCK_GETTIME) && !defined(__APPLE__)`) and
   the darwin backend supplies usbi_get_monotonic_time()/usbi_get_real_time()
   itself via mach_absolute_time(). */
/* #undef HAVE_CLOCK_GETTIME */

/* Not available on macOS; threads_posix.c falls back to the realtime clock */
/* #undef HAVE_PTHREAD_CONDATTR_SETCLOCK */

/* Linux-only event primitives; events_posix.c falls back to pipe() */
/* #undef HAVE_EVENTFD */
/* #undef HAVE_TIMERFD */
/* #undef HAVE_PIPE2 */

#else
#error "Bundled libusb is only configured for Windows and macOS"
#endif
