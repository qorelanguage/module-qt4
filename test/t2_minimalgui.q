#!/usr/bin/env qore
%requires qt4

printf("ARGS: %N\n", $ARGV);
my $a = new QApplication(0, list("test"));

my $w = new QWidget();
printf("widget: %N\n", $w);

# this should fail, because there is no QWidget's parent at all
try {
    my $parent = $w.parent();
    printf("parent: %N\n", $parent);
    printf("tr call: %N\n", $parent.tr("foo bar"));
} catch ($ex) {
    printf("Expected exception: %N %N\n", $ex.desc, $ex.err);
}

my $b = new QPushButton("A Button", $w);
printf("Button: %N %N\n", $b, $b.text());

# this should NOT fail
try {
    my $parent = $b.parent();
    printf("parent: %N\n", $parent);
    printf("tr call: %N\n", $parent.tr("foo bar"));
} catch ($ex) {
    printf("!!! Unexpected exception: %N %N\n", $ex.desc, $ex.err);
}

printf("Show: %N\n", $w.show());

#my $tmpMenu =  new QMenu();
#$b.setMenu($tmpMenu);
#my $m = $b.menu();
#printf("Button's menu: %N\n", $m);
#$m.setTitle("foo");
#printf("Menu title: %N\n", $m.title());
$w.setVisible(True);
printf("Exec result: %N\n", $a.exec());

