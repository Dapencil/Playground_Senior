# -*- coding: utf-8 -*-
# Generated by the protocol buffer compiler.  DO NOT EDIT!
# source: weights.proto

from google.protobuf import descriptor as _descriptor
from google.protobuf import message as _message
from google.protobuf import reflection as _reflection
from google.protobuf import symbol_database as _symbol_database
# @@protoc_insertion_point(imports)

_sym_db = _symbol_database.Default()




DESCRIPTOR = _descriptor.FileDescriptor(
  name='weights.proto',
  package='',
  syntax='proto3',
  serialized_options=None,
  create_key=_descriptor._internal_create_key,
  serialized_pb=b'\n\rweights.proto\"\x19\n\x07Weights\x12\x0e\n\x06values\x18\x01 \x03(\x01\x62\x06proto3'
)




_WEIGHTS = _descriptor.Descriptor(
  name='Weights',
  full_name='Weights',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  create_key=_descriptor._internal_create_key,
  fields=[
    _descriptor.FieldDescriptor(
      name='values', full_name='Weights.values', index=0,
      number=1, type=1, cpp_type=5, label=3,
      has_default_value=False, default_value=[],
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  serialized_options=None,
  is_extendable=False,
  syntax='proto3',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=17,
  serialized_end=42,
)

DESCRIPTOR.message_types_by_name['Weights'] = _WEIGHTS
_sym_db.RegisterFileDescriptor(DESCRIPTOR)

Weights = _reflection.GeneratedProtocolMessageType('Weights', (_message.Message,), {
  'DESCRIPTOR' : _WEIGHTS,
  '__module__' : 'weights_pb2'
  # @@protoc_insertion_point(class_scope:Weights)
  })
_sym_db.RegisterMessage(Weights)


# @@protoc_insertion_point(module_scope)