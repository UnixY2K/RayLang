// this file contains all the "facts" required from C side
// such as type definitions, required headers
// and expected macros
#pragma once
// this just make easier to separate the linkage if compiler under c++
#ifdef __cplusplus
#define RAYLANG_C_LINKAGE extern "C"
#define RAYLANG_CPP_LINKAGE extern "C++"
#define RAYLANG_DEFAULT_LINKAGE RAYLANG_CPP_LINKAGE
#else
#define RAYLANG_C_LINKAGE
#define RAYLANG_CPP_LINKAGE
#define RAYLANG_DEFAULT_LINKAGE RAYLANG_C_LINKAGE
#endif
#ifdef __cplusplus
RAYLANG_C_LINKAGE {
#endif
#include <stddef.h>
#include <stdint.h>
#define u8 uint8_t
#define s8 int8_t
#define u16 uint16_t
#define s16 int16_t
#define u32 uint32_t
#define s32 int32_t
#define u64 uint64_t
#define s64 int64_t
#define f32 float
#define f64 double
#define usize uintmax_t
#define ssize intmax_t
#define c_char char
#define c_int int
#define c_size size_t
#if defined(_WIN32) || defined(__CYGWIN__) || defined(_MSC_VER)
// Microsoft
#define RAYLANG_LINK_IMPORT __declspec(dllimport)
#define RAYLANG_LINK_EXPORT __declspec(dllexport)
#define RAYLANG_LINK_LOCAL
#elif defined(__GNUC__) && __GNUC__ >= 4
// GCC
#define RAYLANG_LINK_IMPORT __attribute__((visibility("default")))
#define RAYLANG_LINK_EXPORT __attribute__((visibility("default")))
#define RAYLANG_LINK_LOCAL __attribute__((visibility("hidden")))
#else
// unknown dynamic link semantics
#pragma warning "Unknown dynamic link import/export semantics."
#define RAYLANG_LINK_IMPORT
#define RAYLANG_LINK_EXPORT
#define RAYLANG_LINK_LOCAL
#endif

#ifdef __cplusplus
}
#endif
