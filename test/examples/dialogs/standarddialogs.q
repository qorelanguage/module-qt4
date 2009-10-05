#!/usr/bin/env qore

# this is basically a direct port of the QT widget example
# "standarddialog" to Qore using Qore's "qt" module.  

# Note that Qore's "qt" module requires QT 4.3 or above 

# use the "qt-gui" module
%requires qt4

# this is an object-oriented program; the application class is "standarddialog_example"
%exec-class standarddialog_example
# require all variables to be explicitly declared
%require-our
# enable all parse warnings
%enable-all-warnings

class Dialog inherits QDialog
{
    private $.native, $.integerLabel, $.doubleLabel, $.itemLabel,
            $.textLabel, $.colorLabel, $.fontLabel, $.directoryLabel,
            $.openFileNameLabel, $.openFileNamesLabel, $.saveFileNameLabel,
            $.criticalLabel, $.informationLabel, $.questionLabel,
            $.warningLabel, $.errorLabel, $.errorMessageDialog,
            $.openFilesPath;

    constructor($parent) : QDialog($parent)
    {
        $.message = $.tr("<p>Message boxes have a caption, a text, "
                       "and any number of buttons, each with standard or custom texts."
                       "<p>Click a button to close the message box. Pressing the Esc button "
                       "will activate the detected escape button (if any).");

        $.openFilesPath = "";
        $.errorMessageDialog = new QErrorMessage($self);

        my $frameStyle = QFrame::Sunken | QFrame::Panel;

        $.integerLabel = new QLabel();
        $.integerLabel.setFrameStyle($frameStyle);
        my $integerButton =
            new QPushButton($.tr("QInputDialog::get&Integer()"));

        $.doubleLabel = new QLabel();
        $.doubleLabel.setFrameStyle($frameStyle);
        my $doubleButton =
            new QPushButton($.tr("QInputDialog::get&Double()"));

        $.itemLabel = new QLabel();
        $.itemLabel.setFrameStyle($frameStyle);
        my $itemButton = new QPushButton($.tr("QInputDialog::getIte&m()"));

        $.textLabel = new QLabel();
        $.textLabel.setFrameStyle($frameStyle);
        my $textButton = new QPushButton($.tr("QInputDialog::get&Text()"));

        $.colorLabel = new QLabel();
        $.colorLabel.setFrameStyle($frameStyle);
        my $colorButton = new QPushButton($.tr("QColorDialog::get&Color()"));

        $.fontLabel = new QLabel();
        $.fontLabel.setFrameStyle($frameStyle);
        my $fontButton = new QPushButton($.tr("QFontDialog::get&Font()"));

        $.directoryLabel = new QLabel();
        $.directoryLabel.setFrameStyle($frameStyle);
        my $directoryButton =
            new QPushButton($.tr("QFileDialog::getE&xistingDirectory()"));

        $.openFileNameLabel = new QLabel();
        $.openFileNameLabel.setFrameStyle($frameStyle);
        my $openFileNameButton =
            new QPushButton($.tr("QFileDialog::get&OpenFileName()"));

        $.openFileNamesLabel = new QLabel();
        $.openFileNamesLabel.setFrameStyle($frameStyle);
        my $openFileNamesButton =
            new QPushButton($.tr("QFileDialog::&getOpenFileNames()"));

        $.saveFileNameLabel = new QLabel();
        $.saveFileNameLabel.setFrameStyle($frameStyle);
        my $saveFileNameButton =
            new QPushButton($.tr("QFileDialog::get&SaveFileName()"));

        $.criticalLabel = new QLabel();
        $.criticalLabel.setFrameStyle($frameStyle);
        my $criticalButton =
            new QPushButton($.tr("QMessageBox::critica&l()"));

        $.informationLabel = new QLabel();
        $.informationLabel.setFrameStyle($frameStyle);
        my $informationButton =
            new QPushButton($.tr("QMessageBox::i&nformation()"));

        $.questionLabel = new QLabel();
        $.questionLabel.setFrameStyle($frameStyle);
        my $questionButton =
            new QPushButton($.tr("QMessageBox::&question()"));

        $.warningLabel = new QLabel();
        $.warningLabel.setFrameStyle($frameStyle);
        my $warningButton = new QPushButton($.tr("QMessageBox::&warning()"));

        $.errorLabel = new QLabel();
        $.errorLabel.setFrameStyle($frameStyle);
        my $errorButton =
            new QPushButton($.tr("QErrorMessage::show&M&essage()"));

        $.connect($integerButton, SIGNAL("clicked()"), SLOT("setInteger()"));
        $.connect($doubleButton, SIGNAL("clicked()"), SLOT("setDouble()"));
        $.connect($itemButton, SIGNAL("clicked()"), SLOT("setItem()"));
        $.connect($textButton, SIGNAL("clicked()"), SLOT("setText()"));
        $.connect($colorButton, SIGNAL("clicked()"), SLOT("setColor()"));
        $.connect($fontButton, SIGNAL("clicked()"), SLOT("setFont()"));
        $.connect($directoryButton, SIGNAL("clicked()"), SLOT("setExistingDirectory()"));
        $.connect($openFileNameButton, SIGNAL("clicked()"), SLOT("setOpenFileName()"));
        $.connect($openFileNamesButton, SIGNAL("clicked()"), SLOT("setOpenFileNames()"));
        $.connect($saveFileNameButton, SIGNAL("clicked()"), SLOT("setSaveFileName()"));
        $.connect($criticalButton, SIGNAL("clicked()"), SLOT("criticalMessage()"));
        $.connect($informationButton, SIGNAL("clicked()"), SLOT("informationMessage()"));
        $.connect($questionButton, SIGNAL("clicked()"), SLOT("questionMessage()"));
        $.connect($warningButton, SIGNAL("clicked()"), SLOT("warningMessage()"));
        $.connect($errorButton, SIGNAL("clicked()"), SLOT("errorMessage()"));

        $.native = new QCheckBox($self);
        $.native.setText("Use native file dialog.");
        $.native.setChecked(True);
        # execute hide if system is not windows or mac
        #$.native.hide();
        my $layout = new QGridLayout();
        $layout.setColumnStretch(1, 1);
        $layout.setColumnMinimumWidth(1, 250);
        $layout.addWidget($integerButton, 0, 0);
        $layout.addWidget($.integerLabel, 0, 1);
        $layout.addWidget($doubleButton, 1, 0);
        $layout.addWidget($.doubleLabel, 1, 1);
        $layout.addWidget($itemButton, 2, 0);
        $layout.addWidget($.itemLabel, 2, 1);
        $layout.addWidget($textButton, 3, 0);
        $layout.addWidget($.textLabel, 3, 1);
        $layout.addWidget($colorButton, 4, 0);
        $layout.addWidget($.colorLabel, 4, 1);
        $layout.addWidget($fontButton, 5, 0);
        $layout.addWidget($.fontLabel, 5, 1);
        $layout.addWidget($directoryButton, 6, 0);
        $layout.addWidget($.directoryLabel, 6, 1);
        $layout.addWidget($openFileNameButton, 7, 0);
        $layout.addWidget($.openFileNameLabel, 7, 1);
        $layout.addWidget($openFileNamesButton, 8, 0);
        $layout.addWidget($.openFileNamesLabel, 8, 1);
        $layout.addWidget($saveFileNameButton, 9, 0);
        $layout.addWidget($.saveFileNameLabel, 9, 1);
        $layout.addWidget($criticalButton, 10, 0);
        $layout.addWidget($.criticalLabel, 10, 1);
        $layout.addWidget($informationButton, 11, 0);
        $layout.addWidget($.informationLabel, 11, 1);
        $layout.addWidget($questionButton, 12, 0);
        $layout.addWidget($.questionLabel, 12, 1);
        $layout.addWidget($warningButton, 13, 0);
        $layout.addWidget($.warningLabel, 13, 1);
        $layout.addWidget($errorButton, 14, 0);
        $layout.addWidget($.errorLabel, 14, 1);
        $layout.addWidget($.native, 15, 0);
        $.setLayout($layout);
        
        $.setWindowTitle($.tr("Standard Dialogs"));
    }

    setInteger()
    {
        my $ok;
        my $i = QInputDialog::getInteger($self, $.tr("QInputDialog::getInteger()"),
                                        $.tr("Percentage:"), 25, 0, 100, 1, \$ok);
        if ($ok)
            $.integerLabel.setText(sprintf("%d", $i));
    }

    setDouble()
    {
        my $ok;
        my $d = QInputDialog::getDouble($self, $.tr("QInputDialog::getDouble()"),
                                       $.tr("Amount:"), 37.56, -10000, 10000, 2, \$ok);
        if ($ok)
            $.doubleLabel.setText(sprintf("%.2f", $d));
    }

    setItem()
    {
        my $items = ($.tr("Spring"), $.tr("Summer"), $.tr("Fall"), $.tr("Winter"));

        my $ok;
        my $item = QInputDialog::getItem($self, $.tr("QInputDialog::getItem()"),
                                        $.tr("Season:"), $items, 0, False, \$ok);
        if ($ok && strlen($item))
            $.itemLabel.setText($item);
    }

    setText()
    {
        my $ok;
        my $text = QInputDialog::getText($self, $.tr("QInputDialog::getText()"),
                                        $.tr("User name:"), QLineEdit::Normal,
                                        QDir::home().dirName(), \$ok);
        if ($ok && strlen($text))
            $.textLabel.setText($text);
    }

    setColor()
    {
        my $color = QColorDialog::getColor(Qt::green, $self);
        if ($color.isValid()) {
            $.colorLabel.setText($color.name());
            $.colorLabel.setPalette(new QPalette($color));
            $.colorLabel.setAutoFillBackground(True);
        }
    }

    setFont()
    {
        my $ok;
        my $font = QFontDialog::getFont(\$ok, new QFont($.fontLabel.text()), $self);
        if ($ok) {
            $.fontLabel.setText($font.key());
            $.fontLabel.setFont($font);
        }
    }

    setExistingDirectory()
    {
        my $options = QFileDialog::DontResolveSymlinks | QFileDialog::ShowDirsOnly;
        if (!$.native.isChecked())
            $options |= QFileDialog::DontUseNativeDialog;
        my $directory = QFileDialog::getExistingDirectory($self,
                                                         $.tr("QFileDialog::getExistingDirectory()"),
                                                         $.directoryLabel.text(),
                                                         $options);
        if (strlen($directory))
            $.directoryLabel.setText($directory);
    }

    setOpenFileName()
    {
        my $options;
        if (!$.native.isChecked())
            $options |= QFileDialog::DontUseNativeDialog;
        my $selectedFilter;
        my $fileName = QFileDialog::getOpenFileName($self,
                                                   $.tr("QFileDialog::getOpenFileName()"),
                                                   $.openFileNameLabel.text(),
                                                   $.tr("All Files (*);;Text Files (*.txt)"),
                                                   \$selectedFilter,
                                                   $options);
        if (strlen($fileName))
            $.openFileNameLabel.setText($fileName);
    }

    setOpenFileNames()
    {
        my $options;
        if (!$.native.isChecked())
            $options |= QFileDialog::DontUseNativeDialog;
        my $selectedFilter;
        my $files = QFileDialog::getOpenFileNames($self, $.tr("QFileDialog::getOpenFileNames()"),
                                                 $.openFilesPath,
                                                 $.tr("All Files (*);;Text Files (*.txt)"),
                                                 \$selectedFilter,
                                                 $options);
        if (elements $files) {
            $.openFilesPath = $files[0];
            $.openFileNamesLabel.setText(sprintf("[%s]", join(", ", $files)));
        }
    }

    setSaveFileName()
    {
        my $options;
        if (!$.native.isChecked())
            $options |= QFileDialog::DontUseNativeDialog;
        my $selectedFilter;
        my $fileName = QFileDialog::getSaveFileName($self,
                                                   $.tr("QFileDialog::getSaveFileName()"),
                                                   $.saveFileNameLabel.text(),
                                                   $.tr("All Files (*);;Text Files (*.txt)"),
                                                   \$selectedFilter,
                                                   $options);
        if (strlen($fileName))
            $.saveFileNameLabel.setText($fileName);
    }

    criticalMessage()
    {
        my $reply;
        $reply = QMessageBox::critical($self, $.tr("QMessageBox::critical()"),
                                      $.message,
                                      QMessageBox::Abort | QMessageBox::Retry | QMessageBox::Ignore);
        if ($reply == QMessageBox::Abort)
            $.criticalLabel.setText($.tr("Abort"));
        else if ($reply == QMessageBox::Retry)
            $.criticalLabel.setText($.tr("Retry"));
        else
            $.criticalLabel.setText($.tr("Ignore"));
    }

    informationMessage()
    {
        my $reply;
        $reply = QMessageBox::information($self, $.tr("QMessageBox::information()"), $.message);
        if ($reply == QMessageBox::Ok)
            $.informationLabel.setText($.tr("OK"));
        else
            $.informationLabel.setText($.tr("Escape"));
    }

    questionMessage()
    {
        my $reply;
        $reply = QMessageBox::question($self, $.tr("QMessageBox::question()"),
                                      $.message,
                                      QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        if ($reply == QMessageBox::Yes)
            $.questionLabel.setText($.tr("Yes"));
        else if ($reply == QMessageBox::No)
            $.questionLabel.setText($.tr("No"));
        else
            $.questionLabel.setText($.tr("Cancel"));
    }

    warningMessage()
    {
        my $msgBox = new QMessageBox(QMessageBox::Warning, $.tr("QMessageBox::warning()"),
                                     $.message, 0, $self);
        $msgBox.addButton($.tr("Save &Again"), QMessageBox::AcceptRole);
        $msgBox.addButton($.tr("&Continue"), QMessageBox::RejectRole);
        if ($msgBox.exec() == QMessageBox::AcceptRole)
            $.warningLabel.setText($.tr("Save Again"));
        else
            $.warningLabel.setText($.tr("Continue"));

    }

    errorMessage()
    {
        $.errorMessageDialog.showMessage(
            $.tr("This dialog shows and remembers error messages. "
               "If the checkbox is checked (as it is by default), "
               "the shown message will be shown again, "
               "but if the user unchecks the box the message "
               "will not appear again if QErrorMessage::showMessage() "
               "is called with the same message."));
        $.errorLabel.setText($.tr("If the box is unchecked, the message "
                                "won't appear again."));
    }
}

class standarddialog_example inherits QApplication
{
    constructor()
    {
        my $translatorFileName = "qt_" + QLocale::system().name();
        my $translator = new QTranslator($self);
        if ($translator.load($translatorFileName, QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
            QCoreApplication::installTranslator($translator);

        my $dialog = new Dialog();
        $dialog.exec();
    }
}
