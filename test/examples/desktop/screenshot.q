#!/usr/bin/env qore

# this is basically a direct port of the QT widget example
# "screenshot" to Qore using Qore's "qt4" module.  

# Note that Qore's "qt4" module requires QT 4.3 or above 

# use the "qt4" module
%requires qt4

# this is an object-oriented program; the application class is "screenshot_example"
%exec-class screenshot_example
# require all variables to be explicitly declared
%require-our
# enable all parse warnings
%enable-all-warnings

class Screenshot inherits QWidget {
    private {
        QPixmap $.originalPixmap();
        QLabel $.screenshotLabel();
        QVBoxLayout $.mainLayout();
        QHBoxLayout $.buttonsLayout();
        QGroupBox $.optionsGroupBox(QObject::tr("Options"));
        QSpinBox $.delaySpinBox();        
        QLabel $.delaySpinBoxLabel(QObject::tr("Screenshot Delay:"));
        QCheckBox $.hideThisWindowCheckBox(QObject::tr("Hide This Window"));
        QGridLayout $.optionsGroupBoxLayout();
        QPushButton $.newScreenshotButton;
        QPushButton $.saveScreenshotButton;
        QPushButton $.quitScreenshotButton;
    }
    
    constructor() {
        $.screenshotLabel.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        $.screenshotLabel.setAlignment(Qt::AlignCenter);
        $.screenshotLabel.setMinimumSize(240, 160);

        $.createOptionsGroupBox();
        $.createButtonsLayout();

        $.mainLayout.addWidget($.screenshotLabel);
        $.mainLayout.addWidget($.optionsGroupBox);
        $.mainLayout.addLayout($.buttonsLayout);
        $.setLayout($.mainLayout);

        $.shootScreen();

        $.delaySpinBox.setValue(5);

        $.setWindowTitle(Screenshot::tr("Screenshot"));
        $.resize(300, 200);
    }

    resizeEvent(QResizeEvent $event) {
        my $scaledSize = $.originalPixmap.size();
        $scaledSize.scale($.screenshotLabel.size(), Qt::KeepAspectRatio);
        if (!$.screenshotLabel.pixmap()
            || $scaledSize != $.screenshotLabel.pixmap().size())
            $.updateScreenshotLabel();
    }

    newScreenshot() {
        if ($.hideThisWindowCheckBox.isChecked())
            $.hide();
        $.newScreenshotButton.setDisabled(True);

        QTimer::singleShot($.delaySpinBox.value() * 1000, $self, SLOT("shootScreen()"));
    }

    saveScreenshot() {
        my string $format = "png";
        my string $initialPath = QDir::currentPath() + $.tr("/untitled.") + $format;
        
        my $fileName = QFileDialog::getSaveFileName($self, $.tr("Save As"), $initialPath, sprintf($.tr("%s Files (*.%s);;All Files (*)"), toupper($format), $format));
        printf("FILE %s %s\n", $fileName, $format);
        if (strlen($fileName))
            $.originalPixmap.save($fileName, $format);
    }
    
    shootScreen() {
        if ($.delaySpinBox.value() != 0)
            QApplication::beep();

        $.originalPixmap = QPixmap::grabWindow(QApplication::desktop().winId());
        $.updateScreenshotLabel();

        $.newScreenshotButton.setDisabled(False);
        if ($.hideThisWindowCheckBox.isChecked())
            $.show();
    }

    updateCheckBox() {
        if ($.delaySpinBox.value() == 0)
            $.hideThisWindowCheckBox.setDisabled(True);
        else
            $.hideThisWindowCheckBox.setDisabled(False);
    }

    createOptionsGroupBox() {
        $.delaySpinBox.setSuffix($.tr(" s"));
        $.delaySpinBox.setMaximum(60);
        $.connect($.delaySpinBox, SIGNAL("valueChanged(int)"), SLOT("updateCheckBox()"));
        $.optionsGroupBoxLayout.addWidget($.delaySpinBoxLabel, 0, 0);
        $.optionsGroupBoxLayout.addWidget($.delaySpinBox, 0, 1);
        $.optionsGroupBoxLayout.addWidget($.hideThisWindowCheckBox, 1, 0, 1, 2);
        $.optionsGroupBox.setLayout($.optionsGroupBoxLayout);
    }

    createButtonsLayout() {
        $.newScreenshotButton = $.createButton($.tr("New Screenshot"), $self, SLOT("newScreenshot()"));
        $.saveScreenshotButton = $.createButton($.tr("Save Screenshot"), $self, SLOT("saveScreenshot()"));
        $.quitScreenshotButton = $.createButton($.tr("Quit"), $self, SLOT("close()"));

        $.buttonsLayout.addStretch();
        $.buttonsLayout.addWidget($.newScreenshotButton);
        $.buttonsLayout.addWidget($.saveScreenshotButton);
        $.buttonsLayout.addWidget($.quitScreenshotButton);
    }

    createButton($text, $receiver, $member) returns QPushButton {
        my QPushButton $button($text);
        $receiver.connect($button, SIGNAL("clicked()"), $member);
        return $button;
    }

    updateScreenshotLabel() {
        $.screenshotLabel.setPixmap($.originalPixmap.scaled($.screenshotLabel.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}

class screenshot_example inherits QApplication {
    constructor() {
        my Screenshot $screenshot();
        $screenshot.show();
        $.exec();
    }
}
