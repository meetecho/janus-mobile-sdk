cmake_minimum_required(VERSION 3.6)
project(janus-client-sdk)

include(ExternalProject)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include_directories(${CMAKE_BINARY_DIR}/third_party/include)
link_directories(${CMAKE_BINARY_DIR}/third_party/lib)

set(EXTRA_TEST "VOID" CACHE STRING "values: COVERAGE, THREADS, ADDRESS")

set(ROOT ${CMAKE_CURRENT_LIST_DIR})
set(GENERATED_DIR ${ROOT}/generated)
set(THIRD_PARTY_DIR ${ROOT}/third_party)
set(EXT_DIR ${ROOT}/cmake)

set(ENV_ARGS -DCMAKE_CXX_STANDARD=11 -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DCMAKE_BUILD_TYPE=Release CACHE STRING "Env related external CMake arguments" FORCE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wshadow -Werror -Wno-error=dangling-else -fexceptions -Wextra -Wno-unused-parameter -Wno-missing-field-initializers")

if(ANDROID)
  add_definitions(-DANDROID)
  set(ENV_ARGS ${ENV_ARGS} -DANDROID=ON -DANDROID_ABI=${ANDROID_ABI} -DANDROID_PLATFORM=${ANDROID_PLATFORM} -DANDROID_NDK=${ANDROID_NDK} -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS} -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE} -DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM})
endif()

# if(IOS)
  # set(ENV_ARGS ${ENV_ARGS} -DCMAKE_OSX_SYSROOT=iphoneos -DCMAKE_OSX_ARCHITECTURES=armv7 -DCMAKE_XCODE_EFFECTIVE_PLATFORMS="-iphoneos;-iphonesimulator" -DCMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED="NO" -DCMAKE_XCODE_ATTRIBUTE_ENABLE_BITCODE="NO" -DCMAKE_CXX_COMPILER_WORKS=TRUE -DCMAKE_C_COMPILER_WORKS=TRUE -DCMAKE_MACOSX_BUNDLE=YES)
# endif()

ExternalProject_Add(mbedtls_proj
  PREFIX ${CMAKE_BINARY_DIR}/third_party/mbedtls
  SOURCE_DIR ${THIRD_PARTY_DIR}/mbedtls
  CMAKE_ARGS ${ENV_ARGS} -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/third_party -DENABLE_PROGRAMS=OFF -DENABLE_TESTING=OFF)

ExternalProject_Add(curl_proj
  DEPENDS mbedtls_proj
  PREFIX ${CMAKE_BINARY_DIR}/third_party/curl
  SOURCE_DIR ${THIRD_PARTY_DIR}/curl
  CMAKE_ARGS ${ENV_ARGS} -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/third_party -DHAVE_POLL_FINE_EXITCODE=OFF -DCURL_ZLIB=OFF -DCMAKE_USE_MBEDTLS=ON -DHTTP_ONLY=ON -DBUILD_SHARED_LIBS=OFF -DBUILD_TESTING=OFF -DUSE_MANUAL=OFF -DBUILD_CURL_EXE=OFF -DENABLE_THREADED_RESOLVER=OFF -DMBEDTLS_INCLUDE_DIRS=${CMAKE_BINARY_DIR}/third_party/include -DMBEDCRYPTO_LIBRARY=${CMAKE_BINARY_DIR}/third_party/lib/libmbedcrypto.a -DMBEDTLS_LIBRARY=${CMAKE_BINARY_DIR}/third_party/lib/libmbedtls.a -DMBEDX509_LIBRARY=${CMAKE_BINARY_DIR}/third_party/lib/libmbedx509.a)

ExternalProject_Add(json_proj
  PREFIX ${CMAKE_BINARY_DIR}/third_party/json
  SOURCE_DIR ${THIRD_PARTY_DIR}/json
  CMAKE_ARGS ${ENV_ARGS} -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/third_party -DJSON_BuildTests=OFF)

# Janus library

file(GLOB_RECURSE janus_srcs ${ROOT}/src/*.cc ${ROOT}/generated/cpp/*.cpp)

add_library(janus
  SHARED
  ${janus_srcs})

target_include_directories(janus
  SYSTEM
  PUBLIC
  ${GENERATED_DIR}/cpp
  ${ROOT}/include
  ${THIRD_PARTY_DIR}/include)

target_link_libraries(janus
  curl
  mbedtls
  mbedx509
  mbedcrypto)

add_dependencies(janus
  json_proj
  curl_proj)

# END Janus Library

# Android SDK Library

if(ANDROID)
  file(GLOB_RECURSE jni_srcs ${GENERATED_DIR}/jni/*.cpp ${THIRD_PARTY_DIR}/djinni/support-lib/jni/djinni_*.cpp)

  add_library(janus-android-sdk
    SHARED
    ${jni_srcs})

  target_include_directories(janus-android-sdk
    SYSTEM
    PUBLIC
    ${GENERATED_DIR}/cpp/janus/
    ${THIRD_PARTY_DIR}/djinni/support-lib/jni/)

  target_link_libraries(janus-android-sdk
    janus)

  add_dependencies(janus-android-sdk
    janus)
endif()

# End Android SDK library

# Test lib
if(NOT WITHOUT_TESTS)
  set(CMAKE_BUILD_TYPE Debug)

  file(GLOB_RECURSE test_srcs ${ROOT}/test/*.cc)

  add_executable(janus_tests
    ${THIRD_PARTY_DIR}/googletest_bundle/gmock-gtest-all.cc
    ${test_srcs})

  target_compile_definitions(janus_tests PUBLIC GTEST_HAS_PTHREAD=0)

  target_include_directories(janus_tests
    SYSTEM
    PUBLIC
    ${ROOT}/include
    ${ROOT}/test
    ${THIRD_PARTY_DIR}/googletest_bundle
    ${GENERATED_DIR}/cpp)

  target_link_libraries(janus_tests
    pthread
    janus)

  add_dependencies(janus_tests
    janus)
endif()
# End Test lib

get_target_property(JANUS_COMPILE_FLAGS janus COMPILE_FLAGS)
if(JANUS_COMPILE_FLAGS STREQUAL "JANUS_COMPILE_FLAGS-NOTFOUND")
  SET(JANUS_COMPILE_FLAGS "")
endif()
get_target_property(JANUS_LINK_FLAGS janus LINK_FLAGS)
if(JANUS_LINK_FLAGS STREQUAL "JANUS_LINK_FLAGS-NOTFOUND")
  SET(JANUS_LINK_FLAGS "")
endif()
get_target_property(TESTS_COMPILE_FLAGS janus COMPILE_FLAGS)
if(TESTS_COMPILE_FLAGS STREQUAL "TESTS_COMPILE_FLAGS-NOTFOUND")
  SET(TESTS_COMPILE_FLAGS "")
endif()
get_target_property(TESTS_LINK_FLAGS janus LINK_FLAGS)
if(TESTS_LINK_FLAGS STREQUAL "TESTS_LINK_FLAGS-NOTFOUND")
  SET(TESTS_LINK_FLAGS "")
endif()

if(${EXTRA_TEST} MATCHES "COVERAGE")
  set_target_properties(janus
    PROPERTIES
    COMPILE_FLAGS "${JANUS_COMPILE_FLAGS} -fprofile-arcs -ftest-coverage"
    LINK_FLAGS "${JANUS_LINK_FLAGS} -lgcov --coverage")
  set_target_properties(janus_tests
    PROPERTIES
    COMPILE_FLAGS "${TESTS_COMPILE_FLAGS}  -fprofile-arcs -ftest-coverage"
    LINK_FLAGS "${TESTS_LINK_FLAGS} -lgcov --coverage")
elseif(${EXTRA_TEST} MATCHES "THREADS")
  set_target_properties(janus
    PROPERTIES
    COMPILE_FLAGS "${JANUS_COMPILE_FLAGS} -fsanitize=thread"
    LINK_FLAGS "${JANUS_LINK_FLAGS} -fsanitize=thread")
  set_target_properties(janus_tests
    PROPERTIES
    COMPILE_FLAGS "${TESTS_COMPILE_FLAGS} -fsanitize=thread"
    LINK_FLAGS "${TESTS_LINK_FLAGS} -fsanitize=thread")
elseif(${EXTRA_TEST} MATCHES "ADDRESS")
  set_target_properties(janus
    PROPERTIES
    COMPILE_FLAGS "${JANUS_COMPILE_FLAGS} -fsanitize=address"
    LINK_FLAGS "${JANUS_LINK_FLAGS} -fsanitize=address")
  set_target_properties(janus_tests
    PROPERTIES
    COMPILE_FLAGS "${TESTS_COMPILE_FLAGS} -fsanitize=address"
    LINK_FLAGS "${TESTS_LINK_FLAGS} -fsanitize=address")
endif()
