%requires qt4
%include form.qc

class Form inherits QWidget, Ui::Form
{
        constructor(QWidget $parent) : QWidget($parent)
        {
                $.setupUi($self);
        }

}


my QApplication $app();
my Form $f();
$f.show();
return $app.exec();

