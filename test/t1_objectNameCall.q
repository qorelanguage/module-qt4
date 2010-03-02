#!/usr/bin/env qore
%requires qt4

#printf("1\n");
my $a = new QApplication();
#printf("2\n");
#my $w = new QMainWindow();
my $w = new QMainWindow(NOTHING, Qt::Window);

printf("mainwindow: %N\n", $w);
printf("3\n");
$w.setObjectName("win");
printf("4\n");
printf("object name: %N\n", $w.objectName());


printf("------------------------------------\n");
my $o = new QObject();
$o.setObjectName("Cool Test Object");
printf("QObject: %N\n", $o);
printf("objectName: %N\n", $o.objectName());
printf("isWidgetType: %N\n", $o.isWidgetType());
printf("inherits QWidget: %N\n", $o.qt_inherits("QWidget"));
printf("inherits QObject: %N\n", $o.qt_inherits("QObject"));
printf("parent: %N\n", $o.parent());
#printf("thread: %N\n", $o.thread());
