# Copyright 2015, Trimble Navigation Limited

# This software is provided as an example of using the LayOut C interface through
# the SketchUp Ruby interface.

# Permission to use, copy, modify, and distribute this software for
# any purpose and without fee is hereby granted, provided that the above
# copyright notice appear in all copies.

# THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
#-----------------------------------------------------------------------------
module CreateLayOutExample

def self.run
  model = Sketchup.active_model
  if model.nil?
    UI.messagebox("There must be an active model to run this example.")
    return
  end

  path = model.path
  if path.empty?
    UI.messagebox('The model must be saved prior to running this example.')
    return
  end

  require_relative 'CreateLayOutExample'
  error = CreateLayOutExample.create_layout_doc(path)
  if error.nil?
    iterate_entities(model.entities, Geom::Transformation.new([0,0,0]))

    CreateLayOutExample.save_layout_doc
  else
    UI.messagebox(error)
  end
end

def self.iterate_entities(entities, transformation)
  entities.each { |entity|
    if entity.is_a?(Sketchup::DimensionLinear)
      start_point = entity.start[1].transform(transformation)
      end_point = entity.end[1].transform(transformation)
      start_point_pid = get_point_pid_path(start_point)
      end_point_pid = get_point_pid_path(end_point)
      vec = entity.offset_vector
      
      CreateLayOutExample.add_linear_dimension(start_point.to_a,
                                               end_point.to_a,
                                               start_point_pid.to_s,
                                               end_point_pid.to_s,
                                               vec.to_a)
    elsif entity.is_a?(Sketchup::Text)
      text = entity.text
      point = entity.point.transform(transformation)
      point_pid = get_point_pid_path(point)
      vec = entity.point.transform(entity.vector)
      vec.transform!(transformation)

      CreateLayOutExample.add_label(text, vec.to_a, point.to_a, point_pid.to_s)
    elsif entity.is_a?(Sketchup::Group)
      iterate_entities(entity.entities,
                       transformation * entity.transformation,
                       persistent_path)
    elsif entity.is_a?(Sketchup::ComponentInstance)
      iterate_entities(entity.definition.entities,
                       transformation * entity.transformation)
    end
  }
end

# This method gets the correct PID if the point_3d passed in is in view.
# If not, this method will return an empty string.
def self.get_point_pid_path(point_3d)
  # Get the input point at the specified 3D point
  view = Sketchup.active_model.active_view
  point_2d = view.screen_coords(point_3d)
  input_point = view.inputpoint(point_2d.x, point_2d.y)

  # If we got the wrong input point (i.e. when the point is hidden behind other
  # entities, return an empty string for the PID.
  if input_point.position != point_3d
    return ""
  end

  # If the depth is 0, we won't be able to get the position, so, try getting the
  # picked entity instead. It will either be a vertex, face, or an edge
  if input_point.depth == 0
    vertex = input_point.vertex
    if vertex.valid?
      return vertex.persistent_id
    end
    edge = input_point.edge
    if edge.valid?
      return edge.persistent_id
    end
    face = input_point.face
    if face.valid? 
      return face.persistent_id
    end
  end

  path = input_point.instance_path
  pid = path.persistent_id_path
end

UI.menu('Extensions').add_item('Create Layout Example') { run() }

end # module