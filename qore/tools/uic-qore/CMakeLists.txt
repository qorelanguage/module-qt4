
include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/qore )

add_definitions(-DQT_UIC_QORE_GENERATOR)

set(uic_qore_SRCS 
   customwidgetsinfo.cpp 
   databaseinfo.cpp 
   driver.cpp 
   treewalker.cpp 
   ui4.cpp 
   uic.cpp 
   validator.cpp 
   qore/qoreextractimages.cpp 
   qore/qorewritedeclaration.cpp 
   qore/qorewriteicondata.cpp 
   qore/qorewriteicondeclaration.cpp 
   qore/qorewriteiconinitialization.cpp 
   qore/qorewriteinitialization.cpp 
   main.cpp )

add_executable(uicqore ${uic_qore_SRCS})
set_target_properties(uicqore PROPERTIES OUTPUT_NAME uic-qore)
set_target_properties(uicqore PROPERTIES COMPILE_FLAGS -DQT_UIC_RB_GENERATOR)

target_link_libraries(uicqore  ${QT_QTCORE_LIBRARY} ${QT_QTXML_LIBRARY} )

install(TARGETS uicqore DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

