cmake_minimum_required(VERSION 3.0)

project(uriparser VERSION 1.21.0)

set(CPACK_PACKAGE_VERSION ${uriparser_VERSION})

include(CheckCSourceCompiles)
include(GNUInstallDirs)
include(CPack)

option(URIPARSER_DISABLE_TESTS "Disable building and running tests" OFF)
option(URIPARSER_BUILD_DOCS "Whether or not to build documentation" OFF)
option(URIPARSER_DISABLE_UNICODE "Disable Unicode support, using ANSI characters instead" OFF)

check_c_source_compiles(
	"
	#include <stdio.h>
	#include <wchar.h>
	int main() {
		wprintf((wchar_t *)0, 0);
		return 0;
	}
	"
	HAVE_WPRINTF
)

########################
# liburiparser library target
########################
set(liburiparser_HEADERS
	include/uriparser/Uri.h
	include/uriparser/UriBase.h
	include/uriparser/UriDefsAnsi.h
	include/uriparser/UriDefsConfig.h
	include/uriparser/UriDefsUnicode.h
	include/uriparser/UriIp4.h)

add_library(liburiparser
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
	${liburiparser_HEADERS})

set_target_properties(liburiparser PROPERTIES OUTPUT_NAME "uriparser")

if (URIPARSER_DISABLE_UNICODE)
	target_compile_definitions(liburiparser PUBLIC -DURI_NO_ANSI)
else ()
	target_compile_definitions(liburiparser PUBLIC -DURI_NO_UNICODE)
endif ()

target_include_directories(liburiparser	PUBLIC include)

set_target_properties(liburiparser PROPERTIES VERSION ${uriparser_VERSION})

########################
# uriparser tool target
########################
if (CMAKE_HOST_WIN32)
	message("uriparse tool not supported on Windows")
else ()
	add_executable(uriparser tool/uriparse.c)

	target_link_libraries(uriparser
		liburiparser)
endif ()

########################
# uriparser tests
########################

# TODO: Integrate CPPTest
if (URIPARSER_BUILD_TESTS AND FALSE)
	add_executable(uriparser-test
		test/FourSuite.cpp
		test/FourSuite.h
		test/VersionSuite.cpp
		test/VersionSuite.h
		test/test.cpp)

	target_link_libraries(uriparser-test liburiparser)
endif()

########################
# install rules
########################
function(generate_pc_file)
	set(prefix ${CMAKE_INSTALL_PREFIX})
	set(exec_prefix ${CMAKE_INSTALL_FULL_LIBEXECDIR})
	set(libdir ${CMAKE_INSTALL_FULL_LIBDIR})
	set(includedir ${CMAKE_INSTALL_FULL_INCLUDEDIR})
	set(VERSION ${uriparser_VERSION})

	configure_file(liburiparser.pc.in liburiparser.pc)
endfunction()

generate_pc_file()
install(TARGETS liburiparser DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${liburiparser_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/uriparser)

if (NOT CMAKE_HOST_WIN32)
	install(TARGETS uriparser DESTINATION ${CMAKE_INSTALL_LIBEXECDIR})
endif ()

install(FILES ${uriparser_BINARY_DIR}/liburiparser.pc DESTINATION ${CMAKE_INSTALL_DATAROOTDIR})