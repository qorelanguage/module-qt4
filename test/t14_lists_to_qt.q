#!/usr/bin/env qore
%requires qt4


my $a = new QApplication();

my $model = new QDirModel();
my $cv = new QColumnView();
$cv.setModel($model);
my $l = 10, 20, 30;

printf("\ncall start\n");
$cv.setColumnWidths($l);
printf("\ncall end\n\n");

printf("locumns: %N\n", $cv.columnWidths());




##################
my $fd = new QFontDatabase ();
printf("QStringList from qt: %N\n", $fd.families());

##################

my $qb = new QComboBox();
my $items = "foo", "bar", 123;
$qb.addItems($items);

for (my $i = 0; $i < $qb.count(); $i++)
	printf("item: %N\n", $qb.itemText($i));

