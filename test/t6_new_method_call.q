#!/usr/bin/env qore
%requires qt4

my $o = new QObject();
$o.setObjectName("foo");
printf("name: %N\n", $o.objectName());

