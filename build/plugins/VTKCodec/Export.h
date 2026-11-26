
#ifndef Plugin_pujOgreVTKCodec_EXPORT_H
#define Plugin_pujOgreVTKCodec_EXPORT_H

#ifdef PLUGIN_PUJOGREVTKCODEC_STATIC_DEFINE
#  define Plugin_pujOgreVTKCodec_EXPORT
#  define PLUGIN_PUJOGREVTKCODEC_NO_EXPORT
#else
#  ifndef Plugin_pujOgreVTKCodec_EXPORT
#    ifdef Plugin_pujOgreVTKCodec_EXPORTS
        /* We are building this library */
#      define Plugin_pujOgreVTKCodec_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define Plugin_pujOgreVTKCodec_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef PLUGIN_PUJOGREVTKCODEC_NO_EXPORT
#    define PLUGIN_PUJOGREVTKCODEC_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef PLUGIN_PUJOGREVTKCODEC_DEPRECATED
#  define PLUGIN_PUJOGREVTKCODEC_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef PLUGIN_PUJOGREVTKCODEC_DEPRECATED_EXPORT
#  define PLUGIN_PUJOGREVTKCODEC_DEPRECATED_EXPORT Plugin_pujOgreVTKCodec_EXPORT PLUGIN_PUJOGREVTKCODEC_DEPRECATED
#endif

#ifndef PLUGIN_PUJOGREVTKCODEC_DEPRECATED_NO_EXPORT
#  define PLUGIN_PUJOGREVTKCODEC_DEPRECATED_NO_EXPORT PLUGIN_PUJOGREVTKCODEC_NO_EXPORT PLUGIN_PUJOGREVTKCODEC_DEPRECATED
#endif

/* NOLINTNEXTLINE(readability-avoid-unconditional-preprocessor-if) */
#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef PLUGIN_PUJOGREVTKCODEC_NO_DEPRECATED
#    define PLUGIN_PUJOGREVTKCODEC_NO_DEPRECATED
#  endif
#endif

#endif /* Plugin_pujOgreVTKCodec_EXPORT_H */
