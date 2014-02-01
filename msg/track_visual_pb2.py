# Generated by the protocol buffer compiler.  DO NOT EDIT!

from google.protobuf import descriptor
from google.protobuf import message
from google.protobuf import reflection
from google.protobuf import descriptor_pb2
# @@protoc_insertion_point(imports)



DESCRIPTOR = descriptor.FileDescriptor(
  name='track_visual.proto',
  package='gazebo.msgs',
  serialized_pb='\n\x12track_visual.proto\x12\x0bgazebo.msgs\"h\n\x0bTrackVisual\x12\x0c\n\x04name\x18\x01 \x02(\t\x12\n\n\x02id\x18\x02 \x01(\r\x12\x1b\n\x13inherit_orientation\x18\x03 \x01(\x08\x12\x10\n\x08min_dist\x18\x04 \x01(\x01\x12\x10\n\x08max_dist\x18\x05 \x01(\x01')




_TRACKVISUAL = descriptor.Descriptor(
  name='TrackVisual',
  full_name='gazebo.msgs.TrackVisual',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    descriptor.FieldDescriptor(
      name='name', full_name='gazebo.msgs.TrackVisual.name', index=0,
      number=1, type=9, cpp_type=9, label=2,
      has_default_value=False, default_value=unicode("", "utf-8"),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    descriptor.FieldDescriptor(
      name='id', full_name='gazebo.msgs.TrackVisual.id', index=1,
      number=2, type=13, cpp_type=3, label=1,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    descriptor.FieldDescriptor(
      name='inherit_orientation', full_name='gazebo.msgs.TrackVisual.inherit_orientation', index=2,
      number=3, type=8, cpp_type=7, label=1,
      has_default_value=False, default_value=False,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    descriptor.FieldDescriptor(
      name='min_dist', full_name='gazebo.msgs.TrackVisual.min_dist', index=3,
      number=4, type=1, cpp_type=5, label=1,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    descriptor.FieldDescriptor(
      name='max_dist', full_name='gazebo.msgs.TrackVisual.max_dist', index=4,
      number=5, type=1, cpp_type=5, label=1,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  options=None,
  is_extendable=False,
  extension_ranges=[],
  serialized_start=35,
  serialized_end=139,
)

DESCRIPTOR.message_types_by_name['TrackVisual'] = _TRACKVISUAL

class TrackVisual(message.Message):
  __metaclass__ = reflection.GeneratedProtocolMessageType
  DESCRIPTOR = _TRACKVISUAL
  
  # @@protoc_insertion_point(class_scope:gazebo.msgs.TrackVisual)

# @@protoc_insertion_point(module_scope)
