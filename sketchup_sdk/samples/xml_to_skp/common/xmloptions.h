// Copyright 2013 Trimble Navigation Limited. All Rights Reserved.

#ifndef XMLTOSKP_COMMON_XMLOPTIONS_H
#define XMLTOSKP_COMMON_XMLOPTIONS_H

// XML importer plugin options
class CXmlOptions {
 public:
  CXmlOptions() {
   merge_coplanar_faces_ = true;
  }

  inline bool merge_coplanar_faces() const
      { return merge_coplanar_faces_; }
  inline void set_merge_coplanar_faces(bool value)
      { merge_coplanar_faces_ = value; }

 private:
  bool merge_coplanar_faces_;
};

#endif // XMLTOSKP_COMMON_XMLOPTIONS_H
