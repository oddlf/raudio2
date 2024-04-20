#ifndef RAUDIO2_CONFIG_H
#define RAUDIO2_CONFIG_H

#define RAUDIO2_VERSION_MAJOR 1
#define RAUDIO2_VERSION_MINOR 0
#define RAUDIO2_VERSION_PATCH 1

#if defined(_WIN32)
    #define RAUDIO2_SYSTEM_WINDOWS
#elif defined(__APPLE__) && defined(__MACH__)
    #include "TargetConditionals.h"
    #if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
        #define RAUDIO2_SYSTEM_IOS
    #elif TARGET_OS_MAC
        #define RAUDIO2_SYSTEM_MACOS
    #else
        #define RAUDIO2_SYSTEM_APPLE
    #endif
#elif defined(__unix__)
    #if defined(__ANDROID__)
        #define RAUDIO2_SYSTEM_ANDROID
    #elif defined(__linux__)
        #define RAUDIO2_SYSTEM_LINUX
    #elif defined(__FreeBSD__) || defined(__FreeBSD_kernel__)
        #define RAUDIO2_SYSTEM_FREEBSD
    #elif defined(__OpenBSD__)
        #define RAUDIO2_SYSTEM_OPENBSD
    #elif defined(__NetBSD__)
        #define RAUDIO2_SYSTEM_NETBSD
    #else
        #define RAUDIO2_SYSTEM_UNIX
    #endif
#endif

#if defined(RAUDIO2_SYSTEM_WINDOWS)
    #define RAUDIO2_EXPORT __declspec(dllexport)
    #define RAUDIO2_IMPORT __declspec(dllimport)
    #define RAUDIO2_CALLING_CONVENTION __cdecl

    #ifdef _MSC_VER
        #pragma warning(disable: 4251)
    #endif
#else
    #if __GNUC__ >= 4
        #define RAUDIO2_EXPORT __attribute__((__visibility__("default")))
        #define RAUDIO2_IMPORT __attribute__((__visibility__("default")))
        #define RAUDIO2_CALLING_CONVENTION
    #else
        // GCC < 4 exports everything
        #define RAUDIO2_EXPORT
        #define RAUDIO2_IMPORT
        #define RAUDIO2_CALLING_CONVENTION
    #endif
#endif

#if defined(RAUDIO2_NO_DEPRECATED_WARNINGS)
    #define RAUDIO2_DEPRECATED
#elif defined(_MSC_VER)
    #define RAUDIO2_DEPRECATED __declspec(deprecated)
#elif defined(__GNUC__) // gcc and clang
    #define RAUDIO2_DEPRECATED __attribute__ ((deprecated))
#else
    #define RAUDIO2_DEPRECATED
#endif

#endif // RAUDIO2_CONFIG_H
