add_library(libabsl STATIC)

target_include_directories(libabsl PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/abseil-cpp
)

target_sources(libabsl ${libabsl_loc} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/abseil-cpp/absl/base/internal/raw_logging.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/abseil-cpp/absl/base/internal/throw_delegate.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/abseil-cpp/absl/numeric/int128.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/abseil-cpp/absl/strings/ascii.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/abseil-cpp/absl/strings/charconv.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/abseil-cpp/absl/strings/cord.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/abseil-cpp/absl/strings/escaping.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/abseil-cpp/absl/strings/internal/charconv_bigint.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/abseil-cpp/absl/strings/internal/charconv_parse.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/abseil-cpp/absl/strings/internal/escaping.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/abseil-cpp/absl/strings/internal/memutil.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/abseil-cpp/absl/strings/internal/ostringstream.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/abseil-cpp/absl/strings/internal/pow10_helper.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/abseil-cpp/absl/strings/internal/str_format/arg.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/abseil-cpp/absl/strings/internal/str_format/bind.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/abseil-cpp/absl/strings/internal/str_format/extension.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/abseil-cpp/absl/strings/internal/str_format/float_conversion.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/abseil-cpp/absl/strings/internal/str_format/output.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/abseil-cpp/absl/strings/internal/str_format/parser.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/abseil-cpp/absl/strings/internal/utf8.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/abseil-cpp/absl/strings/match.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/abseil-cpp/absl/strings/numbers.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/abseil-cpp/absl/strings/str_cat.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/abseil-cpp/absl/strings/str_replace.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/abseil-cpp/absl/strings/str_split.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/abseil-cpp/absl/strings/string_view.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/abseil-cpp/absl/strings/substitute.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/abseil-cpp/absl/types/bad_optional_access.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/abseil-cpp/absl/types/bad_variant_access.cc
)