#!/bin/sh
xcrun install_name_tool -change @rpath/slapi.framework/Versions/Current/slapi @executable_path/../Frameworks/slapi.framework/Versions/Current/slapi $BUILT_PRODUCTS_DIR/$EXECUTABLE_PATH
