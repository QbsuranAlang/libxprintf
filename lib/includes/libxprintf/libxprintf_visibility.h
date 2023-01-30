/*
 * ORIGINAL LICENSE
 */


#ifndef LIBXPRINTF_VISIBILITY_H
#define LIBXPRINTF_VISIBILITY_H

/* https://gcc.gnu.org/wiki/Visibility */
#if defined(LIBXPRINTF_STATICLIB)
#  define X_EXPORT
#elif defined(WIN32)
#  if defined(BUILDING_LIBXPRINTF)
#    define X_EXPORT __declspec(dllexport)
#  else /* defined(BUILDING_LIBXPRINTF) */
#    define X_EXPORT __declspec(dllimport)
#  endif /* !defined(BUILDING_LIBXPRINTF) */
#else
#  if defined(BUILDING_LIBXPRINTF)
#    if __GNUC__ >= 4
#      define X_EXPORT __attribute__((visibility("default")))
#      define X_LOCAL  __attribute__((visibility("hidden")))
#    endif /* !(__GNUC__ >= 4) */
#  else /* defined(BUILDING_LIBXPRINTF) */
#    define X_EXPORT
#  endif /* !defined(BUILDING_LIBXPRINTF) */
#endif

#if !defined(X_LOCAL)
#  define X_LOCAL /* nothing */
#endif /* !!defined(X_LOCAL) */

#endif /* !LIBXPRINTF_VISIBILITY_H */
