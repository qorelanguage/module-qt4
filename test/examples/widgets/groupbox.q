#!/usr/bin/env qore

# This is basically a direct port of the QT widget example
# "groupbox" to Qore using Qore's "qt" module.  

# Note that Qore's "qt" module requires QT 4.3 or above 

# use the "qt-gui" module
%requires qt4

# this is an object-oriented program, the application class is "groupbox_example"
%exec-class groupbox_example
# require all variables to be explicitly declared
%require-our
# enable all parse warnings
%enable-all-warnings

class Window inherits QWidget
{
    constructor($parent) : QWidget($parent)
    {
        my $grid = new QGridLayout();
        $grid.addWidget($.createFirstExclusiveGroup(), 0, 0);
        $grid.addWidget($.createSecondExclusiveGroup(), 1, 0);
        $grid.addWidget($.createNonExclusiveGroup(), 0, 1);
        $grid.addWidget($.createPushButtonGroup(), 1, 1);
        $.setLayout($grid);
        
        $.setWindowTitle($.tr("Group Boxes"));
        $.resize(480, 320);
    }

    createFirstExclusiveGroup()
    {
        my $groupBox = new QGroupBox($.tr("Exclusive Radio Buttons"));

        my $radio1 = new QRadioButton($.tr("&Radio button 1"));
        my $radio2 = new QRadioButton($.tr("R&adio button 2"));
        my $radio3 = new QRadioButton($.tr("Ra&dio button 3"));

        $radio1.setChecked(True);

        my $vbox = new QVBoxLayout();
        $vbox.addWidget($radio1);
        $vbox.addWidget($radio2);
        $vbox.addWidget($radio3);
        $vbox.addStretch(1);
        $groupBox.setLayout($vbox);

        return $groupBox;
    }

    createSecondExclusiveGroup()
    {
        my $groupBox = new QGroupBox($.tr("E&xclusive Radio Buttons"));
        $groupBox.setCheckable(True);
        $groupBox.setChecked(False);

        my $radio1 = new QRadioButton($.tr("Rad&io button 1"));
        my $radio2 = new QRadioButton($.tr("Radi&o button 2"));
        my $radio3 = new QRadioButton($.tr("Radio &button 3"));
        $radio1.setChecked(True);
        my $checkBox = new QCheckBox($.tr("Ind&ependent checkbox"));
        $checkBox.setChecked(True);

        my $vbox = new QVBoxLayout();
        $vbox.addWidget($radio1);
        $vbox.addWidget($radio2);
        $vbox.addWidget($radio3);
        $vbox.addWidget($checkBox);
        $vbox.addStretch(1);
        $groupBox.setLayout($vbox);

        return $groupBox;
    }

    createNonExclusiveGroup()
    {
        my $groupBox = new QGroupBox($.tr("Non-Exclusive Checkboxes"));
        $groupBox.setFlat(True);

        my $checkBox1 = new QCheckBox($.tr("&Checkbox 1"));
        my $checkBox2 = new QCheckBox($.tr("C&heckbox 2"));
        $checkBox2.setChecked(True);
        my $tristateBox = new QCheckBox($.tr("Tri-&state button"));
        $tristateBox.setTristate(True);
        $tristateBox.setCheckState(Qt::PartiallyChecked);

        my $vbox = new QVBoxLayout();
        $vbox.addWidget($checkBox1);
        $vbox.addWidget($checkBox2);
        $vbox.addWidget($tristateBox);
        $vbox.addStretch(1);
        $groupBox.setLayout($vbox);

        return $groupBox;
    }

    createPushButtonGroup()
    {
        my $groupBox = new QGroupBox($.tr("&Push Buttons"));
        $groupBox.setCheckable(True);
        $groupBox.setChecked(True);

        my $pushButton = new QPushButton($.tr("&Normal Button"));
        my $toggleButton = new QPushButton($.tr("&Toggle Button"));
        $toggleButton.setCheckable(True);
        $toggleButton.setChecked(True);
        my $flatButton = new QPushButton($.tr("&Flat Button"));
        $flatButton.setFlat(True);

        my $popupButton = new QPushButton($.tr("Pop&up Button"));
        my $menu = new QMenu($self);
        $menu.addAction($.tr("&First Item"));
        $menu.addAction($.tr("&Second Item"));
        $menu.addAction($.tr("&Third Item"));
        $menu.addAction($.tr("F&ourth Item"));
        $popupButton.setMenu($menu);

        my $newAction = $menu.addAction($.tr("Submenu"));
        my $subMenu = new QMenu($.tr("Popup Submenu"));
        $subMenu.addAction($.tr("Item 1"));
        $subMenu.addAction($.tr("Item 2"));
        $subMenu.addAction($.tr("Item 3"));
        $newAction.setMenu($subMenu);

        my $vbox = new QVBoxLayout();
        $vbox.addWidget($pushButton);
        $vbox.addWidget($toggleButton);
        $vbox.addWidget($flatButton);
        $vbox.addWidget($popupButton);
        $vbox.addStretch(1);
        $groupBox.setLayout($vbox);

        return $groupBox;
    }
}

class groupbox_example inherits QApplication
{
    constructor()
    {
        my $window = new Window();
        $window.show();
        $.exec();
    }
}
