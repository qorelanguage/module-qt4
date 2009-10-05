#!/usr/bin/env qore

# this is basically a direct port of the QT widget example
# "systray" to Qore using Qore's "qt" module.  

# Note that Qore's "qt" module requires QT 4.3 or above 

# use the "qt-gui" module
%requires qt4

# this is an object-oriented program; the application class is "systray_example"
%exec-class systray_example
# require all variables to be explicitly declared
%require-our
# enable all parse warnings
%enable-all-warnings

class Window inherits QWidget
{
    private $.iconGroupBox, $.iconLabel, $.iconComboBox, $.showIconCheckBox,
    $.messageGroupBox, $.typeLabel, $.durationLabel, $.durationWarningLabel,
    $.titleLabel, $.bodyLabel, $.typeComboBox, $.durationSpinBox, $.titleEdit,
    $.bodyEdit, $.showMessageButton, $.minimizeAction, $.maximizeAction, 
    $.restoreAction, $.quitAction, $.trayIcon, $.trayIconMenu;

    constructor()
    {
        $.createIconGroupBox();
        $.createMessageGroupBox();

        $.iconLabel.setMinimumWidth($.durationLabel.sizeHint().width());

        $.createActions();
        $.createTrayIcon();

        $.connect($.showMessageButton, SIGNAL("clicked()"), SLOT("showMessage()"));
        $.trayIcon.connect($.showIconCheckBox, SIGNAL("toggled(bool)"), SLOT("setVisible(bool)"));
        $.connect($.iconComboBox, SIGNAL("currentIndexChanged(int)"), SLOT("setIcon(int)"));
        $.connect($.trayIcon, SIGNAL("messageClicked()"), SLOT("messageClicked()"));
        $.connect($.trayIcon, SIGNAL("activated(QSystemTrayIcon::ActivationReason)"), SLOT("iconActivated(QSystemTrayIcon::ActivationReason)"));

        my $mainLayout = new QVBoxLayout();
        $mainLayout.addWidget($.iconGroupBox);
        $mainLayout.addWidget($.messageGroupBox);
        $.setLayout($mainLayout);

        $.iconComboBox.setCurrentIndex(1);
        $.trayIcon.show();

        $.setWindowTitle($.tr("Systray"));
        $.resize(400, 300);
    }

    setVisible($visible)
    {
        $.minimizeAction.setEnabled($visible);
        $.maximizeAction.setEnabled(!$.isMaximized());
        $.restoreAction.setEnabled($.isMaximized() || !$visible);
        QWidget::$.setVisible($visible);
    }

    closeEvent($event)
    {
        if ($.trayIcon.isVisible()) {
            QMessageBox::information($self, $.tr("Systray"),
                                    $.tr("The program will keep running in the "
                                       "system tray. To terminate the program, "
                                       "choose <b>Quit</b> in the context menu "
                                       "of the system tray entry."));
            $.hide();
            $event.ignore();
        }
    }

    private setIcon($index)
    {
        my $icon = $.iconComboBox.itemIcon($index);
        $.trayIcon.setIcon($icon);
        $.setWindowIcon($icon);

        $.trayIcon.setToolTip($.iconComboBox.itemText($index));
    }

    private iconActivated($reason)
    {
        switch ($reason) {
            case QSystemTrayIcon::Trigger:
            case QSystemTrayIcon::DoubleClick: {
                $.iconComboBox.setCurrentIndex(($.iconComboBox.currentIndex() + 1)
                                               % $.iconComboBox.count());
                break;
            }
            case QSystemTrayIcon::MiddleClick: {
                $.showMessage();
                break;
            }
          default:
        }
    }

    private showMessage()
    {
        my $icon = $.typeComboBox.itemData($.typeComboBox.currentIndex());
        $.trayIcon.showMessage($.titleEdit.text(), $.bodyEdit.toPlainText(), $icon,
                               $.durationSpinBox.value() * 1000);
    }

    private messageClicked()
    {
        QMessageBox::information(NOTHING, $.tr("Systray"),
                                $.tr("Sorry, I already gave what help I could.\n"
                                   "Maybe you should try asking a human?"));
    }

    private createIconGroupBox()
    {
        $.iconGroupBox = new QGroupBox($.tr("Tray Icon"));

        $.iconLabel = new QLabel("Icon:");

        $.iconComboBox = new QComboBox();
        $.iconComboBox.addItem(new QIcon($dir + "images/bad.svg"), $.tr("Bad"));
        $.iconComboBox.addItem(new QIcon($dir + "images/heart.svg"), $.tr("Heart"));
        $.iconComboBox.addItem(new QIcon($dir + "images/trash.svg"), $.tr("Trash"));

        $.showIconCheckBox = new QCheckBox($.tr("Show icon"));
        $.showIconCheckBox.setChecked(True);

        my $iconLayout = new QHBoxLayout();
        $iconLayout.addWidget($.iconLabel);
        $iconLayout.addWidget($.iconComboBox);
        $iconLayout.addStretch();
        $iconLayout.addWidget($.showIconCheckBox);
        $.iconGroupBox.setLayout($iconLayout);
    }

    private createMessageGroupBox()
    {
        $.messageGroupBox = new QGroupBox($.tr("Balloon Message"));

        $.typeLabel = new QLabel($.tr("Type:"));

        $.typeComboBox = new QComboBox();
        $.typeComboBox.addItem($.tr("None"), QSystemTrayIcon::NoIcon);
        $.typeComboBox.addItem($.style().standardIcon(QStyle::SP_MessageBoxInformation), $.tr("Information"),
                               QSystemTrayIcon::Information);
        $.typeComboBox.addItem($.style().standardIcon(QStyle::SP_MessageBoxWarning), $.tr("Warning"),
                               QSystemTrayIcon::Warning);
        $.typeComboBox.addItem($.style().standardIcon(QStyle::SP_MessageBoxCritical), $.tr("Critical"),
                               QSystemTrayIcon::Critical);
        $.typeComboBox.setCurrentIndex(1);

        $.durationLabel = new QLabel($.tr("Duration:"));

        $.durationSpinBox = new QSpinBox();
        $.durationSpinBox.setRange(5, 60);
        $.durationSpinBox.setSuffix(" s");
        $.durationSpinBox.setValue(15);

        $.durationWarningLabel = new QLabel($.tr("(some systems might ignore this "
                                               "hint)"));
        $.durationWarningLabel.setIndent(10);
        
        $.titleLabel = new QLabel($.tr("Title:"));
        
        $.titleEdit = new QLineEdit($.tr("Cannot connect to network"));
        
        $.bodyLabel = new QLabel($.tr("Body:"));
        
        $.bodyEdit = new QTextEdit();
        $.bodyEdit.setPlainText($.tr("Don't believe me. Honestly, I don't have a "
                                   "clue.\nClick this balloon for details."));

        $.showMessageButton = new QPushButton($.tr("Show Message"));
        $.showMessageButton.setDefault(True);

        my $messageLayout = new QGridLayout();
        $messageLayout.addWidget($.typeLabel, 0, 0);
        $messageLayout.addWidget($.typeComboBox, 0, 1, 1, 2);
        $messageLayout.addWidget($.durationLabel, 1, 0);
        $messageLayout.addWidget($.durationSpinBox, 1, 1);
        $messageLayout.addWidget($.durationWarningLabel, 1, 2, 1, 3);
        $messageLayout.addWidget($.titleLabel, 2, 0);
        $messageLayout.addWidget($.titleEdit, 2, 1, 1, 4);
        $messageLayout.addWidget($.bodyLabel, 3, 0);
        $messageLayout.addWidget($.bodyEdit, 3, 1, 2, 4);
        $messageLayout.addWidget($.showMessageButton, 5, 4);
        $messageLayout.setColumnStretch(3, 1);
        $messageLayout.setRowStretch(4, 1);
        $.messageGroupBox.setLayout($messageLayout);
    }

    private createActions()
    {
        $.minimizeAction = new QAction($.tr("Mi&nimize"), $self);
        $.connect($.minimizeAction, SIGNAL("triggered()"), SLOT("hide()"));

        $.maximizeAction = new QAction($.tr("Ma&ximize"), $self);
        $.connect($.maximizeAction, SIGNAL("triggered()"), SLOT("showMaximized()"));

        $.restoreAction = new QAction($.tr("&Restore"), $self);
        $.connect($.restoreAction, SIGNAL("triggered()"), SLOT("showNormal()"));

        $.quitAction = new QAction($.tr("&Quit"), $self);
        qApp().connect($.quitAction, SIGNAL("triggered()"), SLOT("quit()"));
    }

    private createTrayIcon()
    {
        $.trayIconMenu = new QMenu($self);
        $.trayIconMenu.addAction($.minimizeAction);
        $.trayIconMenu.addAction($.maximizeAction);
        $.trayIconMenu.addAction($.restoreAction);
        $.trayIconMenu.addSeparator();
        $.trayIconMenu.addAction($.quitAction);

        $.trayIcon = new QSystemTrayIcon($self);
        $.trayIcon.setContextMenu($.trayIconMenu);
    }
}

class systray_example inherits QApplication
{
    constructor()
    {
        our $dir = get_script_dir();

        if (!QSystemTrayIcon::isSystemTrayAvailable()) {
            QMessageBox::critical(NOTHING, $.tr("Systray"),
                                 $.tr("I couldn't detect any system tray on this system."));
            return;
        }
        
        my $window = new Window();
        $window.show();
        $.exec();
    }
}
