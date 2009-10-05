#!/usr/bin/env qore

%requires qt4

%include t5_simplegui.q

printf("ARGV=%n\n", $QORE_ARGV);
my $a = new QApplication();
my $u = new Ui::MainWindow();
my $w = new QMainWindow();
printf("w=%N\n", $w);
printf("w=%s\n", dbg_node_info($w));
$u.setupUi($w);
$w.show();
$a.exec();

