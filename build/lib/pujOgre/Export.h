
#ifndef pujOgre_EXPORT_H
#define pujOgre_EXPORT_H

#ifdef PUJOGRE_STATIC_DEFINE
#  define pujOgre_EXPORT
#  define PUJOGRE_NO_EXPORT
#else
#  ifndef pujOgre_EXPORT
#    ifdef pujOgre_EXPORTS
        /* We are building this library */
#      define pujOgre_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define pujOgre_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef PUJOGRE_NO_EXPORT
#    define PUJOGRE_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef PUJOGRE_DEPRECATED
#  define PUJOGRE_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef PUJOGRE_DEPRECATED_EXPORT
#  define PUJOGRE_DEPRECATED_EXPORT pujOgre_EXPORT PUJOGRE_DEPRECATED
#endif

#ifndef PUJOGRE_DEPRECATED_NO_EXPORT
#  define PUJOGRE_DEPRECATED_NO_EXPORT PUJOGRE_NO_EXPORT PUJOGRE_DEPRECATED
#endif

/* NOLINTNEXTLINE(readability-avoid-unconditional-preprocessor-if) */
#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef PUJOGRE_NO_DEPRECATED
#    define PUJOGRE_NO_DEPRECATED
#  endif
#endif

#endif /* pujOgre_EXPORT_H */
