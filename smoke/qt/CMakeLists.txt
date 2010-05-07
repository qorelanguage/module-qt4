
include_directories( ${QT_INCLUDES} ${CMAKE_SOURCE_DIR}/smoke )


# common variable to be used as a main include dir for config.xml.
# It's handled below for some Apple versions.
set(CONFIGXML_INCLUDE_DIR ${QT_INCLUDE_DIR})
# Qt includes for smokegen. It's handled below for some Apple versions.
set(QT_INCLUDES_H qt_includes.h)


# macosx specific tests for frameworks and include paths
if (APPLE)
    # we will use frameworks only if is it specified in paths.
    # It will allow to handle "use libs first" option for macport's qt4-mac too
    # QT_USE_FRAMEWORKS is not sufficient here.
    string(REGEX MATCH "^/Library/Frameworks/.*" USE_FRAMEWORKS ${QT_QTCORE_INCLUDE_DIR})
    if (USE_FRAMEWORKS)
        set(QT_INCLUDES_H qt_includes_macframework.h)
    endif(USE_FRAMEWORKS)

    # HACK to fixup macosx issue with QT_INCLUDE_DIR:
    # QT_INCLUDE_DIR /opt/local/libexec/qt4-mac/include;/opt/local/libexec/qt4-mac/lib/QtCore.framework
    # it should be only:
    # QT_INCLUDE_DIR /opt/local/libexec/qt4-mac/include
    list(LENGTH QT_INCLUDE_DIR QT_INCLUDE_DIR_LENGTH)
    if (QT_INCLUDE_DIR_LENGTH)
        list(GET QT_INCLUDE_DIR 0 CONFIGXML_INCLUDE_DIR) 
    endif (QT_INCLUDE_DIR_LENGTH)

    # debug
    #MESSAGE("QT_INCLUDES_H ${QT_INCLUDES_H}")
    #message("CONFIGXML_INCLUDE_DIR ${CONFIGXML_INCLUDE_DIR}")
    #message("QT_INCLUDE_DIR ${QT_INCLUDE_DIR}")
    #message("QT_QTGUI_INCLUDE_DIR ${QT_QTGUI_INCLUDE_DIR}")
endif (APPLE)


configure_file(config.xml.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.xml @ONLY)

# for QtGuess.txt
if (APPLE)
set(qt_test_threshold 15)
else (APPLE)
set(qt_test_threshold 10)
endif (APPLE)

include(QtGuess.txt)

########### next target ###############

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/smokedata.cpp 
    ${CMAKE_CURRENT_BINARY_DIR}/x_1.cpp 
    ${CMAKE_CURRENT_BINARY_DIR}/x_2.cpp 
    ${CMAKE_CURRENT_BINARY_DIR}/x_3.cpp 
    ${CMAKE_CURRENT_BINARY_DIR}/x_4.cpp 
    ${CMAKE_CURRENT_BINARY_DIR}/x_5.cpp 
    ${CMAKE_CURRENT_BINARY_DIR}/x_6.cpp 
    ${CMAKE_CURRENT_BINARY_DIR}/x_7.cpp 
    ${CMAKE_CURRENT_BINARY_DIR}/x_8.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/x_9.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/x_10.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/x_11.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/x_12.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/x_13.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/x_14.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/x_15.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/x_16.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/x_17.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/x_18.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/x_19.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/x_20.cpp

    COMMAND smokegen
    ARGS -config "${CMAKE_CURRENT_BINARY_DIR}/config.xml" -smokeconfig "${CMAKE_CURRENT_SOURCE_DIR}/smokeconfig.xml"
         -- ${CMAKE_CURRENT_SOURCE_DIR}/${QT_INCLUDES_H}
    DEPENDS smokegen generator_smoke WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

macro_add_file_dependencies( ${CMAKE_CURRENT_BINARY_DIR}/smokedata.cpp ${CMAKE_CURRENT_BINARY_DIR}/x_1.cpp )

include_directories (${CMAKE_SOURCE_DIR}/smoke ${CMAKE_SOURCE_DIR}/smoke/qt  )

set(smokeqt_LIB_SRCS ${CMAKE_CURRENT_BINARY_DIR}/smokedata.cpp  
                ${CMAKE_CURRENT_BINARY_DIR}/x_1.cpp 
                ${CMAKE_CURRENT_BINARY_DIR}/x_2.cpp
                ${CMAKE_CURRENT_BINARY_DIR}/x_3.cpp
                ${CMAKE_CURRENT_BINARY_DIR}/x_4.cpp
                ${CMAKE_CURRENT_BINARY_DIR}/x_5.cpp
                ${CMAKE_CURRENT_BINARY_DIR}/x_6.cpp
                ${CMAKE_CURRENT_BINARY_DIR}/x_7.cpp
                ${CMAKE_CURRENT_BINARY_DIR}/x_8.cpp
                ${CMAKE_CURRENT_BINARY_DIR}/x_9.cpp
                ${CMAKE_CURRENT_BINARY_DIR}/x_10.cpp
                ${CMAKE_CURRENT_BINARY_DIR}/x_11.cpp
                ${CMAKE_CURRENT_BINARY_DIR}/x_12.cpp
                ${CMAKE_CURRENT_BINARY_DIR}/x_13.cpp
                ${CMAKE_CURRENT_BINARY_DIR}/x_14.cpp
                ${CMAKE_CURRENT_BINARY_DIR}/x_15.cpp
                ${CMAKE_CURRENT_BINARY_DIR}/x_16.cpp
                ${CMAKE_CURRENT_BINARY_DIR}/x_17.cpp
                ${CMAKE_CURRENT_BINARY_DIR}/x_18.cpp
                ${CMAKE_CURRENT_BINARY_DIR}/x_19.cpp
                ${CMAKE_CURRENT_BINARY_DIR}/x_20.cpp
)

IF(CMAKE_CXX_FLAGS MATCHES "-fvisibility")
    ADD_DEFINITIONS(-DGCC_VISIBILITY)
ENDIF(CMAKE_CXX_FLAGS MATCHES "-fvisibility")

add_definitions(-DBASE_SMOKE_BUILDING)

# kde4_add_library(smokeqt SHARED ${smokeqt_LIB_SRCS})
add_library(smokeqt SHARED ${smokeqt_LIB_SRCS})

target_link_libraries(smokeqt ${QT_QTCORE_LIBRARY}
                              ${QT_QTGUI_LIBRARY}
                              ${QT_QTNETWORK_LIBRARY}
                              ${QT_QTOPENGL_LIBRARY}
                              ${QT_QTDBUS_LIBRARY}
                              ${QT_QTSQL_LIBRARY}
                              ${QT_QTSVG_LIBRARY}
                              ${QT_QTXML_LIBRARY} )

set_target_properties(smokeqt PROPERTIES VERSION 3.0.0 SOVERSION 3 )

# Don't use the KDE ${LIB_INSTALL_DIR} macro here as this file is
# for Qt only installs
install(TARGETS smokeqt DESTINATION ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX} )
install(FILES ${CMAKE_SOURCE_DIR}/smoke/qt_smoke.h DESTINATION include/smoke)
