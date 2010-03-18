#
# Form generated from reading ui file 'form.ui'
#
# Created: Thu Mar 18 16:33:40 2010
#      by: Qt User Interface Compiler version 4.6.2
#
#

class Ui_Form
{

    private { QAction $.actionFoo; }
    private { QGridLayout $.gridLayout_6; }
    private { QVBoxLayout $.verticalLayout; }
    private { QPushButton $.pushButton; }
    private { QHBoxLayout $.horizontalLayout; }
    private { QToolButton $.toolButton; }
    private { QRadioButton $.radioButton; }
    private { QCheckBox $.checkBox; }
    private { QCommandLinkButton $.commandLinkButton; }
    private { QScrollBar $.verticalScrollBar; }
    private { QDialogButtonBox $.buttonBox; }
    private { QSplitter $.splitter_3; }
    private { QSplitter $.splitter_2; }
    private { QListView $.listView; }
    private { QTreeView $.treeView; }
    private { QSplitter $.splitter; }
    private { QTableView $.tableView; }
    private { QColumnView $.columnView; }
    private { QListWidget $.listWidget; }
    private { QTreeWidget $.treeWidget; }
    private { QTableWidget $.tableWidget; }
    private { QScrollBar $.horizontalScrollBar; }
    private { QTextBrowser $.textBrowser; }
    private { QGroupBox $.groupBox; }
    private { QGridLayout $.gridLayout_2; }
    private { QScrollArea $.scrollArea; }
    private { QWidget $.scrollAreaWidgetContents; }
    private { QGridLayout $.gridLayout; }
    private { QRadioButton $.radioButton_2; }
    private { QToolButton $.toolButton_2; }
    private { QToolBox $.toolBox; }
    private { QWidget $.page; }
    private { QGridLayout $.gridLayout_3; }
    private { QFrame $.frame; }
    private { QWidget $.page_2; }
    private { QTabWidget $.tabWidget; }
    private { QWidget $.tab; }
    private { QGridLayout $.gridLayout_4; }
    private { QWidget $.widget; }
    private { QWidget $.tab_2; }
    private { QFontComboBox $.fontComboBox; }
    private { QLineEdit $.lineEdit; }
    private { QDoubleSpinBox $.doubleSpinBox; }
    private { QSpinBox $.spinBox; }
    private { QLabel $.label; }
    private { QTextEdit $.textEdit; }
    private { QPlainTextEdit $.plainTextEdit; }
    private { QDockWidget $.dockWidget; }
    private { QWidget $.dockWidgetContents; }
    private { QGridLayout $.gridLayout_5; }
    private { QComboBox $.comboBox; }
    private { QMdiArea $.mdiArea; }
    private { QTimeEdit $.timeEdit; }
    private { QDateEdit $.dateEdit; }
    private { QSlider $.horizontalSlider; }
    private { QDateTimeEdit $.dateTimeEdit; }
    private { QDial $.dial; }
    private { QSlider $.verticalSlider; }
    private { QGraphicsView $.graphicsView; }
    private { QCalendarWidget $.calendarWidget; }
    private { QProgressBar $.progressBar; }
    private { QFrame $.line_2; }
    private { QLCDNumber $.lcdNumber; }
    private { QFrame $.line; }

    setupUi( any $Form ) returns nothing {
    if ($Form.objectName() == "") {
        $Form.setObjectName("$Form");
    }
    $Form.resize(646, 793);
    $.actionFoo = new QAction( $Form);
    $.actionFoo.setObjectName("$.actionFoo");
    $.gridLayout_6 = new QGridLayout($Form);
    $.gridLayout_6.setObjectName("$.gridLayout_6");
    $.verticalLayout = new QVBoxLayout();
    $.verticalLayout.setObjectName("$.verticalLayout");
    $.pushButton = new QPushButton($Form);
    $.pushButton.setObjectName("$.pushButton");

    $.verticalLayout.addWidget($.pushButton);

    $.horizontalLayout = new QHBoxLayout();
    $.horizontalLayout.setObjectName("$.horizontalLayout");
    $.toolButton = new QToolButton($Form);
    $.toolButton.setObjectName("$.toolButton");

    $.horizontalLayout.addWidget($.toolButton);

    $.radioButton = new QRadioButton($Form);
    $.radioButton.setObjectName("$.radioButton");

    $.horizontalLayout.addWidget($.radioButton);


    $.verticalLayout.addLayout($.horizontalLayout);


    $.gridLayout_6.addLayout($.verticalLayout, 0, 0, 2, 2);

    $.checkBox = new QCheckBox($Form);
    $.checkBox.setObjectName("$.checkBox");

    $.gridLayout_6.addWidget($.checkBox, 0, 3, 1, 2);

    $.commandLinkButton = new QCommandLinkButton($Form);
    $.commandLinkButton.setObjectName("$.commandLinkButton");

    $.gridLayout_6.addWidget($.commandLinkButton, 1, 3, 2, 3);

    $.verticalScrollBar = new QScrollBar($Form);
    $.verticalScrollBar.setObjectName("$.verticalScrollBar");
    $.verticalScrollBar.setOrientation( Qt::Vertical);

    $.gridLayout_6.addWidget($.verticalScrollBar, 2, 8, 3, 1);

    $.buttonBox = new QDialogButtonBox($Form);
    $.buttonBox.setObjectName("$.buttonBox");
    $.buttonBox.setStandardButtons( QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

    $.gridLayout_6.addWidget($.buttonBox, 3, 0, 1, 4);

    $.splitter_3 = new QSplitter($Form);
    $.splitter_3.setObjectName("$.splitter_3");
    $.splitter_3.setOrientation( Qt::Vertical);
    $.splitter_2 = new QSplitter($.splitter_3);
    $.splitter_2.setObjectName("$.splitter_2");
    $.splitter_2.setOrientation( Qt::Horizontal);
    $.listView = new QListView($.splitter_2);
    $.listView.setObjectName("$.listView");
    $.splitter_2.addWidget($.listView);
    $.treeView = new QTreeView($.splitter_2);
    $.treeView.setObjectName("$.treeView");
    $.splitter_2.addWidget($.treeView);
    $.splitter_3.addWidget($.splitter_2);
    $.splitter = new QSplitter($.splitter_3);
    $.splitter.setObjectName("$.splitter");
    $.splitter.setOrientation( Qt::Horizontal);
    $.tableView = new QTableView($.splitter);
    $.tableView.setObjectName("$.tableView");
    $.splitter.addWidget($.tableView);
    $.columnView = new QColumnView($.splitter);
    $.columnView.setObjectName("$.columnView");
    $.splitter.addWidget($.columnView);
    $.splitter_3.addWidget($.splitter);

    $.gridLayout_6.addWidget($.splitter_3, 4, 0, 1, 7);

    $.listWidget = new QListWidget($Form);
    $.listWidget.setObjectName("$.listWidget");

    $.gridLayout_6.addWidget($.listWidget, 5, 0, 2, 1);

    $.treeWidget = new QTreeWidget($Form);
    $.treeWidget.setObjectName("$.treeWidget");

    $.gridLayout_6.addWidget($.treeWidget, 5, 1, 2, 4);

    $.tableWidget = new QTableWidget($Form);
    $.tableWidget.setObjectName("$.tableWidget");

    $.gridLayout_6.addWidget($.tableWidget, 5, 5, 2, 2);

    $.horizontalScrollBar = new QScrollBar($Form);
    $.horizontalScrollBar.setObjectName("$.horizontalScrollBar");
    $.horizontalScrollBar.setOrientation( Qt::Horizontal);

    $.gridLayout_6.addWidget($.horizontalScrollBar, 5, 7, 1, 4);

    $.textBrowser = new QTextBrowser($Form);
    $.textBrowser.setObjectName("$.textBrowser");

    $.gridLayout_6.addWidget($.textBrowser, 6, 8, 2, 3);

    $.groupBox = new QGroupBox($Form);
    $.groupBox.setObjectName("$.groupBox");
    $.gridLayout_2 = new QGridLayout($.groupBox);
    $.gridLayout_2.setObjectName("$.gridLayout_2");
    $.scrollArea = new QScrollArea($.groupBox);
    $.scrollArea.setObjectName("$.scrollArea");
    $.scrollArea.setWidgetResizable( True);
    $.scrollAreaWidgetContents = new QWidget();
    $.scrollAreaWidgetContents.setObjectName("$.scrollAreaWidgetContents");
    $.scrollAreaWidgetContents.setGeometry( new QRect(0, 0, 123, 61));
    $.gridLayout = new QGridLayout($.scrollAreaWidgetContents);
    $.gridLayout.setObjectName("$.gridLayout");
    $.radioButton_2 = new QRadioButton($.scrollAreaWidgetContents);
    $.radioButton_2.setObjectName("$.radioButton_2");

    $.gridLayout.addWidget($.radioButton_2, 0, 0, 1, 1);

    $.toolButton_2 = new QToolButton($.scrollAreaWidgetContents);
    $.toolButton_2.setObjectName("$.toolButton_2");

    $.gridLayout.addWidget($.toolButton_2, 1, 0, 1, 1);

    $.scrollArea.setWidget($.scrollAreaWidgetContents);

    $.gridLayout_2.addWidget($.scrollArea, 0, 0, 1, 1);


    $.gridLayout_6.addWidget($.groupBox, 7, 0, 2, 2);

    $.toolBox = new QToolBox($Form);
    $.toolBox.setObjectName("$.toolBox");
    $.page = new QWidget();
    $.page.setObjectName("$.page");
    $.page.setGeometry( new QRect(0, 0, 186, 73));
    $.gridLayout_3 = new QGridLayout($.page);
    $.gridLayout_3.setObjectName("$.gridLayout_3");
    $.frame = new QFrame($.page);
    $.frame.setObjectName("$.frame");
    $.frame.setFrameShape( QFrame::StyledPanel);
    $.frame.setFrameShadow( QFrame::Raised);

    $.gridLayout_3.addWidget($.frame, 0, 0, 1, 1);

    $.toolBox.addItem($.page, QApplication::translate("Form", "Page 1", "", QCoreApplication::UnicodeUTF8));
    $.page_2 = new QWidget();
    $.page_2.setObjectName("$.page_2");
    $.page_2.setGeometry( new QRect(0, 0, 186, 73));
    $.toolBox.addItem($.page_2, QApplication::translate("Form", "Page 2", "", QCoreApplication::UnicodeUTF8));

    $.gridLayout_6.addWidget($.toolBox, 7, 3, 3, 3);

    $.tabWidget = new QTabWidget($Form);
    $.tabWidget.setObjectName("$.tabWidget");
    $.tab = new QWidget();
    $.tab.setObjectName("$.tab");
    $.gridLayout_4 = new QGridLayout($.tab);
    $.gridLayout_4.setObjectName("$.gridLayout_4");
    $.widget = new QWidget($.tab);
    $.widget.setObjectName("$.widget");

    $.gridLayout_4.addWidget($.widget, 0, 0, 1, 1);

    $.tabWidget.addTab($.tab, QApplication::translate("Form", "Tab 1", "", QCoreApplication::UnicodeUTF8));
    $.tab_2 = new QWidget();
    $.tab_2.setObjectName("$.tab_2");
    $.tabWidget.addTab($.tab_2, QApplication::translate("Form", "Tab 2", "", QCoreApplication::UnicodeUTF8));

    $.gridLayout_6.addWidget($.tabWidget, 7, 6, 1, 1);

    $.fontComboBox = new QFontComboBox($Form);
    $.fontComboBox.setObjectName("$.fontComboBox");

    $.gridLayout_6.addWidget($.fontComboBox, 8, 6, 1, 5);

    $.lineEdit = new QLineEdit($Form);
    $.lineEdit.setObjectName("$.lineEdit");

    $.gridLayout_6.addWidget($.lineEdit, 9, 6, 1, 2);

    $.doubleSpinBox = new QDoubleSpinBox($Form);
    $.doubleSpinBox.setObjectName("$.doubleSpinBox");

    $.gridLayout_6.addWidget($.doubleSpinBox, 9, 8, 1, 1);

    $.spinBox = new QSpinBox($Form);
    $.spinBox.setObjectName("$.spinBox");

    $.gridLayout_6.addWidget($.spinBox, 9, 9, 1, 2);

    $.label = new QLabel($Form);
    $.label.setObjectName("$.label");

    $.gridLayout_6.addWidget($.label, 10, 4, 1, 1);

    $.textEdit = new QTextEdit($Form);
    $.textEdit.setObjectName("$.textEdit");

    $.gridLayout_6.addWidget($.textEdit, 10, 6, 2, 1);

    $.plainTextEdit = new QPlainTextEdit($Form);
    $.plainTextEdit.setObjectName("$.plainTextEdit");

    $.gridLayout_6.addWidget($.plainTextEdit, 10, 8, 2, 3);

    $.dockWidget = new QDockWidget($Form);
    $.dockWidget.setObjectName("$.dockWidget");
    $.dockWidgetContents = new QWidget($.dockWidget);
    $.dockWidgetContents.setObjectName("$.dockWidgetContents");
    $.gridLayout_5 = new QGridLayout($.dockWidgetContents);
    $.gridLayout_5.setObjectName("$.gridLayout_5");
    $.gridLayout_5.setContentsMargins(0, 0, 0, 0);
    $.comboBox = new QComboBox($.dockWidgetContents);
    $.comboBox.setObjectName("$.comboBox");

    $.gridLayout_5.addWidget($.comboBox, 0, 0, 1, 1);

    $.dockWidget.setWidget($.dockWidgetContents);

    $.gridLayout_6.addWidget($.dockWidget, 11, 0, 2, 1);

    $.mdiArea = new QMdiArea($Form);
    $.mdiArea.setObjectName("$.mdiArea");

    $.gridLayout_6.addWidget($.mdiArea, 11, 3, 4, 3);

    $.timeEdit = new QTimeEdit($Form);
    $.timeEdit.setObjectName("$.timeEdit");

    $.gridLayout_6.addWidget($.timeEdit, 12, 6, 1, 2);

    $.dateEdit = new QDateEdit($Form);
    $.dateEdit.setObjectName("$.dateEdit");

    $.gridLayout_6.addWidget($.dateEdit, 12, 8, 1, 3);

    $.horizontalSlider = new QSlider($Form);
    $.horizontalSlider.setObjectName("$.horizontalSlider");
    $.horizontalSlider.setOrientation( Qt::Horizontal);

    $.gridLayout_6.addWidget($.horizontalSlider, 13, 0, 1, 3);

    $.dateTimeEdit = new QDateTimeEdit($Form);
    $.dateTimeEdit.setObjectName("$.dateTimeEdit");

    $.gridLayout_6.addWidget($.dateTimeEdit, 13, 6, 1, 4);

    $.dial = new QDial($Form);
    $.dial.setObjectName("$.dial");

    $.gridLayout_6.addWidget($.dial, 13, 10, 2, 1);

    $.verticalSlider = new QSlider($Form);
    $.verticalSlider.setObjectName("$.verticalSlider");
    $.verticalSlider.setOrientation( Qt::Vertical);

    $.gridLayout_6.addWidget($.verticalSlider, 14, 0, 2, 1);

    $.graphicsView = new QGraphicsView($Form);
    $.graphicsView.setObjectName("$.graphicsView");

    $.gridLayout_6.addWidget($.graphicsView, 15, 3, 1, 2);

    $.calendarWidget = new QCalendarWidget($Form);
    $.calendarWidget.setObjectName("$.calendarWidget");

    $.gridLayout_6.addWidget($.calendarWidget, 15, 6, 2, 5);

    $.progressBar = new QProgressBar($Form);
    $.progressBar.setObjectName("$.progressBar");
    $.progressBar.setValue( 24);

    $.gridLayout_6.addWidget($.progressBar, 16, 0, 1, 1);

    $.line_2 = new QFrame($Form);
    $.line_2.setObjectName("$.line_2");
    $.line_2.setFrameShape(QFrame::VLine);
    $.line_2.setFrameShadow(QFrame::Sunken);

    $.gridLayout_6.addWidget($.line_2, 16, 2, 1, 1);

    $.lcdNumber = new QLCDNumber($Form);
    $.lcdNumber.setObjectName("$.lcdNumber");

    $.gridLayout_6.addWidget($.lcdNumber, 16, 3, 1, 2);

    $.line = new QFrame($Form);
    $.line.setObjectName("$.line");
    $.line.setFrameShape(QFrame::HLine);
    $.line.setFrameShadow(QFrame::Sunken);

    $.gridLayout_6.addWidget($.line, 17, 0, 1, 1);

    QWidget::setTabOrder($.checkBox, $.toolButton);
    QWidget::setTabOrder($.toolButton, $.radioButton);
    QWidget::setTabOrder($.radioButton, $.pushButton);
    QWidget::setTabOrder($.pushButton, $.commandLinkButton);
    QWidget::setTabOrder($.commandLinkButton, $.buttonBox);
    QWidget::setTabOrder($.buttonBox, $.listView);
    QWidget::setTabOrder($.listView, $.treeView);
    QWidget::setTabOrder($.treeView, $.tableView);
    QWidget::setTabOrder($.tableView, $.columnView);
    QWidget::setTabOrder($.columnView, $.listWidget);
    QWidget::setTabOrder($.listWidget, $.treeWidget);
    QWidget::setTabOrder($.treeWidget, $.tableWidget);
    QWidget::setTabOrder($.tableWidget, $.tabWidget);
    QWidget::setTabOrder($.tabWidget, $.fontComboBox);
    QWidget::setTabOrder($.fontComboBox, $.lineEdit);
    QWidget::setTabOrder($.lineEdit, $.textEdit);
    QWidget::setTabOrder($.textEdit, $.plainTextEdit);
    QWidget::setTabOrder($.plainTextEdit, $.spinBox);
    QWidget::setTabOrder($.spinBox, $.doubleSpinBox);
    QWidget::setTabOrder($.doubleSpinBox, $.timeEdit);
    QWidget::setTabOrder($.timeEdit, $.dateEdit);
    QWidget::setTabOrder($.dateEdit, $.dateTimeEdit);
    QWidget::setTabOrder($.dateTimeEdit, $.dial);
    QWidget::setTabOrder($.dial, $.horizontalSlider);
    QWidget::setTabOrder($.horizontalSlider, $.verticalSlider);
    QWidget::setTabOrder($.verticalSlider, $.textBrowser);
    QWidget::setTabOrder($.textBrowser, $.graphicsView);
    QWidget::setTabOrder($.graphicsView, $.calendarWidget);
    QWidget::setTabOrder($.calendarWidget, $.scrollArea);
    QWidget::setTabOrder($.scrollArea, $.radioButton_2);
    QWidget::setTabOrder($.radioButton_2, $.toolButton_2);
    QWidget::setTabOrder($.toolButton_2, $.comboBox);

    $.retranslateUi( $Form );
    $Form.connect($Form, SIGNAL('customContextMenuRequested(QPoint)'), SLOT('update()'));

    $.toolBox.setCurrentIndex(0);


    QMetaObject::connectSlotsByName($Form);
    } # setupUi

    retranslateUi( any $Form) returns nothing {
    $Form.setWindowTitle( QApplication::translate("Form", "Form", "", QCoreApplication::UnicodeUTF8));
    $.actionFoo.setText( QApplication::translate("Form", "Foo", "", QCoreApplication::UnicodeUTF8));
    $.pushButton.setText( QApplication::translate("Form", "PushButton", "", QCoreApplication::UnicodeUTF8));
    $.toolButton.setText( QApplication::translate("Form", "...", "", QCoreApplication::UnicodeUTF8));
    $.radioButton.setText( QApplication::translate("Form", "RadioButton", "", QCoreApplication::UnicodeUTF8));
    $.checkBox.setText( QApplication::translate("Form", "CheckBox", "", QCoreApplication::UnicodeUTF8));
    $.commandLinkButton.setText( QApplication::translate("Form", "CommandLinkButton", "", QCoreApplication::UnicodeUTF8));
    $.treeWidget.headerItem().setText(0, QApplication::translate("Form", "1", "", QCoreApplication::UnicodeUTF8));
    $.groupBox.setTitle( QApplication::translate("Form", "GroupBox", "", QCoreApplication::UnicodeUTF8));
    $.radioButton_2.setText( QApplication::translate("Form", "RadioButton", "", QCoreApplication::UnicodeUTF8));
    $.toolButton_2.setText( QApplication::translate("Form", "...", "", QCoreApplication::UnicodeUTF8));
    $.toolBox.setItemText($.toolBox.indexOf($.page), QApplication::translate("Form", "Page 1", "", QCoreApplication::UnicodeUTF8));
    $.toolBox.setItemText($.toolBox.indexOf($.page_2), QApplication::translate("Form", "Page 2", "", QCoreApplication::UnicodeUTF8));
    $.tabWidget.setTabText($.tabWidget.indexOf($.tab), QApplication::translate("Form", "Tab 1", "", QCoreApplication::UnicodeUTF8));
    $.tabWidget.setTabText($.tabWidget.indexOf($.tab_2), QApplication::translate("Form", "Tab 2", "", QCoreApplication::UnicodeUTF8));
    $.label.setText( QApplication::translate("Form", "TextLabel", "", QCoreApplication::UnicodeUTF8));
    } # retranslateUi

}

namespace Ui {
    class Form inherits Ui_Form {
    }
}  # namespace Ui

