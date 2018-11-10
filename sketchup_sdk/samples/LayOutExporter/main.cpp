// Copyright 2015 Trimble Navigation Limited. All Rights Reserved.
// This program exports a LayOut file to an image, either png, pdf, or jpg.

/*
Usage examples:
 -i /Users/aparker/Desktop/layout_files/asdf.layout -o /Users/aparker/Desktop/layout_files/asdf.pdf -r medium
 -i /Users/aparker/Desktop/layout_files/asdf.layout -o /Users/aparker/Desktop/layout_files/asdf.pdf -r high
 -i /Users/aparker/Desktop/layout_files/asdf.layout -o /Users/aparker/Desktop/layout_files/asdf.png -dpi 600
 -i /Users/aparker/Desktop/layout_files/asdf.layout -o /Users/aparker/Desktop/layout_files/asdf.jpg -dpi 1200
*/

#include <LayOutAPI/common.h>
#include <LayOutAPI/initialize.h>
#include <LayOutAPI/model/dictionary.h>
#include <LayOutAPI/model/document.h>
#include <LayOutAPI/model/documentexportoptions.h>
#include <LayOutAPI/model/image.h>
#include <LayOutAPI/model/imagerep.h>
#include <LayOutAPI/model/pageinfo.h>
#include <LayOutAPI/model/typed_value.h>
#include <iostream>
#include <string>
#include <cstdlib>

using namespace std;

void PrintHelp() {
  cout << "LayOutExporter -i inputfile -o outputfile [-r resolution] [-dpi dpi]" << endl;
  cout << "               [-sp startpage] [-ep endpage] [-nc] [-cq quality] [-h]" << endl;
  cout << "  -i inputfile  Specifies the input .layout file. Required." << endl;
  cout << "  -o outputfile Specifies the output PDF, JPG, or PNG file. Required." << endl;
  cout << "  -r resolution Specifies the output resolution for images when exporting to" << endl;
  cout << "                PDF. Must be one of 'low', 'medium', or 'high'. If not" << endl;
  cout << "                specified, it defaults to the document's output resolution" << endl;
  cout << "                setting." << endl;
  cout << "  -dpi dpi      Specifies the output image DPI when exporting to PNG or PNG." << endl;
  cout << "                If not specified, it defaults to 96." << endl;
  cout << "  -sp startpage Specifies the index of the first page to export. If not" << endl;
  cout << "                specified, it defaults to 0." << endl;
  cout << "  -ep endpage   Specifies the index of the last page to export. If not" << endl;
  cout << "                specified, it defaults to the index of the last page in the" << endl;
  cout << "                document." << endl;
  cout << "  -nc           Specifies that JPEG image compression should be disabled when" << endl;
  cout << "                exporting to PDF. This usually results in larger PDF files. If" << endl;
  cout << "                not specified, JPEG image compression will be enabled." << endl;
  cout << "  -cq quality   Specifies the JPEG image compression quality from 0.0 to 1.0" << endl;
  cout << "                to use when exporting to PDF with JPEG image compression" << endl;
  cout << "                enabled. If not specified, it defaults to 0.5." << endl;
  cout << "  -h            Displays this help information." << endl;
}

int main(int argc, const char * argv[]) {

  // Create variables to assign our arguments to
  string input_file, output_file, resolution_str, dpi_str, startpage_str,
         endpage_str, compression_quality_str;
  bool disable_image_compression = false;

  if (argc == 1) {
    PrintHelp();
    return 0;
  } else {
    // Loop through the args and get the values to the right of each flag
    for (int i = 1; i < argc; i++) {
      const char* arg = argv[i];
      if (strcmp(arg, "-i") == 0) {
        i++;
        if (i == argc) {
          cout << "Expecting input filename after -i" << endl;
          return -1;
        }
        input_file = argv[i];
      } else if (strcmp(arg, "-o") == 0) {
        i++;
        if (i == argc) {
          cout << "Expecting output filename after -o" << endl;
          return -1;
        }
        output_file = argv[i];
      } else if (strcmp(arg, "-r") == 0) {
        i++;
        if (i == argc) {
          cout << "Expecting low, medium or high after -r" << endl;
          return -1;
        }
        resolution_str = argv[i];
      } else if (strcmp(arg, "-dpi") == 0) {
        i++;
        if (i == argc) {
          cout << "Expecting dpi value after -dpi" << endl;
          return -1;
        }
        dpi_str = argv[i];
      } else if (strcmp(arg, "-sp") == 0) {
        i++;
        if (i == argc) {
          cout << "Expecting start page index after -sp" << endl;
          return -1;
        }
        startpage_str = argv[i];
      } else if (strcmp(arg, "-ep") == 0) {
        i++;
        if (i == argc) {
          cout << "Expecting end page index after -ep" << endl;
          return -1;
        }
        endpage_str = argv[i];
      } else if (strcmp(arg, "-nc") == 0) {
        disable_image_compression = true;
      } else if (strcmp(arg, "-cq") == 0) {
        i++;
        if (i == argc) {
          cout << "Expecting compression quality after -cq" << endl;
          return -1;
        }
        compression_quality_str = argv[i];
      } else if (strcmp(arg, "-h") == 0) {
        PrintHelp();
      } else {
        cout << "Unrecognized argument: " << arg << endl;
        return -1;
      }
    }
  }

  if (input_file.empty()) {
    cout << "An input .layout file must be specified using the -i argument." << endl;
    return -1;
  }

  if (output_file.empty()) {
    cout << "An output file must be specified using the -i argument." << endl;
    return -1;
  }

  // Separate the export path from the file base name in output_file
  string output_extension;
  string output_path;
  string output_name;
  string::size_type e_idx = output_file.rfind('.');
  string::size_type f_idx = output_file.find_last_of("/\\");
  if (e_idx != string::npos /*&& f_idx != string::npos*/) {
     output_extension = output_file.substr(e_idx + 1);
     if (f_idx != string::npos)
       output_path = output_file.substr(0, f_idx + 1);
     output_name = output_file.substr(f_idx + 1, output_file.length() -
         output_extension.length() -1 - output_path.length());
  } else {
    cout << "There was an error parsing the output file argument." << endl;
    return -1;
  }

  // Map the -r argument to our LOImageResolution enum.
  int resolution = -1;
  if (!resolution_str.empty()) {
    if (resolution_str == "low") {
      resolution = LOImageResolution_Low;
    } else if (resolution_str == "medium") {
      resolution = LOImageResolution_Medium;
    } else if (resolution_str == "high") {
      resolution = LOImageResolution_High;
    } else {
      cout << "Invalid resolution specified by -r.  Must be 'low', 'medium', or 'high'." << endl;
      return -1;
    }
  }

  // Map the extension type to our LOImageRepOutputFormat enum
  LOImageRepOutputFormat image_format = LOImageRepOutputFormat_PNG;
  if (output_extension != "pdf") {
    if (output_extension == "png") {
      image_format = LOImageRepOutputFormat_PNG;
    } else if (output_extension == "jpg" || output_extension == "jpeg") {
      image_format = LOImageRepOutputFormat_JPG;
    } else {
      cout << "Unsupported output image file type." << endl;
      return -1;
    }
  }

  int dpi = 96;
  if (!dpi_str.empty()) {
    dpi = atoi(dpi_str.c_str());
    if (dpi < 1 || dpi > 1200) {
      cout << "Invalid dpi.  Must be an integer value from 1 to 1200." << endl;
      return -1;
    }
  }

  int startpage = 0;
  if (!startpage_str.empty()) {
    startpage = atoi(startpage_str.c_str());
  }

  int endpage = -1;
  if (!endpage_str.empty()) {
    endpage = atoi(endpage_str.c_str());
  }

  double compression_quality = 0.5;
  if (!compression_quality_str.empty()) {
    compression_quality = atof(compression_quality_str.c_str());
    if (compression_quality < 0.0 || compression_quality > 1.0) {
      cout << "Invalid image compression quality.  Must be a double value from 0.0 to 1.0." << endl;
      return -1;
    }
  }

  LOInitialize();

  // Create the document reference for the LO file
  LODocumentRef document = SU_INVALID;
  SUResult result = LODocumentCreateFromFile(&document, input_file.c_str());
  if (result != SU_ERROR_NONE) {
    cout << "Unable to load the input LayOut file." << endl;
    LOTerminate();
    return -1;
  }

  // Validate the start/end page.
  size_t num_pages = 0;
  LODocumentGetNumberOfPages(document, &num_pages);
  if (startpage < 0 || startpage >= static_cast<int>(num_pages)) {
    cout << "Invalid start page." << endl;
    LODocumentRelease(&document);
    LOTerminate();
    return -1;
  }
  if (endpage == -1) {
    endpage = static_cast<int>(num_pages) - 1;
  } else if (endpage < startpage || endpage >= static_cast<int>(num_pages)) {
    cout << "Invalid end page." << endl;
    LODocumentRelease(&document);
    LOTerminate();
    return -1;
  }

  // Create a dictionary for the options
  LODictionaryRef options_dict = SU_INVALID;
  LODictionaryCreate(&options_dict);
  LOTypedValueRef typed_value = SU_INVALID;
  LOTypedValueCreate(&typed_value);
  LOTypedValueSetInt32(typed_value, startpage);
  LODictionarySetValue(options_dict, LOExportOption_StartPage, typed_value);
  LOTypedValueSetInt32(typed_value, endpage);
  LODictionarySetValue(options_dict, LOExportOption_EndPage, typed_value);
  LOTypedValueSetInt32(typed_value, dpi);
  LODictionarySetValue(options_dict, LOExportOption_DPI, typed_value);
  if (resolution != -1) {
    LOTypedValueSetInt32(typed_value, resolution);
    LODictionarySetValue(options_dict, LOExportOption_OutputResolution,
                         typed_value);
  }
  if (disable_image_compression) {
    LOTypedValueSetBool(typed_value, false);
    LODictionarySetValue(options_dict, LOExportOption_CompressImages,
                         typed_value);
  } else {
    LOTypedValueSetDouble(typed_value, compression_quality);
    LODictionarySetValue(options_dict, LOExportOption_ImageCompressionQuality,
                         typed_value);
  }

  int return_value = 0;
  if (output_extension == "pdf") {
    // Make the export to pdf
    result = LODocumentExportToPDF(document,
                                   output_file.c_str(),
                                   options_dict);
    if (result != SU_ERROR_NONE) {
      cout << "Unable to save the document to pdf." << endl;
      return_value = -1;
    }
  } else if (output_extension == "png") {
    //Make the export to png
    result = LODocumentExportToImageSet(document,
                                        output_path.c_str(),
                                        output_name.c_str(),
                                        image_format,
                                        options_dict);
    if (result != SU_ERROR_NONE) {
      cout << "Unable to save the document to png." << endl;
      return_value = -1;
    }
  } else if (output_extension == "jpg" || output_extension == "jpeg") {
    // Make the export to jpg
    result = LODocumentExportToImageSet(document,
                                        output_path.c_str(),
                                        output_name.c_str(),
                                        image_format,
                                        options_dict);
    if (result != SU_ERROR_NONE) {
      cout << "Unable to save the document to jpg." << endl;
      return_value = -1;
    }
  }

  // Release the objects that we created
  LOTypedValueRelease(&typed_value);
  LODictionaryRelease(&options_dict);
  LODocumentRelease(&document);
  LOTerminate();

  return return_value;
}