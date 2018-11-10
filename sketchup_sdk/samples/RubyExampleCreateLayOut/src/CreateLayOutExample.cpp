
#include "RubyUtils/RubyUtils.h"
#include <LayOutAPI/common.h>
#include <LayOutAPI/initialize.h>
#include <LayOutAPI/model/connectionpoint.h>
#include <LayOutAPI/model/document.h>
#include <LayOutAPI/model/label.h>
#include <LayOutAPI/model/layer.h>
#include <LayOutAPI/model/lineardimension.h>
#include <LayOutAPI/model/sketchupmodel.h>
#include <LayOutAPI/model/style.h>
#include <SketchUpAPI/geometry.h>
#include <string>

static LODocumentRef lo_document_ref;
static LOLayerRef lo_label_layer_ref;
static LOLayerRef lo_dimension_layer_ref;
static LOSketchUpModelRef lo_skp_ref;
static LOStyleRef lo_dimension_style_ref;
static std::string file_path;

VALUE create_layout_doc(VALUE self, VALUE skp_path) {
  LOInitialize();
  SUResult result;
  LOEntityRef entity_ref = SU_INVALID;
  SUSetInvalid(lo_document_ref);
  SUSetInvalid(lo_label_layer_ref);
  SUSetInvalid(lo_dimension_layer_ref);
  SUSetInvalid(lo_skp_ref);
  SUSetInvalid(lo_dimension_style_ref);

  LOStyleCreate(&lo_dimension_style_ref);
  // Make the dimension units be horizontal and below the dimension line.
  LOStyleSetDimensionRotationAlignment(lo_dimension_style_ref,
      LODimensionRotationAlignment_Horizontal);
  LOStyleSetDimensionVerticalAlignment(lo_dimension_style_ref,
      LODimensionVerticalAlignment_Below);
  // Set the units to architectural inches with a 1/16" precision.
  LOStyleSetDimensionUnits(lo_dimension_style_ref,
      LODimensionUnits_ArchitecturalInches, 1./16.);

  std::string path = GetRubyString(skp_path);
  size_t path_end = path.find_last_of("\\/");
  if (path_end != std::string::npos)
    file_path = path.substr(0, path_end + 1);
  // Load the SketchUp model.
  LOAxisAlignedRect2D bounds = {{1., 1.}, {10., 7.5}};
  result = LOSketchUpModelCreate(&lo_skp_ref, path.c_str(), &bounds);
  if (SU_ERROR_NONE != result) {
    LOTerminate();
    return GetRubyInterface("Loading SketchUp file failed!");
  }

  // Convert the model ref to an entity ref for adding to the LayOut document.
  entity_ref = LOSketchUpModelToEntity(lo_skp_ref);

  // Create a new LayOut document.
  result = LODocumentCreateEmpty(&lo_document_ref);
  if (SU_ERROR_NONE != result) {
    LOTerminate();
    return GetRubyInterface("Creating LO Document failed!");
  }

  // Add the SketchUp model to the document on the default layer on the first page.
  result = LODocumentAddEntityUsingIndexes(lo_document_ref, entity_ref, 0, 0);
  if (SU_ERROR_NONE != result) {
    LOTerminate();
    return GetRubyInterface("Adding SketchUpModel to LO Document failed!");
  }

  // Release our references and return success.
  SUSetInvalid(entity_ref);
  return Qnil;
}

VALUE add_linear_dimension(VALUE self,
                           VALUE start_point,
                           VALUE end_point,
                           VALUE start_point_pid,
                           VALUE end_point_pid,
                           VALUE text_offset) {
  LOLinearDimensionRef lo_dimension_ref = SU_INVALID;
  LOEntityRef lo_entity_ref = SU_INVALID;
  
  std::string start_pid_str = GetRubyString(start_point_pid);
  std::string end_pid_str = GetRubyString(end_point_pid);

  long start_point_len = RARRAY_LEN(start_point);
  long end_point_len = RARRAY_LEN(end_point);
  long text_offset_len = RARRAY_LEN(text_offset);
  if (start_point_len != 3 || end_point_len != 3 || text_offset_len != 3)
    return Qfalse;

  // Get our points and offset vector from ruby.
  VALUE *start_arr = RARRAY_PTR(start_point);
  LOPoint3D start_3d = { GetRubyDouble(start_arr[0]),
                         GetRubyDouble(start_arr[1]),
                         GetRubyDouble(start_arr[2]) };

  VALUE *end_arr = RARRAY_PTR(end_point);
  LOPoint3D end_3d = { GetRubyDouble(end_arr[0]),
                       GetRubyDouble(end_arr[1]),
                       GetRubyDouble(end_arr[2]) };

  VALUE *offset_arr = RARRAY_PTR(text_offset);
  SUVector3D offset_3d = { GetRubyDouble(offset_arr[0]),
                           GetRubyDouble(offset_arr[1]),
                           GetRubyDouble(offset_arr[2]) };

  // The offset vector is relative to the start point, add in the start point
  // location to determine the world space location of the offset position.
  LOPoint3D start_offset_3d = { offset_3d.x + start_3d.x,
                                offset_3d.y + start_3d.y,
                                offset_3d.z + start_3d.z };

  LOPoint2D start_2d, end_2d, start_offset_2d;
  LOSketchUpModelConvertModelPointToPaperPoint(lo_skp_ref, &start_3d, &start_2d);
  LOSketchUpModelConvertModelPointToPaperPoint(lo_skp_ref, &end_3d, &end_2d);
  LOSketchUpModelConvertModelPointToPaperPoint(lo_skp_ref, &start_offset_3d,
                                               &start_offset_2d);

  double start_offset_length = sqrt((start_2d.x - start_offset_2d.x) *
                                    (start_2d.x - start_offset_2d.x) +
                                    (start_2d.y - start_offset_2d.y) *
                                    (start_2d.y - start_offset_2d.y));

  // Determine the direction the dimension lines will extend.
  LOPoint2D start_end = { end_2d.x - start_2d.x, end_2d.y - start_2d.y };
  LOPoint2D start_end_perp = { start_end.y, -start_end.x };
  LOPoint2D start_offset = { start_offset_2d.x - start_2d.x,
                             start_offset_2d.y - start_2d.y };

  // Dot product between the line perpendicular to the start and end points,
  // and the line from the start point to the start offset. A negative value
  // indicates that we want the dimension length to be negative.
  double dot_product = (start_end_perp.x * start_offset.x) +
                       (start_end_perp.y * start_offset.y);
  if (dot_product < 0.)
    start_offset_length *= -1.;

  SUResult result;
  result = LOLinearDimensionCreate(&lo_dimension_ref, &start_2d, &end_2d,
                                   start_offset_length);

  if (SU_ERROR_NONE != result)
    return Qfalse;

  lo_entity_ref = LOLinearDimensionToEntity(lo_dimension_ref);
  LOEntitySetStyle(lo_entity_ref, lo_dimension_style_ref);

  LOPageRef lo_page_ref = SU_INVALID;
  LODocumentGetPageAtIndex(lo_document_ref, 0, &lo_page_ref);

  // Create the layer that we want to put the dimensions on if we haven't yet.
  if (SUIsInvalid(lo_dimension_layer_ref)) {
    LODocumentAddLayer(lo_document_ref, false, &lo_dimension_layer_ref);
    LOLayerSetName(lo_dimension_layer_ref, "Dimensions");
  }

  LODocumentAddEntity(lo_document_ref, lo_entity_ref, lo_dimension_layer_ref,
                      lo_page_ref);

  // Now that the linear dimension has been added to the document, we can
  // connect it to the SketchUp model entity.
  LOConnectionPointRef start_connection = SU_INVALID;
  if(!start_pid_str.empty()) {
    LOConnectionPointCreateFromPID(&start_connection,
                                   lo_skp_ref,
                                   &start_3d,
                                   start_pid_str.c_str());
  } else {
    // If the pid path isn't found, an empty string is passed in. This will cause
    // the connections to be in "soft disconnect" mode, meaning that connection
    // points will not update with entity they connect.
    LOConnectionPointCreateFromPoint3D(&start_connection, lo_skp_ref, &start_3d);
  }

  LOConnectionPointRef end_connection = SU_INVALID;
  if(!end_pid_str.empty()) {
    LOConnectionPointCreateFromPID(&end_connection,
                                   lo_skp_ref,
                                   &end_3d,
                                   end_pid_str.c_str());
  } else {
    // If the pid path isn't found, an empty string is passed in. This will cause
    // the connections to be in "soft disconnect" mode, meaning that connection
    // points will not update with entity they connect.
    LOConnectionPointCreateFromPoint3D(&end_connection, lo_skp_ref, &end_3d);
  }


  LOLinearDimensionConnectTo(lo_dimension_ref, start_connection, end_connection);
  LOLinearDimensionRelease(&lo_dimension_ref);
  LOConnectionPointRelease(&start_connection);
  LOConnectionPointRelease(&end_connection);
  return Qnil;
}

VALUE add_label(VALUE self, VALUE text, VALUE anchor,
                VALUE target, VALUE point_pid) {
  LOLabelRef lo_label_ref = SU_INVALID;
  LOEntityRef lo_entity_ref = SU_INVALID;

  // Get our text and points from ruby.
  std::string text_string = GetRubyString(text);
  
  std::string pid_str = GetRubyString(point_pid);
  
  long anchor_len = RARRAY_LEN(anchor);
  long target_len = RARRAY_LEN(target);
  if (anchor_len != 3 || target_len != 3)
    return Qfalse;

  VALUE *anchor_arr = RARRAY_PTR(anchor);
  LOPoint3D anchor_3d = { GetRubyDouble(anchor_arr[0]),
                          GetRubyDouble(anchor_arr[1]),
                          GetRubyDouble(anchor_arr[2]) };

  VALUE *target_arr = RARRAY_PTR(target);
  LOPoint3D target_3d = { GetRubyDouble(target_arr[0]),
                          GetRubyDouble(target_arr[1]),
                          GetRubyDouble(target_arr[2]) };

  LOPoint2D anchor_2d, target_2d;
  LOSketchUpModelConvertModelPointToPaperPoint(lo_skp_ref, &anchor_3d,
                                               &anchor_2d);
  LOSketchUpModelConvertModelPointToPaperPoint(lo_skp_ref, &target_3d,
                                               &target_2d);

  LOFormattedTextAnchorType anchor_type = LOFormattedTextAnchorType_CenterLeft;
  LOLabelLeaderLineType leader_line_type = LOLabelLeaderLineType_TwoSegment;
  SUResult result;
  result = LOLabelCreateAtPoint(&lo_label_ref, &anchor_2d, anchor_type,
                                text_string.c_str(), leader_line_type,
                                &target_2d);

  if (SU_ERROR_NONE != result)
    return Qfalse;
  lo_entity_ref = LOLabelToEntity(lo_label_ref);

  LOPageRef lo_page_ref = SU_INVALID;
  LODocumentGetPageAtIndex(lo_document_ref, 0, &lo_page_ref);

  // Create the layer that we want to put the labels on if we haven't yet.
  if (SUIsInvalid(lo_label_layer_ref)) {
    LODocumentAddLayer(lo_document_ref, false, &lo_label_layer_ref);
    LOLayerSetName(lo_label_layer_ref, "Labels");
  }

  LODocumentAddEntity(lo_document_ref, lo_entity_ref, lo_label_layer_ref,
                      lo_page_ref);

  // Now that the label has been added to the document, we can connect it to
  // the SketchUp model.
  LOConnectionPointRef connection = SU_INVALID;
  if(!pid_str.empty()) {
    LOConnectionPointCreateFromPID(&connection,
                                   lo_skp_ref,
                                   &target_3d,
                                   pid_str.c_str());
  } else {
    // If the pid path isn't found, an empty string is passed in. This will cause
    // the connections to be in "soft disconnect" mode, meaning that connection
    // points will not update with entity they connect.
    LOConnectionPointCreateFromPoint3D(&connection, lo_skp_ref, &target_3d);
  }
  
  LOLabelConnectTo(lo_label_ref, connection);
  LOConnectionPointRelease(&connection);
  LOLabelRelease(&lo_label_ref);
  return Qtrue;
}

VALUE save_layout_doc() {
  // Save the document to disk.
  std::string lo_filepath = file_path + "CreatedFromRuby.layout";
  SUResult result;
  result = LODocumentSaveToFile(lo_document_ref,
                                lo_filepath.c_str(),
                                LODocumentVersion_Current);
  LODocumentRelease(&lo_document_ref);
  LOSketchUpModelRelease(&lo_skp_ref);
  LOStyleRelease(&lo_dimension_style_ref);
  LOTerminate();
  if (SU_ERROR_NONE != result)
    return GetRubyInterface("Saving LO Document failed!");
  return GetRubyInterface("Saving LO Document successful!");
}

// Load this module from Ruby using:
//   require 'CreateLayOutExample'
extern "C"
void Init_CreateLayOutExample() {
  VALUE mCreateLayOutExample = rb_define_module("CreateLayOutExample");
  rb_define_module_function(mCreateLayOutExample, "create_layout_doc",
                            VALUEFUNC(create_layout_doc), 1);
  rb_define_module_function(mCreateLayOutExample, "save_layout_doc",
                            VALUEFUNC(save_layout_doc), 0);
  rb_define_module_function(mCreateLayOutExample, "add_linear_dimension",
                            VALUEFUNC(add_linear_dimension), 5);
  rb_define_module_function(mCreateLayOutExample, "add_label",
                            VALUEFUNC(add_label), 4);
}
