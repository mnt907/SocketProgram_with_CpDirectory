// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: header.proto

#ifndef GOOGLE_PROTOBUF_INCLUDED_header_2eproto
#define GOOGLE_PROTOBUF_INCLUDED_header_2eproto

#include <limits>
#include <string>

#include <google/protobuf/port_def.inc>
#if PROTOBUF_VERSION < 3008000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers. Please update
#error your headers.
#endif
#if 3008000 < PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers. Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/port_undef.inc>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_table_driven.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/inlined_string_field.h>
#include <google/protobuf/metadata.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include <google/protobuf/generated_enum_reflection.h>
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)
#include <google/protobuf/port_def.inc>
#define PROTOBUF_INTERNAL_EXPORT_header_2eproto
PROTOBUF_NAMESPACE_OPEN
namespace internal {
class AnyMetadata;
}  // namespace internal
PROTOBUF_NAMESPACE_CLOSE

// Internal implementation detail -- do not use these members.
struct TableStruct_header_2eproto {
  static const ::PROTOBUF_NAMESPACE_ID::internal::ParseTableField entries[]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::AuxillaryParseTableField aux[]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::ParseTable schema[1]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::FieldMetadata field_metadata[];
  static const ::PROTOBUF_NAMESPACE_ID::internal::SerializationTable serialization_table[];
  static const ::PROTOBUF_NAMESPACE_ID::uint32 offsets[];
};
extern const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable descriptor_table_header_2eproto;
class Header;
class HeaderDefaultTypeInternal;
extern HeaderDefaultTypeInternal _Header_default_instance_;
PROTOBUF_NAMESPACE_OPEN
template<> ::Header* Arena::CreateMaybeMessage<::Header>(Arena*);
PROTOBUF_NAMESPACE_CLOSE

enum Header_TYPE : int {
  Header_TYPE_CONNECT = 0,
  Header_TYPE_CHECK_DIR = 1,
  Header_TYPE_NOT_EXIST_DIR = 2,
  Header_TYPE_ERROR_SIG = 3,
  Header_TYPE_Header_TYPE_INT_MIN_SENTINEL_DO_NOT_USE_ = std::numeric_limits<::PROTOBUF_NAMESPACE_ID::int32>::min(),
  Header_TYPE_Header_TYPE_INT_MAX_SENTINEL_DO_NOT_USE_ = std::numeric_limits<::PROTOBUF_NAMESPACE_ID::int32>::max()
};
bool Header_TYPE_IsValid(int value);
constexpr Header_TYPE Header_TYPE_TYPE_MIN = Header_TYPE_CONNECT;
constexpr Header_TYPE Header_TYPE_TYPE_MAX = Header_TYPE_ERROR_SIG;
constexpr int Header_TYPE_TYPE_ARRAYSIZE = Header_TYPE_TYPE_MAX + 1;

const ::PROTOBUF_NAMESPACE_ID::EnumDescriptor* Header_TYPE_descriptor();
template<typename T>
inline const std::string& Header_TYPE_Name(T enum_t_value) {
  static_assert(::std::is_same<T, Header_TYPE>::value ||
    ::std::is_integral<T>::value,
    "Incorrect type passed to function Header_TYPE_Name.");
  return ::PROTOBUF_NAMESPACE_ID::internal::NameOfEnum(
    Header_TYPE_descriptor(), enum_t_value);
}
inline bool Header_TYPE_Parse(
    const std::string& name, Header_TYPE* value) {
  return ::PROTOBUF_NAMESPACE_ID::internal::ParseNamedEnum<Header_TYPE>(
    Header_TYPE_descriptor(), name, value);
}
// ===================================================================

class Header :
    public ::PROTOBUF_NAMESPACE_ID::Message /* @@protoc_insertion_point(class_definition:Header) */ {
 public:
  Header();
  virtual ~Header();

  Header(const Header& from);
  Header(Header&& from) noexcept
    : Header() {
    *this = ::std::move(from);
  }

  inline Header& operator=(const Header& from) {
    CopyFrom(from);
    return *this;
  }
  inline Header& operator=(Header&& from) noexcept {
    if (GetArenaNoVirtual() == from.GetArenaNoVirtual()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }

  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* descriptor() {
    return GetDescriptor();
  }
  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* GetDescriptor() {
    return GetMetadataStatic().descriptor;
  }
  static const ::PROTOBUF_NAMESPACE_ID::Reflection* GetReflection() {
    return GetMetadataStatic().reflection;
  }
  static const Header& default_instance();

  static void InitAsDefaultInstance();  // FOR INTERNAL USE ONLY
  static inline const Header* internal_default_instance() {
    return reinterpret_cast<const Header*>(
               &_Header_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    0;

  void Swap(Header* other);
  friend void swap(Header& a, Header& b) {
    a.Swap(&b);
  }

  // implements Message ----------------------------------------------

  inline Header* New() const final {
    return CreateMaybeMessage<Header>(nullptr);
  }

  Header* New(::PROTOBUF_NAMESPACE_ID::Arena* arena) const final {
    return CreateMaybeMessage<Header>(arena);
  }
  void CopyFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) final;
  void MergeFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) final;
  void CopyFrom(const Header& from);
  void MergeFrom(const Header& from);
  PROTOBUF_ATTRIBUTE_REINITIALIZES void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  #if GOOGLE_PROTOBUF_ENABLE_EXPERIMENTAL_PARSER
  const char* _InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) final;
  #else
  bool MergePartialFromCodedStream(
      ::PROTOBUF_NAMESPACE_ID::io::CodedInputStream* input) final;
  #endif  // GOOGLE_PROTOBUF_ENABLE_EXPERIMENTAL_PARSER
  void SerializeWithCachedSizes(
      ::PROTOBUF_NAMESPACE_ID::io::CodedOutputStream* output) const final;
  ::PROTOBUF_NAMESPACE_ID::uint8* InternalSerializeWithCachedSizesToArray(
      ::PROTOBUF_NAMESPACE_ID::uint8* target) const final;
  int GetCachedSize() const final { return _cached_size_.Get(); }

  private:
  inline void SharedCtor();
  inline void SharedDtor();
  void SetCachedSize(int size) const final;
  void InternalSwap(Header* other);
  friend class ::PROTOBUF_NAMESPACE_ID::internal::AnyMetadata;
  static ::PROTOBUF_NAMESPACE_ID::StringPiece FullMessageName() {
    return "Header";
  }
  private:
  inline ::PROTOBUF_NAMESPACE_ID::Arena* GetArenaNoVirtual() const {
    return nullptr;
  }
  inline void* MaybeArenaPtr() const {
    return nullptr;
  }
  public:

  ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadata() const final;
  private:
  static ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadataStatic() {
    ::PROTOBUF_NAMESPACE_ID::internal::AssignDescriptors(&::descriptor_table_header_2eproto);
    return ::descriptor_table_header_2eproto.file_level_metadata[kIndexInFileMessages];
  }

  public:

  // nested types ----------------------------------------------------

  typedef Header_TYPE TYPE;
  static constexpr TYPE CONNECT =
    Header_TYPE_CONNECT;
  static constexpr TYPE CHECK_DIR =
    Header_TYPE_CHECK_DIR;
  static constexpr TYPE NOT_EXIST_DIR =
    Header_TYPE_NOT_EXIST_DIR;
  static constexpr TYPE ERROR_SIG =
    Header_TYPE_ERROR_SIG;
  static inline bool TYPE_IsValid(int value) {
    return Header_TYPE_IsValid(value);
  }
  static constexpr TYPE TYPE_MIN =
    Header_TYPE_TYPE_MIN;
  static constexpr TYPE TYPE_MAX =
    Header_TYPE_TYPE_MAX;
  static constexpr int TYPE_ARRAYSIZE =
    Header_TYPE_TYPE_ARRAYSIZE;
  static inline const ::PROTOBUF_NAMESPACE_ID::EnumDescriptor*
  TYPE_descriptor() {
    return Header_TYPE_descriptor();
  }
  template<typename T>
  static inline const std::string& TYPE_Name(T enum_t_value) {
    static_assert(::std::is_same<T, TYPE>::value ||
      ::std::is_integral<T>::value,
      "Incorrect type passed to function TYPE_Name.");
    return Header_TYPE_Name(enum_t_value);
  }
  static inline bool TYPE_Parse(const std::string& name,
      TYPE* value) {
    return Header_TYPE_Parse(name, value);
  }

  // accessors -------------------------------------------------------

  // string m_file_path = 4;
  void clear_m_file_path();
  static const int kMFilePathFieldNumber = 4;
  const std::string& m_file_path() const;
  void set_m_file_path(const std::string& value);
  void set_m_file_path(std::string&& value);
  void set_m_file_path(const char* value);
  void set_m_file_path(const char* value, size_t size);
  std::string* mutable_m_file_path();
  std::string* release_m_file_path();
  void set_allocated_m_file_path(std::string* m_file_path);

  // int64 m_length = 2;
  void clear_m_length();
  static const int kMLengthFieldNumber = 2;
  ::PROTOBUF_NAMESPACE_ID::int64 m_length() const;
  void set_m_length(::PROTOBUF_NAMESPACE_ID::int64 value);

  // .Header.TYPE m_type = 1;
  void clear_m_type();
  static const int kMTypeFieldNumber = 1;
  ::Header_TYPE m_type() const;
  void set_m_type(::Header_TYPE value);

  // bool m_is_dir = 3;
  void clear_m_is_dir();
  static const int kMIsDirFieldNumber = 3;
  bool m_is_dir() const;
  void set_m_is_dir(bool value);

  // @@protoc_insertion_point(class_scope:Header)
 private:
  class HasBitSetters;

  ::PROTOBUF_NAMESPACE_ID::internal::InternalMetadataWithArena _internal_metadata_;
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr m_file_path_;
  ::PROTOBUF_NAMESPACE_ID::int64 m_length_;
  int m_type_;
  bool m_is_dir_;
  mutable ::PROTOBUF_NAMESPACE_ID::internal::CachedSize _cached_size_;
  friend struct ::TableStruct_header_2eproto;
};
// ===================================================================


// ===================================================================

#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
// Header

// .Header.TYPE m_type = 1;
inline void Header::clear_m_type() {
  m_type_ = 0;
}
inline ::Header_TYPE Header::m_type() const {
  // @@protoc_insertion_point(field_get:Header.m_type)
  return static_cast< ::Header_TYPE >(m_type_);
}
inline void Header::set_m_type(::Header_TYPE value) {
  
  m_type_ = value;
  // @@protoc_insertion_point(field_set:Header.m_type)
}

// int64 m_length = 2;
inline void Header::clear_m_length() {
  m_length_ = PROTOBUF_LONGLONG(0);
}
inline ::PROTOBUF_NAMESPACE_ID::int64 Header::m_length() const {
  // @@protoc_insertion_point(field_get:Header.m_length)
  return m_length_;
}
inline void Header::set_m_length(::PROTOBUF_NAMESPACE_ID::int64 value) {
  
  m_length_ = value;
  // @@protoc_insertion_point(field_set:Header.m_length)
}

// bool m_is_dir = 3;
inline void Header::clear_m_is_dir() {
  m_is_dir_ = false;
}
inline bool Header::m_is_dir() const {
  // @@protoc_insertion_point(field_get:Header.m_is_dir)
  return m_is_dir_;
}
inline void Header::set_m_is_dir(bool value) {
  
  m_is_dir_ = value;
  // @@protoc_insertion_point(field_set:Header.m_is_dir)
}

// string m_file_path = 4;
inline void Header::clear_m_file_path() {
  m_file_path_.ClearToEmptyNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
}
inline const std::string& Header::m_file_path() const {
  // @@protoc_insertion_point(field_get:Header.m_file_path)
  return m_file_path_.GetNoArena();
}
inline void Header::set_m_file_path(const std::string& value) {
  
  m_file_path_.SetNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:Header.m_file_path)
}
inline void Header::set_m_file_path(std::string&& value) {
  
  m_file_path_.SetNoArena(
    &::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), ::std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:Header.m_file_path)
}
inline void Header::set_m_file_path(const char* value) {
  GOOGLE_DCHECK(value != nullptr);
  
  m_file_path_.SetNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:Header.m_file_path)
}
inline void Header::set_m_file_path(const char* value, size_t size) {
  
  m_file_path_.SetNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:Header.m_file_path)
}
inline std::string* Header::mutable_m_file_path() {
  
  // @@protoc_insertion_point(field_mutable:Header.m_file_path)
  return m_file_path_.MutableNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
}
inline std::string* Header::release_m_file_path() {
  // @@protoc_insertion_point(field_release:Header.m_file_path)
  
  return m_file_path_.ReleaseNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
}
inline void Header::set_allocated_m_file_path(std::string* m_file_path) {
  if (m_file_path != nullptr) {
    
  } else {
    
  }
  m_file_path_.SetAllocatedNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), m_file_path);
  // @@protoc_insertion_point(field_set_allocated:Header.m_file_path)
}

#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__

// @@protoc_insertion_point(namespace_scope)


PROTOBUF_NAMESPACE_OPEN

template <> struct is_proto_enum< ::Header_TYPE> : ::std::true_type {};
template <>
inline const EnumDescriptor* GetEnumDescriptor< ::Header_TYPE>() {
  return ::Header_TYPE_descriptor();
}

PROTOBUF_NAMESPACE_CLOSE

// @@protoc_insertion_point(global_scope)

#include <google/protobuf/port_undef.inc>
#endif  // GOOGLE_PROTOBUF_INCLUDED_GOOGLE_PROTOBUF_INCLUDED_header_2eproto