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


# get Qore version form 'qore --short-version' command etc
FIND_PROGRAM(QORE_EXECUTABLE NAMES qore DOC "Searching for qore executable")
IF (NOT QORE_EXECUTABLE)
    MESSAGE( FATAL_ERROR "Cannot find 'qore' executable. Check if it's in your PATH" )
ENDIF (NOT QORE_EXECUTABLE)


IF (QORE_MIN_VERSION)
    #now parse the parts of the user given version string into variables
    # qore version is taken from qore executable - format major.minor.patch-build
    STRING(REGEX MATCH "^[0-9]+\\.[0-9]+\\.[0-9]+" req_qore_version "${QORE_MIN_VERSION}")
    IF (NOT req_qore_version)
        MESSAGE( FATAL_ERROR "Invalid Qore version string given: \"${QORE_MIN_VERSION}\", expected e.g. \"0.7.3\"")
    ENDIF (NOT req_qore_version)

    # now parse the parts of the user given version string into variables
    STRING(REGEX REPLACE "^([0-9]+)\\.[0-9]+\\.[0-9]+" "\\1" req_qore_major_v "${QORE_MIN_VERSION}")
    STRING(REGEX REPLACE "^[0-9]+\\.([0-9])+\\.[0-9]+" "\\1" req_qore_minor_v "${QORE_MIN_VERSION}")
    STRING(REGEX REPLACE "^[0-9]+\\.[0-9]+\\.([0-9]+)" "\\1" req_qore_patch_v "${QORE_MIN_VERSION}")

    EXEC_PROGRAM(${QORE_EXECUTABLE} ARGS "--short-version" OUTPUT_VARIABLE QORE_VERSION)
    IF (NOT QORE_VERSION)
        MESSAGE( FATAL_ERROR "Unknown Qore version. No data taken from 'qore --short-version'" )
    ELSE (NOT QORE_VERSION)
        MESSAGE( STATUS "Qore version: ${QORE_VERSION}" )
    ENDIF (NOT QORE_VERSION)

    # compare requested version with the one found
    STRING(REGEX REPLACE "^([0-9]+)\\.[0-9]+\\.[0-9]+\\-[0-9]+" "\\1" qore_major_v "${QORE_VERSION}")
    STRING(REGEX REPLACE "^[0-9]+\\.([0-9])+\\.[0-9]+\\-[0-9]+" "\\1" qore_minor_v "${QORE_VERSION}")
    STRING(REGEX REPLACE "^[0-9]+\\.[0-9]+\\.([0-9]+)\\-[0-9]+" "\\1" qore_patch_v "${QORE_VERSION}")
    # compute an overall version number which can be compared at once
    MATH(EXPR req_v "${req_qore_major_v}*10000 + ${req_qore_minor_v}*100 + ${req_qore_patch_v}")
    MATH(EXPR found_v "${qore_major_v}*10000 + ${qore_minor_v}*100 + ${qore_patch_v}")
    #MESSAGE (STATUS "${req_qore_major_v} ${req_qore_minor_v} ${req_qore_patch_v}")
    #MESSAGE (STATUS "${qore_major_v} ${qore_minor_v} ${qore_patch_v}")
    # and decide finally
    IF (found_v LESS req_v)
        SET (QORE_FOUND FALSE)
        MESSAGE (STATUS "Requested Qore version is bigger than found. ${QORE_MIN_VERSION} vs ${QORE_VERSION}" )
    ENDIF (found_v LESS req_v)

ENDIF (QORE_MIN_VERSION)

# module install dir
EXEC_PROGRAM(${QORE_EXECUTABLE} ARGS "--module-dir" OUTPUT_VARIABLE QORE_MODULES_DIR)
IF (NOT QORE_MODULES_DIR)
    MESSAGE( FATAL_ERROR "Unknown Qore modules location. No data taken from 'qore --module-dir'" )
ELSE (NOT QORE_MODULES_DIR)
    MESSAGE( STATUS "Qore modules dir: ${QORE_MODULES_DIR}" )
ENDIF (NOT QORE_MODULES_DIR)

# module API version
IF (NOT QORE_API_VERSION)
    EXEC_PROGRAM(${QORE_EXECUTABLE} ARGS "--latest-module-api" OUTPUT_VARIABLE QORE_API_VERSION)
ENDIF (NOT QORE_API_VERSION)
IF (NOT QORE_API_VERSION)
    MESSAGE( FATAL_ERROR "Unknown Qore module API version. No data taken from 'qore --latest-module-api'. It can be specified manually with -DQORE_API_VERSION")
ELSE (NOT QORE_API_VERSION)
    MESSAGE( STATUS "Qore module API version: ${QORE_API_VERSION}")
ENDIF (NOT QORE_API_VERSION)


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

