#!/usr/bin/env qore

%requires qt4
%require-our

sub indexof($list, $v) {
    for (my $i = 0; $i < elements $list; ++$i) {
        if ($list[$i] == $v) {
#            printf("\n\n    indexof %N\n\n", $i);
            return $i;
        }
    }
    printf("EE! indexof out of index\n");
    return -1;
}


class Item
{
    private $.data;
    private $.parent;
    private $.children;

    constructor($data, $parent)
    {
        $.data = $data;
        $.parent = $parent;

        if (exists $parent)
            $parent.addChild($self);

        $.children = ();
#        printf("Item::constructor() data: %N parent: %N\n", $.data, exists $.parent);
    }

    children()
    {
        return $.children;
    }

    child($row)
    {
        #printf("child(row) %N %N\n", $row, exists $.children[$row]);
        return $.children[$row];
    }

    childrenCount()
    {
        return elements $.children;
    }

    addChild($item)
    {
        #printf("addChild %N\n", $item); 
        push $.children, $item;
    }

    childNumber()
    {
        if (exists $.parent)
            return indexof($.parent.children(), $self);
        return 0;
    }

    parentItem()
    {
#         printf("Item::parentItem() %N\n", $.parent);
#         if (!exists $.parent) printf("%N\n", $self);
        return $.parent;
    }

    data()
    {
        return $.data;
    }

}


class Model inherits QAbstractItemModel
{
    private $.rootItem;

    constructor($parent) : QAbstractItemModel($parent)
    {
        $.rootItem = new Item("root");
        my $c1 = new Item("child1", $.rootItem);
        new Item("subchild 1", $c1);
        new Item("subchild 2", $c1);
        new Item("subchild 3", $c1);
        new Item("child2", $.rootItem);
#        printf("struct: %N\n", $.rootItem);
    }

    columnCount($index)
    {
        return 1;
    }

    rowCount($parentIndex)
    {
        my $parent = !$parentIndex.isValid() ? $.rootItem : $parentIndex.internalPointer();
	return $parent.childrenCount();
    }

    index($row, $column, $parentIndex)
    {
        #printf("index() row: %N column: %N parent: %N (%N, %N)\n", $row, $column, $parentIndex, $parentIndex.row(), $parentIndex.column());
        if (!$.hasIndex($row, $column, $parentIndex))
        {
#            printf("index() not hasIndex\n");
            return new QModelIndex();
        }

        my $parent = !$parentIndex.isValid() ? $.rootItem : $parentIndex.internalPointer();

        my $child = $parent.child($row);
        if (exists $child)
        {
#            printf("index() creating index for child: %N, %N, %N\n", $row, $parent.data(), $child.data());
            return $.createIndex($row, $column, $child);
        }
        else
        {
#            printf("index() child does not exist\n");
            return new QModelIndex();
        }
    }

    parent($index)
    {
#        printf("parent() start\n");
        if (!$index.isValid())
        {
#            printf("parent() invalid index\n");
            return new QModelIndex();
        }
#         printf("parent() index %N internalPointer %N\n", $index, $index.internalPointer());
        my $child = $index.internalPointer();
        if (! exists $child)
        {
            printf("parent() child object does not exist! QModelIndex( %N, %N )\n", $index.row(), $index.column());
            return new QModelIndex();
        }
        my $parent = $child.parentItem();
#         printf("parent() %N\n%N\n\n", $child, $parent);
        if (!exists $parent
            ||
            $parent == $.rootItem)
        {
#            printf("parent() exiting. parent: %N root %N\n", exists $parent, $parent == $.rootItem);
            return new QModelIndex();
        }

#         printf("parent() p=%N\n", $parent);
#        printf("parent() p-------------------\n");
        return $.createIndex($parent.childNumber(), 0, $parent);
    }

    data($index, $role)
    {
#        printf("data()\n");
        if ($role == Qt::DisplayRole)
            return $index.internalPointer().data();
        return;
    }

    do_doubleClicked($item)
    {
       printf("do_doubleClicked: %N\n", $item);
    }

}



my $app = new QApplication();
my $w = new QTreeView();
my $m = new Model($w);

QObject::connect($w, SIGNAL("doubleClicked(const QModelIndex &)"), $m, SLOT("do_doubleClicked(const QModelIndex &)"));

$w.setModel($m);
$w.show();
$app.exec();

