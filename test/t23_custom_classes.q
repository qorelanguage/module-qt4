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



my QApplication $a();
my QTreeWidget $w();
new MyItem($w, ("item1"));


# This is the issue. QTreeWidget::topLevelItem(int) returns QTreeWidgetItem, but
# the item is MyItem for sure. This is how it's used in C++ Qt apps...
for (my int $i = 0; $w.topLevelItemCount(); $i++)
    my MyItem $item = $w.topLevelItem($i);


$w.show();
return $a.exec();

