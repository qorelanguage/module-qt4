#!/usr/bin/env qore

%requires qt4

%include untitled.q

my $a = new QApplication();
my $u = new Ui::MainWindow();
my $w = new QMainWindow();
$u.setupUi($w);
$w.show();
$a.exec();

