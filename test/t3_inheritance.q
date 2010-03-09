#!/usr/bin/env qore
%requires qt4


class MyObject inherits QObject {
    constructor() : QObject() {
	printf("MyObject constructor\n");
    }
}

my $m1 = new QObject();
printf("isWidget1 %N\n", $m1.isWidgetType());
my $m = new MyObject();
printf("isWidget %N\n", $m.isWidgetType());

class MyQApp inherits QApplication {
    constructor() : QApplication() {
	printf("MyQApp constructed\n");
    }

    foo() {
	printf("foo!\n");
    }
}

my $q = new MyQApp();
$q.foo();

class MyTableModel inherits QAbstractTableModel {
#	constructor($parent) : QAbstractTableModel($parent)
    constructor() : QAbstractTableModel() {
	printf("MyTableModel constructed\n");
    }

    index(int $row, int $column) returns QModelIndex {
	printf("index-------------------------------------\n");
	return QAbstractTableModel::$.index($row, $column);
    }

    columnCount() returns int {
	return 1;
    }

    rowCount() returns int {
	return 0;
    }
}

my $tm = new MyTableModel();
printf("tm=%N instanceof QAbstractTableModel=%n QAbstractItemModel=%n (%s)\n", $tm, $tm instanceof QAbstractTableModel, $tm instanceof QAbstractItemModel, $tm.metaObject().className());
my $vm = new QTableView();
$vm.setModel($tm);
printf("currentIndex: %N\n", $vm.currentIndex());
