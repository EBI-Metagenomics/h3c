#ifndef H3C_EXPORT_H
#define H3C_EXPORT_H

/* clang-format off */
#ifdef H3C_STATIC_DEFINE
#  define H3C_API
#else
#  ifdef h3c_EXPORTS /* We are building this library */
#    ifdef _WIN32
#      define H3C_API __declspec(dllexport)
#    else
#      define H3C_API __attribute__((visibility("default")))
#    endif
#  else /* We are using this library */
#    ifdef _WIN32
#      define H3C_API __declspec(dllimport)
#    else
#      define H3C_API __attribute__((visibility("default")))
#    endif
#  endif
#endif
/* clang-format on */

#endif
