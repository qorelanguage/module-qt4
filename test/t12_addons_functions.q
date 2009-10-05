#!/usr/bin/env qore
%requires qt4


my $app = new QApplication();
printf("QCoreApplication::instance() %N\n", QCoreApplication::instance());
printf("qApp                         %N\n", qApp());
printf("isLeftToRight: %N\n", qApp().isLeftToRight());

qDebug("foo", "bar", 0, new QObject());

qWarning("foo", "bar", 0, QObject::tr("foo bar"));

qCritical("foo", "bar", 0, new QObject());


printf("Q_WS_X11: %N\nQ_WS_MAC: %N\nQ_WS_QWS: %N\nQ_WS_WIN: %N\n", Q_WS_X11, Q_WS_MAC, Q_WS_QWS, Q_WS_WIN);

printf("Ver: %N\nQT_VERSION: 0x%x\nQT_VERSION_STR: %N\n", qVersion(), QT_VERSION, QT_VERSION_STR);


printf("qRound: %N -> %N\n", 13.4, qRound(13.4));

qsrand(now_ms());
printf("random: %N\n", qrand());
printf("random: %N\n", qrand());
printf("random: %N\n", qrand());


my $a = 10;
my $b = 20;
printf("Before qSwap: %N %N\n", $a, $b);
qSwap(\$a, \$b);
printf("After qSwap: %N %N\n", $a, $b);



qFatal("die!");
