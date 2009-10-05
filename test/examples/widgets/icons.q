#!/usr/bin/env qore

# This is basically a direct port of the QT widget example
# "icons" to Qore using Qore's "qt" module.  

# Note that Qore's "qt" module requires QT 4.3 or above 

# use the "qt-gui" module
%requires qt4

# this is an object-oriented program, the application class is "icons_example"
%exec-class icons_example
# require all variables to be explicitly declared
%require-our
# enable all parse warnings
%enable-all-warnings

const NumModes = 4;
const NumStates = 2;

class IconPreviewArea inherits QWidget
{
    constructor($parent) : QWidget($parent)
    {
        $.icon = new QIcon();
        $.size = new QSize();

        my $mainLayout = new QGridLayout();

        my $stateLabels[0] = $.createHeaderLabel($.tr("Off"));
        $stateLabels[1] = $.createHeaderLabel($.tr("On"));
        #Q_ASSERT(NumStates == 2);

        my $modeLabels[0] = $.createHeaderLabel($.tr("Normal"));
        $modeLabels[1] = $.createHeaderLabel($.tr("Active"));
        $modeLabels[2] = $.createHeaderLabel($.tr("Disabled"));
        $modeLabels[3] = $.createHeaderLabel($.tr("Selected"));
        #Q_ASSERT(NumModes == 4);

        for (my $j = 0; $j < NumStates; ++$j)
            $mainLayout.addWidget($stateLabels[$j], $j + 1, 0);

        for (my $i = 0; $i < NumModes; ++$i) {
            $mainLayout.addWidget($modeLabels[$i], 0, $i + 1);

            for (my $j = 0; $j < NumStates; ++$j) {
                $.pixmapLabels[$i][$j] = $.createPixmapLabel();
                $mainLayout.addWidget($.pixmapLabels[$i][$j], $j + 1, $i + 1);
            }
        }
        $.setLayout($mainLayout);
    }

    setIcon($icon)
    {
        $.icon = $icon;
        $.updatePixmapLabels();
    }

    setSize($size)
    {
        if ($size.height() != $.size.height() || $size.width() != $.size.width()) {
            $.size = $size;
            $.updatePixmapLabels();
        }
    }

    createHeaderLabel($text)
    {
        my $label = new QLabel($.tr(sprintf("<b>%s</b>", $text)));
        $label.setAlignment(Qt::AlignCenter);
        return $label;
    }

    createPixmapLabel()
    {
        my $label = new QLabel();
        $label.setEnabled(False);
        $label.setAlignment(Qt::AlignCenter);
        $label.setFrameShape(QFrame::Box);
        $label.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        $label.setBackgroundRole(QPalette::Base);
        $label.setAutoFillBackground(True);
        $label.setMinimumSize(132, 132);
        return $label;
    }

    updatePixmapLabels()
    {
        for (my $i = 0; $i < NumModes; ++$i) {
            my $mode;
            if ($i == 0) {
                $mode = QIcon::Normal;
            } else if ($i == 1) {
                $mode = QIcon::Active;
            } else if ($i == 2) {
                $mode = QIcon::Disabled;
            } else {
                $mode = QIcon::Selected;
            }

            for (my $j = 0; $j < NumStates; ++$j) {
                my $state = ($j == 0) ? QIcon::Off : QIcon::On;
                my $pixmap = $.icon.pixmap($.size, $mode, $state);
                $.pixmapLabels[$i][$j].setPixmap($pixmap);
                $.pixmapLabels[$i][$j].setEnabled(!$pixmap.isNull());
            }
        }
    }
}

class IconSizeSpinBox inherits QSpinBox
{
    constructor($parent) : QSpinBox($parent)
    {
    }

    valueFromText($text)
    {
        return int($text =~ x/(\\d+)(\\s*[xx]\\s*\\d+)?/)[0]; #x/
    }

    textFromValue($value)
    {
        return $.tr(sprintf("%d x %d", $value, $value));
    }

}

class ImageDelegate inherits QItemDelegate
{
    constructor($parent) : QItemDelegate($parent)
    {
        $.createSignal("emitCommitData()");
    }

    createEditor($parent, $option, $index)
    {
        my $comboBox = new QComboBox($parent);
        if ($index.column() == 1) {
            $comboBox.addItem($.tr("Normal"));
            $comboBox.addItem($.tr("Active"));
            $comboBox.addItem($.tr("Disabled"));
            $comboBox.addItem($.tr("Selected"));
        } else if ($index.column() == 2) {
            $comboBox.addItem($.tr("Off"));
            $comboBox.addItem($.tr("On"));
        }
        
        $.connect($comboBox, SIGNAL("activated(int)"), SLOT("emitCommitData()"));
        
        return $comboBox;
    }
    
    setEditorData($comboBox, $index)
    {
        if (getClassName($comboBox) != "QComboBox")
            return;

        my $pos = $comboBox.findText(string($index.model().data($index)), Qt::MatchExactly);
        $comboBox.setCurrentIndex($pos);
    }

    setModelData($comboBox, $model, $index)
    {
        if (getClassName($comboBox) != "QComboBox")
            return;

        $model.setData($index, $comboBox.currentText());
    }
    
    emitCommitData()
    {
        $.emit("commitData(QWidget *)", $.sender());
    }
}

class MainWindow inherits QMainWindow
{
    constructor()
    {
        my $centralWidget = new QWidget();
        $.setCentralWidget($centralWidget);
        
        $.createPreviewGroupBox();
        $.createImagesGroupBox();
        $.createIconSizeGroupBox();

        $.createActions();
        $.createMenus();
        $.createContextMenu();

        my $mainLayout = new QGridLayout();
        $mainLayout.addWidget($.previewGroupBox, 0, 0, 1, 2);
        $mainLayout.addWidget($.imagesGroupBox, 1, 0);
        $mainLayout.addWidget($.iconSizeGroupBox, 1, 1);
        $centralWidget.setLayout($mainLayout);

        $.setWindowTitle($.tr("Icons"));
        $.checkCurrentStyle();
        $.otherRadioButton.click();

        $.resize($.minimumSizeHint());
    }
    about()
    {
        QMessageBox::about($self, $.tr("About Icons"), $.tr("The <b>Icons</b> example illustrates how Qt renders an icon in different modes (active, normal, disabled, and selected) and states (on and off) based on a set of images."));
    }

    changeStyle($checked)
    {
        if (!$checked)
            return;

        my $action = $.sender();
        my $style = QStyleFactory::create($action.data());
        #Q_ASSERT(style);

        QApplication::setStyle($style);

        my $size = $style.pixelMetric(QStyle::PM_SmallIconSize);
        $.smallRadioButton.setText($.tr(sprintf("Small (%d x %d)", $size, $size)));

        $size = $style.pixelMetric(QStyle::PM_LargeIconSize);
        $.largeRadioButton.setText($.tr(sprintf("Large (%d x %d)", $size, $size)));

        $size = $style.pixelMetric(QStyle::PM_ToolBarIconSize);
        $.toolBarRadioButton.setText($.tr(sprintf("Toolbars (%d x %d)", $size, $size)));

        $size = $style.pixelMetric(QStyle::PM_ListViewIconSize);
        $.listViewRadioButton.setText($.tr(sprintf("List views (%d x %d)", $size, $size)));

        $size = $style.pixelMetric(QStyle::PM_IconViewIconSize);
        $.iconViewRadioButton.setText($.tr(sprintf("Icon views (%d x %d)", $size, $size)));

        $size = $style.pixelMetric(QStyle::PM_TabBarIconSize);
        $.tabBarRadioButton.setText($.tr(sprintf("Tab bars (%d x %d)", $size, $size)));
        
        $.changeSize(True);
    }

    changeSize($checked)
    {
        if (!$checked)
            return;

        my $extent;

        if ($.otherRadioButton.isChecked()) {
            $extent = $.otherSpinBox.value();
        } else {
            my $metric;

            if ($.smallRadioButton.isChecked()) {
                $metric = QStyle::PM_SmallIconSize;
            } else if ($.largeRadioButton.isChecked()) {
                $metric = QStyle::PM_LargeIconSize;
            } else if ($.toolBarRadioButton.isChecked()) {
                $metric = QStyle::PM_ToolBarIconSize;
            } else if ($.listViewRadioButton.isChecked()) {
                $metric = QStyle::PM_ListViewIconSize;
            } else if ($.iconViewRadioButton.isChecked()) {
                $metric = QStyle::PM_IconViewIconSize;
            } else {
                $metric = QStyle::PM_TabBarIconSize;
            }
            $extent = QApplication::style().pixelMetric($metric);
        }
        $.previewArea.setSize(new QSize($extent, $extent));
        $.otherSpinBox.setEnabled($.otherRadioButton.isChecked());
    }

    changeIcon()
    {
        my $icon = new QIcon();

        for (my $row = 0; $row < $.imagesTable.rowCount(); ++$row) {
            my $item0 = $.imagesTable.item($row, 0);
            my $item1 = $.imagesTable.item($row, 1);
            my $item2 = $.imagesTable.item($row, 2);

            if ($item0.checkState() == Qt::Checked) {
                my $mode;
                if ($item1.text() == $.tr("Normal")) {
                    $mode = QIcon::Normal;
                } else if ($item1.text() == $.tr("Active")) {
                    $mode = QIcon::Active;
                } else if ($item1.text() == $.tr("Disabled")) {
                    $mode = QIcon::Disabled;
                } else {
                    $mode = QIcon::Selected;
                }

                my $state;
                if ($item2.text() == $.tr("On")) {
                    $state = QIcon::On;
                } else {
                    $state = QIcon::Off;
                }

                my $fileName = $item0.data(Qt::UserRole);
                my $image = new QImage($fileName);
                if (!$image.isNull())
                    $icon.addPixmap(QPixmap::fromImage($image), $mode, $state);
            }
        }

        $.previewArea.setIcon($icon);
    }

    addImages()
    {
        my $fileNames = QFileDialog::getOpenFileNames($self, $.tr("Open Images"), "", $.tr("Images (*.png *.xpm *.jpg);;All Files (*)"));
        foreach my $fileName in ($fileNames) {
            my $row = $.imagesTable.rowCount();
            $.imagesTable.setRowCount($row + 1);
            
            my $imageName = basename($fileName);
            my $item0 = new QTableWidgetItem($imageName);
            $item0.setData(Qt::UserRole, $fileName);
            #printf("flags=%d new flags=%d\n", $item0.flags(), $item0.flags() & ~Qt::ItemIsEditable);
            $item0.setFlags($item0.flags() & ~Qt::ItemIsEditable);
            
            my $item1 = new QTableWidgetItem($.tr("Normal"));
            my $item2 = new QTableWidgetItem($.tr("Off"));
            
            if ($.guessModeStateAct.isChecked()) {
                if ($fileName =~ /_act/) {
                    $item1.setText($.tr("Active"));
                } else if ($fileName =~ /_dis/) {
                    $item1.setText($.tr("Disabled"));
                } else if ($fileName =~ /_sel/) {
                    $item1.setText($.tr("Selected"));
                }
                
                if ($fileName =~ /_on/)
                    $item2.setText($.tr("On"));
            }
            
            $.imagesTable.setItem($row, 0, $item0);
            $.imagesTable.setItem($row, 1, $item1);
            $.imagesTable.setItem($row, 2, $item2);
            $.imagesTable.openPersistentEditor($item1);
            $.imagesTable.openPersistentEditor($item2);
            
            $item0.setCheckState(Qt::Checked);
        }
    }

    removeAllImages()
    {
        $.imagesTable.setRowCount(0);
        $.changeIcon();
    }

    createPreviewGroupBox()
    {
        $.previewGroupBox = new QGroupBox($.tr("Preview"));

        $.previewArea = new IconPreviewArea();

        my $layout = new QVBoxLayout();
        $layout.addWidget($.previewArea);
        $.previewGroupBox.setLayout($layout);
    }

    createImagesGroupBox()
    {
        $.imagesGroupBox = new QGroupBox($.tr("Images"));

        $.imagesTable = new QTableWidget();
        $.imagesTable.setSelectionMode(QAbstractItemView::NoSelection);
        $.imagesTable.setItemDelegate(new ImageDelegate($self));

        my $labels = ();
        $labels += $.tr("Image");
        $labels += $.tr("Mode");
        $labels += $.tr("State");

        $.imagesTable.horizontalHeader().setDefaultSectionSize(90);
        $.imagesTable.setColumnCount(3);
        $.imagesTable.setHorizontalHeaderLabels($labels);
        $.imagesTable.horizontalHeader().setResizeMode(0, QHeaderView::Stretch);
        $.imagesTable.horizontalHeader().setResizeMode(1, QHeaderView::Fixed);
        $.imagesTable.horizontalHeader().setResizeMode(2, QHeaderView::Fixed);
        $.imagesTable.verticalHeader().hide();

        $.connect($.imagesTable, SIGNAL("itemChanged(QTableWidgetItem *)"), SLOT("changeIcon()"));

        my $layout = new QVBoxLayout();
        $layout.addWidget($.imagesTable);
        $.imagesGroupBox.setLayout($layout);
    }

    createIconSizeGroupBox()
    {
        $.iconSizeGroupBox = new QGroupBox($.tr("Icon Size"));

        $.smallRadioButton = new QRadioButton();
        $.largeRadioButton = new QRadioButton();
        $.toolBarRadioButton = new QRadioButton();
        $.listViewRadioButton = new QRadioButton();
        $.iconViewRadioButton = new QRadioButton();
        $.tabBarRadioButton = new QRadioButton();
        $.otherRadioButton = new QRadioButton($.tr("Other:"));

        $.otherSpinBox = new IconSizeSpinBox();
        $.otherSpinBox.setRange(8, 128);
        $.otherSpinBox.setValue(64);

        $.connect($.smallRadioButton,    SIGNAL("toggled(bool)"), SLOT("changeSize(bool)"));
        $.connect($.largeRadioButton,    SIGNAL("toggled(bool)"), SLOT("changeSize(bool)"));
        $.connect($.toolBarRadioButton,  SIGNAL("toggled(bool)"), SLOT("changeSize(bool)"));
        $.connect($.listViewRadioButton, SIGNAL("toggled(bool)"), SLOT("changeSize(bool)"));
        $.connect($.iconViewRadioButton, SIGNAL("toggled(bool)"), SLOT("changeSize(bool)"));
        $.connect($.tabBarRadioButton,   SIGNAL("toggled(bool)"), SLOT("changeSize(bool)"));
        $.connect($.otherRadioButton,    SIGNAL("toggled(bool)"), SLOT("changeSize(bool)"));
        $.connect($.otherSpinBox,        SIGNAL("valueChanged(int)"), SLOT("changeSize()"));

        my $otherSizeLayout = new QHBoxLayout();
        $otherSizeLayout.addWidget($.otherRadioButton);
        $otherSizeLayout.addWidget($.otherSpinBox);
        $otherSizeLayout.addStretch();

        my $layout = new QGridLayout();
        $layout.addWidget($.smallRadioButton, 0, 0);
        $layout.addWidget($.largeRadioButton, 1, 0);
        $layout.addWidget($.toolBarRadioButton, 2, 0);
        $layout.addWidget($.listViewRadioButton, 0, 1);
        $layout.addWidget($.iconViewRadioButton, 1, 1);
        $layout.addWidget($.tabBarRadioButton, 2, 1);
        $layout.addLayout($otherSizeLayout, 3, 0, 1, 2);
        $layout.setRowStretch(4, 1);
        $.iconSizeGroupBox.setLayout($layout);
    }

    createActions()
    {
        $.addImagesAct = new QAction($.tr("&Add Images..."), $self);
        $.addImagesAct.setShortcut($.tr("Ctrl+A"));
        $.connect($.addImagesAct, SIGNAL("triggered()"), SLOT("addImages()"));

        $.removeAllImagesAct = new QAction($.tr("&Remove All Images"), $self);
        $.removeAllImagesAct.setShortcut($.tr("Ctrl+R"));
        $.connect($.removeAllImagesAct, SIGNAL("triggered()"), SLOT("removeAllImages()"));

        $.exitAct = new QAction($.tr("&Quit"), $self);
        $.exitAct.setShortcut($.tr("Ctrl+Q"));
        $.connect($.exitAct, SIGNAL("triggered()"), SLOT("close()"));

        $.styleActionGroup = new QActionGroup($self);
        foreach my $styleName in (QStyleFactory::keys()) {
            my $action = new QAction($.styleActionGroup);
            $action.setText(sprintf($.tr("%s Style"), $styleName));
            $action.setData($styleName);
            $action.setCheckable(True);
            $.connect($action, SIGNAL("triggered(bool)"), SLOT("changeStyle(bool)"));
        }

        $.guessModeStateAct = new QAction($.tr("&Guess Image Mode/State"), $self);
        $.guessModeStateAct.setCheckable(True);
        $.guessModeStateAct.setChecked(True);

        $.aboutAct = new QAction($.tr("&About"), $self);
        $.connect($.aboutAct, SIGNAL("triggered()"), SLOT("about()"));

        $.aboutQtAct = new QAction($.tr("About &Qt"), $self);
        qApp().connect($.aboutQtAct, SIGNAL("triggered()"), SLOT("aboutQt()"));
    }

    createMenus()
    {
        $.fileMenu = $.menuBar().addMenu($.tr("&File"));
        $.fileMenu.addAction($.addImagesAct);
        $.fileMenu.addAction($.removeAllImagesAct);
        $.fileMenu.addSeparator();
        $.fileMenu.addAction($.exitAct);

        $.viewMenu = $.menuBar().addMenu($.tr("&View"));
        foreach my $action in ($.styleActionGroup.actions())
            $.viewMenu.addAction($action);
        $.viewMenu.addSeparator();
        $.viewMenu.addAction($.guessModeStateAct);

        $.menuBar().addSeparator();

        $.helpMenu = $.menuBar().addMenu($.tr("&Help"));
        $.helpMenu.addAction($.aboutAct);
        $.helpMenu.addAction($.aboutQtAct);
    }

    createContextMenu()
    {
        $.imagesTable.setContextMenuPolicy(Qt::ActionsContextMenu);
        $.imagesTable.addAction($.addImagesAct);
        $.imagesTable.addAction($.removeAllImagesAct);
    }

    checkCurrentStyle()
    {
        foreach my $action in ($.styleActionGroup.actions()) {
            my $styleName = $action.data();
            my $candidate = QStyleFactory::create($styleName);
            #Q_ASSERT(candidate);
            if ($candidate.metaObject().className()
                == QApplication::style().metaObject().className()) {
                $action.trigger();
                return;
            }
            delete $candidate;
        }
    }
}

class icons_example inherits QApplication
{
    constructor()
    {
        my $mainwin = new MainWindow();
        $mainwin.show();
        $.exec();
    }
}
