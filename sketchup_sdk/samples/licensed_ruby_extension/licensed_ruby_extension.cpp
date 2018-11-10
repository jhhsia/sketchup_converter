// Copyright 2014 Trimble Navigation Limited. All Rights Reserved.
//
// This is a sample Ruby C Extension demonstrating the use of SketchUp's
// Extension Licensing API. Note that this API does not link with SketchUpAPI.dll
// (or slapi.framework) but uses symbols exported from the SketchUp executable.
// On Windows, this is achieved by linking with the included import library
// SketchUp.lib. On Mac, SketchUp 2015 (or later) must be installed and located
// at "/Applications/Sketchup 2015/Sketchup.app", which is then set as the
// "Bundle Loader" for the library.

#include "RubyUtils/RubyUtils.h"
#include <SketchUpAPI/extension_license.h>
#include "./ExportMacros.h"

// Checks our extension's license.
static bool CheckLicense() {
  const char* ext_id = "4e215280-dd23-40c4-babb-b8a8dd29d5ee";
  SUExtensionLicense ext_lic;
  SUResult res = SUGetExtensionLicense(ext_id, &ext_lic);
  if (res != SU_ERROR_NONE || !ext_lic.is_licensed) {
    // Our extension is not licensed, we better abort.
    return false;
  }
  // else we are good to run
  return true;
}

// Implementation of SUEX_Licensed.do_work
static VALUE DoWork() {
  if (!CheckLicense()) {
    // We aren't allowed to run. Raising a ruby error here for demonstration
    // purposes but a well-behaved extension would display a user-friendly
    // message in this case.
    rb_raise(rb_eRuntimeError, "This extension is not licensed.");
    return Qnil;
  }
  // We are good to run. Do the work here.
  // ...
  return Qtrue;
}

// Entry point of the C extension.
extern "C" CEXT_EXPORT
void Init_licensed_ruby_extension()
{
  VALUE mSUEX_HelloWorld = rb_define_module("SUEX_Licensed");
  rb_define_module_function(mSUEX_HelloWorld, "do_work",
      VALUEFUNC(DoWork), 0);
}
