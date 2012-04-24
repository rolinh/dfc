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

set(LOCALE_DIR "${CMAKE_INSTALL_PREFIX}/share/locale")
add_definitions(
	-DLOCALEDIR="${LOCALE_DIR}"
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

set(EXECUTABLE_NAME ${CMAKE_PROJECT_NAME})
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

set(DFC_DOC_FILES
	AUTHORS
	HACKING
	LICENSE
	README
	TRANSLATORS
)

# load gettext module for translation
set(GettextTranslate_ALL true)
set(GettextTranslate_GMO_BINARY true)
include(GettextTranslate)
add_subdirectory(po)

# defines for installation
if(DEFINED PREFIX)
	set(PREFIX ${PREFIX} CACHE PATH "install prefix")
	set(CMAKE_INSTALL_PREFIX ${PREFIX})
else()
	set(PREFIX ${CMAKE_INSTALL_PREFIX} CACHE PATH "install prefix")
endif()

# use a sysconfdir if specified
if(DEFINED SYSCONFDIR)
	set(SYSCONFDIR ${SYSCONFDIR} CACHE PATH "config directory")
else()
	set(SYSCONFDIR ${PREFIX}/etc CACHE PATH "config directory")
endif()

# locale
if(DEFINED LOCALEDIR)
	set(LOCALEDIR ${LOCALEDIR} CACHE PATH "locale directory")
else()
	set(LOCALEDIR ${PREFIX}/share/locale CACHE PATH "locale directory")
endif()

# if XDG Config Dir is specified, use it
if(DEFINED XDG_CONFIG_DIR)
	set(XDG_CONFIG_DIR ${XDG_CONFIG_DIR} CACHE PATH "xdg config directory")
else()
	set(XDG_CONFIG_DIR ${SYSCONFDIR}/xdg CACHE PATH "xdf config directory")
endif()

# man path
if(DEFINED MAN_PATH)
	set(MAN_PATH ${MAN_PATH} CACHE PATH "manpage directory")
else()
	set(MAN_PATH ${PREFIX}/share/man CACHE PATH "manpage directory")
endif()

# doc path
if(DEFINED DOC_PATH)
	set(DFC_DOC_PATH ${DFC_DOC_PATH} CACHE PATH "doc directory")
else()
	set(DFC_DOC_PATH ${PREFIX}/share/doc/${PACKAGE} CACHE PATH "doc directory")
endif()

mark_as_advanced(CMAKE_INSTALL_PREFIX)

set(DFC_SYSCONFDIR	${XDG_CONFIG_DIR}/${PACKAGE})

file(STRINGS ${PO_DIR}/LINGUAS LINGUAS REGEX "^[^#].*")
string(REGEX MATCHALL "[^ ]+" languages ${LINGUAS})

# installation
install(TARGETS ${EXECUTABLE_NAME} RUNTIME DESTINATION bin)
install(FILES ${MAN_DIR}/dfc.1 DESTINATION ${MAN_PATH}/man1)
foreach(lang ${languages})
	install(FILES ${MAN_DIR}/${lang}/${PACKAGE}.1 DESTINATION ${MAN_PATH}/${lang}/man1)
endforeach()
install(DIRECTORY ${CONF_DIR}/ DESTINATION ${DFC_SYSCONFDIR})
install(FILES ${DFC_DOC_FILES} DESTINATION ${DFC_DOC_PATH})
