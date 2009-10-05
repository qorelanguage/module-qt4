#
# Form generated from reading ui file 'untitled.ui'
#
# Created: Sat Aug 22 20:37:50 2009
#      by: Qt User Interface Compiler version 4.5.1
#
#

class Ui_MainWindow
{

    # public widget: $.centralwidget;
    # public layout: $.gridLayout_2;
    # public layout: $.verticalLayout;
    # public widget: $.pushButton;
    # public widget: $.pushButton_2;
    # public widget: $.pushButton_3;
    # public layout: $.verticalLayout_2;
    # public widget: $.radioButton;
    # public widget: $.radioButton_2;
    # public spacer: $.verticalSpacer;
    # public widget: $.checkBox;
    # public widget: $.groupBox;
    # public layout: $.gridLayout;
    # public widget: $.fontComboBox;
    # public widget: $.spinBox;
    # public widget: $.dateTimeEdit;
    # public spacer: $.verticalSpacer_2;
    # public spacer: $.verticalSpacer_4;
    # public spacer: $.verticalSpacer_3;
    # public widget: $.line;
    # public layout: $.horizontalLayout;
    # public widget: $.label;
    # public widget: $.progressBar;
    # public widget: $.horizontalSlider;
    # public widget: $.lineEdit;
    # public widget: $.menubar;
    # public widget: $.statusbar;

    setupUi( $MainWindow ) {
    #if ($MainWindow.objectName() == "") {
    #    $MainWindow.setObjectName("$MainWindow");
    #}
    $MainWindow.resize(669, 280);
    $.centralwidget = new QWidget($MainWindow);
    $.centralwidget.setObjectName("$.centralwidget");
    $.gridLayout_2 = new QGridLayout($.centralwidget);
    $.gridLayout_2.setObjectName("$.gridLayout_2");
    $.verticalLayout = new QVBoxLayout();
    $.verticalLayout.setObjectName("$.verticalLayout");
    $.pushButton = new QPushButton($.centralwidget);
    $.pushButton.setObjectName("$.pushButton");

    $.verticalLayout.addWidget($.pushButton);

    $.pushButton_2 = new QPushButton($.centralwidget);
    $.pushButton_2.setObjectName("$.pushButton_2");

    $.verticalLayout.addWidget($.pushButton_2);

    $.pushButton_3 = new QPushButton($.centralwidget);
    $.pushButton_3.setObjectName("$.pushButton_3");

    $.verticalLayout.addWidget($.pushButton_3);


    $.gridLayout_2.addLayout($.verticalLayout, 0, 0, 1, 1);

    $.verticalLayout_2 = new QVBoxLayout();
    $.verticalLayout_2.setObjectName("$.verticalLayout_2");
    $.radioButton = new QRadioButton($.centralwidget);
    $.radioButton.setObjectName("$.radioButton");

    $.verticalLayout_2.addWidget($.radioButton);

    $.radioButton_2 = new QRadioButton($.centralwidget);
    $.radioButton_2.setObjectName("$.radioButton_2");

    $.verticalLayout_2.addWidget($.radioButton_2);

    $.verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, Expanding);


    $.verticalLayout_2.addItem($.verticalSpacer);

    $.checkBox = new QCheckBox($.centralwidget);
    $.checkBox.setObjectName("$.checkBox");

    $.verticalLayout_2.addWidget($.checkBox);


    $.gridLayout_2.addLayout($.verticalLayout_2, 0, 1, 1, 1);

    $.groupBox = new QGroupBox($.centralwidget);
    $.groupBox.setObjectName("$.groupBox");
    $.gridLayout = new QGridLayout($.groupBox);
    $.gridLayout.setObjectName("$.gridLayout");
    $.fontComboBox = new QFontComboBox($.groupBox);
    $.fontComboBox.setObjectName("$.fontComboBox");

    $.gridLayout.addWidget($.fontComboBox, 0, 0, 1, 1);

    $.spinBox = new QSpinBox($.groupBox);
    $.spinBox.setObjectName("$.spinBox");

    $.gridLayout.addWidget($.spinBox, 1, 0, 1, 1);

    $.dateTimeEdit = new QDateTimeEdit($.groupBox);
    $.dateTimeEdit.setObjectName("$.dateTimeEdit");

    $.gridLayout.addWidget($.dateTimeEdit, 2, 0, 1, 1);


    $.gridLayout_2.addWidget($.groupBox, 0, 2, 2, 1);

    $.verticalSpacer_2 = new QSpacerItem(20, 47, QSizePolicy::Minimum, Expanding);


    $.gridLayout_2.addItem($.verticalSpacer_2, 1, 0, 2, 1);

    $.verticalSpacer_4 = new QSpacerItem(20, 47, QSizePolicy::Minimum, Expanding);


    $.gridLayout_2.addItem($.verticalSpacer_4, 1, 1, 2, 1);

    $.verticalSpacer_3 = new QSpacerItem(20, 43, QSizePolicy::Minimum, Expanding);


    $.gridLayout_2.addItem($.verticalSpacer_3, 2, 2, 1, 1);

    $.line = new QFrame($.centralwidget);
    $.line.setObjectName("$.line");
    $.line.setFrameShape(QFrame::HLine);
    $.line.setFrameShadow(QFrame::Sunken);

    $.gridLayout_2.addWidget($.line, 3, 0, 1, 3);

    $.horizontalLayout = new QHBoxLayout();
    $.horizontalLayout.setObjectName("$.horizontalLayout");
    $.label = new QLabel($.centralwidget);
    $.label.setObjectName("$.label");

    $.horizontalLayout.addWidget($.label);

    $.progressBar = new QProgressBar($.centralwidget);
    $.progressBar.setObjectName("$.progressBar");
    $.progressBar.setValue( 24);

    $.horizontalLayout.addWidget($.progressBar);

    $.horizontalSlider = new QSlider($.centralwidget);
    $.horizontalSlider.setObjectName("$.horizontalSlider");
    $.horizontalSlider.setOrientation( Qt::Horizontal);

    $.horizontalLayout.addWidget($.horizontalSlider);

    $.lineEdit = new QLineEdit($.centralwidget);
    $.lineEdit.setObjectName("$.lineEdit");

    $.horizontalLayout.addWidget($.lineEdit);


    $.gridLayout_2.addLayout($.horizontalLayout, 4, 0, 1, 3);

    $MainWindow.setCentralWidget($.centralwidget);
    $.menubar = new QMenuBar($MainWindow);
    $.menubar.setObjectName("$.menubar");
    $.menubar.setGeometry( new QRect(0, 0, 669, 24));
    $MainWindow.setMenuBar($.menubar);
    $.statusbar = new QStatusBar($MainWindow);
    $.statusbar.setObjectName("$.statusbar");
    $MainWindow.setStatusBar($.statusbar);

    $.retranslateUi( $MainWindow );

    QMetaObject::connectSlotsByName($MainWindow);
    } # setupUi

    retranslateUi( $MainWindow) {
    $MainWindow.setWindowTitle( QApplication::translate("MainWindow", "MainWindow", "", QCoreApplication::UnicodeUTF8));
    $.pushButton.setText( QApplication::translate("MainWindow", "PushButton", "", QCoreApplication::UnicodeUTF8));
    $.pushButton_2.setText( QApplication::translate("MainWindow", "PushButton", "", QCoreApplication::UnicodeUTF8));
    $.pushButton_3.setText( QApplication::translate("MainWindow", "PushButton", "", QCoreApplication::UnicodeUTF8));
    $.radioButton.setText( QApplication::translate("MainWindow", "RadioButton", "", QCoreApplication::UnicodeUTF8));
    $.radioButton_2.setText( QApplication::translate("MainWindow", "RadioButton", "", QCoreApplication::UnicodeUTF8));
    $.checkBox.setText( QApplication::translate("MainWindow", "CheckBox", "", QCoreApplication::UnicodeUTF8));
    $.groupBox.setTitle( QApplication::translate("MainWindow", "GroupBox", "", QCoreApplication::UnicodeUTF8));
    $.label.setText( QApplication::translate("MainWindow", "TextLabel", "", QCoreApplication::UnicodeUTF8));
    } # retranslateUi

}

namespace Ui {
    class MainWindow inherits Ui_MainWindow {
    }
}  # namespace Ui

