#include <SketchUpAPI/common.h>
#include <SketchUpAPI/geometry.h>
#include <SketchUpAPI/initialize.h>
#include <SketchUpAPI/model/entities.h>
#include <SketchUpAPI/model/geometry_input.h>
#include <SketchUpAPI/model/material.h>
#include <SketchUpAPI/model/model.h>
#include <SketchUpAPI/model/texture.h>

#include "../common/utils.h" // For SU_CALL macro

int main() {
  // Initialize the API
  SUInitialize();

  // Create a model
  SUModelRef model = SU_INVALID;
  if (SUModelCreate(&model) != SU_ERROR_NONE)
    return 1;

  bool success = true;
  SUGeometryInputRef input = SU_INVALID;

  // We don't expect exceptions from SLAPI. This try-catch is just a convenient
  // way to handle error codes that might be returned from SLAPI, turning them
  // into an exception via the SU_CALL macro.
  try {
    // We'll create a textured face using the SUGeometryInput API.
    SU_CALL(SUGeometryInputCreate(&input));

    SUPoint3D vertices[6] = { { 0,   0,  0 },
                              { 50,  0,  0 },
                              { 115, 0,  0 },
                              { 115, 25, 0 },
                              { 75,  25, 0 },
                              { 0,   25, 0 } };
    SU_CALL(SUGeometryInputSetVertices(input, 6, vertices));

    // Add the first face
    SULoopInputRef loop = SU_INVALID;
    SU_CALL(SULoopInputCreate(&loop));
    SU_CALL(SULoopInputAddVertexIndex(loop, 0));
    SU_CALL(SULoopInputAddVertexIndex(loop, 1));
    SU_CALL(SULoopInputAddVertexIndex(loop, 4));
    SU_CALL(SULoopInputAddVertexIndex(loop, 5));
    SU_CALL(SULoopInputEdgeSetHidden(loop, 1, true));
    size_t face_index0;
    SU_CALL(SUGeometryInputAddFace(input, &loop, &face_index0));

    // Add the second face
    SUSetInvalid(loop);
    SU_CALL(SULoopInputCreate(&loop));
    SU_CALL(SULoopInputAddVertexIndex(loop, 1));
    SU_CALL(SULoopInputAddVertexIndex(loop, 2));
    SU_CALL(SULoopInputAddVertexIndex(loop, 3));
    SU_CALL(SULoopInputAddVertexIndex(loop, 4));
    SU_CALL(SULoopInputEdgeSetHidden(loop, 3, true));
    size_t face_index1;
    SU_CALL(SUGeometryInputAddFace(input, &loop, &face_index1));

    // Load texture from file
    SUTextureRef tex = SU_INVALID;
    SU_CALL(SUTextureCreateFromFile(&tex, "SU_Logo_Color.png", 1.0, 1.0));
    // Create material with that texture
    SUMaterialRef mat = SU_INVALID;
    SU_CALL(SUMaterialCreate(&mat));
    SU_CALL(SUMaterialSetTexture(mat, tex));
    // Create material input for texture mapping and set it on the faces
    SUMaterialInput mat_input;
    mat_input.material = mat;
    mat_input.num_uv_coords = 4;
    SUPoint2D uv_coords[4] = { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } };
    size_t uv_vertices[4] = { 0, 2, 3, 5 };
    for (size_t i = 0; i < 4; ++i) {
      mat_input.vertex_indices[i] = uv_vertices[i];
      mat_input.uv_coords[i] = uv_coords[i];
    }
    SU_CALL(SUGeometryInputFaceSetFrontMaterial(input, face_index0, &mat_input));
    SU_CALL(SUGeometryInputFaceSetFrontMaterial(input, face_index1, &mat_input));

    // Fill the root entities of the model using this geometry input
    SUEntitiesRef entities = SU_INVALID;
    SUModelGetEntities(model, &entities);
    SU_CALL(SUEntitiesFill(entities, input, false));

    // Save the model
    SU_CALL(SUModelSaveToFile(model, "textured_face.skp"));
  
  } catch(std::exception&) {
    success = false;
  }

  // Clean up
  SUGeometryInputRelease(&input);
  SUModelRelease(&model);
  SUTerminate();

  return success ? 0 : 1;
}
