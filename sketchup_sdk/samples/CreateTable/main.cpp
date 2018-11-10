// Copyright 2016 Trimble Navigation Limited. All Rights Reserved.
// This program creates a LayOut file containing a door schedule.

#include <LayOutAPI/common.h>
#include <LayOutAPI/initialize.h>
#include <LayOutAPI/model/document.h>
#include <LayOutAPI/model/formattedtext.h>
#include <LayOutAPI/model/pageinfo.h>
#include <LayOutAPI/model/rectangle.h>
#include <LayOutAPI/model/style.h>
#include <LayOutAPI/model/table.h>
#include <iostream>
#include <string>
#include <cstdlib>

using namespace std;

int main(int argc, const char * argv[]) {
  LOInitialize();
  
  LOTableRef table_ref = SU_INVALID;
  LODocumentRef doc_ref = SU_INVALID;
  LOStyleRef style_ref = SU_INVALID;
  LOEntityRef entity_ref = SU_INVALID;
  LOFormattedTextRef cell_text_ref = SU_INVALID;
  LOPageInfoRef page_info = SU_INVALID;
  
  SUResult result = LODocumentCreateEmpty(&doc_ref);
  
  LODocumentGetPageInfo(doc_ref, &page_info);
  LOPageInfoSetWidth(page_info, 15.40);
  
  LOAxisAlignedRect2D bounds = {{0.2, 0.2}, {10.8, 5.0}};
  LOTableCreate(&table_ref, &bounds, 10, 16);
  
  LOStyleCreate(&style_ref);
  
  // Merge and Modify the "DOOR SCHEDULE" cell's text and fill color
  LOTableMergeCells(table_ref, 0, 0, 0, 15);
  LOTableCreateCellTextCopy(table_ref, 0, 0, &cell_text_ref);
  LOEntityRef cell_entity_ref = LOFormattedTextToEntity(cell_text_ref);
  SUColor new_fill_color = {0xe7, 0X2b, 0x2d, 0xff};
  LOStyleSetSolidFillColor(style_ref, new_fill_color);
  LOStyleSetTextAlignment(style_ref, LOTextAlignment_Center);
  LOStyleSetTextBold(style_ref, true);
  LOFormattedTextSetPlainText(cell_text_ref, "DOOR SCHEDULE");
  LOEntitySetStyle(cell_entity_ref, style_ref);
  LOTableSetCellText(table_ref, 0, 0, cell_text_ref);
  
  // Merge and Modify the "Door #" cell's text and fill color
  LOTableMergeCells(table_ref, 1, 0, 3, 0);
  new_fill_color = {0xd4, 0Xd5, 0xd1, 0xff};
  LOStyleSetSolidFillColor(style_ref, new_fill_color);
  LOFormattedTextSetPlainText(cell_text_ref, "Door #");
  LOEntitySetStyle(cell_entity_ref, style_ref);
  LOTableSetCellText(table_ref, 1, 0, cell_text_ref);
  
  // Merge and Modify the "DOORS" cell's text and fill color
  LOTableMergeCells(table_ref, 1, 1, 1, 6);
  LOFormattedTextSetPlainText(cell_text_ref, "DOORS");
  LOEntitySetStyle(cell_entity_ref, style_ref);
  LOTableSetCellText(table_ref, 1, 1, cell_text_ref);
  
  // Merge and Modify the "FRAMES" cell's text and fill color
  LOTableMergeCells(table_ref, 1, 7, 1, 12);
  LOFormattedTextSetPlainText(cell_text_ref, "FRAMES");
  LOEntitySetStyle(cell_entity_ref, style_ref);
  LOTableSetCellText(table_ref, 1, 7, cell_text_ref);
  
  // Merge and Modify the "TYPE" cells' text and fill color
  LOTableMergeCells(table_ref, 2, 1, 3, 1);
  LOTableMergeCells(table_ref, 2, 7, 3, 7);
  LOFormattedTextSetPlainText(cell_text_ref, "TYPE");
  LOEntitySetStyle(cell_entity_ref, style_ref);
  LOTableSetCellText(table_ref, 2, 1, cell_text_ref);
  LOTableSetCellText(table_ref, 2, 7, cell_text_ref);
  
  // Merge and Modify the "MAT." cells' text and fill color
  LOTableMergeCells(table_ref, 2, 2, 3, 2);
  LOTableMergeCells(table_ref, 2, 8, 3, 8);
  LOFormattedTextSetPlainText(cell_text_ref, "MAT.");
  LOEntitySetStyle(cell_entity_ref, style_ref);
  LOTableSetCellText(table_ref, 2, 2, cell_text_ref);
  LOTableSetCellText(table_ref, 2, 8, cell_text_ref);
  
  // Merge and Modify the "FIN." cells' text and fill color
  LOTableMergeCells(table_ref, 2, 3, 3, 3);
  LOTableMergeCells(table_ref, 2, 9, 3, 9);
  LOFormattedTextSetPlainText(cell_text_ref, "FIN.");
  LOEntitySetStyle(cell_entity_ref, style_ref);
  LOTableSetCellText(table_ref, 2, 3, cell_text_ref);
  LOTableSetCellText(table_ref, 2, 9, cell_text_ref);
  
  // Merge and Modify the "SIZE" cell's text and fill color
  LOTableMergeCells(table_ref, 2, 4, 2, 6);
  LOFormattedTextSetPlainText(cell_text_ref, "SIZE");
  LOEntitySetStyle(cell_entity_ref, style_ref);
  LOTableSetCellText(table_ref, 2, 4, cell_text_ref);
  
  // Modify the "WIDTH" cell's text and fill color
  LOTableCreateCellTextCopy(table_ref, 3, 4, &cell_text_ref);
  LOEntitySetStyle(cell_entity_ref, style_ref);
  LOTableSetCellText(table_ref, 3, 4, cell_text_ref);
  
  // Modify the "HEIGHT" cell's text and fill color
  LOFormattedTextSetPlainText(cell_text_ref, "HEIGHT");
  LOEntitySetStyle(cell_entity_ref, style_ref);
  LOTableSetCellText(table_ref, 3, 5, cell_text_ref);
  
  // Modify the "THICK" cell's text and fill color
  LOFormattedTextSetPlainText(cell_text_ref, "THICK");
  LOEntitySetStyle(cell_entity_ref, style_ref);
  LOTableSetCellText(table_ref, 3, 6, cell_text_ref);
  
  // Merge and Modify the "DETAILS" cell's text and fill color
  LOTableMergeCells(table_ref, 2, 10, 2, 12);
  LOFormattedTextSetPlainText(cell_text_ref, "DETAILS");
  LOEntitySetStyle(cell_entity_ref, style_ref);
  LOTableSetCellText(table_ref, 2, 10, cell_text_ref);
  
  // Modify the "JAMB" cell's text and fill color
  LOFormattedTextSetPlainText(cell_text_ref, "JAMB");
  LOEntitySetStyle(cell_entity_ref, style_ref);
  LOTableSetCellText(table_ref, 3, 10, cell_text_ref);
  
  // Modify the "HEAD" cell's text and fill color
  LOFormattedTextSetPlainText(cell_text_ref, "HEAD");
  LOEntitySetStyle(cell_entity_ref, style_ref);
  LOTableSetCellText(table_ref, 3, 11, cell_text_ref);
  
  // Modify the "SILL" cell's text and fill color
  LOFormattedTextSetPlainText(cell_text_ref, "SILL");
  LOEntitySetStyle(cell_entity_ref, style_ref);
  LOTableSetCellText(table_ref, 3, 12, cell_text_ref);
  
  // Merge and Modify the "Fire Rating" cell's text and fill color
  LOTableMergeCells(table_ref, 1, 13, 3, 13);
  LOFormattedTextSetPlainText(cell_text_ref, "Fire Rating");
  LOEntitySetStyle(cell_entity_ref, style_ref);
  LOTableSetCellText(table_ref, 3, 13, cell_text_ref);
  
  // Merge and Modify the "Hdw. Group" cell's text and fill color
  LOTableMergeCells(table_ref, 1, 14, 3, 14);
  LOFormattedTextSetPlainText(cell_text_ref, "Hdw. Group");
  LOEntitySetStyle(cell_entity_ref, style_ref);
  LOTableSetCellText(table_ref, 3, 14, cell_text_ref);
  
  // Merge and Modify the "Remarks" cell's text and fill color
  LOTableMergeCells(table_ref, 1, 15, 3, 15);
  LOFormattedTextSetPlainText(cell_text_ref, "Remarks");
  LOEntitySetStyle(cell_entity_ref, style_ref);
  LOTableSetCellText(table_ref, 3, 15, cell_text_ref);
  
  // Make some tweaks to row and column widths and heights
  // rows
  double height = 0.0;
  LOTableGetRowHeight(table_ref, 0, &height);
  double new_height = height - 0.2;
  const size_t header_rows = 4;
  for(int i = 0; i < header_rows; i++) {
    LOTableSetRowHeight(table_ref, i, new_height);
  }
  
  // columns
  double width = 0.0;
  LOTableGetColumnWidth(table_ref, 4, &width);
  double new_width = width + 0.2;
  
  const size_t size_begin = 4;
  const size_t size_end = 7;
  const size_t details_begin = 10;
  const size_t details_end = 13;
  
  for(int i = size_begin; i < details_end; i++) {
    if(i == size_end) // skip columns 7 thru 9 (TYPE thru MAT.), don't need adjusting
      i = details_begin;
    LOTableSetColumnWidth(table_ref, i, new_width);
  }
  
  // Last few columns need more space
  new_width = new_width + 0.175;
  for(int i = 13; i < 16; i++)
    LOTableSetColumnWidth(table_ref, i, new_width);
  
  // Remarks column should be extra spacious.
  LOTableSetColumnWidth(table_ref, 15, new_width + 2.0);
  
  // Set Edge Styles
  LOTableGetRowEdgeStyle(table_ref, 1, style_ref);
  double stroke_width = 0.0;
  LOStyleGetStrokeWidth(style_ref, &stroke_width);
  double new_stroke_width = stroke_width + 1.0;
  LOStyleSetStrokeWidth(style_ref, new_stroke_width);
  result = LOTableSetRowEdgeStyle(table_ref, 3, style_ref);
  LOTableSetRowEdgeStyle(table_ref, 0, style_ref);
  LOTableSetColumnEdgeStyle(table_ref, 0, style_ref);
  LOTableSetColumnEdgeStyle(table_ref, 6, style_ref);
  LOTableSetColumnEdgeStyle(table_ref, 12, style_ref);
  LOTableSetColumnEdgeStyle(table_ref, 13, style_ref);
  LOTableSetColumnEdgeStyle(table_ref, 14, style_ref);
  
  // Put the table in a document so we can open it in LayOut
  entity_ref = LOTableToEntity(table_ref);
  LODocumentAddEntityUsingIndexes(doc_ref, entity_ref, 0, 0);
  
  result = LODocumentSaveToFile(doc_ref,
                                "./GenerateTable.layout",
                                LODocumentVersion_Current);
  
  // Release allocated memory
  LODocumentRelease(&doc_ref);
  LOTableRelease(&table_ref);
  LOStyleRelease(&style_ref);
  LOFormattedTextRelease(&cell_text_ref);
  
  LOTerminate();
  
  if (result != SU_ERROR_NONE) {
    std::cout << "Unable to save document!";
    return 1;
  }
  
  return 0;
}