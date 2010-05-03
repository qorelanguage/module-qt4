#
# Documentation generator for Qore Qt4 module
#
# This directory is not part of the main build workflow.
# I separated it just because it forces users to have Docbook installed.
#
# Generated HTML files are part of SVN tree of this module and should
# be regenerated only on demand:
#    cmake .
#    make
#
# Into SVN should go only CMakeLists.txt, docbook document (*.xml),
# support files for HTML (css, png?), and generated html files.
# All other files can be deleted:
#    svn st|grep "?"|xargs rm
#

cmake_minimum_required(VERSION 2.6)

FIND_PROGRAM(XSLTPROC_EXECUTABLE xsltproc)
IF (XSLTPROC_EXECUTABLE)
    MESSAGE(STATUS "Xsltproc found: ${XSLTPROC_EXECUTABLE}")
    SET(DOCS 1)
    SET(xsltcmd "${XSLTPROC_EXECUTABLE}")
ELSE (XSLTPROC_EXECUTABLE)
    SET(DOCS 0)
    MESSAGE(STATUS "No XSL processor found! No docs willbe created.")
ENDIF (XSLTPROC_EXECUTABLE)

if (DOCS)
    FIND_FILE(xslstyle  chunk.xsl
              PATHS /usr/share/xml/docbook/stylesheet/nwalsh5/current/xhtml # opensuse
                    /opt/local/share/xsl/docbook-xsl/xhtml                  # macpors
                    /usr/share/sgml/docbook/xsl-stylesheets/xhtml           # fedora
             )
    MESSAGE("xsl style: ${xslstyle}")

    SET(cssstyle "qore.css")

    SET(docbook_index "qore-qt4.xml")

    ADD_CUSTOM_TARGET(documentation ALL
        COMMAND ${xsltcmd}
              #--output ${install_path}
              --stringparam html.stylesheet ${cssstyle}
              --stringparam chunker.output.encoding UTF-8
              --stringparam use.id.as.filename 1
              ${xslstyle}
              ${CMAKE_CURRENT_SOURCE_DIR}/${docbook_index}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${docbook_index}
        COMMENT "processing ${docbook_index}"
        VERBATIM
    )

endif (DOCS)
