#!/usr/bin/env qore

# this is basically a direct port of the QT widget example
# "tabdialog" to Qore using Qore's "qt" module.  

# Note that Qore's "qt" module requires QT 4.3 or above 

# use the "qt-gui" module
%requires qt4

# this is an object-oriented program; the application class is "tabdialog_example"
%exec-class tabdialog_example
# require all variables to be explicitly declared
%require-our
# enable all parse warnings
%enable-all-warnings

sub do_date($d)
{
    return format_date("YYYY-MM-DD HH:mm:SS.ms", $d);
}

class GeneralTab inherits QWidget
{
    constructor($fileInfo, $parent) : QWidget($parent)
    {
        my $fileNameLabel = new QLabel($.tr("File Name:"));
        my $fileNameEdit = new QLineEdit($fileInfo.fileName());

        my $pathLabel = new QLabel($.tr("Path:"));
        my $pathValueLabel = new QLabel($fileInfo.absoluteFilePath());
        $pathValueLabel.setFrameStyle(QFrame::Panel | QFrame::Sunken);

        my $sizeLabel = new QLabel($.tr("Size:"));
        my $size = $fileInfo.size()/1024;
        my $sizeValueLabel = new QLabel(sprintf($.tr("%d K"), $size));
        $sizeValueLabel.setFrameStyle(QFrame::Panel | QFrame::Sunken);

        my $lastReadLabel = new QLabel($.tr("Last Read:"));
        my $lastReadValueLabel = new QLabel(do_date($fileInfo.lastRead()));
        $lastReadValueLabel.setFrameStyle(QFrame::Panel | QFrame::Sunken);

        my $lastModLabel = new QLabel($.tr("Last Modified:"));
        my $lastModValueLabel = new QLabel(do_date($fileInfo.lastModified()));
        $lastModValueLabel.setFrameStyle(QFrame::Panel | QFrame::Sunken);

        my $mainLayout = new QVBoxLayout();
        $mainLayout.addWidget($fileNameLabel);
        $mainLayout.addWidget($fileNameEdit);
        $mainLayout.addWidget($pathLabel);
        $mainLayout.addWidget($pathValueLabel);
        $mainLayout.addWidget($sizeLabel);
        $mainLayout.addWidget($sizeValueLabel);
        $mainLayout.addWidget($lastReadLabel);
        $mainLayout.addWidget($lastReadValueLabel);
        $mainLayout.addWidget($lastModLabel);
        $mainLayout.addWidget($lastModValueLabel);
        $mainLayout.addStretch(1);
        $.setLayout($mainLayout);
    }
}

class PermissionsTab inherits QWidget
{
    constructor($fileInfo, $parent) : QWidget($parent)
    {
        my $permissionsGroup = new QGroupBox($.tr("Permissions"));

        my $readable = new QCheckBox($.tr("Readable"));
        if ($fileInfo.isReadable())
            $readable.setChecked(True);

        my $writable = new QCheckBox($.tr("Writable"));
        if ($fileInfo.isWritable() )
            $writable.setChecked(True);

        my $executable = new QCheckBox($.tr("Executable"));
        if ($fileInfo.isExecutable() )
            $executable.setChecked(True);

        my $ownerGroup = new QGroupBox($.tr("Ownership"));

        my $ownerLabel = new QLabel($.tr("Owner"));
        my $ownerValueLabel = new QLabel($fileInfo.owner());
        $ownerValueLabel.setFrameStyle(QFrame::Panel | QFrame::Sunken);

        my $groupLabel = new QLabel($.tr("Group"));
        my $groupValueLabel = new QLabel($fileInfo.group());
        $groupValueLabel.setFrameStyle(QFrame::Panel | QFrame::Sunken);

        my $permissionsLayout = new QVBoxLayout();
        $permissionsLayout.addWidget($readable);
        $permissionsLayout.addWidget($writable);
        $permissionsLayout.addWidget($executable);
        $permissionsGroup.setLayout($permissionsLayout);

        my $ownerLayout = new QVBoxLayout();
        $ownerLayout.addWidget($ownerLabel);
        $ownerLayout.addWidget($ownerValueLabel);
        $ownerLayout.addWidget($groupLabel);
        $ownerLayout.addWidget($groupValueLabel);
        $ownerGroup.setLayout($ownerLayout);

        my $mainLayout = new QVBoxLayout();
        $mainLayout.addWidget($permissionsGroup);
        $mainLayout.addWidget($ownerGroup);
        $mainLayout.addStretch(1);
        $.setLayout($mainLayout);
    }
}

class ApplicationsTab inherits QWidget
{
    constructor($fileInfo, $parent) : QWidget($parent)
    {
        my $topLabel = new QLabel($.tr("Open with:"));

        my $applicationsListBox = new QListWidget();
        my $applications = ();

        for (my $i = 1; $i <= 30; ++$i)
            $applications += sprintf($.tr("Application %d"), $i);
        $applicationsListBox.insertItems(0, $applications);

        my $alwaysCheckBox;

        if (!strlen($fileInfo.suffix()))
            $alwaysCheckBox = new QCheckBox($.tr("Always use this application to "
                                               "open this type of file"));
        else
            $alwaysCheckBox = new QCheckBox(sprintf($.tr("Always use this application to "
                                                      "open files with the extension '%s'"), $fileInfo.suffix()));

        my $layout = new QVBoxLayout();
        $layout.addWidget($topLabel);
        $layout.addWidget($applicationsListBox);
        $layout.addWidget($alwaysCheckBox);
        $.setLayout($layout);
    }
}

class TabDialog inherits QDialog {
    private $.tabWidget, $.buttonBox;

    constructor($fileName, $parent) : QDialog($parent) {
        my QFileInfo $fileInfo($fileName);

        $.tabWidget = new QTabWidget();
        $.tabWidget.addTab(new GeneralTab($fileInfo), $.tr("General"));
        $.tabWidget.addTab(new PermissionsTab($fileInfo), $.tr("Permissions"));
        $.tabWidget.addTab(new ApplicationsTab($fileInfo), $.tr("Applications"));

        $.buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                           | QDialogButtonBox::Cancel);

        $.connect($.buttonBox, SIGNAL("accepted()"), SLOT("accept()"));
        $.connect($.buttonBox, SIGNAL("rejected()"), SLOT("reject()"));
        
        my QVBoxLayout $mainLayout();
        $mainLayout.addWidget($.tabWidget);
        $mainLayout.addWidget($.buttonBox);
        $.setLayout($mainLayout);

        $.setWindowTitle($.tr("Tab Dialog"));
    }
}

class tabdialog_example inherits QApplication {
    constructor() {
        my string $fileName = elements $ARGV >= 1 ? $ARGV[0] : ".";
        my TabDialog $tabdialog($fileName);
        $tabdialog.exec();
    }
}
