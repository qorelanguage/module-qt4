#!/usr/bin/env qore
%requires qt4


my QApplication $a();

my QGraphicsView $v();
$v.resize(640, 480);

$v.show();

##################

my QGraphicsScene $s(640, 480, $v);
#printf("FONT %N\n", $s.font());
$v.setScene($s);


##################

my QGraphicsTextItem $i();
$i.setHtml("<h3>Item</h3><p>Lorem <i>ipsum</i></p>");
$s.addItem($i);
$i.setPos(10, 10);



exit($a.exec());


