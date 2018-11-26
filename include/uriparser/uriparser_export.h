
#ifndef URIPARSER_EXPORT_H
#define URIPARSER_EXPORT_H

#ifdef URIPARSER_STATIC_DEFINE
#  define URIPARSER_EXPORT
#  define URIPARSER_NO_EXPORT
#else
#  ifndef URIPARSER_EXPORT
#    ifdef uriparser_EXPORTS
        /* We are building this library */
#      define URIPARSER_EXPORT 
#    else
        /* We are using this library */
#      define URIPARSER_EXPORT 
#    endif
#  endif

#  ifndef URIPARSER_NO_EXPORT
#    define URIPARSER_NO_EXPORT 
#  endif
#endif

#ifndef URIPARSER_DEPRECATED
#  define URIPARSER_DEPRECATED __declspec(deprecated)
#endif

#ifndef URIPARSER_DEPRECATED_EXPORT
#  define URIPARSER_DEPRECATED_EXPORT URIPARSER_EXPORT URIPARSER_DEPRECATED
#endif

#ifndef URIPARSER_DEPRECATED_NO_EXPORT
#  define URIPARSER_DEPRECATED_NO_EXPORT URIPARSER_NO_EXPORT URIPARSER_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef URIPARSER_NO_DEPRECATED
#    define URIPARSER_NO_DEPRECATED
#  endif
#endif

#endif /* URIPARSER_EXPORT_H */
