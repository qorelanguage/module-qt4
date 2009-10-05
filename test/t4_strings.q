#!/usr/bin/env qore
%requires qt4


my $o = new QObject();
my $s = "foo bar";
printf("tr: %N -> %N\n", $s, $o.tr($s));

my $a = new QApplication();
my $w = new QMainWindow();
my $s = QApplication::translate("MainWindow", "MainWindow", "", QCoreApplication::UnicodeUTF8);
printf("translate: %N\n", $s);
$w.setObjectName($s);
$w.setWindowTitle($s);

