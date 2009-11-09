#!/usr/bin/env qore

%requires qt4

%require-our
%exec-class test

class test inherits QApplication {
    constructor() {
	$.localeCombo = new QComboBox();

	$.connect($.localeCombo, SIGNAL("currentIndexChanged(int)"), SLOT("indexChanged(int)"));

	$.localeCombo.addItem("first", 1);
	$.localeCombo.addItem("second", 2);
	$.localeCombo.addItem("third", 3);

	$.localeCombo.show();

	$.exec();
    }

    indexChanged($i) {
	printf("indexChanged() i=%d\n", $i);
    }
}
