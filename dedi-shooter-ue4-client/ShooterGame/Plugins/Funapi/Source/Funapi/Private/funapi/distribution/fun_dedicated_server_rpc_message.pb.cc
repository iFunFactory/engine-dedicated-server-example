// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: funapi/distribution/fun_dedicated_server_rpc_message.proto

#define INTERNAL_SUPPRESS_PROTOBUF_FIELD_DEPRECATION
#include "funapi/distribution/fun_dedicated_server_rpc_message.pb.h"

#include <algorithm>

#include <google/protobuf/stubs/common.h>
#include <google/protobuf/stubs/once.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/wire_format_lite_inl.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/reflection_ops.h>
#include <google/protobuf/wire_format.h>
// @@protoc_insertion_point(includes)

namespace {

const ::google::protobuf::Descriptor* FunDedicatedServerRpcMessage_descriptor_ = NULL;
const ::google::protobuf::internal::GeneratedMessageReflection*
  FunDedicatedServerRpcMessage_reflection_ = NULL;
const ::google::protobuf::Descriptor* FunDedicatedServerRpcSystemMessage_descriptor_ = NULL;
const ::google::protobuf::internal::GeneratedMessageReflection*
  FunDedicatedServerRpcSystemMessage_reflection_ = NULL;

}  // namespace


void protobuf_AssignDesc_funapi_2fdistribution_2ffun_5fdedicated_5fserver_5frpc_5fmessage_2eproto() {
  protobuf_AddDesc_funapi_2fdistribution_2ffun_5fdedicated_5fserver_5frpc_5fmessage_2eproto();
  const ::google::protobuf::FileDescriptor* file =
    ::google::protobuf::DescriptorPool::generated_pool()->FindFileByName(
      "funapi/distribution/fun_dedicated_server_rpc_message.proto");
  GOOGLE_CHECK(file != NULL);
  FunDedicatedServerRpcMessage_descriptor_ = file->message_type(0);
  static const int FunDedicatedServerRpcMessage_offsets_[3] = {
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(FunDedicatedServerRpcMessage, xid_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(FunDedicatedServerRpcMessage, type_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(FunDedicatedServerRpcMessage, is_request_),
  };
  FunDedicatedServerRpcMessage_reflection_ =
    new ::google::protobuf::internal::GeneratedMessageReflection(
      FunDedicatedServerRpcMessage_descriptor_,
      FunDedicatedServerRpcMessage::default_instance_,
      FunDedicatedServerRpcMessage_offsets_,
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(FunDedicatedServerRpcMessage, _has_bits_[0]),
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(FunDedicatedServerRpcMessage, _unknown_fields_),
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(FunDedicatedServerRpcMessage, _extensions_),
      ::google::protobuf::DescriptorPool::generated_pool(),
      ::google::protobuf::MessageFactory::generated_factory(),
      sizeof(FunDedicatedServerRpcMessage));
  FunDedicatedServerRpcSystemMessage_descriptor_ = file->message_type(1);
  static const int FunDedicatedServerRpcSystemMessage_offsets_[1] = {
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(FunDedicatedServerRpcSystemMessage, data_),
  };
  FunDedicatedServerRpcSystemMessage_reflection_ =
    new ::google::protobuf::internal::GeneratedMessageReflection(
      FunDedicatedServerRpcSystemMessage_descriptor_,
      FunDedicatedServerRpcSystemMessage::default_instance_,
      FunDedicatedServerRpcSystemMessage_offsets_,
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(FunDedicatedServerRpcSystemMessage, _has_bits_[0]),
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(FunDedicatedServerRpcSystemMessage, _unknown_fields_),
      -1,
      ::google::protobuf::DescriptorPool::generated_pool(),
      ::google::protobuf::MessageFactory::generated_factory(),
      sizeof(FunDedicatedServerRpcSystemMessage));
}

namespace {

GOOGLE_PROTOBUF_DECLARE_ONCE(protobuf_AssignDescriptors_once_);
inline void protobuf_AssignDescriptorsOnce() {
  ::google::protobuf::GoogleOnceInit(&protobuf_AssignDescriptors_once_,
                 &protobuf_AssignDesc_funapi_2fdistribution_2ffun_5fdedicated_5fserver_5frpc_5fmessage_2eproto);
}

void protobuf_RegisterTypes(const ::fun::string&) {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedMessage(
    FunDedicatedServerRpcMessage_descriptor_, &FunDedicatedServerRpcMessage::default_instance());
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedMessage(
    FunDedicatedServerRpcSystemMessage_descriptor_, &FunDedicatedServerRpcSystemMessage::default_instance());
}

}  // namespace

void protobuf_ShutdownFile_funapi_2fdistribution_2ffun_5fdedicated_5fserver_5frpc_5fmessage_2eproto() {
  delete FunDedicatedServerRpcMessage::default_instance_;
  delete FunDedicatedServerRpcMessage_reflection_;
  delete FunDedicatedServerRpcSystemMessage::default_instance_;
  delete FunDedicatedServerRpcSystemMessage_reflection_;
}

void protobuf_AddDesc_funapi_2fdistribution_2ffun_5fdedicated_5fserver_5frpc_5fmessage_2eproto() {
  static bool already_here = false;
  if (already_here) return;
  already_here = true;
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  ::google::protobuf::DescriptorPool::InternalAddGeneratedFile(
    "\n:funapi/distribution/fun_dedicated_serv"
    "er_rpc_message.proto\"W\n\034FunDedicatedServ"
    "erRpcMessage\022\013\n\003xid\030\001 \002(\014\022\014\n\004type\030\002 \002(\t\022"
    "\022\n\nis_request\030\003 \002(\010*\010\010\010\020\200\200\200\200\002\"2\n\"FunDedi"
    "catedServerRpcSystemMessage\022\014\n\004data\030\001 \001("
    "\t:V\n\nds_rpc_sys\022\035.FunDedicatedServerRpcM"
    "essage\030\010 \001(\0132#.FunDedicatedServerRpcSyst"
    "emMessage", 289);
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedFile(
    "funapi/distribution/fun_dedicated_server_rpc_message.proto", &protobuf_RegisterTypes);
  FunDedicatedServerRpcMessage::default_instance_ = new FunDedicatedServerRpcMessage();
  FunDedicatedServerRpcSystemMessage::default_instance_ = new FunDedicatedServerRpcSystemMessage();
  ::google::protobuf::internal::ExtensionSet::RegisterMessageExtension(
    &::FunDedicatedServerRpcMessage::default_instance(),
    8, 11, false, false,
    &::FunDedicatedServerRpcSystemMessage::default_instance());
  FunDedicatedServerRpcMessage::default_instance_->InitAsDefaultInstance();
  FunDedicatedServerRpcSystemMessage::default_instance_->InitAsDefaultInstance();
  ::google::protobuf::internal::OnShutdown(&protobuf_ShutdownFile_funapi_2fdistribution_2ffun_5fdedicated_5fserver_5frpc_5fmessage_2eproto);
}

// Force AddDescriptors() to be called at static initialization time.
struct StaticDescriptorInitializer_funapi_2fdistribution_2ffun_5fdedicated_5fserver_5frpc_5fmessage_2eproto {
  StaticDescriptorInitializer_funapi_2fdistribution_2ffun_5fdedicated_5fserver_5frpc_5fmessage_2eproto() {
    protobuf_AddDesc_funapi_2fdistribution_2ffun_5fdedicated_5fserver_5frpc_5fmessage_2eproto();
  }
} static_descriptor_initializer_funapi_2fdistribution_2ffun_5fdedicated_5fserver_5frpc_5fmessage_2eproto_;

// ===================================================================

#ifndef _MSC_VER
const int FunDedicatedServerRpcMessage::kXidFieldNumber;
const int FunDedicatedServerRpcMessage::kTypeFieldNumber;
const int FunDedicatedServerRpcMessage::kIsRequestFieldNumber;
#endif  // !_MSC_VER

FunDedicatedServerRpcMessage::FunDedicatedServerRpcMessage()
  : ::google::protobuf::Message() {
  SharedCtor();
  // @@protoc_insertion_point(constructor:FunDedicatedServerRpcMessage)
}

void FunDedicatedServerRpcMessage::InitAsDefaultInstance() {
}

FunDedicatedServerRpcMessage::FunDedicatedServerRpcMessage(const FunDedicatedServerRpcMessage& from)
  : ::google::protobuf::Message() {
  SharedCtor();
  MergeFrom(from);
  // @@protoc_insertion_point(copy_constructor:FunDedicatedServerRpcMessage)
}

void FunDedicatedServerRpcMessage::SharedCtor() {
  ::google::protobuf::internal::GetEmptyString();
  _cached_size_ = 0;
  xid_ = const_cast< ::fun::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  type_ = const_cast< ::fun::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  is_request_ = false;
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
}

FunDedicatedServerRpcMessage::~FunDedicatedServerRpcMessage() {
  // @@protoc_insertion_point(destructor:FunDedicatedServerRpcMessage)
  SharedDtor();
}

void FunDedicatedServerRpcMessage::SharedDtor() {
  if (xid_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    delete xid_;
  }
  if (type_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    delete type_;
  }
  if (this != default_instance_) {
  }
}

void FunDedicatedServerRpcMessage::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const ::google::protobuf::Descriptor* FunDedicatedServerRpcMessage::descriptor() {
  protobuf_AssignDescriptorsOnce();
  return FunDedicatedServerRpcMessage_descriptor_;
}

const FunDedicatedServerRpcMessage& FunDedicatedServerRpcMessage::default_instance() {
  if (default_instance_ == NULL) protobuf_AddDesc_funapi_2fdistribution_2ffun_5fdedicated_5fserver_5frpc_5fmessage_2eproto();
  return *default_instance_;
}

FunDedicatedServerRpcMessage* FunDedicatedServerRpcMessage::default_instance_ = NULL;

FunDedicatedServerRpcMessage* FunDedicatedServerRpcMessage::New() const {
  return new FunDedicatedServerRpcMessage;
}

void FunDedicatedServerRpcMessage::Clear() {
  _extensions_.Clear();
  if (_has_bits_[0 / 32] & 7) {
    if (has_xid()) {
      if (xid_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
        xid_->clear();
      }
    }
    if (has_type()) {
      if (type_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
        type_->clear();
      }
    }
    is_request_ = false;
  }
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
  mutable_unknown_fields()->Clear();
}

bool FunDedicatedServerRpcMessage::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!(EXPRESSION)) goto failure
  ::google::protobuf::uint32 tag;
  // @@protoc_insertion_point(parse_start:FunDedicatedServerRpcMessage)
  for (;;) {
    ::std::pair< ::google::protobuf::uint32, bool> p = input->ReadTagWithCutoff(127);
    tag = p.first;
    if (!p.second) goto handle_unusual;
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // required bytes xid = 1;
      case 1: {
        if (tag == 10) {
          DO_(::google::protobuf::internal::WireFormatLite::ReadBytes(
                input, this->mutable_xid()));
        } else {
          goto handle_unusual;
        }
        if (input->ExpectTag(18)) goto parse_type;
        break;
      }

      // required fun::string type = 2;
      case 2: {
        if (tag == 18) {
         parse_type:
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_type()));
          ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
            this->type().data(), this->type().length(),
            ::google::protobuf::internal::WireFormat::PARSE,
            "type");
        } else {
          goto handle_unusual;
        }
        if (input->ExpectTag(24)) goto parse_is_request;
        break;
      }

      // required bool is_request = 3;
      case 3: {
        if (tag == 24) {
         parse_is_request:
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   bool, ::google::protobuf::internal::WireFormatLite::TYPE_BOOL>(
                 input, &is_request_)));
          set_has_is_request();
        } else {
          goto handle_unusual;
        }
        if (input->ExpectAtEnd()) goto success;
        break;
      }

      default: {
      handle_unusual:
        if (tag == 0 ||
            ::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_END_GROUP) {
          goto success;
        }
        if ((64u <= tag)) {
          DO_(_extensions_.ParseField(tag, input, default_instance_,
                                      mutable_unknown_fields()));
          continue;
        }
        DO_(::google::protobuf::internal::WireFormat::SkipField(
              input, tag, mutable_unknown_fields()));
        break;
      }
    }
  }
success:
  // @@protoc_insertion_point(parse_success:FunDedicatedServerRpcMessage)
  return true;
failure:
  // @@protoc_insertion_point(parse_failure:FunDedicatedServerRpcMessage)
  return false;
#undef DO_
}

void FunDedicatedServerRpcMessage::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // @@protoc_insertion_point(serialize_start:FunDedicatedServerRpcMessage)
  // required bytes xid = 1;
  if (has_xid()) {
    ::google::protobuf::internal::WireFormatLite::WriteBytesMaybeAliased(
      1, this->xid(), output);
  }

  // required fun::string type = 2;
  if (has_type()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
      this->type().data(), this->type().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE,
      "type");
    ::google::protobuf::internal::WireFormatLite::WriteStringMaybeAliased(
      2, this->type(), output);
  }

  // required bool is_request = 3;
  if (has_is_request()) {
    ::google::protobuf::internal::WireFormatLite::WriteBool(3, this->is_request(), output);
  }

  // Extension range [8, 536870912)
  _extensions_.SerializeWithCachedSizes(
      8, 536870912, output);

  if (!unknown_fields().empty()) {
    ::google::protobuf::internal::WireFormat::SerializeUnknownFields(
        unknown_fields(), output);
  }
  // @@protoc_insertion_point(serialize_end:FunDedicatedServerRpcMessage)
}

::google::protobuf::uint8* FunDedicatedServerRpcMessage::SerializeWithCachedSizesToArray(
    ::google::protobuf::uint8* target) const {
  // @@protoc_insertion_point(serialize_to_array_start:FunDedicatedServerRpcMessage)
  // required bytes xid = 1;
  if (has_xid()) {
    target =
      ::google::protobuf::internal::WireFormatLite::WriteBytesToArray(
        1, this->xid(), target);
  }

  // required fun::string type = 2;
  if (has_type()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
      this->type().data(), this->type().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE,
      "type");
    target =
      ::google::protobuf::internal::WireFormatLite::WriteStringToArray(
        2, this->type(), target);
  }

  // required bool is_request = 3;
  if (has_is_request()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteBoolToArray(3, this->is_request(), target);
  }

  // Extension range [8, 536870912)
  target = _extensions_.SerializeWithCachedSizesToArray(
      8, 536870912, target);

  if (!unknown_fields().empty()) {
    target = ::google::protobuf::internal::WireFormat::SerializeUnknownFieldsToArray(
        unknown_fields(), target);
  }
  // @@protoc_insertion_point(serialize_to_array_end:FunDedicatedServerRpcMessage)
  return target;
}

int FunDedicatedServerRpcMessage::ByteSize() const {
  int total_size = 0;

  if (_has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    // required bytes xid = 1;
    if (has_xid()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::BytesSize(
          this->xid());
    }

    // required fun::string type = 2;
    if (has_type()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::StringSize(
          this->type());
    }

    // required bool is_request = 3;
    if (has_is_request()) {
      total_size += 1 + 1;
    }

  }
  total_size += _extensions_.ByteSize();

  if (!unknown_fields().empty()) {
    total_size +=
      ::google::protobuf::internal::WireFormat::ComputeUnknownFieldsSize(
        unknown_fields());
  }
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = total_size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
  return total_size;
}

void FunDedicatedServerRpcMessage::MergeFrom(const ::google::protobuf::Message& from) {
  GOOGLE_CHECK_NE(&from, this);
  const FunDedicatedServerRpcMessage* source =
    ::google::protobuf::internal::dynamic_cast_if_available<const FunDedicatedServerRpcMessage*>(
      &from);
  if (source == NULL) {
    ::google::protobuf::internal::ReflectionOps::Merge(from, this);
  } else {
    MergeFrom(*source);
  }
}

void FunDedicatedServerRpcMessage::MergeFrom(const FunDedicatedServerRpcMessage& from) {
  GOOGLE_CHECK_NE(&from, this);
  if (from._has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    if (from.has_xid()) {
      set_xid(from.xid());
    }
    if (from.has_type()) {
      set_type(from.type());
    }
    if (from.has_is_request()) {
      set_is_request(from.is_request());
    }
  }
  _extensions_.MergeFrom(from._extensions_);
  mutable_unknown_fields()->MergeFrom(from.unknown_fields());
}

void FunDedicatedServerRpcMessage::CopyFrom(const ::google::protobuf::Message& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void FunDedicatedServerRpcMessage::CopyFrom(const FunDedicatedServerRpcMessage& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool FunDedicatedServerRpcMessage::IsInitialized() const {
  if ((_has_bits_[0] & 0x00000007) != 0x00000007) return false;


  if (!_extensions_.IsInitialized()) return false;  return true;
}

void FunDedicatedServerRpcMessage::Swap(FunDedicatedServerRpcMessage* other) {
  if (other != this) {
    std::swap(xid_, other->xid_);
    std::swap(type_, other->type_);
    std::swap(is_request_, other->is_request_);
    std::swap(_has_bits_[0], other->_has_bits_[0]);
    _unknown_fields_.Swap(&other->_unknown_fields_);
    std::swap(_cached_size_, other->_cached_size_);
    _extensions_.Swap(&other->_extensions_);
  }
}

::google::protobuf::Metadata FunDedicatedServerRpcMessage::GetMetadata() const {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::Metadata metadata;
  metadata.descriptor = FunDedicatedServerRpcMessage_descriptor_;
  metadata.reflection = FunDedicatedServerRpcMessage_reflection_;
  return metadata;
}


// ===================================================================

#ifndef _MSC_VER
const int FunDedicatedServerRpcSystemMessage::kDataFieldNumber;
#endif  // !_MSC_VER

FunDedicatedServerRpcSystemMessage::FunDedicatedServerRpcSystemMessage()
  : ::google::protobuf::Message() {
  SharedCtor();
  // @@protoc_insertion_point(constructor:FunDedicatedServerRpcSystemMessage)
}

void FunDedicatedServerRpcSystemMessage::InitAsDefaultInstance() {
}

FunDedicatedServerRpcSystemMessage::FunDedicatedServerRpcSystemMessage(const FunDedicatedServerRpcSystemMessage& from)
  : ::google::protobuf::Message() {
  SharedCtor();
  MergeFrom(from);
  // @@protoc_insertion_point(copy_constructor:FunDedicatedServerRpcSystemMessage)
}

void FunDedicatedServerRpcSystemMessage::SharedCtor() {
  ::google::protobuf::internal::GetEmptyString();
  _cached_size_ = 0;
  data_ = const_cast< ::fun::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
}

FunDedicatedServerRpcSystemMessage::~FunDedicatedServerRpcSystemMessage() {
  // @@protoc_insertion_point(destructor:FunDedicatedServerRpcSystemMessage)
  SharedDtor();
}

void FunDedicatedServerRpcSystemMessage::SharedDtor() {
  if (data_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    delete data_;
  }
  if (this != default_instance_) {
  }
}

void FunDedicatedServerRpcSystemMessage::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const ::google::protobuf::Descriptor* FunDedicatedServerRpcSystemMessage::descriptor() {
  protobuf_AssignDescriptorsOnce();
  return FunDedicatedServerRpcSystemMessage_descriptor_;
}

const FunDedicatedServerRpcSystemMessage& FunDedicatedServerRpcSystemMessage::default_instance() {
  if (default_instance_ == NULL) protobuf_AddDesc_funapi_2fdistribution_2ffun_5fdedicated_5fserver_5frpc_5fmessage_2eproto();
  return *default_instance_;
}

FunDedicatedServerRpcSystemMessage* FunDedicatedServerRpcSystemMessage::default_instance_ = NULL;

FunDedicatedServerRpcSystemMessage* FunDedicatedServerRpcSystemMessage::New() const {
  return new FunDedicatedServerRpcSystemMessage;
}

void FunDedicatedServerRpcSystemMessage::Clear() {
  if (has_data()) {
    if (data_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
      data_->clear();
    }
  }
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
  mutable_unknown_fields()->Clear();
}

bool FunDedicatedServerRpcSystemMessage::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!(EXPRESSION)) goto failure
  ::google::protobuf::uint32 tag;
  // @@protoc_insertion_point(parse_start:FunDedicatedServerRpcSystemMessage)
  for (;;) {
    ::std::pair< ::google::protobuf::uint32, bool> p = input->ReadTagWithCutoff(127);
    tag = p.first;
    if (!p.second) goto handle_unusual;
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // optional fun::string data = 1;
      case 1: {
        if (tag == 10) {
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_data()));
          ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
            this->data().data(), this->data().length(),
            ::google::protobuf::internal::WireFormat::PARSE,
            "data");
        } else {
          goto handle_unusual;
        }
        if (input->ExpectAtEnd()) goto success;
        break;
      }

      default: {
      handle_unusual:
        if (tag == 0 ||
            ::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_END_GROUP) {
          goto success;
        }
        DO_(::google::protobuf::internal::WireFormat::SkipField(
              input, tag, mutable_unknown_fields()));
        break;
      }
    }
  }
success:
  // @@protoc_insertion_point(parse_success:FunDedicatedServerRpcSystemMessage)
  return true;
failure:
  // @@protoc_insertion_point(parse_failure:FunDedicatedServerRpcSystemMessage)
  return false;
#undef DO_
}

void FunDedicatedServerRpcSystemMessage::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // @@protoc_insertion_point(serialize_start:FunDedicatedServerRpcSystemMessage)
  // optional fun::string data = 1;
  if (has_data()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
      this->data().data(), this->data().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE,
      "data");
    ::google::protobuf::internal::WireFormatLite::WriteStringMaybeAliased(
      1, this->data(), output);
  }

  if (!unknown_fields().empty()) {
    ::google::protobuf::internal::WireFormat::SerializeUnknownFields(
        unknown_fields(), output);
  }
  // @@protoc_insertion_point(serialize_end:FunDedicatedServerRpcSystemMessage)
}

::google::protobuf::uint8* FunDedicatedServerRpcSystemMessage::SerializeWithCachedSizesToArray(
    ::google::protobuf::uint8* target) const {
  // @@protoc_insertion_point(serialize_to_array_start:FunDedicatedServerRpcSystemMessage)
  // optional fun::string data = 1;
  if (has_data()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
      this->data().data(), this->data().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE,
      "data");
    target =
      ::google::protobuf::internal::WireFormatLite::WriteStringToArray(
        1, this->data(), target);
  }

  if (!unknown_fields().empty()) {
    target = ::google::protobuf::internal::WireFormat::SerializeUnknownFieldsToArray(
        unknown_fields(), target);
  }
  // @@protoc_insertion_point(serialize_to_array_end:FunDedicatedServerRpcSystemMessage)
  return target;
}

int FunDedicatedServerRpcSystemMessage::ByteSize() const {
  int total_size = 0;

  if (_has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    // optional fun::string data = 1;
    if (has_data()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::StringSize(
          this->data());
    }

  }
  if (!unknown_fields().empty()) {
    total_size +=
      ::google::protobuf::internal::WireFormat::ComputeUnknownFieldsSize(
        unknown_fields());
  }
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = total_size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
  return total_size;
}

void FunDedicatedServerRpcSystemMessage::MergeFrom(const ::google::protobuf::Message& from) {
  GOOGLE_CHECK_NE(&from, this);
  const FunDedicatedServerRpcSystemMessage* source =
    ::google::protobuf::internal::dynamic_cast_if_available<const FunDedicatedServerRpcSystemMessage*>(
      &from);
  if (source == NULL) {
    ::google::protobuf::internal::ReflectionOps::Merge(from, this);
  } else {
    MergeFrom(*source);
  }
}

void FunDedicatedServerRpcSystemMessage::MergeFrom(const FunDedicatedServerRpcSystemMessage& from) {
  GOOGLE_CHECK_NE(&from, this);
  if (from._has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    if (from.has_data()) {
      set_data(from.data());
    }
  }
  mutable_unknown_fields()->MergeFrom(from.unknown_fields());
}

void FunDedicatedServerRpcSystemMessage::CopyFrom(const ::google::protobuf::Message& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void FunDedicatedServerRpcSystemMessage::CopyFrom(const FunDedicatedServerRpcSystemMessage& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool FunDedicatedServerRpcSystemMessage::IsInitialized() const {

  return true;
}

void FunDedicatedServerRpcSystemMessage::Swap(FunDedicatedServerRpcSystemMessage* other) {
  if (other != this) {
    std::swap(data_, other->data_);
    std::swap(_has_bits_[0], other->_has_bits_[0]);
    _unknown_fields_.Swap(&other->_unknown_fields_);
    std::swap(_cached_size_, other->_cached_size_);
  }
}

::google::protobuf::Metadata FunDedicatedServerRpcSystemMessage::GetMetadata() const {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::Metadata metadata;
  metadata.descriptor = FunDedicatedServerRpcSystemMessage_descriptor_;
  metadata.reflection = FunDedicatedServerRpcSystemMessage_reflection_;
  return metadata;
}

::google::protobuf::internal::ExtensionIdentifier< ::FunDedicatedServerRpcMessage,
    ::google::protobuf::internal::MessageTypeTraits< ::FunDedicatedServerRpcSystemMessage >, 11, false >
  ds_rpc_sys(kDsRpcSysFieldNumber, ::FunDedicatedServerRpcSystemMessage::default_instance());

// @@protoc_insertion_point(namespace_scope)

// @@protoc_insertion_point(global_scope)
