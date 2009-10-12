#
# to use the file to build a Qt only version of PHP-Qt, copy it to CMakeLists.txt
#
# make a build/ dir, step into
# run: cmake ..
# run: make
#
# for further information read the README file in php/phpqt
#

cmake_minimum_required(VERSION 2.6)

project(Qore-Qt)

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules )

set(CMAKE_BUILD_TYPE Debug)


# HACK for snow leopard - TODO/FIXME!
#IF (APPLE)
#    SET(CMAKE_OSX_ARCHITECTURES "i386")
#    SET(CMAKE_CXX_FLAGS_DEBUG "-g -arch i386")
#ENDIF (APPLE)
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")


MESSAGE(STATUS "*********************************************************")
MESSAGE(STATUS "This is module under development.")
MESSAGE(STATUS "Only debug build with custom Smoke code is built for now.")
MESSAGE(STATUS "*********************************************************\n")


IF (NOT LIB_SUFFIX)
    MESSAGE(STATUS "")
    MESSAGE(STATUS "LIB_SUFFIX variable is not defined. It will be autodetected now.")
    MESSAGE(STATUS "You can set it manually with -DLIB_SUFFIX=<value> (64 for example)")
    IF (NOT APPLE)
        # check 64 bit
        if (CMAKE_SIZEOF_VOID_P EQUAL 4)
            SET (LIB_SUFFIX "")
        else (CMAKE_SIZEOF_VOID_P EQUAL 4)
            set (LIB_SUFFIX "64")
            MESSAGE(STATUS "   LIB_SUFFIX is set to '${LIB_SUFFIX}'")
        endif (CMAKE_SIZEOF_VOID_P EQUAL 4)
    ELSE (NOT APPLE)
        SET (LIB_SUFFIX "")
    ENDIF (NOT APPLE)
    MESSAGE(STATUS "")
ENDIF (NOT LIB_SUFFIX)


FIND_PACKAGE(Perl REQUIRED)
FIND_PACKAGE (Qt4 REQUIRED)
INCLUDE( ${QT_USE_FILE} )
MESSAGE(STATUS Qt4 version: ${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}.${QT_VERSION_PATCH})
#MESSAGE(STATUS "Qt libs ${QT_LIBRARIES}")
#MESSAGE(STATUS "Qt incl ${QT_INCLUDES}")
#MESSAGE(STATUS "Qt defs ${QT_DEFINITIONS}")

add_definitions (${QT_DEFINITIONS}  -DHAVE_CONFIG_H=1)
include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${QT_INCLUDES} )

FIND_PACKAGE (Qore REQUIRED)
FIND_PACKAGE (OpenGL REQUIRED)

OPTION(ENABLE_SMOKE "build Smoke" ON)
OPTION(ENABLE_QTWEBKIT_SMOKE "build QtWebKitSmoke" OFF)
OPTION(ENABLE_QTSCRIPT_SMOKE "build QtScriptSmoke" OFF)
OPTION(ENABLE_QTUITOOLS_SMOKE "build QtUiToolsSmoke" OFF)
OPTION(ENABLE_PHONON_SMOKE "build PhononSmoke" OFF)
OPTION(ENABLE_QSCI_SMOKE "build QsciSmoke" OFF)
OPTION(ENABLE_QWT_SMOKE "build QwtSmoke" OFF)
OPTION(ENABLE_KDE_SMOKE "build KDESmoke" OFF)
OPTION(ENABLE_KDEVPLATFORM_SMOKE "build KDevPlatformSmoke" OFF)
OPTION(ENABLE_KHTML_SMOKE "build KHTMLSmoke" OFF)
OPTION(ENABLE_KTEXTEDITOR_SMOKE "build KTextEditorSmoke" OFF)
OPTION(ENABLE_SOLID_SMOKE "build SolidSmoke" OFF)
OPTION(ENABLE_PLASMA "build Plasma" OFF)
OPTION(ENABLE_QTTEST_SMOKE "build qttest smoke" OFF)
OPTION(ENABLE_QTSCRIPT "build qtscript" OFF)
OPTION(ENABLE_QTTEST "build qttest" OFF)
OPTION(ENABLE_QIMAGEBLITZ_SMOKE "build qimageblitz smoke" OFF)

set(kdebindings_SOURCE_DIR ${CMAKE_SOURCE_DIR})
add_subdirectory(generator)
add_subdirectory(smoke)
add_subdirectory(qore)

IF (APPLE)
MESSAGE(STATUS "______________________________")
MESSAGE(STATUS "System: ${CMAKE_SYSTEM}")
MESSAGE(STATUS "Archs: ${CMAKE_OSX_ARCHITECTURES}")
MESSAGE(STATUS "CXX: ${CMAKE_CXX_COMPILER}")
MESSAGE(STATUS "FLags: ${CMAKE_CXX_FLAGS_DEBUG}")
MESSAGE(STATUS "______________________________")
ENDIF (APPLE)
