#!/usr/bin/env qore

%requires qt4

my $app = new QApplication();

my hash $h = (
	"parent1" : "a value",
	"parent2" : now(),
	"parent3" : ( "key1": 1, "key2": 2, "key3": ("foo" : "bar"))
);


class TreeWidget  inherits QTreeWidget
{

    buildTree(any $stat, any $parent)
    {
        foreach my string $i in (keys $stat)
        {
            printf("Key: %s parent: %n\n", $i, $parent);
            my QTreeWidgetItem $item = new QTreeWidgetItem($parent);
            $item.setText(0, $i);

            switch (type($stat.$i)) {
                case Type::Hash:
                    printf("    building subtree - %s: %N\n", $i, $stat.$i);
                    $.buildTree($stat.$i, $item);
                    $item.setExpanded(True);
                    break;
                case Type::Date:
                    $item.setText(1, sprintf("%n", $stat.$i));
                    break;
                default:
                    $item.setText(1, string($stat.$i));
            }
        } # loop
    }
}


my TreeWidget $w();
$w.setColumnCount(2);
$w.buildTree($h, $w);
$w.show();

$app.exec();

