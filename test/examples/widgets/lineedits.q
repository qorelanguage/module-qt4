#!/usr/bin/env qore

# This is basically a direct port of the QT widget example
# "lineedits" to Qore using Qore's "qt" module.  

# Note that Qore's "qt" module requires QT 4.3 or above 

# use the "qt-gui" module
%requires qt4

# this is an object-oriented program, the application class is "lineedits_example"
%exec-class lineedits_example
# require all variables to be explicitly declared
%require-our
# enable all parse warnings
%enable-all-warnings

class Window inherits QWidget
{
    constructor()
    {
        my $echoGroup = new QGroupBox($.tr("Echo"));

        my $echoLabel = new QLabel($.tr("Mode:"));
        my $echoComboBox = new QComboBox();
        $echoComboBox.addItem($.tr("Normal"));
        $echoComboBox.addItem($.tr("Password"));
        $echoComboBox.addItem($.tr("PasswordEchoOnEdit"));
        $echoComboBox.addItem($.tr("No Echo"));

        $.echoLineEdit = new QLineEdit();
        $.echoLineEdit.setFocus();

        my $validatorGroup = new QGroupBox($.tr("Validator"));

        my $validatorLabel = new QLabel($.tr("Type:"));
        my $validatorComboBox = new QComboBox();
        $validatorComboBox.addItem($.tr("No validator"));
        $validatorComboBox.addItem($.tr("Integer validator"));
        $validatorComboBox.addItem($.tr("Double validator"));

        $.validatorLineEdit = new QLineEdit();

        my $alignmentGroup = new QGroupBox($.tr("Alignment"));

        my $alignmentLabel = new QLabel($.tr("Type:"));
        my $alignmentComboBox = new QComboBox();
        $alignmentComboBox.addItem($.tr("Left"));
        $alignmentComboBox.addItem($.tr("Centered"));
        $alignmentComboBox.addItem($.tr("Right"));

        $.alignmentLineEdit = new QLineEdit();

        my $inputMaskGroup = new QGroupBox($.tr("Input mask"));

        my $inputMaskLabel = new QLabel($.tr("Type:"));
        my $inputMaskComboBox = new QComboBox();
        $inputMaskComboBox.addItem($.tr("No mask"));
        $inputMaskComboBox.addItem($.tr("Phone number"));
        $inputMaskComboBox.addItem($.tr("ISO date"));
        $inputMaskComboBox.addItem($.tr("License key"));

        $.inputMaskLineEdit = new QLineEdit();

        my $accessGroup = new QGroupBox($.tr("Access"));

        my $accessLabel = new QLabel($.tr("Read-only:"));
        my $accessComboBox = new QComboBox();
        $accessComboBox.addItem($.tr("False"));
        $accessComboBox.addItem($.tr("True"));

        $.accessLineEdit = new QLineEdit();

        $.connect($echoComboBox, SIGNAL("activated(int)"), SLOT("echoChanged(int)"));
        $.connect($validatorComboBox, SIGNAL("activated(int)"),        SLOT("validatorChanged(int)"));
        $.connect($alignmentComboBox, SIGNAL("activated(int)"),        SLOT("alignmentChanged(int)"));
        $.connect($inputMaskComboBox, SIGNAL("activated(int)"), SLOT("inputMaskChanged(int)"));
        $.connect($accessComboBox, SIGNAL("activated(int)"), SLOT("accessChanged(int)"));

        my $echoLayout = new QGridLayout();
        $echoLayout.addWidget($echoLabel, 0, 0);
        $echoLayout.addWidget($echoComboBox, 0, 1);
        $echoLayout.addWidget($.echoLineEdit, 1, 0, 1, 2);
        $echoGroup.setLayout($echoLayout);
        
        my $validatorLayout = new QGridLayout();
        $validatorLayout.addWidget($validatorLabel, 0, 0);
        $validatorLayout.addWidget($validatorComboBox, 0, 1);
        $validatorLayout.addWidget($.validatorLineEdit, 1, 0, 1, 2);
        $validatorGroup.setLayout($validatorLayout);

        my $alignmentLayout = new QGridLayout();
        $alignmentLayout.addWidget($alignmentLabel, 0, 0);
        $alignmentLayout.addWidget($alignmentComboBox, 0, 1);
        $alignmentLayout.addWidget($.alignmentLineEdit, 1, 0, 1, 2);
        $alignmentGroup. setLayout($alignmentLayout);

        my $inputMaskLayout = new QGridLayout();
        $inputMaskLayout.addWidget($inputMaskLabel, 0, 0);
        $inputMaskLayout.addWidget($inputMaskComboBox, 0, 1);
        $inputMaskLayout.addWidget($.inputMaskLineEdit, 1, 0, 1, 2);
        $inputMaskGroup.setLayout($inputMaskLayout);

        my $accessLayout = new QGridLayout();
        $accessLayout.addWidget($accessLabel, 0, 0);
        $accessLayout.addWidget($accessComboBox, 0, 1);
        $accessLayout.addWidget($.accessLineEdit, 1, 0, 1, 2);
        $accessGroup.setLayout($accessLayout);

        my $layout = new QGridLayout();
        $layout.addWidget($echoGroup, 0, 0);
        $layout.addWidget($validatorGroup, 1, 0);
        $layout.addWidget($alignmentGroup, 2, 0);
        $layout.addWidget($inputMaskGroup, 0, 1);
        $layout.addWidget($accessGroup, 1, 1);
        $.setLayout($layout);

        $.setWindowTitle($.tr("Line Edits"));
    }

    echoChanged($index)
    {
        switch ($index) {
            case 0:
            $.echoLineEdit.setEchoMode(QLineEdit::Normal);
            break;

            case 1:
            $.echoLineEdit.setEchoMode(QLineEdit::Password);
            break;

            case 2:
            $.echoLineEdit.setEchoMode(QLineEdit::PasswordEchoOnEdit);
            break;

            case 3:
            $.echoLineEdit.setEchoMode(QLineEdit::NoEcho);
        }
    }

    validatorChanged($index)
    {
        switch ($index) {
            case 0:
            $.validatorLineEdit.setValidator();
            break;
            case 1:
            $.validatorLineEdit.setValidator(new QIntValidator($.validatorLineEdit));
            break;
            case 2:
            $.validatorLineEdit.setValidator(new QDoubleValidator(-999.0, 999.0, 2, $.validatorLineEdit));
        }

        $.validatorLineEdit.clear();
    }

    alignmentChanged($index)
    {
        switch ($index) {
            case 0:
            $.alignmentLineEdit.setAlignment(Qt::AlignLeft);
            break;
            case 1:
            $.alignmentLineEdit.setAlignment(Qt::AlignCenter);
            break;
            case 2:
            $.alignmentLineEdit.setAlignment(Qt::AlignRight);
        }
    }

    inputMaskChanged($index)
    {
        switch ($index) {
            case 0:
            $.inputMaskLineEdit.setInputMask("");
            break;
            case 1:
            $.inputMaskLineEdit.setInputMask("+99 99 99 99 99;_");
            break;
            case 2:
            $.inputMaskLineEdit.setInputMask("0000-00-00");
            $.inputMaskLineEdit.setText("00000000");
            $.inputMaskLineEdit.setCursorPosition(0);
            break;
            case 3:
            $.inputMaskLineEdit.setInputMask(">AAAAA-AAAAA-AAAAA-AAAAA-AAAAA;#");
        }
    }

    accessChanged($index)
    {
        switch ($index) {
            case 0:
            $.accessLineEdit.setReadOnly(False);
            break;
            case 1:
            $.accessLineEdit.setReadOnly(True);
        }
    }
}

class lineedits_example inherits QApplication
{
    constructor()
    {
        my $window = new Window();
        $window.show();
        $.exec();
    }
}
