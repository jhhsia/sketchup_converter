//  Copyright (c) 2015 Trimble. All rights reserved.
// This programs reads a LayOut file and prinst the number of pages and layers to the console.

#include <iostream>
#include <LayOutAPI/common.h>
#include <LayOutAPI/initialize.h>
#include <LayOutAPI/model/document.h>


int main(int argc, const char* argv[]) {

  if (argc < 2) {
    std::cout << "Pass in the full path of the layout file." << std::endl;
    return -1;
  }

  // Parse the argument
  const char* layout_path_string_ref = NULL;
  layout_path_string_ref = argv[1];
  LOInitialize();

  // Create a document reference and load the string into it.
  LODocumentRef layout_doc_ref = SU_INVALID;
  SUResult doc_create_res = LODocumentCreateFromFile(&layout_doc_ref,
                                                     layout_path_string_ref);

  // Check return code for creating the layout document.
  if (doc_create_res != SU_ERROR_NONE){
    std::cout << "Unable to create document!\n";
    return 1;
  }

  // Get the number of layers in this document.
  size_t num_layers = 0;
  LODocumentGetNumberOfLayers(layout_doc_ref, &num_layers);

  // Get the number of pages in this document.
  size_t num_pages = 0;
  LODocumentGetNumberOfPages(layout_doc_ref, &num_pages);

  // Print to console the number of pages.
  std::cout << "This file has " << num_pages << " page(s) and " << num_layers << " layer(s).\n";

  // Release the document object from memory.
  LODocumentRelease(&layout_doc_ref);
  LOTerminate();
}
