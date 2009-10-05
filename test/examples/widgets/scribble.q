#!/usr/bin/env qore

# This is basically a direct port of the QT widget example
# "scribble" to Qore using Qore's "qt" module.  

# Note that Qore's "qt" module requires QT 4.3 or above 

# use the "qt-gui" module
%requires qt4

# this is an object-oriented program, the application class is "scribble_example"
%exec-class scribble_example
# require all variables to be explicitly declared
%require-our
# enable all parse warnings
%enable-all-warnings

class MainWindow inherits QMainWindow
{
    constructor()
    {
        $.saveAsActs = ();

        $.scribbleArea = new ScribbleArea();
        $.setCentralWidget($.scribbleArea);

        $.createActions();
        $.createMenus();

        $.setWindowTitle($.tr("Scribble"));
        $.resize(500, 500);
    }

    closeEvent($event)
    {
        if ($.maybeSave()) {
            $event.accept();
        } else {
            $event.ignore();
        }
    }

    open()
    {
        if ($.maybeSave()) {
            my $fileName = QFileDialog::getOpenFileName($self, $.tr("Open File"), QDir::currentPath());
            if (strlen($fileName))
                $.scribbleArea.openImage($fileName);
        }
    }

    save()
    {
        my $action = $.sender();
        my $fileFormat = $action.data();
        $.saveFile($fileFormat);
    }

    penColor()
    {
        my $newColor = QColorDialog::getColor($.scribbleArea.penColor());
        if ($newColor.isValid())
            $.scribbleArea.setPenColor($newColor);
    }

    penWidth()
    {
        my $ok;
        my $newWidth = QInputDialog::getInteger($self, $.tr("Scribble"), $.tr("Select pen width:"), $.scribbleArea.penWidth(), 1, 50, 1, \$ok);
        if ($ok)
            $.scribbleArea.setPenWidth($newWidth);
    }

    about()
    {
        QMessageBox::about($self, $.tr("About Scribble"),
                          $.tr("<p>The <b>Scribble</b> example shows how to use QMainWindow as the base widget for an application, and how to reimplement some of QWidget's event handlers to receive the events generated for the application's widgets:</p><p> We reimplement the mouse event handlers to facilitate drawing, the paint event handler to update the application and the resize event handler to optimize the application's appearance. In addition we reimplement the close event handler to intercept the close events before terminating the application.</p><p> The example also demonstrates how to use QPainter to draw an image in real time, as well as to repaint widgets.</p>"));
    }

    createActions()
    {
        $.openAct = new QAction($.tr("&Open..."), $self);
        $.openAct.setShortcut($.tr("Ctrl+O"));
        $.connect($.openAct, SIGNAL("triggered()"), SLOT("open()"));

        foreach my $format in (QImageWriter::supportedImageFormats()) {
            my $text = $.tr(sprintf("%s...", toupper($format)));

            my $action = new QAction($text, $self);
            $action.setData($format);
            $.connect($action, SIGNAL("triggered()"), SLOT("save()"));
            $.saveAsActs += $action;
        }

        $.printAct = new QAction($.tr("&Print..."), $self);
        $.scribbleArea.connect($.printAct, SIGNAL("triggered()"), SLOT("print()"));

        $.exitAct = new QAction($.tr("E&xit"), $self);
        $.exitAct.setShortcut($.tr("Ctrl+Q"));
        $.connect($.exitAct, SIGNAL("triggered()"), SLOT("close()"));

        $.penColorAct = new QAction($.tr("&Pen Color..."), $self);
        $.connect($.penColorAct, SIGNAL("triggered()"), SLOT("penColor()"));

        $.penWidthAct = new QAction($.tr("Pen &Width..."), $self);
        $.connect($.penWidthAct, SIGNAL("triggered()"), SLOT("penWidth()"));

        $.clearScreenAct = new QAction($.tr("&Clear Screen"), $self);
        $.clearScreenAct.setShortcut($.tr("Ctrl+L"));
        $.scribbleArea.connect($.clearScreenAct, SIGNAL("triggered()"), SLOT("clearImage()"));

        $.aboutAct = new QAction($.tr("&About"), $self);
        $.connect($.aboutAct, SIGNAL("triggered()"), SLOT("about()"));

        $.aboutQtAct = new QAction($.tr("About &Qt"), $self);
        qApp().connect($.aboutQtAct, SIGNAL("triggered()"), SLOT("aboutQt()"));
    }

    createMenus()
    {
        $.saveAsMenu = new QMenu($.tr("&Save As"), $self);
        foreach my $action in ($.saveAsActs)
            $.saveAsMenu.addAction($action);

        $.fileMenu = new QMenu($.tr("&File"), $self);
        $.fileMenu.addAction($.openAct);
        $.fileMenu.addMenu($.saveAsMenu);
        $.fileMenu.addAction($.printAct);
        $.fileMenu.addSeparator();
        $.fileMenu.addAction($.exitAct);

        $.optionMenu = new QMenu($.tr("&Options"), $self);
        $.optionMenu.addAction($.penColorAct);
        $.optionMenu.addAction($.penWidthAct);
        $.optionMenu.addSeparator();
        $.optionMenu.addAction($.clearScreenAct);

        $.helpMenu = new QMenu($.tr("&Help"), $self);
        $.helpMenu.addAction($.aboutAct);
        $.helpMenu.addAction($.aboutQtAct);

        $.menuBar().addMenu($.fileMenu);
        $.menuBar().addMenu($.optionMenu);
        $.menuBar().addMenu($.helpMenu);
    }

    maybeSave()
    {
        if ($.scribbleArea.isModified()) {
            my $ret = QMessageBox::warning($self, $.tr("Scribble"),
                                          $.tr("The image has been modified.\nDo you want to save your changes?"),
                                          QMessageBox::Save | QMessageBox::Discard
                                          | QMessageBox::Cancel);
            if ($ret == QMessageBox::Save) {
                return $.saveFile("png");
            } else if ($ret == QMessageBox::Cancel) {
                return False;
            }
        }
        return True;
    }

    saveFile($fileFormat)
    {
        my $initialPath = QDir::currentPath() + "/untitled." + $fileFormat;

        my $fileName = QFileDialog::getSaveFileName($self, $.tr("Save As"),
                                                   $initialPath,
                                                   $.tr(sprintf("%s Files (*.%s);;All Files (*)", 
                                                              toupper($fileFormat), $fileFormat)));
        if (!strlen($fileName)) {
            return False;
        } else {
            return $.scribbleArea.saveImage($fileName, $fileFormat);
        }
    }
}

class ScribbleArea inherits QWidget
{
    constructor($parent) : QWidget($parent)
    {
        $.image = new QImage();
        $.setAttribute(Qt::WA_StaticContents);
        $.modified = False;
        $.scribbling = False;
        $.myPenWidth = 1;
        $.myPenColor = Qt::blue;
    }

    isModified()
    { 
        return $.modified; 
    }

    penColor()
    {
        return $.myPenColor; 
    }

    penWidth()
    {
        return $.myPenWidth; 
    }

    openImage($fileName)
    {
        my $loadedImage = new QImage();
        if (!$loadedImage.load($fileName))
            return False;

        my $newSize = $loadedImage.size().expandedTo($.size());
        $.resizeImage(\$loadedImage, $newSize);
        $.image = $loadedImage;
        $.modified = False;
        $.update();
        return True;
    }

    saveImage($fileName, $fileFormat)
    {
        my $visibleImage = $.image;
        $.resizeImage(\$visibleImage, $.size());
        
        if ($visibleImage.save($fileName, $fileFormat)) {
            $.modified = False;
            return True;
        } else {
            return False;
        }
    }
    
    setPenColor($newColor)
    {
        $.myPenColor = $newColor;
    }
    
    setPenWidth($newWidth)
    {
        $.myPenWidth = $newWidth;
    }
    
    clearImage()
    {
        $.image.fill(0xffffff);
        $.modified = True;
        $.update();
    }
    
    mousePressEvent($event)
    {
        if ($event.button() == Qt::LeftButton) {
            $.lastPoint = $event.pos();
            $.scribbling = True;
        }
    }
    
    mouseMoveEvent($event)
    {
        if (($event.buttons() & Qt::LeftButton) && $.scribbling)
            $.drawLineTo($event.pos());
    }
    
    mouseReleaseEvent($event)
    {
        if ($event.button() == Qt::LeftButton && $.scribbling) {
            $.drawLineTo($event.pos());
            $.scribbling = False;
        }
    }
    
    paintEvent($event)
    {
        my $painter = new QPainter($self);
        $painter.drawImage(new QPoint(0, 0), $.image);
    }
    
    resizeEvent($event)
    {
        if ($.width() > $.image.width() || $.height() > $.image.height()) {
            my $newWidth = max($.width() + 128, $.image.width());
            my $newHeight = max($.height() + 128, $.image.height());
            $.resizeImage(\$.image, new QSize($newWidth, $newHeight));
            $.update();
        }
        QWidget::$.resizeEvent($event);
    }
    
    drawLineTo($endPoint)
    {
        my $painter = new QPainter($.image);
        $painter.setPen(new QPen($.myPenColor, $.myPenWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        $painter.drawLine($.lastPoint, $endPoint);
        $.modified = True;
        
        my $rad = ($.myPenWidth / 2) + 2;
        $.update((new QRect($.lastPoint, $endPoint)).normalized().adjusted(-$rad, -$rad, $rad, $rad));
        $.lastPoint = $endPoint;
    }
    
    resizeImage($image, $newSize)
    {
        if ($image.size().width() == $newSize.width() && 
            $image.size().height() == $newSize.height())
            return;
        
        my $newImage = new QImage($newSize, QImage::Format_RGB32);
        $newImage.fill(0xffffff);
        my $painter = new QPainter($newImage);
        $painter.drawImage(new QPoint(0, 0), $image);
        $image = $newImage;
    }
    
    print()
    {
        my $printer = new QPrinter(QPrinter::HighResolution);
        
        my $printDialog = new QPrintDialog($printer, $self);
        if ($printDialog.exec() == QDialog::Accepted) {
            my $painter = new QPainter($printer);
            my $rect = $painter.viewport();
            my $size = $.image.size();
            $size.scale($rect.size(), Qt::KeepAspectRatio);
            $painter.setViewport($rect.x(), $rect.y(), $size.width(), $size.height());
            $painter.setWindow($.image.rect());
            $painter.drawImage(0, 0, $.image);
        }
    }    
}

class scribble_example inherits QApplication
{
    constructor()
    {
        my $window = new MainWindow();
        $window.show();
        $.exec();
    }
}
