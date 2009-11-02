#!/usr/bin/env qore

%requires qt4
%require-our

sub indexof($list, $v) {
    for (my $i = 0; $i < elements $list; ++$i) {
        if ($list[$i] == $v) {
            printf("\n\n    indexof %N\n\n", $i);
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
#         if (! exists $.parent)
#             $.parent = $self;

        if (exists $parent)
            $parent.addChild($self);

        $.children = list();
        printf("Item::constructor() data: %N parent: %N\n", $.data, exists $.parent);
    }

    children()
    {
        return $.children;
    }

    child($row)
    {
        printf("child(row) %N %N\n", $row, exists $.children[$row]);
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
        printf("struct: %N\n", $.rootItem);
    }

    columnCount($index)
    {
        return 1;
    }

    rowCount($parentIndex)
    {
        my $parent;
        if (!$parentIndex.isValid())
        {
#             printf("rowCount() index is not valid\n");
            $parent = $.rootItem;
        }
        else
            $parent = $parentIndex.internalPointer();
#         if (exists $parent)
#         {
            printf("rowCount() c=%N\n", $parent.childrenCount());
            return $parent.childrenCount();
#         }
#         printf("rowCount() error, item is not found item: %N\nparent: %N\n", $parent, $index);
#         return 0;
    }

    index($row, $column, $parentIndex)
    {
        printf("index() row: %N column: %N parent: %N (%N, %N)\n", $row, $column,
                $parentIndex, $parentIndex.row(), $parentIndex.column());
        if (!$.hasIndex($row, $column, $parentIndex))
        {
            printf("index() not hasIndex\n");
            return new QModelIndex();
        }
        
        my $parent;
        if (!$parentIndex.isValid())
            $parent = $.rootItem;
        else
            $parent = $parentIndex.internalPointer();

        my $child = $parent.child($row);
        if (exists $child)
        {
            printf("index() creating index for child: %N, %N, %N\n", $row, $parent.data(), $child.data());
            return $.createIndex($row, $column, $child);
        }
        else
        {
            printf("index() child does not exist\n");
            return new QModelIndex();
        }
    }

    parent($index)
    {
        printf("parent() start\n");
        if (!$index.isValid())
        {
            printf("parent() invalid index\n");
            return new QModelIndex();
        }
#         printf("parent() index %N internalPointer %N\n", $index, $index.internalPointer());
        my $child = $index.internalPointer();
        if (! exists $child)
        {
            printf("parent() child object does not exists! QModelIndex( %N, %N )\n", $index.row(), $index.column());
#             return new QModelIndex();
        }
        my $parent = $child.parentItem();
#         printf("parent() %N\n%N\n\n", $child, $parent);
        if (!exists $parent
            ||
            $parent == $.rootItem)
        {
            printf("parent() exiting. parent: %N root %N\n", exists $parent, $parent == $.rootItem);
            return new QModelIndex();
        }

#         printf("parent() p=%N\n", $parent);
        printf("parent() p-------------------\n");
        return $.createIndex($parent.childNumber(), 0, $parent);
    }

    data($index, $role)
    {
        printf("data()\n");
        if ($role == Qt::DisplayRole)
            return $index.internalPointer().data();
        return;
    }

}



my $app = new QApplication();
my $w = new QTreeView();
my $m = new Model($w);
$w.setModel($m);
$w.show();
$app.exec();

