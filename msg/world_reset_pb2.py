# Generated by the protocol buffer compiler.  DO NOT EDIT!

from google.protobuf import descriptor
from google.protobuf import message
from google.protobuf import reflection
from google.protobuf import descriptor_pb2
# @@protoc_insertion_point(imports)


import header_pb2

DESCRIPTOR = descriptor.FileDescriptor(
  name='world_reset.proto',
  package='gazebo.msgs',
  serialized_pb='\n\x11world_reset.proto\x12\x0bgazebo.msgs\x1a\x0cheader.proto\"T\n\nWorldReset\x12\x11\n\x03\x61ll\x18\x01 \x01(\x08:\x04true\x12\x18\n\ttime_only\x18\x02 \x01(\x08:\x05\x66\x61lse\x12\x19\n\nmodel_only\x18\x03 \x01(\x08:\x05\x66\x61lse')




_WORLDRESET = descriptor.Descriptor(
  name='WorldReset',
  full_name='gazebo.msgs.WorldReset',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    descriptor.FieldDescriptor(
      name='all', full_name='gazebo.msgs.WorldReset.all', index=0,
      number=1, type=8, cpp_type=7, label=1,
      has_default_value=True, default_value=True,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    descriptor.FieldDescriptor(
      name='time_only', full_name='gazebo.msgs.WorldReset.time_only', index=1,
      number=2, type=8, cpp_type=7, label=1,
      has_default_value=True, default_value=False,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    descriptor.FieldDescriptor(
      name='model_only', full_name='gazebo.msgs.WorldReset.model_only', index=2,
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
  serialized_start=48,
  serialized_end=132,
)

DESCRIPTOR.message_types_by_name['WorldReset'] = _WORLDRESET

class WorldReset(message.Message):
  __metaclass__ = reflection.GeneratedProtocolMessageType
  DESCRIPTOR = _WORLDRESET
  
  # @@protoc_insertion_point(class_scope:gazebo.msgs.WorldReset)

# @@protoc_insertion_point(module_scope)
