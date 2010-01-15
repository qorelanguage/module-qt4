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




class MyClass inherits QWidget
{
	private $.w;

	constructor() {
		$.w = new QListWidget();
		$.w.addItem("foo");
		$.w.addItem("bar");

		$.connect($.w, SIGNAL("currentItemChanged(QListWidgetItem *,QListWidgetItem *)"),
			  SLOT("w_currentItemChanged(QListWidgetItem *, QListWidgetItem *)"));

		$.w.setCurrentRow(1);
	}

	w_currentItemChanged($curr, $prev) {
		printf("curr: %N\n", $curr);
		printf("prev: %N\n", $prev);
		if (exists $curr)
			printf("curr text: %N\n", $curr.text());
		if (exists $prev)
			printf("prev text: %N\n", $prev.text());
	}
}

my $cls = new MyClass();

