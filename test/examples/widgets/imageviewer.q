#!/usr/bin/env qore

# This is basically a direct port of the QT widget example
# "imageviewer" to Qore using Qore's "qt" module.  

# Note that Qore's "qt" module requires QT 4.3 or above 

# use the "qt-gui" module
%requires qt4

# this is an object-oriented program, the application class is "imageviewer_example"
%exec-class imageviewer_example
# require all variables to be explicitly declared
%require-our
# enable all parse warnings
%enable-all-warnings

class ImageViewer inherits QMainWindow
{
    constructor()
    {
        $.printer = new QPrinter();

        $.imageLabel = new QLabel();
        $.imageLabel.setBackgroundRole(QPalette::Base);
        $.imageLabel.setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
        $.imageLabel.setScaledContents(True);

        $.scrollArea = new QScrollArea();
        $.scrollArea.setBackgroundRole(QPalette::Dark);
        $.scrollArea.setWidget($.imageLabel);
        $.setCentralWidget($.scrollArea);

        $.createActions();
        $.createMenus();

        $.setWindowTitle($.tr("Image Viewer"));
        $.resize(500, 400);
    }
    
    open()
    {
        my $fileName = QFileDialog::getOpenFileName($self, $.tr("Open File"), QDir::currentPath());
        if (strlen($fileName)) {
            my $image = new QImage($fileName);
            if ($image.isNull()) {
                QMessageBox::information($self, $.tr("Image Viewer"), $.tr(sprintf("Cannot load %n.", $fileName)));
                return;
            }
            $.imageLabel.setPixmap(QPixmap::fromImage($image));
            $.scaleFactor = 1.0;

            $.printAct.setEnabled(True);
            $.fitToWindowAct.setEnabled(True);
            $.updateActions();

            if (!$.fitToWindowAct.isChecked())
                $.imageLabel.adjustSize();
        }
    }

    print()
    {
        #Q_ASSERT($.imageLabel.pixmap());
        #printf("printer: %N\n", $.printer);
        my $dialog = new QPrintDialog($.printer, $self);
        #printf("dialog: %N\n", $dialog);
        if ($dialog.exec()) {
            my $painter = new QPainter($.printer);
            my $rect = $painter.viewport();
            my $size = $.imageLabel.pixmap().size();
            $size.scale($rect.size(), Qt::KeepAspectRatio);
            $painter.setViewport($rect.x(), $rect.y(), $size.width(), $size.height());
            $painter.setWindow($.imageLabel.pixmap().rect());
            $painter.drawPixmap(0, 0, $.imageLabel.pixmap());
        }
    }

    zoomIn()
    {
        $.scaleImage(1.25);
    }

    zoomOut()
    {
        $.scaleImage(0.8);
    }

    normalSize()
    {
        $.imageLabel.adjustSize();
        $.scaleFactor = 1.0;
    }

    fitToWindow()
    {
        my $fitToWindow = $.fitToWindowAct.isChecked();
        $.scrollArea.setWidgetResizable($fitToWindow);
        if (!$fitToWindow) {
            $.normalSize();
        }
        $.updateActions();
    }

    about()
    {
        QMessageBox::about($self, $.tr("About Image Viewer"),
                          $.tr("<p>The <b>Image Viewer</b> example shows how to combine QLabel and QScrollArea to display an image. QLabel is typically used for displaying a text, but it can also display an image. QScrollArea provides a scrolling view around another widget. If the child widget exceeds the size of the frame, QScrollArea automatically provides scroll bars. </p><p>The example demonstrates how QLabel's ability to scale its contents (QLabel::scaledContents), and QScrollArea's ability to automatically resize its contents (QScrollArea::widgetResizable), can be used to implement zooming and scaling features. </p><p>In addition the example shows how to use QPainter to print an image.</p>"));
    }

    createActions()
    {
        $.openAct = new QAction($.tr("&Open..."), $self);
        $.openAct.setShortcut($.tr("Ctrl+O"));
        $.connect($.openAct, SIGNAL("triggered()"), SLOT("open()"));

        $.printAct = new QAction($.tr("&Print..."), $self);
        $.printAct.setShortcut($.tr("Ctrl+P"));
        $.printAct.setEnabled(False);
        $.connect($.printAct, SIGNAL("triggered()"), SLOT("print()"));

        $.exitAct = new QAction($.tr("E&xit"), $self);
        $.exitAct.setShortcut($.tr("Ctrl+Q"));
        $.connect($.exitAct, SIGNAL("triggered()"), SLOT("close()"));

        $.zoomInAct = new QAction($.tr("Zoom &In (25%)"), $self);
        $.zoomInAct.setShortcut($.tr("Ctrl++"));
        $.zoomInAct.setEnabled(False);
        $.connect($.zoomInAct, SIGNAL("triggered()"), SLOT("zoomIn()"));

        $.zoomOutAct = new QAction($.tr("Zoom &Out (25%)"), $self);
        $.zoomOutAct.setShortcut($.tr("Ctrl+-"));
        $.zoomOutAct.setEnabled(False);
        $.connect($.zoomOutAct, SIGNAL("triggered()"), SLOT("zoomOut()"));

        $.normalSizeAct = new QAction($.tr("&Normal Size"), $self);
        $.normalSizeAct.setShortcut($.tr("Ctrl+S"));
        $.normalSizeAct.setEnabled(False);
        $.connect($.normalSizeAct, SIGNAL("triggered()"), SLOT("normalSize()"));

        $.fitToWindowAct = new QAction($.tr("&Fit to Window"), $self);
        $.fitToWindowAct.setEnabled(False);
        $.fitToWindowAct.setCheckable(True);
        $.fitToWindowAct.setShortcut($.tr("Ctrl+F"));
        $.connect($.fitToWindowAct, SIGNAL("triggered()"), SLOT("fitToWindow()"));

        $.aboutAct = new QAction($.tr("&About"), $self);
        $.connect($.aboutAct, SIGNAL("triggered()"), SLOT("about()"));
        
        $.aboutQtAct = new QAction($.tr("About &Qt"), $self);
        
        qApp().connect($.aboutQtAct, SIGNAL("triggered()"), SLOT("aboutQt()"));
    }

    createMenus()
    {
        $.fileMenu = new QMenu($.tr("&File"), $self);
        $.fileMenu.addAction($.openAct);
        $.fileMenu.addAction($.printAct);
        $.fileMenu.addSeparator();
        $.fileMenu.addAction($.exitAct);

        $.viewMenu = new QMenu($.tr("&View"), $self);
        $.viewMenu.addAction($.zoomInAct);
        $.viewMenu.addAction($.zoomOutAct);
        $.viewMenu.addAction($.normalSizeAct);
        $.viewMenu.addSeparator();
        $.viewMenu.addAction($.fitToWindowAct);

        $.helpMenu = new QMenu($.tr("&Help"), $self);
        $.helpMenu.addAction($.aboutAct);
        $.helpMenu.addAction($.aboutQtAct);

        $.menuBar().addMenu($.fileMenu);
        $.menuBar().addMenu($.viewMenu);
        $.menuBar().addMenu($.helpMenu);
    }

    updateActions()
    {
        $.zoomInAct.setEnabled(!$.fitToWindowAct.isChecked());
        $.zoomOutAct.setEnabled(!$.fitToWindowAct.isChecked());
        $.normalSizeAct.setEnabled(!$.fitToWindowAct.isChecked());
    }

    scaleImage($factor)
    {
        #Q_ASSERT($.imageLabel.pixmap());
        $.scaleFactor *= $factor;
        # note that you can't multiply a QSize object in Qore... :-(
        #$.imagelabel.resize($.scaleFactor * $.imageLabel.pixmap().size());
        $.imageLabel.resize($.scaleFactor * $.imageLabel.pixmap().size().width(), $.scaleFactor * $.imageLabel.pixmap().size().height());

        $.adjustScrollBar($.scrollArea.horizontalScrollBar(), $factor);
        $.adjustScrollBar($.scrollArea.verticalScrollBar(), $factor);

        $.zoomInAct.setEnabled($.scaleFactor < 3.0);
        $.zoomOutAct.setEnabled($.scaleFactor > 0.333);
    }

    adjustScrollBar($scrollBar, $factor)
    {
        $scrollBar.setValue(int($factor * $scrollBar.value()
                               + (($factor - 1) * $scrollBar.pageStep()/2)));
    }
}

class imageviewer_example inherits QApplication
{
    constructor()
    {
        my $imageViewer = new ImageViewer();
        $imageViewer.show();
        $.exec();
    }
}
