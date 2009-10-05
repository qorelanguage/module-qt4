#!/usr/bin/env qore

%requires qt4

%include t8_modelview.q


class MainWindow inherits QMainWindow, Ui::MainWindow
{
	private $.dirModel;
	private $.proxyModel;

	constructor() : QMainWindow()
	{
		printf("MainWindow is created");
		$.setupUi($self);

		$.setWindowTitle("Model View test app");

		$.dirModel = new QDirModel($self);

		$.treeView.setModel($.dirModel);

		$.columnView.setModel($.dirModel);

		$.proxyModel = new QSortFilterProxyModel($self);
		$.proxyModel.setSourceModel($.dirModel);
		$.filteredView.setModel($.proxyModel);
		$.proxyModel.setFilterWildcard("*");
	}
}

my $a = new QApplication();
my $w = new MainWindow();

$w.show();
$a.exec();

