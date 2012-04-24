cmake_minimum_required(VERSION 2.8.0)

# project informations
project(dfc)
set(PACKAGE ${CMAKE_PROJECT_NAME})
set(VERSION_MAJOR 3)
set(VERSION_MINOR 0)
set(VERSION_PATCH 0)
set(VERSION_BRANCH "devel")
set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}-${VERSION_BRANCH})

# useful defines
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMakeModules")
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(BUILD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin/${CMAKE_BUILD_TYPE})
set(MAN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/man)
set(CONF_DIR ${CMAKE_CURRENT_SOURCE_DIR}/conf)
set(PO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/po)

add_definitions(
	-DPACKAGE_BIN_DIR="${CMAKE_INSTALL_PREFIX}/bin"
	-DPACKAGE_DATA_DIR="${CMAKE_INSTALL_PREFIX}/share/${CMAKE_PROJECT_NAME}"
)
set(LOCALE_DIR "${CMAKE_INSTALL_PREFIX}/locale")
add_definitions(
	-DLOCALEDIR="${LOCALEDIR}"
)

set(CMAKE_BUILD_TYPE RELEASE)

# CFLAGS
add_definitions(-O2 -std=c89)

if(CMAKE_COMPILER_IS_GNUCC)
	option(GRIM "Very strict compiler options" off)
	if(GRIM)
		set(CMAKE_BUILD_TYPE GRIM)
		add_definitions(
			-g
			-pedantic
			-Wall
			-Wunused-parameter
			-Wlong-long
			-Wconversion
			-Wsign-conversion
			-Wimplicit-function-declaration
			-Wstrict-prototypes)
	endif(GRIM)
endif(CMAKE_COMPILER_IS_GNUCC)

configure_file(
	${SOURCE_DIR}/extern.h.in
	${SOURCE_DIR}/extern.h
)

set(EXECUTABLE_NAME dfc)
SET(SRCS
	${SOURCE_DIR}/csv.c
	${SOURCE_DIR}/dotfile.c
	${SOURCE_DIR}/dfc.c
	${SOURCE_DIR}/html.c
	${SOURCE_DIR}/list.c
	#${SOURCE_DIR}/tex.c
	${SOURCE_DIR}/text.c
	${SOURCE_DIR}/util.c
)
add_executable(
	${EXECUTABLE_NAME}
	${SRCS}
)

# load gettext module for translation
set(GettextTranslate_ALL true)
set(GettextTranslate_GMO_BINARY true)
include(GettextTranslate)
add_subdirectory(po)
