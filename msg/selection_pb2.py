# Generated by the protocol buffer compiler.  DO NOT EDIT!

from google.protobuf import descriptor
from google.protobuf import message
from google.protobuf import reflection
from google.protobuf import descriptor_pb2
# @@protoc_insertion_point(imports)


import header_pb2

DESCRIPTOR = descriptor.FileDescriptor(
  name='selection.proto',
  package='gazebo.msgs',
  serialized_pb='\n\x0fselection.proto\x12\x0bgazebo.msgs\x1a\x0cheader.proto\">\n\tSelection\x12\n\n\x02id\x18\x01 \x02(\r\x12\x0c\n\x04name\x18\x02 \x02(\t\x12\x17\n\x08selected\x18\x03 \x01(\x08:\x05\x66\x61lse')




_SELECTION = descriptor.Descriptor(
  name='Selection',
  full_name='gazebo.msgs.Selection',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    descriptor.FieldDescriptor(
      name='id', full_name='gazebo.msgs.Selection.id', index=0,
      number=1, type=13, cpp_type=3, label=2,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    descriptor.FieldDescriptor(
      name='name', full_name='gazebo.msgs.Selection.name', index=1,
      number=2, type=9, cpp_type=9, label=2,
      has_default_value=False, default_value=unicode("", "utf-8"),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    descriptor.FieldDescriptor(
      name='selected', full_name='gazebo.msgs.Selection.selected', index=2,
      number=3, type=8, cpp_type=7, label=1,
      has_default_value=True, default_value=False,
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
  serialized_start=46,
  serialized_end=108,
)

DESCRIPTOR.message_types_by_name['Selection'] = _SELECTION

class Selection(message.Message):
  __metaclass__ = reflection.GeneratedProtocolMessageType
  DESCRIPTOR = _SELECTION
  
  # @@protoc_insertion_point(class_scope:gazebo.msgs.Selection)

# @@protoc_insertion_point(module_scope)
