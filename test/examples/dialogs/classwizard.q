#!/usr/bin/env qore

# this is basically a direct port of the QT widget example
# "classwizard" to Qore using Qore's "qt4" module.  

# Note that Qore's "qt4" module requires QT 4.3 or above 

# use the "qt4" module
%requires qt4

# this is an object-oriented program; the application class is "classwizard_example"
%exec-class classwizard_example
# require all variables to be explicitly declared
%require-our
# enable all parse warnings
%enable-all-warnings

class ClassWizard inherits QWizard {
    constructor($parent) : QWizard($parent) {
        $.addPage(new IntroPage());
        $.addPage(new ClassInfoPage());
        $.addPage(new CodeStylePage());
        $.addPage(new OutputFilesPage());
        $.addPage(new ConclusionPage());

        $.setPixmap(QWizard::BannerPixmap, new QPixmap($dir + "images/banner.png"));
        $.setPixmap(QWizard::BackgroundPixmap, new QPixmap($dir + "images/background.png"));

        $.setWindowTitle($.tr("Class Wizard"));

        QObject::connect($.button(QWizard::FinishButton), SIGNAL("clicked()"), $self, SLOT("accept()"));
    }

    accept() {
        my $className = $.field("className").toString();
        my $baseClass = $.field("baseClass").toString();
        my $macroName = $.field("macroName").toString();
        my $baseInclude = $.field("baseInclude").toString();

        my $outputDir = $.field("outputDir").toString();
        my $header = $.field("header").toString();
        my $implementation = $.field("implementation").toString();

        my string $block;

        if ($.field("comment").toBool()) {
            $block += "/*\n";
            $block += "    " + $header + "\n";
            $block += "*/\n";
            $block += "\n";
        }
        if ($.field("protect").toBool()) {
            $block += "#ifndef " + $macroName + "\n";
            $block += "#define " + $macroName + "\n";
            $block += "\n";
        }
        if ($.field("includeBase").toBool()) {
            $block += "#include " + $baseInclude + "\n";
            $block += "\n";
        }

        $block += "class " + $className;
        if (strlen($baseClass))
            $block += " : public " + $baseClass;
        $block += "\n";
        $block += "{\n";

        if ($.field("qobjectMacro").toBool()) {
                $block += "    Q_OBJECT\n";
                $block += "\n";
        }
        $block += "public:\n";

        if ($.field("qobjectCtor").toBool()) {
            $block += "    " + $className + "(QObject *parent);\n";
        } else if ($.field("qwidgetCtor").toBool()) {
            $block += "    " + $className + "(QWidget *parent);\n";
        } else if ($.field("defaultCtor").toBool()) {
            $block += "    " + $className + "();\n";
            if ($.field("copyCtor").toBool()) {
                $block += "    " + $className + "(const " + $className + " &other);\n";
                $block += "\n";
                $block += "    " + $className + " &operator=" + "(const " + $className + " &other);\n";
            }
        }
        $block += "};\n";

        if ($.field("protect").toBool()) {
            $block += "\n";
            $block += "#endif\n";
        }

        my $filename = $outputDir + "/" + $header;
        my File $headerFile();
        if ($headerFile.open($filename, O_CREAT | O_TRUNC | O_WRONLY)) {
            QMessageBox::warning($self, $.tr("Simple Wizard"),
                                sprintf($.tr("Cannot write file %n:\n%s"), $filename, strerror(errno())));
            return;
        }
        $headerFile.write($block);

        delete $block;

        if ($.field("comment").toBool()) {
            $block += "/*\n";
            $block += "    " + $implementation + "\n";
            $block += "*/\n";
            $block += "\n";
        }
        $block += "#include \"" + $header + "\"\n";
        $block += "\n";

        if ($.field("qobjectCtor").toBool()) {
            $block += $className + "::" + $className + "(QObject *parent)\n";
            $block += "    : " + $baseClass + "(parent)\n";
            $block += "{\n";
            $block += "}\n";
        } else if ($.field("qwidgetCtor").toBool()) {
            $block += $className + "::" + $className + "(QWidget *parent)\n";
            $block += "    : " + $baseClass + "(parent)\n";
            $block += "{\n";
            $block += "}\n";
        } else if ($.field("defaultCtor").toBool()) {
            $block += $className + "::" + $className + "()\n";
            $block += "{\n";
            $block += "    // missing code\n";
            $block += "}\n";

            if ($.field("copyCtor").toBool()) {
                $block += "\n";
                $block += $className + "::" + $className + "(const " + $className
                    + " &other)\n";
                $block += "{\n";
                $block += "    *$self = other;\n";
                $block += "}\n";
                $block += "\n";
                $block += $className + " &" + $className + "::operator=(const " + $className + " &other)\n";
                $block += "{\n";
                if (strlen($baseClass))
                    $block += "    " + $baseClass + "::operator=(other);\n";
                $block += "    // missing code\n";
                $block += "    return *$self;\n";
                $block += "}\n";
            }
        }
        
        $filename = $outputDir + "/" + $implementation;
        my File $implementationFile();
        if ($implementationFile.open($filename, O_CREAT | O_WRONLY | O_TRUNC)) {
            QMessageBox::warning($self, $.tr("Simple Wizard"), sprintf($.tr("Cannot write file %n:\n%s"), $filename, strerror(errno())));
            return;
        }
        $implementationFile.write($block);

        QDialog::$.accept();
    }

}

class IntroPage inherits QWizardPage {
    private $.label;

    constructor($parent) : QWizardPage($parent) {
        $.setTitle($.tr("Introduction"));
        $.setPixmap(QWizard::WatermarkPixmap, new QPixmap($dir + "images/watermark1.png"));

        $.label = new QLabel($.tr("this wizard will generate a skeleton C++ class "
                                "definition, including a few functions. You simply "
                                "need to specify the class name and set a few "
                                "options to produce a header file and an "
                                "implementation file for your new C++ class."));
        $.label.setWordWrap(True);

        my QVBoxLayout $layout();
        $layout.addWidget($.label);
        $.setLayout($layout);
    }

}

class ClassInfoPage inherits QWizardPage {
     private $.classNameLabel, $.baseClassLabel, $.classNameLineEdit, $.baseClassLineEdit, 
     $.qobjectMacroCheckBox, $.groupBox, $.qobjectCtorRadioButton, $.qwidgetCtorRadioButton,
     $.defaultCtorRadioButton, $.copyCtorCheckBox;

     constructor($parent) : QWizardPage($parent) {
         $.setTitle($.tr("Class Information"));
         $.setSubTitle($.tr("Specify basic information about the class for which you "
                          "want to generate skeleton source code files."));
         $.setPixmap(QWizard::LogoPixmap, new QPixmap($dir + "images/logo1.png"));

         $.classNameLabel = new QLabel($.tr("&Class name:"));
         $.classNameLineEdit = new QLineEdit();
         $.classNameLabel.setBuddy($.classNameLineEdit);

         $.baseClassLabel = new QLabel($.tr("B&ase class:"));
         $.baseClassLineEdit = new QLineEdit();
         $.baseClassLabel.setBuddy($.baseClassLineEdit);

         $.qobjectMacroCheckBox = new QCheckBox($.tr("Generate Q_OBJECT &macro"));

         $.groupBox = new QGroupBox($.tr("C&onstructor"));

         $.qobjectCtorRadioButton = new QRadioButton($.tr("&QObject-style constructor"));
         $.qwidgetCtorRadioButton = new QRadioButton($.tr("Q&Widget-style constructor"));
         $.defaultCtorRadioButton = new QRadioButton($.tr("&Default constructor"));
         $.copyCtorCheckBox = new QCheckBox($.tr("&Generate copy constructor and "
                                               "operator="));
	 
         $.defaultCtorRadioButton.setChecked(True);

         $.copyCtorCheckBox.connect($.defaultCtorRadioButton, SIGNAL("toggled(bool)"), SLOT("setEnabled(bool)"));

         $.registerField("className*", $.classNameLineEdit);
         $.registerField("baseClass", $.baseClassLineEdit);
         $.registerField("qobjectMacro", $.qobjectMacroCheckBox);
         $.registerField("qobjectCtor", $.qobjectCtorRadioButton);
         $.registerField("qwidgetCtor", $.qwidgetCtorRadioButton);
         $.registerField("defaultCtor", $.defaultCtorRadioButton);
         $.registerField("copyCtor", $.copyCtorCheckBox);

         my QVBoxLayout $groupBoxLayout();
         $groupBoxLayout.addWidget($.qobjectCtorRadioButton);
         $groupBoxLayout.addWidget($.qwidgetCtorRadioButton);
         $groupBoxLayout.addWidget($.defaultCtorRadioButton);
         $groupBoxLayout.addWidget($.copyCtorCheckBox);
         $.groupBox.setLayout($groupBoxLayout);

         my QGridLayout $layout();
         $layout.addWidget($.classNameLabel, 0, 0);
         $layout.addWidget($.classNameLineEdit, 0, 1);
         $layout.addWidget($.baseClassLabel, 1, 0);
         $layout.addWidget($.baseClassLineEdit, 1, 1);
         $layout.addWidget($.qobjectMacroCheckBox, 2, 0, 1, 2);
         $layout.addWidget($.groupBox, 3, 0, 1, 2);
         $.setLayout($layout);
     }
}

class CodeStylePage inherits QWizardPage {
     private $.commentCheckBox, $.protectCheckBox, $.includeBaseCheckBox, $.macroNameLabel,
     $.baseIncludeLabel, $.macroNameLineEdit, $.baseIncludeLineEdit;
     
     constructor($parent) : QWizardPage($parent) {
         $.setTitle($.tr("Code Style Options"));
         $.setSubTitle($.tr("Choose the formatting of the generated code."));
         $.setPixmap(QWizard::LogoPixmap, new QPixmap($dir + "images/logo2.png"));

         $.commentCheckBox = new QCheckBox($.tr("&Start generated files with a "
                                            "comment"));
         $.commentCheckBox.setChecked(True);

         $.protectCheckBox = new QCheckBox($.tr("&Protect header file against multiple "
                                            "inclusions"));
         $.protectCheckBox.setChecked(True);

         $.macroNameLabel = new QLabel($.tr("&Macro name:"));
         $.macroNameLineEdit = new QLineEdit();
         $.macroNameLabel.setBuddy($.macroNameLineEdit);

         $.includeBaseCheckBox = new QCheckBox($.tr("&Include base class definition"));
         $.baseIncludeLabel = new QLabel($.tr("Base class include:"));
         $.baseIncludeLineEdit = new QLineEdit();
         $.baseIncludeLabel.setBuddy($.baseIncludeLineEdit);

         $.macroNameLabel.connect($.protectCheckBox, SIGNAL("toggled(bool)"), SLOT("setEnabled(bool)"));
         $.macroNameLineEdit.connect($.protectCheckBox, SIGNAL("toggled(bool)"), SLOT("setEnabled(bool)"));
         $.baseIncludeLabel.connect($.includeBaseCheckBox, SIGNAL("toggled(bool)"), SLOT("setEnabled(bool)"));
         $.baseIncludeLineEdit.connect($.includeBaseCheckBox, SIGNAL("toggled(bool)"), SLOT("setEnabled(bool)"));

         $.registerField("comment", $.commentCheckBox);
         $.registerField("protect", $.protectCheckBox);
         $.registerField("macroName", $.macroNameLineEdit);
         $.registerField("includeBase", $.includeBaseCheckBox);
         $.registerField("baseInclude", $.baseIncludeLineEdit);

         my QGridLayout $layout();
         $layout.setColumnMinimumWidth(0, 20);
         $layout.addWidget($.commentCheckBox, 0, 0, 1, 3);
         $layout.addWidget($.protectCheckBox, 1, 0, 1, 3);
         $layout.addWidget($.macroNameLabel, 2, 1);
         $layout.addWidget($.macroNameLineEdit, 2, 2);
         $layout.addWidget($.includeBaseCheckBox, 3, 0, 1, 3);
         $layout.addWidget($.baseIncludeLabel, 4, 1);
         $layout.addWidget($.baseIncludeLineEdit, 4, 2);
         $.setLayout($layout);
     }
     
     initializePage() {
         my $className = $.field("className").toString();
         $.macroNameLineEdit.setText(toupper($className) + "_H");

         my $baseClass = $.field("baseClass").toString();

         $.includeBaseCheckBox.setChecked(boolean(strlen($baseClass)));
         $.includeBaseCheckBox.setEnabled(boolean(strlen($baseClass)));
         $.baseIncludeLabel.setEnabled(boolean(strlen($baseClass)));
         $.baseIncludeLineEdit.setEnabled(boolean(strlen($baseClass)));

         if (!strlen($baseClass)) {
             $.baseIncludeLineEdit.clear();
         } else if ((new QRegExp("Q[A-Z].*")).exactMatch($baseClass)) {
             $.baseIncludeLineEdit.setText("<" + $baseClass + ">");
         } else {
             $.baseIncludeLineEdit.setText("\"" + tolower($baseClass) + ".h\"");
         }         
     }
}

class OutputFilesPage inherits QWizardPage {
    private $.outputDirLabel, $.headerLabel, $.implementationLabel, $.outputDirLineEdit,
    $.headerLineEdit, $.implementationLineEdit;

    constructor($parent) : QWizardPage($parent) {
        $.setTitle($.tr("Output Files"));
        $.setSubTitle($.tr("Specify where you want the wizard to put the generated "
                         "skeleton code."));
        $.setPixmap(QWizard::LogoPixmap, new QPixmap($dir + "images/logo3.png"));

        $.outputDirLabel = new QLabel($.tr("&Output directory:"));
        $.outputDirLineEdit = new QLineEdit();
        $.outputDirLabel.setBuddy($.outputDirLineEdit);

        $.headerLabel = new QLabel($.tr("&Header file name:"));
        $.headerLineEdit = new QLineEdit();
        $.headerLabel.setBuddy($.headerLineEdit);

        $.implementationLabel = new QLabel($.tr("&Implementation file name:"));
        $.implementationLineEdit = new QLineEdit();
        $.implementationLabel.setBuddy($.implementationLineEdit);

        $.registerField("outputDir*", $.outputDirLineEdit);
        $.registerField("header*", $.headerLineEdit);
        $.registerField("implementation*", $.implementationLineEdit);

        my QGridLayout $layout();
        $layout.addWidget($.outputDirLabel, 0, 0);
        $layout.addWidget($.outputDirLineEdit, 0, 1);
        $layout.addWidget($.headerLabel, 1, 0);
        $layout.addWidget($.headerLineEdit, 1, 1);
        $layout.addWidget($.implementationLabel, 2, 0);
        $layout.addWidget($.implementationLineEdit, 2, 1);
        $.setLayout($layout);
    }

    initializePage() {
        my $className = $.field("className").toString();
        $.headerLineEdit.setText(tolower($className) + ".h");
        $.implementationLineEdit.setText(tolower($className) + ".cpp");
        $.outputDirLineEdit.setText(QDir::tempPath());
    }
}

class ConclusionPage inherits QWizardPage {
    private $.label;

    constructor($parent) : QWizardPage($parent) {
        $.setTitle($.tr("Conclusion"));
        $.setPixmap(QWizard::WatermarkPixmap, new QPixmap($dir + "images/watermark2.png"));

        $.label = new QLabel();
        $.label.setWordWrap(True);

        my QVBoxLayout $layout();
        $layout.addWidget($.label);
        $.setLayout($layout);
    }

    initializePage() {
        my $finishText = $.wizard().buttonText(QWizard::FinishButton);
        $finishText =~ s/&//g;
        $.label.setText(sprintf($.tr("Click %s to generate the class skeleton."), $finishText));
    }
}

class classwizard_example inherits QApplication {
    constructor() {      
        our string $dir = get_script_dir();

        my string $translatorFileName = "qt_" + QLocale::system().name();
        my QTranslator $translator(qApp());
        if ($translator.load($translatorFileName, QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
            QCoreApplication::installTranslator($translator);
        
        my ClassWizard $wizard();
        $wizard.show();
        
	$.exec();
    }
}
