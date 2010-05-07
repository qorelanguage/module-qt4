#include <QtCore/QtCore>
#include <QtGui/QtGui>
#include <QtXml/QtXml>
#include <QtSql/QtSql>
#include <QtOpenGL/QtOpenGL>
#include <QtNetwork/QtNetwork>
#include <QtSvg/QtSvg>
//#include <QtWebKit/QtWebKit>

// some MS headers do
// #define interface struct
// un-define it here so QtDBus will build correctly
#undef interface
#include <QtDBus/QtDBus>
