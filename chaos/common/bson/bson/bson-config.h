#ifndef BSON_CONFIG_H
#define BSON_CONFIG_H

#include <boost/detail/endian.hpp>

/*
 * Define to 1234 for Little Endian, 4321 for Big Endian.
 */
#if defined(BOOST_LITTLE_ENDIAN)
#define BSON_BYTE_ORDER 1234
#elif defined(BOOST_BIG_ENDIAN)
#define BSON_BYTE_ORDER 4321
#else
#error "unable to determine system endianness"
#endif

#define BSON_INSIDE
#define BSON_COMPILATION

/*
 * Define to 1 if you have stdbool.h
 */
#define BSON_HAVE_STDBOOL_H 1
#if BSON_HAVE_STDBOOL_H != 1
# undef BSON_HAVE_STDBOOL_H
#endif


/*
 * Define to 1 for POSIX-like systems, 2 for Windows.
 */
#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#define BSON_OS 1
#else
#define BSON_OS 0
#endif


/*
 * Define to 1 if we have access to GCC 32-bit atomic builtins.
 * While this requires GCC 4.1+ in most cases, it is also architecture
 * dependent. For example, some PPC or ARM systems may not have it even
 * if it is a recent GCC version.
 */
#if !defined DISABLE_COMPARE_AND_SWAP
#define BSON_HAVE_ATOMIC_32_ADD_AND_FETCH 1
#else
#define BSON_HAVE_ATOMIC_32_ADD_AND_FETCH 0
#endif

#if BSON_HAVE_ATOMIC_32_ADD_AND_FETCH != 1
# undef BSON_HAVE_ATOMIC_32_ADD_AND_FETCH
#endif

/*
 * Similarly, define to 1 if we have access to GCC 64-bit atomic builtins.
 */
#if !defined DISABLE_COMPARE_AND_SWAP
#define BSON_HAVE_ATOMIC_64_ADD_AND_FETCH 1
#else
#define BSON_HAVE_ATOMIC_64_ADD_AND_FETCH 0
#endif

#if BSON_HAVE_ATOMIC_64_ADD_AND_FETCH != 1
# undef BSON_HAVE_ATOMIC_64_ADD_AND_FETCH
#endif


/*
 * Define to 1 if your system requires {} around PTHREAD_ONCE_INIT.
 * This is typically just Solaris 8-10.
 */
#define BSON_PTHREAD_ONCE_INIT_NEEDS_BRACES 0
#if BSON_PTHREAD_ONCE_INIT_NEEDS_BRACES != 1
# undef BSON_PTHREAD_ONCE_INIT_NEEDS_BRACES
#endif


/*
 * Define to 1 if you have clock_gettime() available.
 */
#define BSON_HAVE_CLOCK_GETTIME 1
#if BSON_HAVE_CLOCK_GETTIME != 1
# undef BSON_HAVE_CLOCK_GETTIME
#endif


/*
 * Define to 1 if you have strnlen available on your platform.
 */
#define BSON_HAVE_STRNLEN 1
#if BSON_HAVE_STRNLEN != 1
# undef BSON_HAVE_STRNLEN
#endif


/*
 * Define to 1 if you have snprintf available on your platform.
 */
#define BSON_HAVE_SNPRINTF 1
#if BSON_HAVE_SNPRINTF != 1
# undef BSON_HAVE_SNPRINTF
#endif


/*
 * Define to 1 if you have reallocf available on your platform.
 */
#define BSON_HAVE_REALLOCF 0
#if BSON_HAVE_REALLOCF != 1
# undef BSON_HAVE_REALLOCF
#endif


/*
 * Define to 1 if you have struct timespec available on your platform.
 */
#define BSON_HAVE_TIMESPEC 1
#if BSON_HAVE_TIMESPEC != 1
# undef BSON_HAVE_TIMESPEC
#endif


/*
 * Define to 1 if you want extra aligned types in libbson
 */
#define BSON_EXTRA_ALIGN 1
#if BSON_EXTRA_ALIGN != 1
# undef BSON_EXTRA_ALIGN
#endif


/*
 * Define to 1 if you have SYS_gettid syscall
 */
#define BSON_HAVE_SYSCALL_TID 0
#if BSON_HAVE_SYSCALL_TID != 1
# undef BSON_HAVE_SYSCALL_TID
#endif


#endif /* BSON_CONFIG_H */
