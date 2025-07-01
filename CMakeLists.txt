# uriparser - RFC 3986 URI parsing library
#
# Copyright (C) 2018, Sebastian Pipping <sebastian@pipping.org>
# All rights reserved.
#
# Redistribution and use in source  and binary forms, with or without
# modification, are permitted provided  that the following conditions
# are met:
#
#     1. Redistributions  of  source  code   must  retain  the  above
#        copyright notice, this list  of conditions and the following
#        disclaimer.
#
#     2. Redistributions  in binary  form  must  reproduce the  above
#        copyright notice, this list  of conditions and the following
#        disclaimer  in  the  documentation  and/or  other  materials
#        provided with the distribution.
#
#     3. Neither the  name of the  copyright holder nor the  names of
#        its contributors may be used  to endorse or promote products
#        derived from  this software  without specific  prior written
#        permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND  ANY EXPRESS OR IMPLIED WARRANTIES,  INCLUDING, BUT NOT
# LIMITED TO,  THE IMPLIED WARRANTIES OF  MERCHANTABILITY AND FITNESS
# FOR  A  PARTICULAR  PURPOSE  ARE  DISCLAIMED.  IN  NO  EVENT  SHALL
# THE  COPYRIGHT HOLDER  OR CONTRIBUTORS  BE LIABLE  FOR ANY  DIRECT,
# INDIRECT, INCIDENTAL, SPECIAL,  EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO,  PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA,  OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
# STRICT  LIABILITY,  OR  TORT (INCLUDING  NEGLIGENCE  OR  OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
# OF THE POSSIBILITY OF SUCH DAMAGE.
#
cmake_minimum_required(VERSION 3.10.0)

project(uriparser
    VERSION
        0.9.8
    LANGUAGES
        C
)
set(CMAKE_C_STANDARD 90)  # same as C89, value "89" not supported by CMake
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)  # i.e. -std=c90 rather than default -std=gnu90

# See https://verbump.de/ for what these numbers do
set(URIPARSER_SO_CURRENT    1)
set(URIPARSER_SO_REVISION  31)
set(URIPARSER_SO_AGE        0)

include(CheckCCompilerFlag)
include(CheckFunctionExists)
include(CheckLibraryExists)
include(CheckSymbolExists)
include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

#
# Configuration
#
if(DEFINED BUILD_SHARED_LIBS)
    set(_URIPARSER_SHARED_LIBS_DEFAULT ${BUILD_SHARED_LIBS})
else()
    set(_URIPARSER_SHARED_LIBS_DEFAULT ON)
endif()
option(URIPARSER_SHARED_LIBS "Build shared libraries (rather than static ones)" ${_URIPARSER_SHARED_LIBS_DEFAULT})
option(URIPARSER_BUILD_DOCS "Build API documentation (requires Doxygen, Graphviz, and (optional) Qt's qhelpgenerator)" ON)
option(URIPARSER_BUILD_TESTS "Build test suite (requires GTest >=1.8.0)" ON)
option(URIPARSER_BUILD_FUZZERS "Build fuzzers (requires Clang)" OFF)
option(URIPARSER_OSSFUZZ_BUILD "Build fuzzers via OSS-Fuzz" OFF)
option(URIPARSER_BUILD_TOOLS "Build tools (e.g. CLI \"uriparse\")" ON)
option(URIPARSER_BUILD_CHAR "Build code supporting data type 'char'" ON)
option(URIPARSER_BUILD_WCHAR_T "Build code supporting data type 'wchar_t'" ON)
option(URIPARSER_ENABLE_INSTALL "Enable installation of uriparser" ON)
option(URIPARSER_WARNINGS_AS_ERRORS "Treat all compiler warnings as errors" OFF)
set(URIPARSER_MSVC_RUNTIME "" CACHE STRING "Use of specific runtime library (/MT /MTd /MD /MDd) with MSVC")

if(NOT URIPARSER_BUILD_CHAR AND NOT URIPARSER_BUILD_WCHAR_T)
    message(SEND_ERROR "One or more of URIPARSER_BUILD_CHAR and URIPARSER_BUILD_WCHAR_T needs to be enabled.")
endif()
if(URIPARSER_BUILD_TESTS AND NOT (URIPARSER_BUILD_CHAR AND URIPARSER_BUILD_WCHAR_T))
    message(SEND_ERROR "URIPARSER_BUILD_TESTS=ON requires both URIPARSER_BUILD_CHAR=ON and URIPARSER_BUILD_WCHAR_T=ON.")
endif()
if(URIPARSER_BUILD_TOOLS AND NOT URIPARSER_BUILD_CHAR)
    message(SEND_ERROR "URIPARSER_BUILD_TOOLS=ON requires URIPARSER_BUILD_CHAR=ON.")
endif()

if(URIPARSER_BUILD_TESTS OR URIPARSER_BUILD_FUZZERS)
    # We have to call enable_language() before modifying any CMAKE_CXX_* variables
    enable_language(CXX)
endif()

if(URIPARSER_SHARED_LIBS)
    set(_URIPARSER_STATIC_OR_SHARED SHARED)
else()
    set(_URIPARSER_STATIC_OR_SHARED STATIC)
endif()

macro(uriparser_apply_msvc_runtime_to ref)
    string(REGEX REPLACE "/M[DT]d?" ${URIPARSER_MSVC_RUNTIME} ${ref} "${${ref}}")
endmacro()

if(MSVC AND URIPARSER_MSVC_RUNTIME)
    uriparser_apply_msvc_runtime_to(CMAKE_C_FLAGS)
    uriparser_apply_msvc_runtime_to(CMAKE_C_FLAGS_DEBUG)
    uriparser_apply_msvc_runtime_to(CMAKE_C_FLAGS_RELEASE)
endif()

macro(uriparser_install)
    if(URIPARSER_ENABLE_INSTALL)
        install(${ARGN})
    endif()
endmacro()

#
# Compiler checks
#
set(URIPARSER_EXTRA_COMPILE_FLAGS)

check_c_compiler_flag("-fvisibility=hidden" URIPARSER_COMPILER_SUPPORTS_VISIBILITY)
if(URIPARSER_COMPILER_SUPPORTS_VISIBILITY)
    set(URIPARSER_EXTRA_COMPILE_FLAGS "${URIPARSER_EXTRA_COMPILE_FLAGS} -fvisibility=hidden")
endif()

#
# UriConfig.h
#
check_symbol_exists(wprintf wchar.h HAVE_WPRINTF)
check_function_exists(reallocarray HAVE_REALLOCARRAY)  # no luck with CheckSymbolExists
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/UriConfig.h.in UriConfig.h)

#
# C library
#
set(API_HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/uriparser/UriBase.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/uriparser/UriDefsAnsi.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/uriparser/UriDefsConfig.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/uriparser/UriDefsUnicode.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/uriparser/Uri.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/uriparser/UriIp4.h
)
set(LIBRARY_CODE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UriCommon.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UriCommon.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UriCompare.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UriCopy.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UriCopy.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UriEscape.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UriFile.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UriIp4Base.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UriIp4Base.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UriIp4.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UriMemory.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UriMemory.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UriNormalizeBase.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UriNormalizeBase.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UriNormalize.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UriNormalize.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UriParseBase.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UriParseBase.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UriParse.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UriQuery.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UriRecompose.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UriResolve.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UriShorten.c
)

add_library(uriparser
    ${_URIPARSER_STATIC_OR_SHARED}
    ${API_HEADER_FILES}
    ${LIBRARY_CODE_FILES}
)

add_library(uriparser::uriparser ALIAS uriparser)

if(NOT MSVC)
    math(EXPR URIPARSER_SO_CURRENT_MINUS_AGE "${URIPARSER_SO_CURRENT} - ${URIPARSER_SO_AGE}")
    set_property(TARGET uriparser PROPERTY VERSION ${URIPARSER_SO_CURRENT_MINUS_AGE}.${URIPARSER_SO_AGE}.${URIPARSER_SO_REVISION})
    set_property(TARGET uriparser PROPERTY SOVERSION ${URIPARSER_SO_CURRENT_MINUS_AGE})
    if(WIN32)
        set_target_properties(uriparser PROPERTIES
            OUTPUT_NAME uriparser
            RUNTIME_OUTPUT_NAME uriparser-${URIPARSER_SO_CURRENT_MINUS_AGE}
            ARCHIVE_OUTPUT_NAME uriparser)
    endif()
endif()

set_property(
    TARGET
        uriparser
    PROPERTY
        PUBLIC_HEADER "${API_HEADER_FILES}"
)

target_compile_definitions(uriparser PRIVATE URI_LIBRARY_BUILD)
if (NOT URIPARSER_SHARED_LIBS)
    target_compile_definitions(uriparser PUBLIC URI_STATIC_BUILD)
endif()
if(NOT URIPARSER_BUILD_CHAR)
    target_compile_definitions(uriparser PUBLIC URI_NO_ANSI)
endif()
if(NOT URIPARSER_BUILD_WCHAR_T)
    target_compile_definitions(uriparser PUBLIC URI_NO_UNICODE)
endif()
if(URIPARSER_COMPILER_SUPPORTS_VISIBILITY)
    target_compile_definitions(uriparser PRIVATE URI_VISIBILITY)
endif()

target_include_directories(uriparser
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}  # for UriConfig.h
)

uriparser_install(
    TARGETS
        uriparser
    EXPORT
        uriparser
    ARCHIVE
        DESTINATION
            ${CMAKE_INSTALL_LIBDIR}
    LIBRARY
        DESTINATION
            ${CMAKE_INSTALL_LIBDIR}
    RUNTIME
        DESTINATION
            ${CMAKE_INSTALL_BINDIR}
    PUBLIC_HEADER
        DESTINATION
            ${CMAKE_INSTALL_INCLUDEDIR}/uriparser
)

#
# C command line tool
#
if(URIPARSER_BUILD_TOOLS)
    add_executable(uriparse
        ${CMAKE_CURRENT_SOURCE_DIR}/tool/uriparse.c
    )

    target_link_libraries(uriparse PUBLIC uriparser)

    if(HAIKU)
        # Function inet_ntop needs -lsocket or -lnetwork (see pull request #45)
        check_library_exists(socket inet_ntop "" HAVE_LIBSOCKET__INET_NTOP)
        check_library_exists(network inet_ntop "" HAVE_LIBNETWORK__INET_NTOP)
        if(HAVE_LIBSOCKET__INET_NTOP)
            target_link_libraries(uriparse PUBLIC socket)
        endif()
        if(HAVE_LIBNETWORK__INET_NTOP)
            target_link_libraries(uriparse PUBLIC network)
        endif()
    endif()

    if(WIN32)
        target_link_libraries(uriparse PUBLIC ws2_32)
    endif()

    uriparser_install(
        TARGETS
            uriparse
        DESTINATION
            ${CMAKE_INSTALL_BINDIR}
    )
endif()

#
# C++ test runner
#
if(URIPARSER_BUILD_TESTS)
    if(MSVC AND URIPARSER_MSVC_RUNTIME)
        uriparser_apply_msvc_runtime_to(CMAKE_CXX_FLAGS)
        uriparser_apply_msvc_runtime_to(CMAKE_CXX_FLAGS_DEBUG)
        uriparser_apply_msvc_runtime_to(CMAKE_CXX_FLAGS_RELEASE)
    endif()

    enable_testing()

    find_package(GTest 1.8.0 REQUIRED)

    add_executable(testrunner
        ${CMAKE_CURRENT_SOURCE_DIR}/test/copy.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/FourSuite.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/MemoryManagerSuite.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/test.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/VersionSuite.cpp

        # These library code files have non-public symbols that the test suite
        # needs to link to, so they appear here as well:
        ${API_HEADER_FILES}
        ${LIBRARY_CODE_FILES}
    )

    target_compile_definitions(testrunner PRIVATE URI_STATIC_BUILD)

    if(MSVC)
        target_compile_definitions(testrunner PRIVATE -D_CRT_NONSTDC_NO_WARNINGS)
        target_compile_definitions(testrunner PRIVATE -D_CRT_SECURE_NO_WARNINGS)
    endif()

    target_include_directories(testrunner SYSTEM PRIVATE
        ${GTEST_INCLUDE_DIRS}
    )

    target_include_directories(testrunner PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_BINARY_DIR}  # for UriConfig.h
    )

    target_link_libraries(testrunner PUBLIC
        ${GTEST_BOTH_LIBRARIES}
    )

    # NOTE: uriparser does not use pthreads itself but gtest does
    find_package(Threads REQUIRED)
    target_link_libraries(testrunner PRIVATE Threads::Threads)

    if(MSVC)
        # Specify unwind semantics so that MSVC knowns how to handle exceptions
        target_compile_options(testrunner PRIVATE /EHsc)
    endif()

    if(MINGW)
        set(_URIPARSER_TEST_COMMAND wine testrunner)
    else()
        set(_URIPARSER_TEST_COMMAND      testrunner)
    endif()

    add_test(
        NAME
            test
        COMMAND
            ${_URIPARSER_TEST_COMMAND}
    )

    add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND})
endif()

#
# Fuzzers
#
if(URIPARSER_BUILD_FUZZERS)
    if(NOT "${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
        message(FATAL_ERROR
            "Building fuzz targets without Clang (but ${CMAKE_C_COMPILER_ID}) is not "
            "supported. Please set -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++.")
    endif()

    if(URIPARSER_OSSFUZZ_BUILD AND NOT DEFINED ENV{LIB_FUZZING_ENGINE})
        message(FATAL_ERROR
            "OSS-Fuzz builds require the environment variable "
            "LIB_FUZZING_ENGINE to be set. If you are seeing this "
            "warning, it points to a deeper problem in the OSS-Fuzz "
            "build setup.")
    endif()

    add_library(fuzzparser STATIC ${API_HEADER_FILES} ${LIBRARY_CODE_FILES})
    target_compile_definitions(fuzzparser PUBLIC URI_STATIC_BUILD)
    target_include_directories(fuzzparser
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/include
        PRIVATE
            ${CMAKE_CURRENT_BINARY_DIR}  # for UriConfig.h
    )

    if(NOT URIPARSER_OSSFUZZ_BUILD)
        target_compile_options(fuzzparser PRIVATE -fsanitize=fuzzer-no-link)
    endif()

    function(add_fuzzer target_name source compile_definitions)
        add_executable(${target_name} ${source})
        target_link_libraries(${target_name} PRIVATE fuzzparser)
        target_compile_definitions(${target_name} PRIVATE ${compile_definitions})
        if(URIPARSER_OSSFUZZ_BUILD)
            set_target_properties(${target_name} PROPERTIES LINK_FLAGS $ENV{LIB_FUZZING_ENGINE})
            set_target_properties(${target_name} PROPERTIES LINKER_LANGUAGE "CXX")
        else()
            target_compile_options(${target_name} PRIVATE -fsanitize=fuzzer)
            set_target_properties(${target_name} PROPERTIES LINK_FLAGS -fsanitize=fuzzer)
        endif()
        set_property(TARGET ${target_name} PROPERTY RUNTIME_OUTPUT_DIRECTORY fuzz)
    endfunction()

    function(add_ansi_and_unicode_fuzzer name source)
        if(URIPARSER_BUILD_CHAR)
            add_fuzzer(${name}_fuzzer ${source} URI_NO_UNICODE)
        endif()

        if(URIPARSER_BUILD_WCHAR_T)
            add_fuzzer(${name}w_fuzzer ${source} URI_NO_ANSI)
        endif()
    endfunction()

    add_ansi_and_unicode_fuzzer(uri_dissect_query_malloc fuzz/DissectQueryMallocFuzzer.cpp)
    add_ansi_and_unicode_fuzzer(uri_free fuzz/FreeFuzzer.cpp)
    add_ansi_and_unicode_fuzzer(uri_parse fuzz/ParseFuzzer.cpp)
else()
    if(URIPARSER_OSSFUZZ_BUILD)
        message(SEND_ERROR
                "Attempting to perform an OSS-Fuzz build without turning on the fuzzer build. "
                "This is likely in error - consider adding "
                "-DURIPARSER_BUILD_FUZZERS=ON to your CMake execution.")
    endif()
endif()

#
# Compiler flags
#
if(URIPARSER_WARNINGS_AS_ERRORS)
    if(MSVC)
        add_definitions(/WX)
    else()
        set(URIPARSER_EXTRA_COMPILE_FLAGS "${URIPARSER_EXTRA_COMPILE_FLAGS} -Werror")
    endif()
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${URIPARSER_EXTRA_COMPILE_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${URIPARSER_EXTRA_COMPILE_FLAGS}")

#
# Doxygen API documentation
#
if(URIPARSER_BUILD_DOCS)
    find_package(Doxygen REQUIRED dot doxygen)

    set(QHG_LOCATION "" CACHE FILEPATH "Path to qhelpgenerator program (default: auto-detect)")
    if(NOT QHG_LOCATION)
        find_package(Qt5Help QUIET)
        if(TARGET Qt5::qhelpgenerator)
            get_target_property(QHG_LOCATION Qt5::qhelpgenerator LOCATION)
            mark_as_advanced(Qt5Core_DIR)
            mark_as_advanced(Qt5Gui_DIR)
            mark_as_advanced(Qt5Help_DIR)
            mark_as_advanced(Qt5Sql_DIR)
            mark_as_advanced(Qt5Widgets_DIR)
        endif()
    endif()

    include(FindHTMLHelp)

    # Generate Doxyfile
    if(HTML_HELP_COMPILER)
        set(GENERATE_HTMLHELP YES)
    else()
        set(GENERATE_HTMLHELP NO)
    endif()
    if(QHG_LOCATION)
        set(GENERATE_QHP YES)
    else()
        set(GENERATE_QHP NO)
    endif()
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile.in doc/Doxyfile @ONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/doc/release.sh.in doc/release.sh @ONLY)

    add_custom_target(doc
        ALL
        COMMAND
            ${DOXYGEN_EXECUTABLE}
            Doxyfile
        WORKING_DIRECTORY
            ${CMAKE_CURRENT_BINARY_DIR}/doc
        COMMENT
            "Generating API documentation with Doxygen"
        VERBATIM
    )

    uriparser_install(
        DIRECTORY
            ${CMAKE_CURRENT_BINARY_DIR}/doc/html
        DESTINATION
            ${CMAKE_INSTALL_DOCDIR}
    )
    if(QHG_LOCATION)
        uriparser_install(
            FILES
                ${CMAKE_CURRENT_BINARY_DIR}/doc/uriparser-${PROJECT_VERSION}-doc.qch
            DESTINATION
                ${CMAKE_INSTALL_DOCDIR}
        )
    endif()
endif()

#
# CMake files for find_package(uriparser [..] CONFIG [..])
#
configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/uriparser-config.cmake.in
        cmake/uriparser-config.cmake
    INSTALL_DESTINATION
        ${CMAKE_INSTALL_LIBDIR}/cmake/uriparser-${PROJECT_VERSION}/
)
write_basic_package_version_file(
    cmake/uriparser-config-version.cmake
    COMPATIBILITY SameMajorVersion  # i.e. semver
)
export(
    TARGETS
        uriparser
    FILE
        cmake/uriparser-targets.cmake  # not going to be installed
)
uriparser_install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/cmake/uriparser-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/cmake/uriparser-config-version.cmake
    DESTINATION
        ${CMAKE_INSTALL_LIBDIR}/cmake/uriparser-${PROJECT_VERSION}/
)
uriparser_install(
    EXPORT
        uriparser
    DESTINATION
        ${CMAKE_INSTALL_LIBDIR}/cmake/uriparser-${PROJECT_VERSION}/
    NAMESPACE
        uriparser::
)

#
# pkg-config file
#
if(NOT MSVC)
    if(CMAKE_INSTALL_LIBDIR MATCHES "^/")
        set(_URIPARSER_PKGCONFIG_LIBDIR "${CMAKE_INSTALL_LIBDIR}")
    else()
        set(_URIPARSER_PKGCONFIG_LIBDIR "\${exec_prefix}/${CMAKE_INSTALL_LIBDIR}")
    endif()

    if(CMAKE_INSTALL_INCLUDEDIR MATCHES "^/")
        set(_URIPARSER_PKGCONFIG_INCLUDEDIR "${CMAKE_INSTALL_INCLUDEDIR}")
    else()
        set(_URIPARSER_PKGCONFIG_INCLUDEDIR "\${prefix}/${CMAKE_INSTALL_INCLUDEDIR}")
    endif()

    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/liburiparser.pc.in liburiparser.pc @ONLY)
    uriparser_install(
        FILES
            ${CMAKE_CURRENT_BINARY_DIR}/liburiparser.pc
        DESTINATION
            ${CMAKE_INSTALL_LIBDIR}/pkgconfig/
    )
endif()

#
# Summary
#
message(STATUS "===========================================================================")
message(STATUS "")
message(STATUS "Configuration")
message(STATUS "  Build type ............. ${CMAKE_BUILD_TYPE}")
message(STATUS "  Shared libraries ....... ${URIPARSER_SHARED_LIBS}")
message(STATUS "  Compiler flags")
message(STATUS "    C .................... ${CMAKE_C_FLAGS}")
message(STATUS "    C++ .................. ${CMAKE_CXX_FLAGS}")
message(STATUS "  Linker flags")
message(STATUS "    Executable ........... ${CMAKE_EXE_LINKER_FLAGS}")
message(STATUS "    Module ............... ${CMAKE_MODULE_LINKER_FLAGS}")
message(STATUS "    Shared ............... ${CMAKE_SHARED_LINKER_FLAGS}")
message(STATUS "  Paths")
message(STATUS "    Prefix ............... ${CMAKE_INSTALL_PREFIX}")
message(STATUS "    qhelpgenerator ....... ${QHG_LOCATION}")
message(STATUS "")
message(STATUS "  Features")
message(STATUS "    Code for char * ...... ${URIPARSER_BUILD_CHAR}")
message(STATUS "    Code for wchar_t * ... ${URIPARSER_BUILD_WCHAR_T}")
message(STATUS "    Tools ................ ${URIPARSER_BUILD_TOOLS}")
message(STATUS "    Test suite ........... ${URIPARSER_BUILD_TESTS}")
message(STATUS "    Fuzzers .............. ${URIPARSER_BUILD_FUZZERS}")
message(STATUS "    Documentation ........ ${URIPARSER_BUILD_DOCS}")
message(STATUS "")
if(CMAKE_GENERATOR STREQUAL "Unix Makefiles")
    message(STATUS "Continue with")
    message(STATUS "  make")
    message(STATUS "  make test")
    message(STATUS "  sudo make install")
    message(STATUS "")
endif()
message(STATUS "===========================================================================")
