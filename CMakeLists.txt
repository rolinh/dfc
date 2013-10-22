cmake_minimum_required(VERSION 2.8.4)

# project informations
project(dfc C)
set(PACKAGE ${CMAKE_PROJECT_NAME})
set(VERSION_MAJOR 3)
set(VERSION_MINOR 0)
set(VERSION_PATCH 4)
set(VERSION_BRANCH "")
set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}${VERSION_BRANCH})

# useful defines
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(BUILD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin/${CMAKE_BUILD_TYPE})
set(MAN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/man)
set(CONF_DIR ${CMAKE_CURRENT_SOURCE_DIR}/conf)
set(PO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/po)

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

# share directory
if(DEFINED SHAREDIR)
	set(SHAREDIR ${SHAREDIR} CACHE PATH "share directory")
else()
	set(SHAREDIR ${PREFIX}/share CACHE PATH "share directory")
endif()

# locale
if(DEFINED LOCALEDIR)
	set(LOCALEDIR ${LOCALEDIR} CACHE PATH "locale directory")
else()
	set(LOCALEDIR ${SHAREDIR}/locale CACHE PATH "locale directory")
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
	set(MAN_PATH ${SHAREDIR}/man CACHE PATH "manpage directory")
endif()

# doc path
if(DEFINED DOC_PATH)
	set(DFC_DOC_PATH ${DFC_DOC_PATH} CACHE PATH "doc directory")
else()
	set(DFC_DOC_PATH ${SHAREDIR}/doc/${PACKAGE} CACHE PATH "doc directory")
endif()

# Check for libintl
find_package(Libintl)
if (LIBINTL_FOUND)
	option(NLS_ENABLED "Translation support with gettext" on)
else()
	option(NLS_ENABLED "Translation support with gettext" off)
endif()

option(LFS_ENABLED "Enable macros for Large File Source. Required on 32-bit systems but should not cause any problems if defined on non 32-bit systems anyway, thus enabled by default." on)
# CFLAGS (using c99 only for long long, all the rest should be c89 compliant)
if(LFS_ENABLED)
    add_definitions(-O2 -std=c99)
else()
    add_definitions(-O2 -std=c99 -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64)
endif()

option(GRIM "Very strict compiler options (use it to debug dfc)" off)
if(GRIM)
	set(CMAKE_BUILD_TYPE GRIM CACHE STRING "Very strict compiler options")
	include(CheckCCompilerFlag)
	check_c_compiler_flag(-Wsign-conversion HAS_WSIGN_CONVERSION)
	check_c_compiler_flag(-Wdouble-promotion HAS_WDOUBLE_PROMOTION)
	check_c_compiler_flag(-Wlogical-op HAS_WLOGICAL_OP)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -g -pedantic -Wall -Wextra -Wconversion -Wstrict-prototypes -Wcast-qual -Wcast-align -Wshadow -Wredundant-decls -Wundef -Wfloat-equal -Wmissing-include-dirs -Wswitch-default -Wswitch-enum -Wpointer-arith -Wbad-function-cast -Wnested-externs -Wold-style-definition")
	if (HAS_WSIGN_CONVERSION)
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wsign-conversion")
	endif()
	if (HAS_WDOUBLE_PROMOTION)
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wdouble-promotion")
	endif()
	if (HAS_WLOGICAL_OP)
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wlogical-op")
	endif()
else()
	set(CMAKE_BUILD_TYPE RELEASE CACHE STRING "Build optimized for release")
endif()

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
	${SOURCE_DIR}/tex.c
	${SOURCE_DIR}/text.c
	${SOURCE_DIR}/util.c
)
add_executable(
	${EXECUTABLE_NAME}
	${SRCS}
)

if(NLS_ENABLED)
	# load gettext module for translation
	set(GettextTranslate_ALL true)
	set(GettextTranslate_GMO_BINARY true)
	include(GettextTranslate)
	add_subdirectory(po)

	include_directories(${LIBINTL_INCLUDE_DIR})
	target_link_libraries(${EXECUTABLE_NAME} ${LIBINTL_LIBRARIES})
endif()

set(DFC_DOC_FILES
	AUTHORS
    CHANGELOG
	HACKING
	LICENSE
	README
	TRANSLATORS
)

mark_as_advanced(CMAKE_INSTALL_PREFIX)

set(DFC_SYSCONFDIR	${XDG_CONFIG_DIR}/${PACKAGE})

file(STRINGS ${PO_DIR}/LINGUAS LINGUAS REGEX "^[^#].*")
string(REGEX MATCHALL "[^ ]+" languages ${LINGUAS})

# rule to generate a source tarball
set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${VERSION}")
set(CPACK_SOURCE_IGNORE_FILES
	"~$"
    ".directory"
	".swp$"
	"^${PROJECT_SOURCE_DIR}/.git/"
	"^${PROJECT_SOURCE_DIR}/build/"
	"${PROJECT_SOURCE_DIR}/.gitignore$"
	"${PROJECT_SOURCE_DIR}/po/dfc.pot$"
	"${PROJECT_SOURCE_DIR}/src/extern.h$"
	"${CPACK_SOURCE_IGNORE_FILES}")
include(CPack)
add_custom_target(dist
	COMMAND ${CMAKE_MAKE_PROGRAM} package_source)

# installation
install(TARGETS ${EXECUTABLE_NAME} RUNTIME DESTINATION bin)
install(FILES ${MAN_DIR}/dfc.1 DESTINATION ${MAN_PATH}/man1)

if(NLS_ENABLED)
    foreach(lang ${languages})
        install(FILES ${MAN_DIR}/${lang}/${PACKAGE}.1 DESTINATION ${MAN_PATH}/${lang}/man1)
    endforeach()
endif()
install(DIRECTORY ${CONF_DIR}/ DESTINATION ${DFC_SYSCONFDIR})
install(FILES ${DFC_DOC_FILES} DESTINATION ${DFC_DOC_PATH})

# uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P
	${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
