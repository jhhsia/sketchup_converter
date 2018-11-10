// Copyright 2010 Google Inc. All Rights Reserved.

// Define things that are used on Windows all the time that are not defined on Mac

#ifndef SKETCHUP_SOURCE_COMMON_UTILS_MAC_WINTYPE_H_
#define SKETCHUP_SOURCE_COMMON_UTILS_MAC_WINTYPE_H_

#if defined(__APPLE__) || defined(__LINUX__)

#ifndef _MAC
#ifdef __APPLE__
#define _MAC
#endif
#endif

#include <limits.h>

// TODO(julmer): Minimize the number of typedefs we use and take control
// over all of them.
#if defined(SU_SDK)
#include <SketchUp/types.h>
#else
#include "sketchup/source/common/utils/types.h"
#endif

// On the Mac, ctype.h defines _T.  This is used all over in SketchUp
// for doing generic text handling, so we need to undefine it
#undef _T

//===================================================================

// Standard constants
#undef FALSE
#undef TRUE
#undef NULL

#define FALSE   0
#define TRUE    1
#define NULL    0

// This used to be hard coded to 255 but that's too short for most blaze
// tmp file paths on Linux.  PATH_MAX is equally unreliable as it's possible
// to make paths which exceed that length, but this is at least much longer.
#undef _MAX_PATH
#define _MAX_PATH PATH_MAX
// MAX_PATH might be defined elsewhere (i.e. teigha OdPlatform.h)
#ifndef MAX_PATH
#define MAX_PATH _MAX_PATH
#endif

//===================================================================

// NOTE: On Windows, BOOL is defined as 'int'. In Objective-C however,
// there is a BOOL type that is a 'char'.  We use the Windows definition
// so that things of type BOOL are serialized correcly.  We cannot redefine
// it in files that include objective-C headers though.
#ifndef __OBJC__
typedef int             BOOL;
#endif
typedef int             CBOOL;

typedef unsigned char   BYTE;
typedef unsigned short  WORD;
typedef long            *LPLONG;
typedef void            *LPVOID;

typedef int             INT;
typedef unsigned int    UINT;
typedef unsigned long   UINT_PTR;

// NOTE(mdurant) This declaration changes between 32-bit and 64-bit
// to match the ULONG type defined by Cocoa.
#ifdef __LP64__
typedef unsigned int ULONG;
#else
typedef unsigned long ULONG;
#endif
typedef unsigned long long ULONGLONG;
typedef long long LONGLONG;

// Required for BSTR.h
#ifndef WCHAR_DEFINED
typedef unsigned short unichar;
typedef unichar WCHAR;
#define WCHAR_DEFINED
#endif

//-------------------------------------------------------------------
// Calling conventions

#define CALLBACK
#define WINAPI

// APIENTRY is defined elsewhere (in GL/gl.h) so we wrap this to avoid a warning
#ifndef APIENTRY
  #define APIENTRY    WINAPI
#endif

//===================================================================

inline long InterlockedIncrement(long* val)
{
    (*val)++;
    return *val;
}

inline long InterlockedDecrement(long* val)
{
    (*val)--;
    return *val;
}

//===================================================================

// These are all used on Mac, apparently to hide NS* objects.
typedef void* HCURSOR;
typedef void* HWND;
typedef long HINSTANCE;

//===================================================================
#endif  // __APPLE__
#endif  // SKETCHUP_SOURCE_COMMON_UTILS_MAC_WINTYPE_H_


