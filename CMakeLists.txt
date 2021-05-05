cmake_minimum_required(VERSION 3.14.0)
project(WASMATI VERSION 1.0.0)
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_PROJECT_VERSION "${CMAKE_PROJECT_VERSION}")

option(USE_ASAN "Use address sanitizer" OFF)
option(USE_MSAN "Use memory sanitizer" OFF)
option(USE_LSAN "Use leak sanitizer" OFF)
option(USE_UBSAN "Use undefined behavior sanitizer" OFF)
option(CODE_COVERAGE "Build with code coverage enabled" OFF)
option(WITH_EXCEPTIONS "Build with exceptions enabled" OFF)
option(WERROR "Build with warnings as errors" OFF)
option(WIP_TOOLS "Build Work-In-Progress functionality" OFF)

if (MSVC)
  set(COMPILER_IS_CLANG 0)
  set(COMPILER_IS_GNU 0)
  set(COMPILER_IS_MSVC 1)
elseif (CMAKE_C_COMPILER_ID MATCHES "Clang")
  set(COMPILER_IS_CLANG 1)
  set(COMPILER_IS_GNU 0)
  set(COMPILER_IS_MSVC 0)
elseif (CMAKE_C_COMPILER_ID STREQUAL "GNU")
  set(COMPILER_IS_CLANG 0)
  set(COMPILER_IS_GNU 1)
  set(COMPILER_IS_MSVC 0)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
  set(COMPILER_IS_CLANG 1)
  set(COMPILER_IS_GNU 0)
  set(COMPILER_IS_MSVC 0)
else ()
  set(COMPILER_IS_CLANG 0)
  set(COMPILER_IS_GNU 0)
  set(COMPILER_IS_MSVC 0)
endif ()

include(CheckIncludeFile)
include(CheckSymbolExists)
include(FetchContent)

check_include_file("alloca.h" HAVE_ALLOCA_H)
check_include_file("unistd.h" HAVE_UNISTD_H)
check_symbol_exists(snprintf "stdio.h" HAVE_SNPRINTF)
check_symbol_exists(sysconf "unistd.h" HAVE_SYSCONF)
check_symbol_exists(strcasecmp "strings.h" HAVE_STRCASECMP)

if (WIN32)
  check_symbol_exists(ENABLE_VIRTUAL_TERMINAL_PROCESSING "windows.h" HAVE_WIN32_VT100)
endif ()

include(CheckTypeSize)
check_type_size(ssize_t SSIZE_T)
check_type_size(size_t SIZEOF_SIZE_T)

configure_file(
  ${WASMATI_SOURCE_DIR}/src/config/config.h.in
  ${WASMATI_BINARY_DIR}/config.h
)

include_directories(${WASMATI_SOURCE_DIR} ${WASMATI_BINARY_DIR} ${WASMATI_SOURCE_DIR}/config)

if (COMPILER_IS_MSVC)
  # disable warning C4018: signed/unsigned mismatch
  # disable warning C4056, C4756: overflow in floating-point constant arithmetic
  #   seems to not like float compare w/ HUGE_VALF; bug?
  # disable warnings C4267 and C4244: conversion/truncation from larger to smaller type.
  # disable warning C4800: implicit conversion from larger int to bool
  add_definitions(-W3 -wd2182 -wd2065 -wd4018 -wd4056 -wd4756 -wd4267 -wd4244 -wd4800 -D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS)

  if (WERROR)
    add_definitions(-WX)
  endif ()

  #if (NOT WITH_EXCEPTIONS)
    # disable exception use in C++ library
   # add_definitions(-D_HAS_EXCEPTIONS=0)
  #endif ()
else ()
  # disable -Wunused-parameter: this is really common when implementing
  #   interfaces, etc.
  # disable -Wpointer-arith: this is a GCC extension, and doesn't work in MSVC.
  add_definitions(
    -Wall -Wextra -Wno-unused-parameter -Wpointer-arith -g
    -Wuninitialized
  )

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wold-style-cast")

  if (WERROR)
    add_definitions(-Werror)
  endif ()

# if (NOT WITH_EXCEPTIONS)
#   add_definitions(-fno-exceptions)
# endif ()

  # Need to define __STDC_*_MACROS because C99 specifies that C++ shouldn't
  # define format (e.g. PRIu64) or limit (e.g. UINT32_MAX) macros without the
  # definition, and some libcs (e.g. glibc2.17 and earlier) follow that.
  add_definitions(-D__STDC_LIMIT_MACROS=1 -D__STDC_FORMAT_MACROS=1)

  if (MINGW)
    # _POSIX is needed to ensure we use mingw printf
    # instead of the VC runtime one.
    add_definitions(-D_POSIX)
  endif ()

  if (COMPILER_IS_GNU)
    # disable -Wclobbered: it seems to be guessing incorrectly about a local
    # variable being clobbered by longjmp.
    add_definitions(-Wno-clobbered)
  endif ()

  if (NOT EMSCRIPTEN)
    # try to get the target architecture by compiling a dummy.c file and
    # checking the architecture using the file command.
    file(WRITE ${WASMATI_BINARY_DIR}/dummy.c "main(){}")
    try_compile(
      COMPILE_OK
      ${WASMATI_BINARY_DIR}
      ${WASMATI_BINARY_DIR}/dummy.c
      COPY_FILE ${WASMATI_BINARY_DIR}/dummy
    )
    if (COMPILE_OK)
      execute_process(
        COMMAND file ${WASMATI_BINARY_DIR}/dummy
        RESULT_VARIABLE FILE_RESULT
        OUTPUT_VARIABLE FILE_OUTPUT
        ERROR_QUIET
      )

      if (FILE_RESULT EQUAL 0)
        if (${FILE_OUTPUT} MATCHES "x86[-_]64")
          set(TARGET_ARCH "x86-64")
        elseif (${FILE_OUTPUT} MATCHES "Intel 80386")
          set(TARGET_ARCH "i386")
        elseif (${FILE_OUTPUT} MATCHES "ARM")
          set(TARGET_ARCH "ARM")
        else ()
          message(WARNING "Unknown target architecture!")
        endif ()
      else ()
        message(WARNING "Error running `file` command on dummy executable")
      endif ()
    else ()
      message(WARNING "Error compiling dummy.c file")
    endif ()

    if (TARGET_ARCH STREQUAL "i386")
      # wasm doesn't allow for x87 floating point math
      add_definitions(-msse2 -mfpmath=sse)
    endif ()
  endif ()
endif ()

set(USE_SANITIZER FALSE)

function(SANITIZER NAME FLAGS)
  if (${NAME})
    if (USE_SANITIZER)
      message(FATAL_ERROR "Only one sanitizer allowed")
    endif ()
    set(USE_SANITIZER TRUE PARENT_SCOPE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FLAGS}" PARENT_SCOPE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAGS}" PARENT_SCOPE)
  endif ()
endfunction()
SANITIZER(USE_ASAN "-fsanitize=address")
SANITIZER(USE_MSAN "-fsanitize=memory")
SANITIZER(USE_LSAN "-fsanitize=leak")

if (USE_UBSAN)
  # -fno-sanitize-recover was deprecated, see if we are compiling with a newer
  # clang that requires -fno-sanitize-recover=all.
  set(UBSAN_BLACKLIST ${WASMATI_SOURCE_DIR}/ubsan.blacklist)
  include(CheckCXXCompilerFlag)
  CHECK_CXX_COMPILER_FLAG("-fsanitize=undefined -fno-sanitize-recover -Wall -Werror" HAS_UBSAN_RECOVER_BARE)
  if (HAS_UBSAN_RECOVER_BARE)
    SANITIZER(USE_UBSAN "-fsanitize=undefined -fno-sanitize-recover -fsanitize-blacklist=${UBSAN_BLACKLIST}")
  endif ()
  CHECK_CXX_COMPILER_FLAG("-fsanitize=undefined -fno-sanitize-recover=all -Wall -Werror" HAS_UBSAN_RECOVER_ALL)
  if (HAS_UBSAN_RECOVER_ALL)
    SANITIZER(USE_UBSAN "-fsanitize=undefined -fno-sanitize-recover=all -fsanitize-blacklist=${UBSAN_BLACKLIST}")
  endif ()
  if (NOT USE_SANITIZER)
    message(FATAL_ERROR "UBSAN is not supported")
  endif ()
endif ()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${WASMATI_SOURCE_DIR}/cmake)

add_custom_target(everything_wasm)

FetchContent_Declare(
  wabt
  GIT_REPOSITORY "https://github.com/WebAssembly/wabt"
  GIT_TAG "4ba97c116d1417b2fb516299a37965820b93ced3"
)

FetchContent_Declare(json
  GIT_REPOSITORY https://github.com/ArthurSonzogni/nlohmann_json_cmake_fetchcontent
  GIT_TAG v3.9.1
)

include(ExternalProject)
externalproject_add(libzip
  PREFIX ${CMAKE_CURRENT_BINARY_DIR}/libzip
  GIT_REPOSITORY "https://github.com/nih-at/libzip.git"
  GIT_TAG "66e496489bdae81bfda8b0088172871d8fda0032"
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/libzip
)

ExternalProject_Get_Property(libzip install_dir)
link_directories(${CMAKE_CURRENT_BINARY_DIR}/libzip/lib)

FetchContent_MakeAvailable(wabt json)
include_directories(${wabt_SOURCE_DIR} ${json_SOURCE_DIR} ${install_dir}/include)

BISON_TARGET(
    DSLParser 
    ${WASMATI_SOURCE_DIR}/src/interpreter/parser.y 
    ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp
    DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/parser.h
    COMPILE_FLAGS "-dtv"
)
FLEX_TARGET(
    DSLScanner 
    ${WASMATI_SOURCE_DIR}/src/interpreter/scanner.l  
    ${CMAKE_CURRENT_BINARY_DIR}/scanner.cpp
)
ADD_FLEX_BISON_DEPENDENCY(DSLScanner DSLParser)

add_library(interp STATIC
     ${FLEX_DSLScanner_OUTPUTS}
     ${BISON_DSLParser_OUTPUTS}  
     ${FLEX_LIBRARIES})

add_library(wasmatilib STATIC
	  src/config/config.h
	  src/config/config.cc
	  src/options.h
	  src/options.cc
	  src/graph.h
	  src/graph.cc
	  src/ast-builder.h
	  src/ast-builder.cc
	  src/cfg-builder.h
	  src/cfg-builder.cc
	  src/pdg-builder.h
	  src/pdg-builder.cc
	  src/query.h
	  src/query.cc
	  src/utils.h
	  src/vulns.h
	  # writters
	  src/writers/csv-writer.h
	  src/writers/datalog-writer.h
	  src/writers/dot-writer.h
	  src/writers/json-writer.h
	  # readers
	  src/readers/csv-reader.h
	  # Queries
	  src/queries/boBuffsStatic.cc
	  src/queries/boBuffsStaticMalloc.cc
	  src/queries/verifyMallocConst.cc
	  src/queries/boMemcpy.cc
	  #src/queries/boLoops.cc
	  src/queries/boScanfLoops.cc
	  src/queries/bufferSizes.cc
	  src/queries/dangerousFunctions.cc
	  src/queries/formatStrings.cc
	  src/queries/isTainted.cc
	  #src/queries/taintedCallIndirect.cc
	  src/queries/taintedFuncToFunc.cc
	  src/queries/taintedLocalToFunc.cc
	  #src/queries/unreachable.cc
	  #src/queries/useAfterFree.cc
	  #src/queries/doubleFree.cc
	  # interpreter
	  src/interpreter/scanner.h
	  src/interpreter/interpreter.h 
	  src/interpreter/interpreter.cc 
	  src/interpreter/nodes.h
	  src/interpreter/nodes.cc
	  src/interpreter/y.tab.h 
      src/interpreter/evaluator.h
      src/interpreter/evaluator.cc
      src/interpreter/symbol-table.h
      src/interpreter/functions.h)

add_dependencies(wasmatilib interp)

if (NOT EMSCRIPTEN)
  if (CODE_COVERAGE)

    add_definitions("-fprofile-arcs -ftest-coverage")
    if (COMPILER_IS_CLANG)
      set(CMAKE_EXE_LINKER_FLAGS "--coverage")
    else ()
      link_libraries(gcov)
    endif ()
  endif ()

  function(wasmati_executable)
    cmake_parse_arguments(EXE "WITH_LIBM;INSTALL" "NAME" "SOURCES;LIBS" ${ARGN})
    # Always link wasmatilib.
    set(EXE_LIBS "${EXE_LIBS};wasmatilib")

    # Optionally link libm.
    if (EXE_WITH_LIBM AND (COMPILER_IS_CLANG OR COMPILER_IS_GNU))
      set(EXE_LIBS "${EXE_LIBS};m")
    endif ()

    add_executable(${EXE_NAME} ${EXE_SOURCES})
    add_dependencies(everything_wasm ${EXE_NAME})
    add_dependencies(${EXE_NAME} libzip)
    target_link_libraries(${EXE_NAME} PRIVATE ${EXE_LIBS} wabt nlohmann_json::nlohmann_json interp zip)
    set_property(TARGET ${EXE_NAME} PROPERTY CXX_STANDARD 11)
    set_property(TARGET ${EXE_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)

    if (EXE_INSTALL)
      list(APPEND WASMATI_EXECUTABLES ${EXE_NAME})
      set(WASMATI_EXECUTABLES ${WASMATI_EXECUTABLES} PARENT_SCOPE)

      add_custom_target(${EXE_NAME}-copy-to-bin ALL
        COMMAND ${CMAKE_COMMAND} -E make_directory ${WASMATI_SOURCE_DIR}/bin
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${EXE_NAME}> ${WASMATI_SOURCE_DIR}/bin
        DEPENDS ${EXE_NAME}
      )
    endif ()
  endfunction()

  if (BUILD_TOOLS)
    # wasmati
    wasmati_executable(
      NAME wasmati
      SOURCES src/tools/wasmati.cc
      INSTALL
    )

    # wasmati-query
    wasmati_executable(
      NAME wasmati-query
      SOURCES src/tools/wasmati-query.cc
      INSTALL
    )
  endif ()

endif()
