# Copyright 2014 Trimble Navigation Limited. All Rights Reserved.
# This is a sample Ruby extension demonstrating the use of SketchUp's
# Extension Licensing API.
#
# Usage:
# 1. Build the C extension in Visual Studio or Xcode using the included projects
#    Note that there are separate build configurations for Win32 and x64 on
#    Windows.
# 2. Copy this file (su_licensedrubyextension.rb) and the directory
#    su_licensedrubyextension into the Plugins directory, e.g.
#    C:\Users\<user>\AppData\Roaming\SketchUp\SketchUp 2015\SketchUp\Plugins
# 3. Run SketchUp, click "Licensed Extension" under the "Extensions" menu. You
#    should see a success message. Requires SketchUp 2015 or later.

require 'sketchup.rb'
require 'extensions.rb'

module MyModule
module LicensedRubyExtension

ext = SketchupExtension.new("Licensed Ruby Extension", "su_licensedrubyextension/loader")
ext.description="Example licensed extension"
ext.version = "1.0.0"
ext.creator = "SketchUp"
ext.copyright = "2016, Trimble Inc."

Sketchup.register_extension(ext, true)

end # module LicensedRubyExtension
end # module MyModule
