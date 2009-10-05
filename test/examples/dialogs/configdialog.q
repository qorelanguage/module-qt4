#!/usr/bin/env qore

# this is basically a direct port of the QT widget example
# "configdialog" to Qore using Qore's "qt" module.  

# Note that Qore's "qt" module requires QT 4.3 or above 

# use the "qt-gui" module
%requires qt4

# this is an object-oriented program; the application class is "configdialog_example"
%exec-class configdialog_example
# require all variables to be explicitly declared
%require-our
# enable all parse warnings
%enable-all-warnings

class ConfigDialog inherits QDialog
{
    private $.contentsWidget, $.pagesWidget;

    constructor()
    {
        $.contentsWidget = new QListWidget();
        $.contentsWidget.setViewMode(QListView::IconMode);
        $.contentsWidget.setIconSize(new QSize(96, 84));
        $.contentsWidget.setMovement(QListView::Static);
        $.contentsWidget.setMaximumWidth(128);
        $.contentsWidget.setSpacing(12);

        $.pagesWidget = new QStackedWidget();
        $.pagesWidget.addWidget(new ConfigurationPage());
        $.pagesWidget.addWidget(new UpdatePage());
        $.pagesWidget.addWidget(new QueryPage());

        my $closeButton = new QPushButton($.tr("Close"));

        $.createIcons();
        $.contentsWidget.setCurrentRow(0);

        $.connect($closeButton, SIGNAL("clicked()"), SLOT("close()"));

        my $horizontalLayout = new QHBoxLayout();
        $horizontalLayout.addWidget($.contentsWidget);
        $horizontalLayout.addWidget($.pagesWidget, 1);

        my $buttonsLayout = new QHBoxLayout();
        $buttonsLayout.addStretch(1);
        $buttonsLayout.addWidget($closeButton);

        my $mainLayout = new QVBoxLayout();
        $mainLayout.addLayout($horizontalLayout);
        $mainLayout.addStretch(1);
        $mainLayout.addSpacing(12);
        $mainLayout.addLayout($buttonsLayout);
        $.setLayout($mainLayout);

        $.setWindowTitle($.tr("Config Dialog"));
    }

    private createIcons()
    {
        my $configButton = new QListWidgetItem($.contentsWidget);
        $configButton.setIcon(new QIcon($dir + "images/config.png"));
        $configButton.setText($.tr("Configuration"));
        $configButton.setTextAlignment(Qt::AlignHCenter);
        $configButton.setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

        my $updateButton = new QListWidgetItem($.contentsWidget);
        $updateButton.setIcon(new QIcon($dir + "images/update.png"));
        $updateButton.setText($.tr("Update"));
        $updateButton.setTextAlignment(Qt::AlignHCenter);
        $updateButton.setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

        my $queryButton = new QListWidgetItem($.contentsWidget);
        $queryButton.setIcon(new QIcon($dir + "images/query.png"));
        $queryButton.setText($.tr("Query"));
        $queryButton.setTextAlignment(Qt::AlignHCenter);
        $queryButton.setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

        $.connect($.contentsWidget, SIGNAL("currentItemChanged(QListWidgetItem *, QListWidgetItem *)"), SLOT("changePage(QListWidgetItem *, QListWidgetItem*)"));
    }
    
    changePage($current, $previous)
    {
        if (!exists $current)
            $current = $previous;

        $.pagesWidget.setCurrentIndex($.contentsWidget.row($current));
    }
}

class ConfigurationPage inherits QWidget
{
    constructor($parent) : QWidget($parent)
    {
        my $configGroup = new QGroupBox($.tr("Server configuration"));

        my $serverLabel = new QLabel($.tr("Server:"));
        my $serverCombo = new QComboBox();
        $serverCombo.addItem($.tr("Trolltech (Australia)"));
        $serverCombo.addItem($.tr("Trolltech (Germany)"));
        $serverCombo.addItem($.tr("Trolltech (Norway)"));
        $serverCombo.addItem($.tr("Trolltech (People's Republic of China)"));
        $serverCombo.addItem($.tr("Trolltech (USA)"));

        my $serverLayout = new QHBoxLayout();
        $serverLayout.addWidget($serverLabel);
        $serverLayout.addWidget($serverCombo);

        my $configLayout = new QVBoxLayout();
        $configLayout.addLayout($serverLayout);
        $configGroup.setLayout($configLayout);

        my $mainLayout = new QVBoxLayout();
        $mainLayout.addWidget($configGroup);
        $mainLayout.addStretch(1);
        $.setLayout($mainLayout);    
    }
}

class QueryPage inherits QWidget
{
    constructor($parent) : QWidget($parent)
    {
        my $packagesGroup = new QGroupBox($.tr("Look for packages"));

        my $nameLabel = new QLabel($.tr("Name:"));
        my $nameEdit = new QLineEdit();

        my $dateLabel = new QLabel($.tr("Released after:"));
        my $dateEdit = new QDateTimeEdit(now());

        my $releasesCheckBox = new QCheckBox($.tr("Releases"));
        my $upgradesCheckBox = new QCheckBox($.tr("Upgrades"));

        my $hitsSpinBox = new QSpinBox();
        $hitsSpinBox.setPrefix($.tr("Return up to "));
        $hitsSpinBox.setSuffix($.tr(" results"));
        $hitsSpinBox.setSpecialValueText($.tr("Return only the first result"));
        $hitsSpinBox.setMinimum(1);
        $hitsSpinBox.setMaximum(100);
        $hitsSpinBox.setSingleStep(10);

        my $startQueryButton = new QPushButton($.tr("Start query"));

        my $packagesLayout = new QGridLayout();
        $packagesLayout.addWidget($nameLabel, 0, 0);
        $packagesLayout.addWidget($nameEdit, 0, 1);
        $packagesLayout.addWidget($dateLabel, 1, 0);
        $packagesLayout.addWidget($dateEdit, 1, 1);
        $packagesLayout.addWidget($releasesCheckBox, 2, 0);
        $packagesLayout.addWidget($upgradesCheckBox, 3, 0);
        $packagesLayout.addWidget($hitsSpinBox, 4, 0, 1, 2);
        $packagesGroup.setLayout($packagesLayout);

        my $mainLayout = new QVBoxLayout();
        $mainLayout.addWidget($packagesGroup);
        $mainLayout.addSpacing(12);
        $mainLayout.addWidget($startQueryButton);
        $mainLayout.addStretch(1);
        $.setLayout($mainLayout);
    }
}

class UpdatePage inherits QWidget
{
    constructor($parent) : QWidget($parent)
    {
        my $updateGroup = new QGroupBox($.tr("Package selection"));
        my $systemCheckBox = new QCheckBox($.tr("Update system"));
        my $appsCheckBox = new QCheckBox($.tr("Update applications"));
        my $docsCheckBox = new QCheckBox($.tr("Update documentation"));

        my $packageGroup = new QGroupBox($.tr("Existing packages"));

        my $packageList = new QListWidget();
        my $qtItem = new QListWidgetItem($packageList);
        $qtItem.setText($.tr("Qt"));

        #$.qtItem = new QListWidgetItem($packageList);
        #$.qtItem.setText($.tr("Qt"));

        my $qsaItem = new QListWidgetItem($packageList);
        $qsaItem.setText($.tr("QSA"));
        my $teamBuilderItem = new QListWidgetItem($packageList);
        $teamBuilderItem.setText($.tr("Teambuilder"));

        my $startUpdateButton = new QPushButton($.tr("Start update"));

        my $updateLayout = new QVBoxLayout();
        $updateLayout.addWidget($systemCheckBox);
        $updateLayout.addWidget($appsCheckBox);
        $updateLayout.addWidget($docsCheckBox);
        $updateGroup.setLayout($updateLayout);
        
        my $packageLayout = new QVBoxLayout();
        $packageLayout.addWidget($packageList);
        $packageGroup.setLayout($packageLayout);

        my $mainLayout = new QVBoxLayout();
        $mainLayout.addWidget($updateGroup);
        $mainLayout.addWidget($packageGroup);
        $mainLayout.addSpacing(12);
        $mainLayout.addWidget($startUpdateButton);
        $mainLayout.addStretch(1);
        $.setLayout($mainLayout);
    }
}

class configdialog_example inherits QApplication
{
    constructor()
    {
        our $dir = get_script_dir();

        my $dialog = new ConfigDialog();
        $dialog.show();
        
            $.exec();
    }
}
