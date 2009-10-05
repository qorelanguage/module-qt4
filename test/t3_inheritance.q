#!/usr/bin/env qore
%requires qt4


class MyObject inherits QObject
{
        constructor() : QObject()
        {
                printf("MyObject constructor");
        }

}

my $m1 = new QObject();
printf("isWidget1 %N\n", $m1.isWidgetType());
my $m = new MyObject();
printf("isWidget %N\n", $m.isWidgetType());



class MyQApp inherits QApplication
{
	constructor() : QApplication()
	{
		printf("\n\nMyQApp constructed\n\n");
	}

	foo()
	{
		printf("\n\nfoo!\n\n");
	}
}

my $q = new MyQApp();
$q.foo();



class MyTableModel inherits QAbstractTableModel
{
#	constructor($parent) : QAbstractTableModel($parent)
	constructor() : QAbstractTableModel()
	{
		printf("\n\nMyTableModel constructed\n\n");
	}

	index($row, $column)
	{
		printf("\n\nindex-------------------------------------\n\n");
		return QAbstractTableModel::$.index($row, $column);
	}

	columnCount()
	{
		return 1;
	}

	rowCount()
	{
		return 0;
	}
}

my $tm = new MyTableModel();
my $vm = new QTableView();
$vm.setModel($tm);
printf("currentIndex: %N\n", $vm.currentIndex());

