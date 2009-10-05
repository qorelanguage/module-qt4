#
# Form generated from reading ui file 't8_modelview.ui'
#
# Created: Wed Sep 9 12:52:32 2009
#      by: Qt User Interface Compiler version 4.5.2
#
#

class Ui_MainWindow
{

    # public action: $.action_Quit;
    # public widget: $.centralwidget;
    # public layout: $.gridLayout_2;
    # public widget: $.splitter_2;
    # public widget: $.treeView;
    # public widget: $.splitter;
    # public widget: $.columnView;
    # public widget: $.groupBox;
    # public layout: $.gridLayout;
    # public widget: $.lineEdit;
    # public widget: $.filteredView;
    # public widget: $.menubar;
    # public widget: $.menu_File;
    # public widget: $.statusbar;

    setupUi( $MainWindow ) {
    if ($MainWindow.objectName() == "") {
        $MainWindow.setObjectName("$MainWindow");
    }
    $MainWindow.resize(800, 600);
    $.action_Quit = new QAction( $MainWindow);
    $.action_Quit.setObjectName("$.action_Quit");
    $.centralwidget = new QWidget($MainWindow);
    $.centralwidget.setObjectName("$.centralwidget");
    $.gridLayout_2 = new QGridLayout($.centralwidget);
    $.gridLayout_2.setObjectName("$.gridLayout_2");
    $.splitter_2 = new QSplitter($.centralwidget);
    $.splitter_2.setObjectName("$.splitter_2");
    $.splitter_2.setOrientation( Qt::Horizontal);
    $.treeView = new QTreeView($.splitter_2);
    $.treeView.setObjectName("$.treeView");
    $.splitter_2.addWidget($.treeView);
    $.splitter = new QSplitter($.splitter_2);
    $.splitter.setObjectName("$.splitter");
    $.splitter.setOrientation( Qt::Vertical);
    $.columnView = new QColumnView($.splitter);
    $.columnView.setObjectName("$.columnView");
    $.splitter.addWidget($.columnView);
    $.groupBox = new QGroupBox($.splitter);
    $.groupBox.setObjectName("$.groupBox");
    $.gridLayout = new QGridLayout($.groupBox);
    $.gridLayout.setObjectName("$.gridLayout");
    $.lineEdit = new QLineEdit($.groupBox);
    $.lineEdit.setObjectName("$.lineEdit");

    $.gridLayout.addWidget($.lineEdit, 0, 0, 1, 1);

    $.filteredView = new QTreeView($.groupBox);
    $.filteredView.setObjectName("$.filteredView");

    $.gridLayout.addWidget($.filteredView, 1, 0, 1, 1);

    $.splitter.addWidget($.groupBox);
    $.splitter_2.addWidget($.splitter);

    $.gridLayout_2.addWidget($.splitter_2, 0, 0, 1, 1);

    $MainWindow.setCentralWidget($.centralwidget);
    $.menubar = new QMenuBar($MainWindow);
    $.menubar.setObjectName("$.menubar");
    $.menubar.setGeometry( new QRect(0, 0, 800, 24));
    $.menu_File = new QMenu($.menubar);
    $.menu_File.setObjectName("$.menu_File");
    $MainWindow.setMenuBar($.menubar);
    $.statusbar = new QStatusBar($MainWindow);
    $.statusbar.setObjectName("$.statusbar");
    $MainWindow.setStatusBar($.statusbar);

    $.menubar.addAction($.menu_File.menuAction());
    $.menu_File.addAction($.action_Quit);

    $.retranslateUi( $MainWindow );

    QMetaObject::connectSlotsByName($MainWindow);
    } # setupUi

    retranslateUi( $MainWindow) {
    $MainWindow.setWindowTitle( QApplication::translate("MainWindow", "MainWindow", "", QCoreApplication::UnicodeUTF8));
    $.action_Quit.setText( QApplication::translate("MainWindow", "&Quit", "", QCoreApplication::UnicodeUTF8));
    $.groupBox.setTitle( QApplication::translate("MainWindow", "Filter", "", QCoreApplication::UnicodeUTF8));
    $.menu_File.setTitle( QApplication::translate("MainWindow", "&File", "", QCoreApplication::UnicodeUTF8));
    } # retranslateUi

}

namespace Ui {
    class MainWindow inherits Ui_MainWindow {
    }
}  # namespace Ui

