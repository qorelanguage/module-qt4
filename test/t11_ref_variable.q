#!/usr/bin/env qore
%requires qt4


my $col = new QColor(100, 200, 1);

my $c;
my $m;
my $y;
my $k;

$col.getCmyk($c, $m, $y, $k);
printf("!ref: C %N M %N Y %N K %N\n", $c, $m, $y, $k);

$col.getCmyk(\$c, \$m, \$y, \$k);
printf(" ref: C %N M %N Y %N K %N\n", $c, $m, $y, $k);

my $rf;
my $gf;
my $bf;
my $af;
$col.getRgbF(\$rf, \$gf, \$bf, \$af);
printf(" ref: R %N G %N B %N A %N\n", $rf, $gf, $bf, $af);

