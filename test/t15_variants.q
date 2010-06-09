#!/usr/bin/env qore
%requires qt4

my $model = new QDirModel();
my $variant = $model.data($model.index(QDir::currentPath()));
printf("variant: %N\n", $variant);
printf("toQore: %N\n", $variant.toQore());


# Settings test

my $s = new QSettings("t15_variants.cfg", QSettings::IniFormat);

my $v1 = $s.value("intval", 0);
printf("INT: %N, %N\n", $v1, $v1.toInt());

my $v2 = $s.value("floatval", 0.0);
printf("FLOAT: %N, %N\n", $v2, $v2.toDouble());

my $v3 = $s.value("strval", "defaultstr");
printf("STR: %N, %N\n", $v3, $v3.toString());

my $v4 = $s.value("boolval", False);
printf("BOOL: %N, %N\n", $v4, $v4.toBool());

my $v5 = $s.value("nothing", NOTHING);
printf("NOTHING: %N valid:%N null:%N\n", $v5, $v5.isValid(), $v5.isNull());



$s.setValue("intval", 6);
$s.setValue("floatval", 2.1);
$s.setValue("strval", "foo bar!");
$s.setValue("boolval", True);
$s.setValue("nothing", NOTHING);

# objects
my QByteArray $ba("foobar");
my QVariant $vba = $ba;
printf("%N -> %N -> %N\n", $ba, $vba, $vba.toString());

my QFont $f();
my QVariant $vf = $f;
printf("%N -> %N\n", $f, $vf); 


