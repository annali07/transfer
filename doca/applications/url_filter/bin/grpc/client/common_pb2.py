# -*- coding: utf-8 -*-
# Generated by the protocol buffer compiler.  DO NOT EDIT!
# source: common.proto
"""Generated protocol buffer code."""
from google.protobuf.internal import enum_type_wrapper
from google.protobuf import descriptor as _descriptor
from google.protobuf import message as _message
from google.protobuf import reflection as _reflection
from google.protobuf import symbol_database as _symbol_database
# @@protoc_insertion_point(imports)

_sym_db = _symbol_database.Default()




DESCRIPTOR = _descriptor.FileDescriptor(
  name='common.proto',
  package='',
  syntax='proto3',
  serialized_options=None,
  create_key=_descriptor._internal_create_key,
  serialized_pb=b'\n\x0c\x63ommon.proto\"\x1d\n\tLogRecord\x12\x10\n\x08log_line\x18\x01 \x01(\t\"\x10\n\x0eHealthCheckReq\"\x11\n\x0fHealthCheckResp\"\x0c\n\nDestroyReq\"\r\n\x0b\x44\x65stroyResp*\xb4\x01\n\x0c\x65NetworkPort\x12\x10\n\x0ck_DummyValue\x10\x00\x12\x1c\n\x16k_DocaGrpcOrchestrator\x10\xb8\x8e\x03\x12\x11\n\x0bk_UrlFilter\x10\xc2\x8e\x03\x12\x11\n\x0bk_DnsFilter\x10\xc3\x8e\x03\x12\x1e\n\x18k_ApplicationRecognition\x10\xc4\x8e\x03\x12\x0b\n\x05k_Ips\x10\xc5\x8e\x03\x12\x0f\n\tk_DocaDPI\x10\xa0\x96\x03\x12\x10\n\nk_DocaFlow\x10\x88\x9e\x03\x32k\n\x11\x44ocaOrchestration\x12\x30\n\x0bHealthCheck\x12\x0f.HealthCheckReq\x1a\x10.HealthCheckResp\x12$\n\x07\x44\x65stroy\x12\x0b.DestroyReq\x1a\x0c.DestroyRespb\x06proto3'
)

_ENETWORKPORT = _descriptor.EnumDescriptor(
  name='eNetworkPort',
  full_name='eNetworkPort',
  filename=None,
  file=DESCRIPTOR,
  create_key=_descriptor._internal_create_key,
  values=[
    _descriptor.EnumValueDescriptor(
      name='k_DummyValue', index=0, number=0,
      serialized_options=None,
      type=None,
      create_key=_descriptor._internal_create_key),
    _descriptor.EnumValueDescriptor(
      name='k_DocaGrpcOrchestrator', index=1, number=51000,
      serialized_options=None,
      type=None,
      create_key=_descriptor._internal_create_key),
    _descriptor.EnumValueDescriptor(
      name='k_UrlFilter', index=2, number=51010,
      serialized_options=None,
      type=None,
      create_key=_descriptor._internal_create_key),
    _descriptor.EnumValueDescriptor(
      name='k_DnsFilter', index=3, number=51011,
      serialized_options=None,
      type=None,
      create_key=_descriptor._internal_create_key),
    _descriptor.EnumValueDescriptor(
      name='k_ApplicationRecognition', index=4, number=51012,
      serialized_options=None,
      type=None,
      create_key=_descriptor._internal_create_key),
    _descriptor.EnumValueDescriptor(
      name='k_Ips', index=5, number=51013,
      serialized_options=None,
      type=None,
      create_key=_descriptor._internal_create_key),
    _descriptor.EnumValueDescriptor(
      name='k_DocaDPI', index=6, number=52000,
      serialized_options=None,
      type=None,
      create_key=_descriptor._internal_create_key),
    _descriptor.EnumValueDescriptor(
      name='k_DocaFlow', index=7, number=53000,
      serialized_options=None,
      type=None,
      create_key=_descriptor._internal_create_key),
  ],
  containing_type=None,
  serialized_options=None,
  serialized_start=114,
  serialized_end=294,
)
_sym_db.RegisterEnumDescriptor(_ENETWORKPORT)

eNetworkPort = enum_type_wrapper.EnumTypeWrapper(_ENETWORKPORT)
k_DummyValue = 0
k_DocaGrpcOrchestrator = 51000
k_UrlFilter = 51010
k_DnsFilter = 51011
k_ApplicationRecognition = 51012
k_Ips = 51013
k_DocaDPI = 52000
k_DocaFlow = 53000



_LOGRECORD = _descriptor.Descriptor(
  name='LogRecord',
  full_name='LogRecord',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  create_key=_descriptor._internal_create_key,
  fields=[
    _descriptor.FieldDescriptor(
      name='log_line', full_name='LogRecord.log_line', index=0,
      number=1, type=9, cpp_type=9, label=1,
      has_default_value=False, default_value=b"".decode('utf-8'),
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
  serialized_start=16,
  serialized_end=45,
)


_HEALTHCHECKREQ = _descriptor.Descriptor(
  name='HealthCheckReq',
  full_name='HealthCheckReq',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  create_key=_descriptor._internal_create_key,
  fields=[
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
  serialized_start=47,
  serialized_end=63,
)


_HEALTHCHECKRESP = _descriptor.Descriptor(
  name='HealthCheckResp',
  full_name='HealthCheckResp',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  create_key=_descriptor._internal_create_key,
  fields=[
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
  serialized_start=65,
  serialized_end=82,
)


_DESTROYREQ = _descriptor.Descriptor(
  name='DestroyReq',
  full_name='DestroyReq',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  create_key=_descriptor._internal_create_key,
  fields=[
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
  serialized_start=84,
  serialized_end=96,
)


_DESTROYRESP = _descriptor.Descriptor(
  name='DestroyResp',
  full_name='DestroyResp',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  create_key=_descriptor._internal_create_key,
  fields=[
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
  serialized_start=98,
  serialized_end=111,
)

DESCRIPTOR.message_types_by_name['LogRecord'] = _LOGRECORD
DESCRIPTOR.message_types_by_name['HealthCheckReq'] = _HEALTHCHECKREQ
DESCRIPTOR.message_types_by_name['HealthCheckResp'] = _HEALTHCHECKRESP
DESCRIPTOR.message_types_by_name['DestroyReq'] = _DESTROYREQ
DESCRIPTOR.message_types_by_name['DestroyResp'] = _DESTROYRESP
DESCRIPTOR.enum_types_by_name['eNetworkPort'] = _ENETWORKPORT
_sym_db.RegisterFileDescriptor(DESCRIPTOR)

LogRecord = _reflection.GeneratedProtocolMessageType('LogRecord', (_message.Message,), {
  'DESCRIPTOR' : _LOGRECORD,
  '__module__' : 'common_pb2'
  # @@protoc_insertion_point(class_scope:LogRecord)
  })
_sym_db.RegisterMessage(LogRecord)

HealthCheckReq = _reflection.GeneratedProtocolMessageType('HealthCheckReq', (_message.Message,), {
  'DESCRIPTOR' : _HEALTHCHECKREQ,
  '__module__' : 'common_pb2'
  # @@protoc_insertion_point(class_scope:HealthCheckReq)
  })
_sym_db.RegisterMessage(HealthCheckReq)

HealthCheckResp = _reflection.GeneratedProtocolMessageType('HealthCheckResp', (_message.Message,), {
  'DESCRIPTOR' : _HEALTHCHECKRESP,
  '__module__' : 'common_pb2'
  # @@protoc_insertion_point(class_scope:HealthCheckResp)
  })
_sym_db.RegisterMessage(HealthCheckResp)

DestroyReq = _reflection.GeneratedProtocolMessageType('DestroyReq', (_message.Message,), {
  'DESCRIPTOR' : _DESTROYREQ,
  '__module__' : 'common_pb2'
  # @@protoc_insertion_point(class_scope:DestroyReq)
  })
_sym_db.RegisterMessage(DestroyReq)

DestroyResp = _reflection.GeneratedProtocolMessageType('DestroyResp', (_message.Message,), {
  'DESCRIPTOR' : _DESTROYRESP,
  '__module__' : 'common_pb2'
  # @@protoc_insertion_point(class_scope:DestroyResp)
  })
_sym_db.RegisterMessage(DestroyResp)



_DOCAORCHESTRATION = _descriptor.ServiceDescriptor(
  name='DocaOrchestration',
  full_name='DocaOrchestration',
  file=DESCRIPTOR,
  index=0,
  serialized_options=None,
  create_key=_descriptor._internal_create_key,
  serialized_start=296,
  serialized_end=403,
  methods=[
  _descriptor.MethodDescriptor(
    name='HealthCheck',
    full_name='DocaOrchestration.HealthCheck',
    index=0,
    containing_service=None,
    input_type=_HEALTHCHECKREQ,
    output_type=_HEALTHCHECKRESP,
    serialized_options=None,
    create_key=_descriptor._internal_create_key,
  ),
  _descriptor.MethodDescriptor(
    name='Destroy',
    full_name='DocaOrchestration.Destroy',
    index=1,
    containing_service=None,
    input_type=_DESTROYREQ,
    output_type=_DESTROYRESP,
    serialized_options=None,
    create_key=_descriptor._internal_create_key,
  ),
])
_sym_db.RegisterServiceDescriptor(_DOCAORCHESTRATION)

DESCRIPTOR.services_by_name['DocaOrchestration'] = _DOCAORCHESTRATION

# @@protoc_insertion_point(module_scope)
