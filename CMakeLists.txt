cmake_minimum_required(VERSION 2.8.4)

# project informations
project(dfc C)
set(PACKAGE ${CMAKE_PROJECT_NAME})
set(VERSION_MAJOR 3)
set(VERSION_MINOR 1)
set(VERSION_PATCH 0)
execute_process(
    COMMAND git rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE VERSION_REVISION
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
# prepend "-" to version_revision
if(VERSION_REVISION)
    set(VERSION_REVISION "-${VERSION_REVISION}")
endif()
set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}${VERSION_REVISION})

# supported platforms
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(BSD true)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "DragonFly")
    set(BSD true)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
    set(BSD true)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(LINUX true)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "NetBSD")
    set(BSD true)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "OpenBSD")
    set(BSD true)
else()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()

# useful defines
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(BUILD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin/${CMAKE_BUILD_TYPE})
set(MAN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/man)
set(CONF_DIR ${CMAKE_CURRENT_SOURCE_DIR}/conf)
set(PO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/po)
include_directories("src")

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

# set compiler flags
# long long is required, all the rest should be c89 compliant
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c89 -Wno-long-long")
include(CheckCCompilerFlag)
check_c_compiler_flag(-Wsign-conversion HAS_WSIGN_CONVERSION)
check_c_compiler_flag(-Wdouble-promotion HAS_WDOUBLE_PROMOTION)
check_c_compiler_flag(-Wlogical-op HAS_WLOGICAL_OP)
check_c_compiler_flag(-Wjump-misses-init HAS_WJUMP_MISSES_INIT)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic -Wconversion -Wstrict-prototypes -Wcast-qual -Wcast-align -Wshadow -Wredundant-decls -Wundef -Wfloat-equal -Wmissing-include-dirs -Wswitch-default -Wswitch-enum -Wpointer-arith -Wbad-function-cast -Wnested-externs -Wold-style-definition -Wformat=2 -Winit-self -Wwrite-strings -Wmissing-prototypes -pipe")
if(HAS_WSIGN_CONVERSION)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wsign-conversion")
endif()
if(HAS_WDOUBLE_PROMOTION)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wdouble-promotion")
endif()
if(HAS_WLOGICAL_OP)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wlogical-op")
endif()
if(HAS_WJUMP_MISSES_INIT)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wjump-misses-init")
endif()

set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS} -O3 -flto -DNDEBUG")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS} -g3 -ggdb3 -Wpadded -Wpacked")

if(CMAKE_COMPILER_IS_GNUCC)
    # gcc warns about strftime format when using std=c89. According to
    # strftime(3), it is safe to remove this warning
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-format-y2k")
endif()


# platform specific source files
if(BSD)
    set(SERVICE_SRC_FILE "${SOURCE_DIR}/platform/services-bsd.c")
elseif(LINUX)
    set(SERVICE_SRC_FILE "${SOURCE_DIR}/platform/services-linux.c")
else()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()

set(EXECUTABLE_NAME ${CMAKE_PROJECT_NAME})
SET(SRCS
    ${SOURCE_DIR}/dotfile.c
    ${SOURCE_DIR}/dfc.c
    ${SOURCE_DIR}/list.c
    ${SOURCE_DIR}/util.c
    ${SOURCE_DIR}/export/csv.c
    ${SOURCE_DIR}/export/html.c
    ${SOURCE_DIR}/export/tex.c
    ${SOURCE_DIR}/export/text.c
    ${SERVICE_SRC_FILE}
)
add_executable(
    ${EXECUTABLE_NAME}
    ${SRCS}
)


if(LFS_ENABLED)
    add_definitions(-D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64)
endif()

if(NLS_ENABLED)
    add_definitions(-DNLS_ENABLED)
    # load gettext module for translation
    set(GettextTranslate_ALL true)
    set(GettextTranslate_GMO_BINARY true)
    include(GettextTranslate)
    add_subdirectory(po)

    include_directories(${LIBINTL_INCLUDE_DIR})
    target_link_libraries(${EXECUTABLE_NAME} ${LIBINTL_LIBRARIES})
endif()

# link libraries
target_link_libraries(${EXECUTABLE_NAME} m)

add_definitions(-DPACKAGE="${PACKAGE}" -DVERSION="${VERSION}" -DLOCALEDIR="${LOCALEDIR}")

set(DFC_DOC_FILES
    AUTHORS.md
    CHANGELOG.md
    HACKING.md
    LICENSE
    README.md
    TRANSLATORS.md
)

mark_as_advanced(CMAKE_INSTALL_PREFIX)

set(DFC_SYSCONFDIR ${XDG_CONFIG_DIR}/${PACKAGE})

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
    "${CPACK_SOURCE_IGNORE_FILES}")
include(CPack)
add_custom_target(dist
    COMMAND ${CMAKE_MAKE_PROGRAM} package_source)

# installation
install(TARGETS ${EXECUTABLE_NAME} RUNTIME DESTINATION bin)
install(FILES ${MAN_DIR}/dfc.1 DESTINATION ${MAN_PATH}/man1)
install(FILES ${CONF_DIR}/dfcrc DESTINATION ${DFC_SYSCONFDIR})
if(NLS_ENABLED)
    foreach(lang ${languages})
        install(FILES ${MAN_DIR}/${lang}/${PACKAGE}.1 DESTINATION ${MAN_PATH}/${lang}/man1)
        install(FILES ${CONF_DIR}/${lang}/dfcrc DESTINATION ${DFC_SYSCONFDIR}/${lang})
    endforeach()
endif()
install(FILES ${DFC_DOC_FILES} DESTINATION ${DFC_DOC_PATH})

# uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P
    ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
