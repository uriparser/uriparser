cmake_minimum_required(VERSION 2.6)

project(UriParser)

set(VERSION_STRING "0.9.0")
IF(EXISTS "${CMAKE_SOURCE_DIR}/.git")
    FIND_PROGRAM(GIT git)
    IF(GIT)
        EXECUTE_PROCESS(
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMAND ${GIT} describe --tags
            OUTPUT_VARIABLE GIT_VERSION  OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        if(NOT GIT_VERSION)
                EXECUTE_PROCESS(
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                    COMMAND ${GIT} rev-parse --short HEAD
                    OUTPUT_VARIABLE GIT_VERSION  OUTPUT_STRIP_TRAILING_WHITESPACE
                )
        endif()
        string(REPLACE "uriparser-" "" GIT_VERSION ${GIT_VERSION})
        SET(VERSION_STRING ${GIT_VERSION})
    ENDIF(GIT)
ENDIF()
MESSAGE("Version: ${VERSION_STRING}\n")
set(URIPARSER_VERSION ${VERSION_STRING})

include(CMakePackageConfigHelpers)
include(GNUInstallDirs OPTIONAL)
include(GenerateExportHeader)
include(CheckIncludeFile)
include(CheckFunctionExists)

check_include_file("wchar.h" HAVE_WCHAR_H)
check_include_file("stdio.h" HAVE_STDIO_H)
check_include_file("assert.h" HAVE_ASSERT_H)
check_include_file("config.h" HAVE_CONFIG_H)
check_include_file("ctype.h" HAVE_CTYPE_H)
check_include_file("errno.h" HAVE_ERRNO_H)
check_include_file("limits.h" HAVE_LIMITS_H)
check_include_file("stdlib.h" HAVE_STDLIB_H)
check_include_file("string.h" HAVE_STRING_H)

if(WIN32)
    check_include_file("winsock2.h" HAVE_WINSOCK2_H)
    check_include_file("ws2tcpip.h" HAVE_WS2TCPIP_H)
else(WIN32)
    check_include_file("sys/socket.h" HAVE_SOCKET_H)
    check_include_file("arpa/inet.h" HAVE_INET_H)
    check_include_file("netinet/in.h" HAVE_IN_H)
endif(WIN32)

check_function_exists(wprintf HAVE_WPRINTF)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

set(URIPARSER_SRCS
  src/UriCommon.c
  src/UriCommon.h
  src/UriCompare.c
  src/UriEscape.c
  src/UriFile.c
  src/UriIp4.c
  src/UriIp4Base.c
  src/UriIp4Base.h
  src/UriNormalize.c
  src/UriNormalizeBase.c
  src/UriNormalizeBase.h
  src/UriParse.c
  src/UriParseBase.c
  src/UriParseBase.h
  src/UriQuery.c
  src/UriRecompose.c
  src/UriResolve.c
  src/UriShorten.c
  src/UriMemory.c
  src/UriMemory.h
  )

set(URIPARSER_INCS
  include/uriparser/Uri.h
  include/uriparser/UriBase.h
  include/uriparser/UriDefsAnsi.h
  include/uriparser/UriDefsConfig.h
  include/uriparser/UriDefsUnicode.h
  include/uriparser/UriIp4.h
  )

set(LIB_NAME uriparser)
add_library(${LIB_NAME} ${URIPARSER_SRCS})
set_target_properties(${LIB_NAME} PROPERTIES SOVERSION ${VERSION_STRING})
target_include_directories(${LIB_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/include ${CMAKE_CURRENT_BINARY_DIR})

option(DISABLE_CHAR "disable char: -DURI_NO_ANSI" OFF)
option(DISABLE_WCHAR_T "disable wchar_t: -DURI_NO_UNICODE " OFF)
if(DISABLE_WCHAR_T AND DISABLE_CHAR)
   message(FATAL_ERROR "DISABLE_WCHAR_T and DISABLE_CHAR are mutually exclusive")
endif()
if(DISABLE_CHAR)
    add_compile_options(-DURI_NO_ANSI)
endif()
if(DISABLE_WCHAR_T)
    add_compile_options(-DURI_NO_UNICODE)
endif()

IF(MSVC)
    # This option is to enable the /MP switch for Visual Studio 2005 and above compilers
    OPTION(WIN32_USE_MP "Set to ON to build with the /MP option (Visual Studio 2005 and above)." ON)
    MARK_AS_ADVANCED(WIN32_USE_MP)
    IF(WIN32_USE_MP)
        add_compile_options(/MP)
    ENDIF(WIN32_USE_MP)
ENDIF(MSVC)

if (BUILD_SHARED_LIBS)
    if (CMAKE_COMPILER_IS_GNUCXX)
       # Just setting CMAKE_POSITION_INDEPENDENT_CODE should be enough to set
       # -fPIC for GCC but sometimes it still doesn't get set, so make sure it
       # does.
       #add_definitions("-fPIC")
       target_compile_definitions(${LIB_NAME} PUBLIC "-fPIC")
    endif()
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)   
endif()

GENERATE_EXPORT_HEADER(${LIB_NAME})

set(INSTALL_CMAKE_DIR CMake CACHE PATH
    "Installation directory for CMake files")

export(TARGETS ${LIB_NAME}
  FILE "${CMAKE_BINARY_DIR}/UriParserTargets.cmake")

# Install uriparser library
install(TARGETS ${LIB_NAME}
    EXPORT UriParserTargets
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    )
install(FILES
            ${URIPARSER_INCS}
            ${CMAKE_CURRENT_BINARY_DIR}/uriparser_export.h
        DESTINATION
            "${CMAKE_INSTALL_INCLUDEDIR}/uriparser"
    )

# PkgConfig configure file
if(NOT MSVC)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/liburiparser.pc.in
        ${CMAKE_CURRENT_BINARY_DIR}/liburiparser.pc @ONLY)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/liburiparser.pc
            DESTINATION
                "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
endif()

# Cmake configure files
configure_package_config_file(cmake/UriParserConfig.cmake.in
    "${CMAKE_BINARY_DIR}/UriParserConfig.cmake"
    INSTALL_DESTINATION ${INSTALL_CMAKE_DIR})
write_basic_package_version_file(
    "${CMAKE_BINARY_DIR}/UriParserConfigVersion.cmake"
    VERSION ${URIPARSER_VERSION}
    COMPATIBILITY AnyNewerVersion)
install(FILES
    "${CMAKE_BINARY_DIR}/UriParserConfig.cmake"
    "${CMAKE_BINARY_DIR}/UriParserConfigVersion.cmake"
    DESTINATION ${INSTALL_CMAKE_DIR}
  )

# Unistall
CONFIGURE_FILE(
    "${CMAKE_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
    "${CMAKE_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)
ADD_CUSTOM_TARGET(uninstall
    "${CMAKE_COMMAND}" -P "${CMAKE_BINARY_DIR}/cmake_uninstall.cmake")

# Test
option(BUILD_TESTS "Build tests." ON)
if(BUILD_TESTS)
    enable_testing()
    find_package(GTest 1.8.1 REQUIRED NO_CMAKE_PATH)

    check_function_exists(asprintf HAVE_ASPRINTF)
    
    SET(TEST_SOURCES
        test/FourSuite.cpp 
        test/MemoryManagerSuite.cpp 
        test/VersionSuite.cpp 
        test/test.cpp)
    add_executable(testUriParser ${TEST_SOURCES})
    target_link_libraries(testUriParser GTest::gtest ${LIB_NAME})
    target_compile_definitions(testUriParser PUBLIC PACKAGE_VERSION="${GTest_VERSION}")

    add_test(AlltestUriParser testUriParser)
endif(BUILD_TESTS)
