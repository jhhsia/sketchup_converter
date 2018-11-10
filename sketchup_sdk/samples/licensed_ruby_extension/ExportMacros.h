// Copyright 2014 Trimble Navigation Limited. All Rights Reserved.
#ifndef EXPORT_MACROS_H_
#define EXPORT_MACROS_H_

#ifdef _WINDOWS

#define CEXT_EXPORT __declspec(dllexport)

#else // Mac

#define CEXT_EXPORT __attribute__ ((visibility("default")))

#endif

#endif // EXPORT_MACROS_H_
