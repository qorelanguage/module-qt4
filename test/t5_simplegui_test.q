#!/usr/bin/env qore

%requires qt4

%include t5_simplegui.q

%exec-class MyApp

class MyApp inherits QApplication {
    private { Ui::MainWindow $.u(); }
    constructor() {
	printf("ARGV=%n\n", $QORE_ARGV);
	my $w = new QMainWindow();
	printf("w=%N\n", $w);
	printf("w=%s\n", dbg_node_info($w));
	$.u.setupUi($w);

	$.u.comboBox.addItem("test 1", 1);
	$.u.comboBox.addItem("test 2", 2);

	QObject::connect($.u.comboBox, SIGNAL("currentIndexChanged(int)"), $self, SLOT("hi(int)"));

	$w.show();
	$.exec();
    }
    hi(int $index) {
	printf("combobox index %n value %n\n", $index, $.u.comboBox.itemData($index).toQore());
    }
}
