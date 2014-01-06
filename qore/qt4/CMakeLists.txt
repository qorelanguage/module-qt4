include_directories( ${CMAKE_SOURCE_DIR}/smoke )

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
    typeinfohelpers.cpp
)

SET (module_name "qt4")

add_library(${module_name} SHARED ${qoreqt_LIB_SRCS})
qore_binary_module(${module_name} "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" ${QT_LIBRARIES} qoresmokeqt)

