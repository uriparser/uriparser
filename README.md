[![Build and test (Linux)](https://github.com/uriparser/uriparser/actions/workflows/build-and-test.yml/badge.svg)](https://github.com/uriparser/uriparser/actions/workflows/build-and-test.yml)
[![Build and test (Windows)](https://github.com/uriparser/uriparser/actions/workflows/windows.yml/badge.svg)](https://github.com/uriparser/uriparser/actions/workflows/windows.yml)


# uriparser

uriparser is a
strictly [RFC 3986](https://datatracker.ietf.org/doc/html/rfc3986) compliant
URI parsing and handling library
written in C89 ("ANSI C").
uriparser is cross-platform,
fast,
supports both `char` and `wchar_t`, and
is licensed under the [BSD-3-Clause license](https://spdx.org/licenses/BSD-3-Clause.html)
(except for the test suite
that is licensed under the
[LGPL-2.1-or-later license](https://spdx.org/licenses/LGPL-2.1-or-later.html)
and for the fuzzing code
that is licensed under the
[Apache-2.0 license](https://spdx.org/licenses/Apache-2.0.html)).

To learn more about uriparser,
please check out [https://uriparser.github.io/](https://uriparser.github.io/).


# Example use from an existing CMake project

```cmake
cmake_minimum_required(VERSION 3.5.0)

project(hello VERSION 1.0.0)

find_package(uriparser 0.9.2 CONFIG REQUIRED char wchar_t)

add_executable(hello
    hello.c
)

target_link_libraries(hello PUBLIC uriparser::uriparser)
```


# Compilation

## Compilation (standalone, GNU make, Linux)
```console
# mkdir build
# cd build
# cmake -DCMAKE_BUILD_TYPE=Release ..  # see CMakeLists.txt for options
# make
# make test
# make install
```

## Available CMake options (and defaults)
```console
# rm -f CMakeCache.txt ; cmake -LH . | grep -B1 ':.*=' | sed 's,--,,'
// Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel ...
CMAKE_BUILD_TYPE:STRING=

// Install path prefix, prepended onto install directories.
CMAKE_INSTALL_PREFIX:PATH=/usr/local

// Path to qhelpgenerator program (default: auto-detect)
QHG_LOCATION:FILEPATH=

// Build code supporting data type 'char'
URIPARSER_BUILD_CHAR:BOOL=ON

// Build API documentation (requires Doxygen, Graphviz, and (optional) Qt's qhelpgenerator)
URIPARSER_BUILD_DOCS:BOOL=ON

// Build test suite (requires GTest >=1.8.0)
URIPARSER_BUILD_TESTS:BOOL=ON

// Build fuzzers (requires Clang)
URIPARSER_BUILD_FUZZERS:BOOL=OFF

// Build fuzzers via OSS-Fuzz
URIPARSER_OSSFUZZ_BUILD:BOOL=OFF

// Build tools (e.g. CLI "uriparse")
URIPARSER_BUILD_TOOLS:BOOL=ON

// Build code supporting data type 'wchar_t'
URIPARSER_BUILD_WCHAR_T:BOOL=ON

// Enable installation of uriparser
URIPARSER_ENABLE_INSTALL:BOOL=ON

// Use /MT flag (static CRT) when compiling in MSVC
URIPARSER_MSVC_STATIC_CRT:BOOL=OFF

// Build shared libraries (rather than static ones)
URIPARSER_SHARED_LIBS:BOOL=ON

// Treat all compiler warnings as errors
URIPARSER_WARNINGS_AS_ERRORS:BOOL=OFF
```
