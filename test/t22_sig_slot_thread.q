#!/usr/bin/env qore
%requires qt4


class GeneratorObj inherits QObject
{
        constructor($parent) : QObject($parent)
        {
                printf("generator obj constructed\n");
                $.createSignal("simpleSignal()");
                $.createSignal("stringSignal(const QString&)");

                background $.simpleThread();
                background $.stringThread();
        }

        simpleThread()
        {
                printf("simpleThread started\n");
                while (1) {
                    $.emit("simpleSignal()");
                    printf("SIG simpleSignal emitted. TID %d\n", gettid());
                    sleep(1s);
                }
        }

        stringThread()
        {
                printf("stringThread started\n");
                while (1) {
                    $.emit("stringSignal(const QString&)", "foobar");
                    printf("SIG stringSignal emitted. TID %d\n", gettid());
                    sleep(2s);
                }
        }
}


class TargetObj inherits QObject
{
        constructor($parent) : QObject($parent)
        {
                printf("target obj constructed\n");
        }

        slotSimple()
        {
                printf("SLOT simple. TID %d\n", gettid());
        }

        slotString($str)
        {
                printf("SLOT string: %s. TID %d\n", $str, gettid());
        }
}



my $app = new QApplication();
my $src = new GeneratorObj();
my $tgt = new TargetObj();

QObject::connect($src, SIGNAL("simpleSignal()"), $tgt, SLOT("slotSimple()"));
QObject::connect($src, SIGNAL("stringSignal(const QString&)"), $tgt, SLOT("slotString(const QString&)"));

exit($app.exec());

