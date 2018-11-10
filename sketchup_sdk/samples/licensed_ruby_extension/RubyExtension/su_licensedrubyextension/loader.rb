# Copyright 2014 Trimble Navigation Limited. All Rights Reserved.

# NOTE: This file should be scrambled (i.e. turned into an rbs file) since it
# includes a call to the licensing API. Note that prior to SketchUp 2014, it was
# not possible to scramble this loader file. The workaround would be to place
# the contents of this file in another rbs file and just Sketchup::require that
# file from here.

module MyModule::LicensedRubyExtension

def self.execute
  # Run some code from the C extension
  if Sketchup.respond_to?(:is_64bit?) && Sketchup.is_64bit?
    # Load 64-bit binary.
    require_relative "bin64/licensed_ruby_extension"
  else
    # Load 32-bit binary.
    require_relative "bin/licensed_ruby_extension"
  end
  if (SUEX_Licensed.do_work)
    UI.messagebox("Success!")
  else
    UI.messagebox("FAILED!")
  end
end

#----------------------------------------------------------------------------
# Check our extension license and add our menu item only if we are licensed.
ext_id = "4e215280-dd23-40c4-babb-b8a8dd29d5ee"
ext_lic = Sketchup::Licensing.get_extension_license(ext_id)
if ext_lic.licensed?
  # Our extension is licensed, add our UI elements
  UI.menu('Extensions').add_item('Licensed Extension') { execute() }
end

end # module MyModule::LicensedRubyExtension
