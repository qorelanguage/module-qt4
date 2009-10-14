#!/usr/bin/env qore
%requires qt4

my $a = new QApplication();


class Dialog inherits QDialog
{
        constructor() :  QDialog()
        {
                $.resize(320,200);
        }

        accept()
        {
                printf("accept call\n");
                QDialog::$.accept();
        }

        reject()
        {
                printf("reject call\n");
                QDialog::$.reject();
        }

        done($r)
        {
                printf("done call: %N\n", $r);
                QDialog::$.done($r);
        }
}

my $d = new Dialog();
printf("dialog: %N\n", $d);
$d.exec();

