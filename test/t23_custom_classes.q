#!/usr/bin/env qore
%requires qt4


class MyItem inherits QTreeWidgetItem
{
        constructor(any $parent) : QTreeWidgetItem($parent, $args)
        {
                printf("MyItem constructor\n");
        }

        foo()
        {
                printf("special method called\n");
        }
}

class TreeWidget inherits QTreeWidget
{
        constructor(QWidget $parent) : QTreeWidget($parent)
        {
            new MyItem($self, ("item1"));
            new MyItem($self, ("item2"));
            $.connect($self, SIGNAL("currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)"),
                      SLOT("self_currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)"));
        }

        self_currentItemChanged(QTreeWidgetItem $current, any $previous)
        {
            $current.foo();
        }
}

my QApplication $a();
my QTreeWidget $w();
new MyItem($w, ("item1"));


my TreeWidget $tw();
$tw.show();


# This is the issue. QTreeWidget::topLevelItem(int) returns QTreeWidgetItem, but
# the item is MyItem for sure. This is how it's used in C++ Qt apps...
#for (my int $i = 0; $w.topLevelItemCount(); $i++)
#    my MyItem $item = $w.topLevelItem($i);


#$w.show();
return $a.exec();

