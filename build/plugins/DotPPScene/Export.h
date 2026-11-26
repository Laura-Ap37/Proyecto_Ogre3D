
#ifndef Plugin_pujOgreDotPPSceneCodec_EXPORT_H
#define Plugin_pujOgreDotPPSceneCodec_EXPORT_H

#ifdef PLUGIN_PUJOGREDOTPPSCENECODEC_STATIC_DEFINE
#  define Plugin_pujOgreDotPPSceneCodec_EXPORT
#  define PLUGIN_PUJOGREDOTPPSCENECODEC_NO_EXPORT
#else
#  ifndef Plugin_pujOgreDotPPSceneCodec_EXPORT
#    ifdef Plugin_pujOgreDotPPSceneCodec_EXPORTS
        /* We are building this library */
#      define Plugin_pujOgreDotPPSceneCodec_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define Plugin_pujOgreDotPPSceneCodec_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef PLUGIN_PUJOGREDOTPPSCENECODEC_NO_EXPORT
#    define PLUGIN_PUJOGREDOTPPSCENECODEC_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef PLUGIN_PUJOGREDOTPPSCENECODEC_DEPRECATED
#  define PLUGIN_PUJOGREDOTPPSCENECODEC_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef PLUGIN_PUJOGREDOTPPSCENECODEC_DEPRECATED_EXPORT
#  define PLUGIN_PUJOGREDOTPPSCENECODEC_DEPRECATED_EXPORT Plugin_pujOgreDotPPSceneCodec_EXPORT PLUGIN_PUJOGREDOTPPSCENECODEC_DEPRECATED
#endif

#ifndef PLUGIN_PUJOGREDOTPPSCENECODEC_DEPRECATED_NO_EXPORT
#  define PLUGIN_PUJOGREDOTPPSCENECODEC_DEPRECATED_NO_EXPORT PLUGIN_PUJOGREDOTPPSCENECODEC_NO_EXPORT PLUGIN_PUJOGREDOTPPSCENECODEC_DEPRECATED
#endif

/* NOLINTNEXTLINE(readability-avoid-unconditional-preprocessor-if) */
#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef PLUGIN_PUJOGREDOTPPSCENECODEC_NO_DEPRECATED
#    define PLUGIN_PUJOGREDOTPPSCENECODEC_NO_DEPRECATED
#  endif
#endif

#endif /* Plugin_pujOgreDotPPSceneCodec_EXPORT_H */
