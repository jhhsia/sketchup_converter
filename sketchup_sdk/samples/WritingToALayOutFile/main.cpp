// Copyright (c) 2015 LayOut Team. All rights reserved.
// This program creates a .layout file at the specified path.

#include <stdio.h>
#include <iostream>
#include <LayOutAPI/common.h>
#include <LayOutAPI/initialize.h>
#include <LayOutAPI/model/document.h>
#include <LayOutAPI/model/pageinfo.h>
#include <LayOutAPI/model/rectangle.h>

int main(int argc, const char* argv[]) {

  if (argc < 2) {
    std::cout << "Pass in a full path for the layout file." << std::endl;
    return -1;
  }

  // Parse the argument
  const char* layout_path_string_ref = NULL;
  layout_path_string_ref = argv[1];
  LOInitialize();

  // Create a new document in memory. It has one page and one layer by default.
  LODocumentRef doc = SU_INVALID;
  SUResult result = LODocumentCreateEmpty(&doc);
  if (result != SU_ERROR_NONE) {
    std::cout << "Unable to create document!";
    LOTerminate();
    return 1;
  }

  // Create a reference for the first page (added by default).
  LOPageRef pageOne = SU_INVALID;
  LODocumentGetPageAtIndex(doc, 0, &pageOne);

  // Create a reference for the first layer (added by default).
  LOLayerRef layerOne = SU_INVALID;
  LODocumentGetLayerAtIndex(doc, 0, &layerOne);

  // Set the width and height of document's page.
  LOPageInfoRef docPageInfo = SU_INVALID;
  LODocumentGetPageInfo(doc, &docPageInfo);
  double width = 8.5;
  double height = 11;
  LOPageInfoSetWidth(docPageInfo, width);
  LOPageInfoSetHeight(docPageInfo, height);

  // Give the page a nice margin.
  double halfInchMargin = 0.5;
  SUColor marginColor = {/*red*/ 51, /*green*/ 204, /*blue*/ 231, /*alpha*/ 255};
  LOPageInfoSetLeftMargin(docPageInfo, halfInchMargin);
  LOPageInfoSetTopMargin(docPageInfo, halfInchMargin);
  LOPageInfoSetRightMargin(docPageInfo, halfInchMargin);
  LOPageInfoSetBottomMargin(docPageInfo, halfInchMargin);
  LOPageInfoSetMarginColor(docPageInfo, marginColor);
  bool showMargins = true;
  LOPageInfoSetShowMargins(docPageInfo, showMargins);

  // Create a rectangle.
  LOAxisAlignedRect2D bounds = {{1.0, 1.0}, {5.0, 5.0}};
  LORectangleRef rectangle = SU_INVALID;
  LORectangleCreate(&rectangle, &bounds);

  // Get the rectangle object as an entity.
  LOEntityRef entity = LORectangleToEntity(rectangle);

  // Add the rectangle entity to the document.
  LODocumentAddEntity(doc, entity, layerOne, pageOne);

  // Save out the document.
  result = LODocumentSaveToFile(doc, layout_path_string_ref,
                                LODocumentVersion_Current);

  LORectangleRelease(&rectangle);
  LODocumentRelease(&doc);
  LOTerminate();

  if (result != SU_ERROR_NONE) {
    std::cout << "Unable to save document!";
    return 1;
  }

  return 0;
}











