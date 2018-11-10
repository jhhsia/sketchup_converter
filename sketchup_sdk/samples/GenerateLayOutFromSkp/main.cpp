// Copyright 2015 Trimble Navigation Ltd, All rights reserved.
// This file is intended for public distribution.

#include <iostream>
#include <vector>
#include <LayOutAPI/common.h>
#include <LayOutAPI/initialize.h>
#include <LayOutAPI/model/document.h>
#include <LayOutAPI/model/layer.h>
#include <LayOutAPI/model/page.h>
#include <LayOutAPI/model/sketchupmodel.h>


int main() {
  LOInitialize();

  const char* skp_file_path = "./SimpleHouse.skp";
  LODocumentRef document_ref = SU_INVALID;
  LOLayerRef layer_ref = SU_INVALID;
  LOSketchUpModelRef lo_model_ref = SU_INVALID;
  LOEntityRef entity_ref = SU_INVALID;
  LOAxisAlignedRect2D bounds = {{.25, .5}, {10.75, 6.5}};
  SUResult result = LOSketchUpModelCreate(&lo_model_ref, skp_file_path, &bounds);

  // Check return code for loading the skp file.
  if (result != SU_ERROR_NONE) {
    std::cout << "Unable to open SketchUp file!" << std::endl;
    return -1;
  }

  LODocumentCreateFromTemplate(&document_ref, "./Letter Landscape.layout");
  LODocumentAddLayer(document_ref, false /* shared */, &layer_ref);
  LOLayerSetName(layer_ref, "Models");

  // Get the scene count so we know how many pages we'll need in our LayOut
  // document.
  size_t scene_count = 0;
  LOSketchUpModelGetNumberOfAvailableScenes(lo_model_ref, &scene_count);
  std::vector<SUStringRef> scene_names(scene_count);
  for (size_t count = 0; count < scene_count; ++count) {
    SUSetInvalid(scene_names[count]);
    SUStringCreate(&scene_names[count]);
  }
  LOSketchUpModelGetAvailableScenes(lo_model_ref, scene_count, &scene_names[0],
                                    &scene_count);

  // The first name from available scenes is the default scene, we want to skip
  // that one.
  for (size_t count = 1; count < scene_count; ++count) {
    // The template document contains a title page and one inside page on creation.
    LOPageRef page_ref = SU_INVALID;
    if (count <= 2)
      LODocumentGetPageAtIndex(document_ref, count-1, &page_ref);
    else
      LODocumentAddPage(document_ref, &page_ref);

    size_t scene_name_length = 0;
    SUStringGetUTF8Length(scene_names[count], &scene_name_length);
    char *scene_name_chars = new char[scene_name_length + 1];
    SUStringGetUTF8(scene_names[count], scene_name_length + 1, scene_name_chars,
                    &scene_name_length);
    LOPageSetName(page_ref, scene_name_chars);

    if (count > 1) {
      SUSetInvalid(lo_model_ref);
      SUSetInvalid(entity_ref);
      // Different bounds for the internal pages
      bounds.upper_left.y = 1.;
      bounds.lower_right.x = 10.5;
      bounds.lower_right.y = 7.5;
      result = LOSketchUpModelCreate(&lo_model_ref, skp_file_path, &bounds);
      if (SU_ERROR_NONE != result) {
        return -1;
      }
    }

    entity_ref = LOSketchUpModelToEntity(lo_model_ref);
    LOSketchUpModelSetCurrentScene(lo_model_ref, count);

    result = LODocumentAddEntity(document_ref, entity_ref, layer_ref, page_ref);
    if (SU_ERROR_NONE != result) {
      return -1;
    }
  }

  std::string full_path = std::string(skp_file_path);
  size_t path_end = full_path.find_last_of("/\\");
  if (path_end != std::string::npos) {
    std::string filename = full_path.substr(path_end + 1,
                                            strlen(skp_file_path) - path_end - 5);
    std::string path = full_path.substr(0, path_end);
    path.append("/");
    path.append(filename);
    path.append(".layout");
    result = LODocumentSaveToFile(document_ref, path.c_str(),
                                  LODocumentVersion_Current);

    if (result != SU_ERROR_NONE)
      std::cout << "Unable to save LayOut file!" << std::endl;
  }

  for (size_t count = 0; count < scene_count; ++count)
    SUStringRelease(&scene_names[count]);
  LODocumentRelease(&document_ref);

  LOTerminate();
}
