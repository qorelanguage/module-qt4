#!/usr/bin/env qore

# $Self is basically a direct port of the QT tutorial to Qore 
# using Qore's "qt4" module.  

# Note that Qore's "qt4" module requires QT 4.3 or above 

# use the "qt4" module
%requires qt4

# $self is an object-oriented program, the application class is "editable_tree_model"
%exec-class editable_tree_model
# require all variables to be explicitly declared
%require-our
# enable all parse warnings
%enable-all-warnings

class Ui_MainWindow {
    public {
	QTreeView $.view;
	QAction $.exitAction;
	QAction $.insertRowAction;
	QAction $.removeRowAction;
	QAction $.insertColumnAction;
	QAction $.removeColumnAction;
	QAction $.insertChildAction;
	QWidget $.centralwidget;
	QVBoxLayout $.vboxLayout;
	QMenuBar $.menubar;
	QStatusBar $.statusbar;
	QMenu $.fileMenu;
	QMenu $.actionsMenu;
    }

    setupUi(QMainWindow $MainWindow) {
        if (!strlen($MainWindow.objectName()))
            $MainWindow.setObjectName("MainWindow");
        $MainWindow.resize(573, 468);
        $.exitAction = new QAction($MainWindow);
        $.exitAction.setObjectName("exitAction");
        $.insertRowAction = new QAction($MainWindow);
        $.insertRowAction.setObjectName("insertRowAction");
        $.removeRowAction = new QAction($MainWindow);
        $.removeRowAction.setObjectName("removeRowAction");
        $.insertColumnAction = new QAction($MainWindow);
        $.insertColumnAction.setObjectName("insertColumnAction");
        $.removeColumnAction = new QAction($MainWindow);
        $.removeColumnAction.setObjectName("removeColumnAction");
        $.insertChildAction = new QAction($MainWindow);
        $.insertChildAction.setObjectName("insertChildAction");
        $.centralwidget = new QWidget($MainWindow);
        $.centralwidget.setObjectName("centralwidget");
        $.vboxLayout = new QVBoxLayout($.centralwidget);
        $.vboxLayout.setSpacing(0);
        $.vboxLayout.setMargin(0);
        $.vboxLayout.setObjectName("vboxLayout");
        $.view = new QTreeView($.centralwidget);
        $.view.setObjectName("view");
        $.view.setAlternatingRowColors(True);
        $.view.setSelectionBehavior(QAbstractItemView::SelectItems);
        $.view.setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
        $.view.setAnimated(False);
        $.view.setAllColumnsShowFocus(True);

        $.vboxLayout.addWidget($.view);

        $MainWindow.setCentralWidget($.centralwidget);
        $.menubar = new QMenuBar($MainWindow);
        $.menubar.setObjectName("menubar");
        $.menubar.setGeometry(new QRect(0, 0, 573, 31));
        $.fileMenu = new QMenu($.menubar);
        $.fileMenu.setObjectName("fileMenu");
        $.actionsMenu = new QMenu($.menubar);
        $.actionsMenu.setObjectName("actionsMenu");
        $MainWindow.setMenuBar($.menubar);
        $.statusbar = new QStatusBar($MainWindow);
        $.statusbar.setObjectName("statusbar");
        $MainWindow.setStatusBar($.statusbar);

        $.menubar.addAction($.fileMenu.menuAction());
        $.menubar.addAction($.actionsMenu.menuAction());
        $.fileMenu.addAction($.exitAction);
        $.actionsMenu.addAction($.insertRowAction);
        $.actionsMenu.addAction($.insertColumnAction);
        $.actionsMenu.addSeparator();
        $.actionsMenu.addAction($.removeRowAction);
        $.actionsMenu.addAction($.removeColumnAction);
        $.actionsMenu.addSeparator();
        $.actionsMenu.addAction($.insertChildAction);

        $.retranslateUi($MainWindow);

        QMetaObject::connectSlotsByName($MainWindow);
    } # setupUi

    retranslateUi($MainWindow) {
        $MainWindow.setWindowTitle(QApplication::translate("MainWindow", "Editable Tree Model", 0, QCoreApplication::UnicodeUTF8));
        $.exitAction.setText(QApplication::translate("MainWindow", "E&xit", 0, QCoreApplication::UnicodeUTF8));
        $.exitAction.setShortcut(QApplication::translate("MainWindow", "Ctrl+Q", 0, QCoreApplication::UnicodeUTF8));
        $.insertRowAction.setText(QApplication::translate("MainWindow", "Insert Row", 0, QCoreApplication::UnicodeUTF8));
        $.insertRowAction.setShortcut(QApplication::translate("MainWindow", "Ctrl+I, R", 0, QCoreApplication::UnicodeUTF8));
        $.removeRowAction.setText(QApplication::translate("MainWindow", "Remove Row", 0, QCoreApplication::UnicodeUTF8));
        $.removeRowAction.setShortcut(QApplication::translate("MainWindow", "Ctrl+R, R", 0, QCoreApplication::UnicodeUTF8));
        $.insertColumnAction.setText(QApplication::translate("MainWindow", "Insert Column", 0, QCoreApplication::UnicodeUTF8));
        $.insertColumnAction.setShortcut(QApplication::translate("MainWindow", "Ctrl+I, C", 0, QCoreApplication::UnicodeUTF8));
        $.removeColumnAction.setText(QApplication::translate("MainWindow", "Remove Column", 0, QCoreApplication::UnicodeUTF8));
        $.removeColumnAction.setShortcut(QApplication::translate("MainWindow", "Ctrl+R, C", 0, QCoreApplication::UnicodeUTF8));
        $.insertChildAction.setText(QApplication::translate("MainWindow", "Insert Child", 0, QCoreApplication::UnicodeUTF8));
        $.insertChildAction.setShortcut(QApplication::translate("MainWindow", "Ctrl+N", 0, QCoreApplication::UnicodeUTF8));
        $.fileMenu.setTitle(QApplication::translate("MainWindow", "&File", 0, QCoreApplication::UnicodeUTF8));
        $.actionsMenu.setTitle(QApplication::translate("MainWindow", "&Actions", 0, QCoreApplication::UnicodeUTF8));
    } # retranslateUi
}

class MainWindow inherits QMainWindow, private Ui_MainWindow {

    constructor($parent) : QMainWindow($parent) {
        $.setupUi($self);

        my $headers = ($.tr("Title"), $.tr("Description"));

        my $file = new File();
        $file.open(get_script_dir() + "default.txt");
        my $model = new TreeModel($headers, $file.read(-1), $.view);
        $file.close();

        $.view.setModel($model);
        for (my $column = 0; $column < $model.columnCount(); ++$column)
            $.view.resizeColumnToContents($column);

        qApp().connect($.exitAction, SIGNAL("triggered()"), SLOT("quit()"));

        $.connect($.view.selectionModel(),
                  SIGNAL("selectionChanged(const QItemSelection &, const QItemSelection &)"),
                  SLOT("updateActions()"));

        $.connect($.actionsMenu,        SIGNAL("aboutToShow()"), SLOT("updateActions()"));
        $.connect($.insertRowAction,    SIGNAL("triggered()"),   SLOT("insertRow()"));
        $.connect($.insertColumnAction, SIGNAL("triggered()"),   SLOT("insertColumn()"));
        $.connect($.removeRowAction,    SIGNAL("triggered()"),   SLOT("removeRow()"));
        $.connect($.removeColumnAction, SIGNAL("triggered()"),   SLOT("removeColumn()"));
        $.connect($.insertChildAction,  SIGNAL("triggered()"),   SLOT("insertChild()"));
        
        $.updateActions();
    }

    updateActions() {
        my bool $hasSelection = !$.view.selectionModel().selection().isEmpty();
        $.removeRowAction.setEnabled($hasSelection);
        $.removeColumnAction.setEnabled($hasSelection);

        my $hasCurrent = $.view.selectionModel().currentIndex().isValid();
        $.insertRowAction.setEnabled($hasCurrent);
        $.insertColumnAction.setEnabled($hasCurrent);

        if ($hasCurrent) {
            $.view.closePersistentEditor($.view.selectionModel().currentIndex());

            my $row = $.view.selectionModel().currentIndex().row();
            my $column = $.view.selectionModel().currentIndex().column();
            if ($.view.selectionModel().currentIndex().parent().isValid())
                $.statusBar().showMessage(sprintf($.tr("Position: (%d,%d)"), $row, $column));
            else
                $.statusBar().showMessage(sprintf($.tr("Position: (%d,%d) in top level"), $row, $column));
        }
    }

    private insertChild() {
        my $index = $.view.selectionModel().currentIndex();
        my $model = $.view.model();

        if ($model.columnCount($index) == 0) {
            if (!$model.insertColumn(0, $index))
                return;
        }
        
        if (!$model.insertRow(0, $index))
            return;

        for (my $column = 0; $column < $model.columnCount($index); ++$column) {
            my $child = $model.index(0, $column, $index);
            $model.setData($child, new QVariant("[No data]"), Qt::EditRole);
            if (!strlen($model.headerData($column, Qt::Horizontal)))
                $model.setHeaderData($column, Qt::Horizontal, new QVariant("[No header]"), Qt::EditRole);
        }

        $.view.selectionModel().setCurrentIndex($model.index(0, 0, $index), QItemSelectionModel::ClearAndSelect);
        $.updateActions();
    }

    private insertColumn($parent) {
        if (!exists $parent) {
            $parent = new QModelIndex();
        }

        my $model = $.view.model();
        my $column = $.view.selectionModel().currentIndex().column();

        # Insert a column in the parent item.
        my $changed = $model.insertColumn($column + 1, $parent);
        if ($changed)
            $model.setHeaderData($column + 1, Qt::Horizontal, new QVariant("[No header]"), Qt::EditRole);
        
        $.updateActions();

        return $changed;
    }

    private insertRow() {
        my $index = $.view.selectionModel().currentIndex();
        my $model = $.view.model();

	printf("model=%N calling insertRow(%N, %N)\n", $model, $index.row() + 1, $index.parent();

        if (!$model.insertRow($index.row() + 1, $index.parent()))
            return;

	printf("insertRow returned True\n");
        $.updateActions();
        
        for (my int $column = 0; $column < $model.columnCount($index.parent()); ++$column) {
            my $child = $model.index($index.row() + 1, $column, $index.parent());
	    printf("column %d: row=%d child=%N\n", $column, $index.row(), $child);
            $model.setData($child, new QVariant("[No data]"), Qt::EditRole);
        }
    }

    private removeColumn($parent) {
        if (!exists $parent) {
            $parent = new QModelIndex();
        }

        my $model = $.view.model();
        my $column = $.view.selectionModel().currentIndex().column();

        # Insert columns in each child of the parent item.
        my $changed = $model.removeColumn($column, $parent);

        if (!$parent.isValid() && $changed)
            $.updateActions();

        return $changed;
    }

    private removeRow() {
        my $index = $.view.selectionModel().currentIndex();
        my $model = $.view.model();
        if ($model.removeRow($index.row(), $index.parent()))
            $.updateActions();
    }
}

sub indexof($list, $v) {
    for (my $i = 0; $i < elements $list; ++$i) {
        if ($list[$i] == $v) {
            return $i;
        }
    }
    return -1;
}

class TreeItem {
    private $.childItems, $.itemData, $.parentItem;

    constructor($data, $parent) {
        $.parentItem = $parent;
        $.itemData = $data;
        $.childItems = ();
    }

    child($number) {
        return $.childItems[$number];
    }

    childCount() {
        return elements $.childItems;
    }

    childNumber() {
        if (exists $.parentItem) {
            return indexof($.parentItem.childItems, $self);
        }

        return 0;
    }

    columnCount() {
        return elements $.itemData;
    }

    data($column) {
        return $.itemData[$column];
    }

    insertChildren($position, $count, $columns) {
        if ($position < 0 || $position > elements $.childItems)
            return False;

        for (my $row = 0; $row < $count; ++$row) {
            my $data[$columns - 1] = NOTHING;
            my $item = new TreeItem($data, $self);
            if (elements $.childTems > $position)
                splice $.childItems, $position, 0, $item;
            else
                $.childItems[$position] = $item;
        }
        
        return True;
    }

    insertColumns($position, $columns) {
        if ($position < 0 || $position > elements $.itemData)
            return False;

        for (my $column = 0; $column < $columns; ++$column) {
            if (elements $.itemData > $position)
                splice $.itemData, $position, 0, NOTHING;
            else
                $.itemData[$position] = NOTHING;
        }

        foreach my $child in ($.childItems)
            $child.insertColumns($position, $columns);

        return True;
    }

    parent() {
        return $.parentItem;
    }

    removeChildren($position, $count) {
        if ($position < 0 || $position + $count > elements $.childItems)
            return False;

        for (my $row = 0; $row < $count; ++$row)
            splice $.childItems, $position, 1;

        return True;
    }

    removeColumns($position, $columns) {
        if ($position < 0 || $position + $columns > elements $.itemData)
            return False;

        for (my $column = 0; $column < $columns; ++$column)
            splice $.itemData, $position, 1;
        foreach my $child in ($.childItems)
            $child.removeColumns($position, $columns);

        return True;
    }

    setData($column, $value) {
        if ($column < 0 || $column >= elements $.itemData)
            return False;

	if ($value instanceof QVariant)
	    $value = $value.toString();
        $.itemData[$column] = $value;
        return True;
    }
}

sub gb($num) {
    if (!$num)
        return;
    my $str = "                                                ";
    return substr($str, -$num);
}

class TreeModel inherits QAbstractItemModel {
    private { TreeItem $.rootItem; }

    constructor($headers, $data, $parent) : QAbstractItemModel($parent) {
        my $rootData = ();
        foreach my $header in ($headers)
            $rootData += $header;

        $.rootItem = new TreeItem($rootData);
        $.setupModelData(split("\n", $data), $.rootItem);
    }

    columnCount() {
        return $.rootItem.columnCount();
    }

    data($index, $role) {
        if (!$index.isValid()) {
            return;
        }

        if ($role != Qt::DisplayRole && $role != Qt::EditRole)
            return;
        
        my $item = $.getItem($index);
        return $item.data($index.column());
    }

    flags($index) {
        if (!$index.isValid())
            return 0;

        return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }

    getItem($index) {
        if (exists $index && $index.isValid()) {
            my $item = $index.internalPointer();
            if (exists $item) {
                    return $item;
            }
        }
        return $.rootItem;
    }

    headerData($section, $orientation, $role) {
        if ($orientation == Qt::Horizontal && $role == Qt::DisplayRole)
            return $.rootItem.data($section);

        return;
    }

    index($row, $column, $parent) {
        if (exists $parent && $parent.isValid() && $parent.column() != 0)
            return new QModelIndex();

        my $parentItem = $.getItem($parent);

        my $childItem = $parentItem.child($row);

        return exists $childItem ? $.createIndex($row, $column, $childItem) : new QModelIndex();
    }

    insertColumns($position, $columns, $parent) {
        $.beginInsertColumns($parent, $position, $position + $columns - 1);
        my $success = $.rootItem.insertColumns($position, $columns);
        $.endInsertColumns();

        return $success;
    }

    insertRows($position, $rows, $parent) {
        my $parentItem = $.getItem($parent);
        
        $.beginInsertRows($parent, $position, $position + $rows - 1);
        my $success = $parentItem.insertChildren($position, $rows, $.rootItem.columnCount());
        $.endInsertRows();

        return $success;
    }

    parent($index) {
        if (!$index.isValid()) {
            return $index;
        }

        my $childItem = $.getItem($index);
        #printf("childItem %N\n", $childItem);
        my $parentItem = $childItem.parent();
        #printf("parentItem %N\n", $parentItem);

        return ($parentItem == $.rootItem || $parentItem == NOTHING)
                ? new QModelIndex()
                : $.createIndex($parentItem.childNumber(), 0, $parentItem);
    }

    removeColumns($position, $columns, $parent) {
        $.beginRemoveColumns($parent, $position, $position + $columns - 1);
        my $success = $.rootItem.removeColumns($position, $columns);
        $.endRemoveColumns();

        if ($.rootItem.columnCount() == 0)
            $.removeRows(0, $.rowCount());

        return $success;
    }

    removeRows($position, $rows, $parent) {
        my $parentItem = $.getItem($parent);
        
        $.beginRemoveRows($parent, $position, $position + $rows - 1);
        my $success = $parentItem.removeChildren($position, $rows);
        $.endRemoveRows();
        
        return $success;
    }

    rowCount($parent) {
        my $parentItem = $.getItem($parent);
        
        return $parentItem.childCount();
    }
    
    setData($index, $value, $role) {
        if ($role != Qt::EditRole)
            return False;

        my $item = $.getItem($index);
        my $result = $item.setData($index.column(), $value);

        if ($result)
            $.emit("dataChanged(const QModelIndex &, const QModelIndex &)", $index, $index);
        
        return $result;
    }
    
    setHeaderData($section, $orientation, $value, $role) {
        if ($role != Qt::EditRole || $orientation != Qt::Horizontal)
            return False;
        
        my $result = $.rootItem.setData($section, $value);
        
        if ($result)
            $.emit("headerDataChanged(Qt::Orientation, int, int)", $orientation, $section, $section);
        
        return $result;
    }
    
    setupModelData($lines, $parent) {
        my $parents = list($parent);
        my $indentations = list(0);

        my $number = 0;

        while ($number < elements $lines) {
            my $position = 0;
            while ($position < elements $lines[$number]) {
                if ($lines[$number][$position] != " ")
                    break;
                $position++;
            }
 
            my $lineData = trim(substr($lines[$number], $position));

            if (strlen($lineData)) {
                # Read the column data from the rest of the line.
                my $columnData = select split("\t", $lineData), strlen($1);                
                
                if ($position > $indentations[elements $indentations - 1]) {
                    # The last child of the current parent is now the new parent
                    # unless the current parent has no children.
                    
                    my $lp = $parents[elements $parents - 1];
                    if ($lp.childCount() > 0) {
                        my $lc = $lp.child($lp.childCount() - 1);
                        $parents += $lp.child($lp.childCount() - 1);
                        $indentations += $position;
                    }
                } else {
                    while ($position < $indentations[elements $indentations - 1] && elements $parents > 0) {
                        pop $parents;
                        pop $indentations;
                    }
                }

                # Append a new item to the current parent's list of children.
                my $lp = $parents[elements $parents - 1];
                $lp.insertChildren($lp.childCount(), 1, $.rootItem.columnCount());
                for (my $column = 0; $column < elements $columnData; ++$column) {
                    $lp.child($lp.childCount() - 1).setData($column, $columnData[$column]);
                }
            }
            
            $number++;
        }
    }
}

class editable_tree_model inherits QApplication {
    constructor() {
        my MainWindow $window();
        $window.show();
        $.exec();
    }
}
