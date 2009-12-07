add_definitions("-DPACKAGE_VERSION=\"0.1\"")
add_definitions("-DDEBUG=1")

include_directories( ${CMAKE_SOURCE_DIR}/smoke
                     ${QORE_INCLUDE_DIR} )

include_directories( ${QT_INCLUDES} )


set(qoreqt_LIB_SRCS
    qore-main.cpp
    
    qoresmokebinding.cpp
    qoresmokeclass.cpp
    qoremarshalling.cpp
    commonqoremethod.cpp
    commonargument.cpp
    qoreqtenumnode.cpp
    qtfunctions.cpp
    qoreqtdynamicmethod.cpp
)

SET (module_name "qt4")

add_library(${module_name} SHARED ${qoreqt_LIB_SRCS})
set_target_properties(${module_name} PROPERTIES PREFIX "" SUFFIX "-api-0.8.qmod")

#find_library(smokeqtlib NAMES smokeqt PATHS /export/home/pvanek/work/qore-qt/LIB/lib NO_DEFAULT_PATH)
target_link_libraries(${module_name} ${QT_LIBRARIES} ${QORE_LIBRARY} smokeqt )#${smokeqtlib})

# install( TARGETS ${module_name} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}/qore-modules )
install( TARGETS ${module_name} DESTINATION ${QORE_MODULES_DIR})
