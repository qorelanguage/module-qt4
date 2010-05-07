#include <QtCore>
#include <QtGui>
#include <QtXml>
#include <QtSql>
#include <QtOpenGL>
#include <QtNetwork>
#include <QtSvg>
//#include <QtWebKit>

// some MS headers do
// #define interface struct
// un-define it here so QtDBus will build correctly
#undef interface
#include <QtDBus>
