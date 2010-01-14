#!/usr/bin/env qore

%requires qt4

my $app = new QApplication();

my $l = new QTreeWidget();

sub foo() {
	my $i = new QTreeWidgetItem($l, ("foo") );
	#$l.addTopLevelItem($i);
}

foo();

printf("items (should be 1): %N (%s)\n", $l.topLevelItemCount(), $l.topLevelItemCount() == 1 ? "success" : "ERROR");

