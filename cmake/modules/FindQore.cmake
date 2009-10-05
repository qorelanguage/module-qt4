#
# Cmake module for Qore programming language
#
# (c) 2009 Qore Technologies, http://qoretechnologies.com
#
# QORE_FOUND - system has it
# QORE_INCLUDE_DIR 
# QORE_LIBRARY
# QORE_MODULES_DIR

SET(QORE_FOUND FALSE)


FIND_PATH (QORE_INCLUDE_DIR qore/Qore.h
    /usr/include
    /usr/include/qore
    /opt/qore/include/qore
    /opt/qore/include
    ${QORE_INCLUDE_PATH}
)

FIND_LIBRARY (QORE_LIBRARY
    NAMES libqore libqore.so libqore.dylib
    PATHS /usr/local/lib
	  /usr/lib
	  /opt/qore/lib
	  ${QORE_LIBRARY_PATH}
    )

IF (QORE_LIBRARY AND QORE_INCLUDE_DIR)
    SET(QORE_LIBRARIES ${QORE_LIBRARY})
    SET(QORE_FOUND TRUE)
ENDIF (QORE_LIBRARY AND QORE_INCLUDE_DIR)


IF (QORE_FOUND)
    MESSAGE(STATUS "Found Qore lib: ${QORE_LIBRARY}")
    MESSAGE(STATUS "      includes: ${QORE_INCLUDE_DIR}")
ELSE (QORE_FOUND)
    MESSAGE(STATUS "E! Found Qore lib: ${QORE_LIBRARY}")
    MESSAGE(STATUS "E!      includes: ${QORE_INCLUDE_DIR}")

  IF (Qore_FIND_REQUIRED)
    MESSAGE(FATAL_ERROR "Could not find Qore library")
  ENDIF (Qore_FIND_REQUIRED)
ENDIF (QORE_FOUND)

MARK_AS_ADVANCED(QORE_INCLUDE_DIR QORE_LIBRARY)

