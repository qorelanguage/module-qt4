#!/usr/bin/env qore

# this is basically a direct port of the QT widget example
# "spinboxes" to Qore using Qore's "qt" module.  

# Note that Qore's "qt" module requires QT 4.3 or above 

# use the "qt-gui" module
%requires qt4

# this is an object-oriented program; the application class is "spinboxes_example"
%exec-class spinboxes_example
# require all variables to be explicitly declared
%require-our
# enable all parse warnings
%enable-all-warnings

class Window inherits QWidget
{
    constructor()
    {
        $.createSpinBoxes();
        $.createDateTimeEdits();
        $.createDoubleSpinBoxes();

        my $layout = new QHBoxLayout();
        $layout.addWidget($.spinBoxesGroup);
        $layout.addWidget($.editsGroup);
        $layout.addWidget($.doubleSpinBoxesGroup);
        $.setLayout($layout);

        $.setWindowTitle($.tr("Spin Boxes"));
    }

    createSpinBoxes()
    {
        $.spinBoxesGroup = new QGroupBox($.tr("Spinboxes"));

        my $integerLabel = new QLabel($.tr(sprintf("Enter a value between %d and %d:", -20, 20)));
        my $integerSpinBox = new QSpinBox();
        $integerSpinBox.setRange(-20, 20);
        $integerSpinBox.setSingleStep(1);
        $integerSpinBox.setValue(0);

        my $zoomLabel = new QLabel($.tr(sprintf("Enter a zoom value between %d and %d:", 0, 1000)));
        my $zoomSpinBox = new QSpinBox();
        $zoomSpinBox.setRange(0, 1000);
        $zoomSpinBox.setSingleStep(10);
        $zoomSpinBox.setSuffix("%");
        $zoomSpinBox.setSpecialValueText($.tr("Automatic"));
        $zoomSpinBox.setValue(100);

        my $priceLabel = new QLabel($.tr(sprintf("Enter a price between %d and %d:", 0, 999)));
        my $priceSpinBox = new QSpinBox();
        $priceSpinBox.setRange(0, 999);
        $priceSpinBox.setSingleStep(1);
        $priceSpinBox.setPrefix("\$");
        $priceSpinBox.setValue(99);

        my $spinBoxLayout = new QVBoxLayout();
        $spinBoxLayout.addWidget($integerLabel);
        $spinBoxLayout.addWidget($integerSpinBox);
        $spinBoxLayout.addWidget($zoomLabel);
        $spinBoxLayout.addWidget($zoomSpinBox);
        $spinBoxLayout.addWidget($priceLabel);
        $spinBoxLayout.addWidget($priceSpinBox);
        $.spinBoxesGroup.setLayout($spinBoxLayout);
    }

    createDateTimeEdits()
    {
        $.editsGroup = new QGroupBox($.tr("Date and time spin boxes"));

        my $dateLabel = new QLabel();
        my $dateEdit = new QDateEdit(now());
        $dateEdit.setDateRange(2005-01-01, 2010-12-31);
        $dateLabel.setText(sprintf($.tr("Appointment date (between %s and %s):"), 
                                   $dateEdit.minimumDate().toString(Qt::ISODate),
                                   $dateEdit.maximumDate().toString(Qt::ISODate)));

        my $timeLabel = new QLabel();
        my $timeEdit = new QTimeEdit(now());
        $timeEdit.setTimeRange(new QTime(9, 0, 0, 0), new QTime(16, 30, 0, 0));
        $timeLabel.setText(sprintf($.tr("Appointment time (between %s and %s):"), 
                                   $timeEdit.minimumTime().toString(Qt::ISODate),
                                   $timeEdit.maximumTime().toString(Qt::ISODate)));

        $.meetingLabel = new QLabel();
        $.meetingEdit = new QDateTimeEdit(now());

        my $formatLabel = new QLabel($.tr("Format string for the meeting date and time:"));
        my $formatComboBox = new QComboBox();
        $formatComboBox.addItem("yyyy-MM-dd hh:mm:ss (zzz 'ms')");
        $formatComboBox.addItem("hh:mm:ss MM/dd/yyyy");
        $formatComboBox.addItem("hh:mm:ss dd/MM/yyyy");
        $formatComboBox.addItem("hh:mm:ss");
        $formatComboBox.addItem("hh:mm ap");

        $.connect($formatComboBox, SIGNAL("activated(const QString &)"), SLOT("setFormatString(const QString &)"));

        $.setFormatString($formatComboBox.currentText());

        my $editsLayout = new QVBoxLayout();
        $editsLayout.addWidget($dateLabel);
        $editsLayout.addWidget($dateEdit);
        $editsLayout.addWidget($timeLabel);
        $editsLayout.addWidget($timeEdit);
        $editsLayout.addWidget($.meetingLabel);
        $editsLayout.addWidget($.meetingEdit);
        $editsLayout.addWidget($formatLabel);
        $editsLayout.addWidget($formatComboBox);
        $.editsGroup.setLayout($editsLayout);
    }

    setFormatString($formatString)
    {
        $.meetingEdit.setDisplayFormat($formatString);
        if ($.meetingEdit.displayedSections() & QDateTimeEdit::DateSections_Mask) {
            $.meetingEdit.setDateRange(2004-11-01, 2005-11-30);
            $.meetingLabel.setText(sprintf($.tr("Meeting date (between %s and %s):"),
                                           $.meetingEdit.minimumDate().toString(Qt::ISODate),
                                           $.meetingEdit.maximumDate().toString(Qt::ISODate)));
        } else {
            $.meetingEdit.setTimeRange(new QTime(0, 7, 20, 0), new QTime(21, 0, 0, 0));
            $.meetingLabel.setText(sprintf($.tr("Meeting time (between %s and %s):"),
                                           $.meetingEdit.minimumTime().toString(Qt::ISODate),
                                           $.meetingEdit.maximumTime().toString(Qt::ISODate)));
        }
    }

    createDoubleSpinBoxes()
    {
        $.doubleSpinBoxesGroup = new QGroupBox($.tr("Double precision spinboxes"));

        my $precisionLabel = new QLabel($.tr("Number of decimal places to show:"));
        my $precisionSpinBox = new QSpinBox();
        $precisionSpinBox.setRange(0, 100);
        $precisionSpinBox.setValue(2);

        my $doubleLabel = new QLabel(sprintf($.tr("Enter a value between %d and %d:"), -20, 20));
        $.doubleSpinBox = new QDoubleSpinBox();
        $.doubleSpinBox.setRange(-20.0, 20.0);
        $.doubleSpinBox.setSingleStep(1.0);
        $.doubleSpinBox.setValue(0.0);

        my $scaleLabel = new QLabel(sprintf($.tr("Enter a scale factor between %d and %d:"), 0, 1000.0));
        $.scaleSpinBox = new QDoubleSpinBox();
        $.scaleSpinBox.setRange(0.0, 1000.0);
        $.scaleSpinBox.setSingleStep(10.0);
        $.scaleSpinBox.setSuffix("%");
        $.scaleSpinBox.setSpecialValueText($.tr("No scaling"));
        $.scaleSpinBox.setValue(100.0);

        my $priceLabel = new QLabel(sprintf($.tr("Enter a price between %d and %d:"), 0, 1000));
        $.priceSpinBox = new QDoubleSpinBox();
        $.priceSpinBox.setRange(0.0, 1000.0);
        $.priceSpinBox.setSingleStep(1.0);
        $.priceSpinBox.setPrefix("\$");
        $.priceSpinBox.setValue(99.99);
        
        $.connect($precisionSpinBox, SIGNAL("valueChanged(int)"), SLOT("changePrecision(int)"));
        
        my $spinBoxLayout = new QVBoxLayout();
        $spinBoxLayout.addWidget($precisionLabel);
        $spinBoxLayout.addWidget($precisionSpinBox);
        $spinBoxLayout.addWidget($doubleLabel);
        $spinBoxLayout.addWidget($.doubleSpinBox);
        $spinBoxLayout.addWidget($scaleLabel);
        $spinBoxLayout.addWidget($.scaleSpinBox);
        $spinBoxLayout.addWidget($priceLabel);
        $spinBoxLayout.addWidget($.priceSpinBox);
        $.doubleSpinBoxesGroup.setLayout($spinBoxLayout);
    }
    
    changePrecision($decimals)
    {
        $.doubleSpinBox.setDecimals($decimals);
        $.scaleSpinBox.setDecimals($decimals);
        $.priceSpinBox.setDecimals($decimals);
    }
}

class spinboxes_example inherits QApplication
{
    constructor()
    {
        my $window = new Window();
        $window.show();
        $.exec();
    }
}
