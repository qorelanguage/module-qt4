cmake_minimum_required(VERSION 2.8.3)

project(qore-qt4-module)

# for now we are still incompatible with the current Smoke trunk. It will be merged later
OPTION(ENABLE_BUILTIN_SMOKE "Use builtin Smoke build" ON)
# changing these options is dangerous for now
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

set (VERSION_MAJOR 0)
set (VERSION_MINOR 1)
set (VERSION_PATCH 1)

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules )



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



FIND_PACKAGE (Qt4 REQUIRED)
INCLUDE( ${QT_USE_FILE} )
MESSAGE(STATUS "Qt4 version: ${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}.${QT_VERSION_PATCH}")

add_definitions (${QT_DEFINITIONS}  -DHAVE_CONFIG_H=1)
include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${QT_INCLUDES} )

FIND_PACKAGE (Qore REQUIRED)
FIND_PACKAGE (OpenGL REQUIRED)


# additional compiler variables like for MSVC or CYGWIN
IF (CMAKE_SYSTEM MATCHES "SunOS*")
    SET(SUN 1)
    # stlport4 library is mandatory here to simulete gcc extensions
    SET(CMAKE_CXX_FLAGS_DEBUG "-g -library=stlport4")
ELSE()
    SET(SUN 0)
ENDIF()


# set(kdebindings_SOURCE_DIR ${CMAKE_SOURCE_DIR})
if (ENABLE_BUILTIN_SMOKE)
    add_subdirectory(generator)
    add_subdirectory(smoke)
endif (ENABLE_BUILTIN_SMOKE)

add_subdirectory(qore)


qore_dist("${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
qore_config_info()

