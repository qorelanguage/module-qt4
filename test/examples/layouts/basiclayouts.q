#!/usr/bin/env qore

# This is basically a direct port of the QT tutorial to Qore 
# using Qore's "qt" module.  

# Note that Qore's "qt" module requires QT 4.3 or above 

# use the "qt-gui" module
%requires qt4

# this is an object-oriented program, the application class is "basiclayouts"
%exec-class basiclayouts
# require all variables to be explicitly  declared
%require-our
# enable all parse warnings
%enable-all-warnings

const NumGridRows = 3;
const NumButtons  = 4;

class Dialog inherits QDialog
{
    private $.menuBar, $.horizontalGroupBox, $.gridGroupBox, $.smallEditor,
            $.bigEditor, $.labels, $.lineEdits, $.buttons, $.buttonBox,
            $.fileMenu, $.exitAction;

    constructor()
    {
        $.createMenu();
        $.createHorizontalGroupBox();
        $.createGridGroupBox();

        $.bigEditor = new QTextEdit();
        $.bigEditor.setPlainText($.tr("This widget takes up all the remaining space "
                                    "in the top-level layout."));

        $.buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        
        $.connect($.buttonBox, SIGNAL("accepted()"), SLOT("accept()"));
        $.connect($.buttonBox, SIGNAL("rejected()"), SLOT("reject()"));

        my $mainLayout = new QVBoxLayout();
        $mainLayout.setMenuBar($.menuBar);
        $mainLayout.addWidget($.horizontalGroupBox);
        $mainLayout.addWidget($.gridGroupBox);
        $mainLayout.addWidget($.bigEditor);
        $mainLayout.addWidget($.buttonBox);
        $.setLayout($mainLayout);

        $.setWindowTitle($.tr("Basic Layouts"));
    }

    createMenu()
    {
        $.menuBar = new QMenuBar();

        $.fileMenu = new QMenu($.tr("&File"), $self);
        $.exitAction = $.fileMenu.addAction($.tr("E&xit"));
        $.menuBar.addMenu($.fileMenu);

        $.connect($.exitAction, SIGNAL("triggered()"), SLOT("accept()"));
    }

    createHorizontalGroupBox()
    {
        $.horizontalGroupBox = new QGroupBox($.tr("Horizontal layout"));
        my $layout = new QHBoxLayout();

        for (my $i = 0; $i < NumButtons; ++$i) {
            $.buttons[$i] = new QPushButton(sprintf($.tr("Button %d"), $i + 1));
            $layout.addWidget($.buttons[$i]);
        }
        $.horizontalGroupBox.setLayout($layout);
    }

    createGridGroupBox()
    {
        $.gridGroupBox = new QGroupBox($.tr("Grid layout"));
        my $layout = new QGridLayout();

        for (my $i = 0; $i < NumGridRows; ++$i) {
            $.labels[$i] = new QLabel(sprintf($.tr("Line %d:"), $i + 1));
            $.lineEdits[$i] = new QLineEdit();
            $layout.addWidget($.labels[$i], $i + 1, 0);
            $layout.addWidget($.lineEdits[$i], $i + 1, 1);
        }

        $.smallEditor = new QTextEdit();
        $.smallEditor.setPlainText($.tr("This widget takes up about two thirds of the "
                                     "grid layout."));
        $layout.addWidget($.smallEditor, 0, 2, 4, 1);

        $layout.setColumnStretch(1, 10);
        $layout.setColumnStretch(2, 20);
        $.gridGroupBox.setLayout($layout);
    }
}

class basiclayouts inherits QApplication 
{
    constructor() {
        my $dialog = new Dialog();        
        $dialog.exec();
    }
}
