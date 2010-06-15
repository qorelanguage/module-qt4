#!/usr/bin/env qore

%requires qt4

my QApplication $a();

sub getPM(string $name) {
    my $pm;
    if (!QPixmapCache::find($name, \$pm)) {
        printf("Creating new pixmap %s.png\n", $name);
        $pm = new QPixmap(sprintf("examples/widgets/images/%s.png", $name));
        QPixmapCache::insert($name, $pm);
    }
    else
        printf("Cached pixmap %s.png: %N\n", $name, $pm);
    return $pm;
}

#circle.png  open.png  pause.png  play.png  quit.png  square.png  stop.png  triangle.png  woodbackground.png  woodbutton.png
printf("pixmap 1: %N\n", getPM("circle"));
printf("pixmap 2: %N\n", getPM("woodbutton"));
printf("pixmap 3: %N\n", getPM("woodbutton"));

