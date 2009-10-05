#
# Form generated from reading ui file 'simplegui.ui'
#
# Created: Mon Aug 24 16:01:06 2009
#      by: Qt User Interface Compiler version 4.5.2
#
#

class Ui_MainWindow
{

    # public action: $.actionBar;
    # public widget: $.centralwidget;
    # public layout: $.gridLayout;
    # public widget: $.comboBox;
    # public spacer: $.verticalSpacer;
    # public widget: $.radioButton;
    # public widget: $.pushButton;
    # public widget: $.comboBox_2;
    # public widget: $.fontComboBox;
    # public widget: $.lineEdit;
    # public widget: $.statusbar;

    setupUi( $MainWindow ) {
    if ($MainWindow.objectName() == "") {
        $MainWindow.setObjectName("$MainWindow");
    }
    $MainWindow.resize(418, 244);
    $.actionBar = new QAction( $MainWindow);
    $.actionBar.setObjectName("$.actionBar");
    $.centralwidget = new QWidget($MainWindow);
    $.centralwidget.setObjectName("$.centralwidget");
    $.gridLayout = new QGridLayout($.centralwidget);
    $.gridLayout.setObjectName("$.gridLayout");
    $.comboBox = new QComboBox($.centralwidget);
    $.comboBox.setObjectName("$.comboBox");

    $.gridLayout.addWidget($.comboBox, 0, 0, 1, 1);

    $.verticalSpacer = new QSpacerItem(20, 56, QSizePolicy::Minimum, Expanding);


    $.gridLayout.addItem($.verticalSpacer, 6, 0, 1, 1);

    $.radioButton = new QRadioButton($.centralwidget);
    $.radioButton.setObjectName("$.radioButton");

    $.gridLayout.addWidget($.radioButton, 1, 0, 1, 1);

    $.pushButton = new QPushButton($.centralwidget);
    $.pushButton.setObjectName("$.pushButton");

    $.gridLayout.addWidget($.pushButton, 2, 0, 1, 1);

    $.comboBox_2 = new QComboBox($.centralwidget);
    $.comboBox_2.setObjectName("$.comboBox_2");

    $.gridLayout.addWidget($.comboBox_2, 3, 0, 1, 1);

    $.fontComboBox = new QFontComboBox($.centralwidget);
    $.fontComboBox.setObjectName("$.fontComboBox");

    $.gridLayout.addWidget($.fontComboBox, 4, 0, 1, 1);

    $.lineEdit = new QLineEdit($.centralwidget);
    $.lineEdit.setObjectName("$.lineEdit");

    $.gridLayout.addWidget($.lineEdit, 5, 0, 1, 1);

    $MainWindow.setCentralWidget($.centralwidget);
    $.statusbar = new QStatusBar($MainWindow);
    $.statusbar.setObjectName("$.statusbar");
    $MainWindow.setStatusBar($.statusbar);

    printf("MainWindow title: %N\n", $MainWindow.windowTitle());

    $.retranslateUi( $MainWindow );

    QMetaObject::connectSlotsByName($MainWindow);
    } # setupUi

    retranslateUi( $MainWindow) {
    $MainWindow.setWindowTitle( QApplication::translate("MainWindow", "MainWindow", "", QCoreApplication::UnicodeUTF8));
    $.actionBar.setText( QApplication::translate("MainWindow", "Bar!", "", QCoreApplication::UnicodeUTF8));
    $.radioButton.setText( QApplication::translate("MainWindow", "RadioButton", "", QCoreApplication::UnicodeUTF8));
    $.pushButton.setText( QApplication::translate("MainWindow", "PushButton", "", QCoreApplication::UnicodeUTF8));
    } # retranslateUi

}

namespace Ui {
    class MainWindow inherits Ui_MainWindow {
    }
}  # namespace Ui

