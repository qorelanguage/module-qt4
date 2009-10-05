#!/usr/bin/env qore
%requires qt4


my $a = new QApplication();


# vector
my $vc = new QPen();
my $pattern = 1, 2.3, 3, 1;
$vc.setDashPattern($pattern);
my $vec = $vc.dashPattern();
printf("vector: %N\n\n", $vec);

#
# objects, non-QObject based
my $ksq = QKeySequence::keyBindings(QKeySequence::Copy);
printf("ksq: %N\n", $ksq);
foreach my $i in ($ksq)
{
	printf("Item: %N\n", $i.toString());
}


#
# enums
if (!Q_WS_MAC) {
    my $pl = new QPluginLoader();
    printf("PluginLoader: %N\n", $pl);
    my $pli = $pl.staticInstances();
    printf("Static Instances: %N\n", $pli);
}


#
# int
my $p = new QPrinter();
my $sr = $p.supportedResolutions();
printf("supportedResolutions: %N\n", $sr);

#
# QByteArray
my $sif = QImageReader::supportedImageFormats();
printf("supportedImageFormats: %N\n", $sif);



#
# QObjects
my $ag = new QActionGroup($a);
my $a1 = $ag.addAction("foo");
my $a2 = $ag.addAction("bar");
my $a3 = $ag.addAction("act1");
my $al = $ag.actions();
printf("actions: %N\n", $al);
foreach my $i in ($al)
{
    printf("comparation: %N==%N : %N\n", $a2, $i, $a2==$i);
    printf("iisVisible: %N\n", $i.isVisible());
    printf("parent: %N\n", $i.parentWidget());
    printf("text: %N\n", $i.text());
}
printf("actions: %N\n", $al);


