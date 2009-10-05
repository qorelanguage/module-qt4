#!/usr/bin/env qore

# This is basically a direct port of the QT widget example
# "movieplayer" to Qore using Qore's "qt" module.  

# Note that Qore's "qt" module requires QT 4.3 or above 

# use the "qt-gui" module
%requires qt4

# this is an object-oriented program, the application class is "movieplayer_example"
%exec-class movieplayer_example
# require all variables to be explicitly declared
%require-our
# enable all parse warnings
%enable-all-warnings

class MoviePlayer inherits QWidget
{
    constructor($parent) : QWidget($parent)
    {
        $.movie = new QMovie($self);
        $.movie.setCacheMode(QMovie::CacheAll);

        $.movieLabel = new QLabel($.tr("No movie loaded"));
        $.movieLabel.setAlignment(Qt::AlignCenter);
        $.movieLabel.setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
        $.movieLabel.setBackgroundRole(QPalette::Dark);
        $.movieLabel.setAutoFillBackground(True);

        $.currentMovieDirectory = "movies";

        $.createControls();
        $.createButtons();

        $.connect($.movie, SIGNAL("frameChanged(int)"), SLOT("updateFrameSlider()"));
        $.connect($.movie, SIGNAL("stateChanged(QMovie::MovieState)"), SLOT("updateButtons()"));
        $.connect($.fitCheckBox, SIGNAL("clicked()"), SLOT("fitToWindow()"));
        $.connect($.frameSlider, SIGNAL("valueChanged(int)"), SLOT("goToFrame(int)"));
        $.movie.connect($.speedSpinBox, SIGNAL("valueChanged(int)"), SLOT("setSpeed(int)"));

        $.mainLayout = new QVBoxLayout();
        $.mainLayout.addWidget($.movieLabel);
        $.mainLayout.addLayout($.controlsLayout);
        $.mainLayout.addLayout($.buttonsLayout);
        $.setLayout($.mainLayout);

        $.updateFrameSlider();
        $.updateButtons();

        $.setWindowTitle($.tr("Movie Player"));
        $.resize(400, 400);
    }

    open()
    {
        my $fileName = QFileDialog::getOpenFileName($self, $.tr("Open a Movie"), $.currentMovieDirectory);
        if (strlen(!$fileName))
            $.openFile($fileName);
    }

    openFile($fileName)
    {
        $.currentMovieDirectory = (new QFileInfo($fileName)).path();

        $.movie.stop();
        $.movieLabel.setMovie($.movie);
        $.movie.setFileName($fileName);
        $.movie.start();

        $.updateFrameSlider();
        $.updateButtons();
    }

    goToFrame($frame)
    {
        $.movie.jumpToFrame($frame);
    }

    fitToWindow()
    {
        $.movieLabel.setScaledContents($.fitCheckBox.isChecked());
    }

    updateFrameSlider()
    {
        my $hasFrames = ($.movie.currentFrameNumber() >= 0);

        if ($hasFrames) {
            if ($.movie.frameCount() > 0) {
                $.frameSlider.setMaximum($.movie.frameCount() - 1);
            } else {
                if ($.movie.currentFrameNumber() > $.frameSlider.maximum())
                    $.frameSlider.setMaximum($.movie.currentFrameNumber());
            }
            $.frameSlider.setValue($.movie.currentFrameNumber());
        } else {
            $.frameSlider.setMaximum(0);
        }
        $.frameLabel.setEnabled($hasFrames);
        $.frameSlider.setEnabled($hasFrames);
    }

    updateButtons()
    {
        $.playButton.setEnabled($.movie.isValid() && $.movie.frameCount() != 1 && $.movie.state() == QMovie::NotRunning);
        $.pauseButton.setEnabled($.movie.state() != QMovie::NotRunning);
        $.pauseButton.setChecked($.movie.state() == QMovie::Paused);
        $.stopButton.setEnabled($.movie.state() != QMovie::NotRunning);
    }

    createControls()
    {
        $.fitCheckBox = new QCheckBox($.tr("Fit to Window"));

        $.frameLabel = new QLabel($.tr("Current frame:"));

        $.frameSlider = new QSlider(Qt::Horizontal);
        $.frameSlider.setTickPosition(QSlider::TicksBelow);
        $.frameSlider.setTickInterval(10);

        $.speedLabel = new QLabel($.tr("Speed:"));

        $.speedSpinBox = new QSpinBox();
        $.speedSpinBox.setRange(1, 9999);
        $.speedSpinBox.setValue(100);
        $.speedSpinBox.setSuffix($.tr("%"));

        $.controlsLayout = new QGridLayout();
        $.controlsLayout.addWidget($.fitCheckBox, 0, 0, 1, 2);
        $.controlsLayout.addWidget($.frameLabel, 1, 0);
        $.controlsLayout.addWidget($.frameSlider, 1, 1, 1, 2);
        $.controlsLayout.addWidget($.speedLabel, 2, 0);
        $.controlsLayout.addWidget($.speedSpinBox, 2, 1);
    }

    createButtons()
    {
        my $iconSize = new QSize(36, 36);
        
        $.openButton = new QToolButton();
        $.openButton.setIcon(new QIcon($dir + "images/open.png"));
        $.openButton.setIconSize($iconSize);
        $.openButton.setToolTip($.tr("Open File"));
        $.connect($.openButton, SIGNAL("clicked()"), SLOT("open()"));

        $.playButton = new QToolButton();
        $.playButton.setIcon(new QIcon($dir + "images/play.png"));
        $.playButton.setIconSize($iconSize);
        $.playButton.setToolTip($.tr("Play"));
        $.movie.connect($.playButton, SIGNAL("clicked()"), SLOT("start()"));

        $.pauseButton = new QToolButton();
        $.pauseButton.setCheckable(True);
        $.pauseButton.setIcon(new QIcon($dir + "images/pause.png"));
        $.pauseButton.setIconSize($iconSize);
        $.pauseButton.setToolTip($.tr("Pause"));
        $.movie.connect($.pauseButton, SIGNAL("clicked(bool)"), SLOT("setPaused(bool)"));

        $.stopButton = new QToolButton();
        $.stopButton.setIcon(new QIcon($dir + "images/stop.png"));
        $.stopButton.setIconSize($iconSize);
        $.stopButton.setToolTip($.tr("Stop"));
        $.movie.connect($.stopButton, SIGNAL("clicked()"), SLOT("stop()"));
        
        $.quitButton = new QToolButton();
        $.quitButton.setIcon(new QIcon($dir + "images/quit.png"));
        $.quitButton.setIconSize($iconSize);
        $.quitButton.setToolTip($.tr("Quit"));
        $.connect($.quitButton, SIGNAL("clicked()"), SLOT("close()"));
        
        $.buttonsLayout = new QHBoxLayout();
        $.buttonsLayout.addStretch();
        $.buttonsLayout.addWidget($.openButton);
        $.buttonsLayout.addWidget($.playButton);
        $.buttonsLayout.addWidget($.pauseButton);
        $.buttonsLayout.addWidget($.stopButton);
        $.buttonsLayout.addWidget($.quitButton);
        $.buttonsLayout.addStretch();
    }
}


class movieplayer_example inherits QApplication
{
    constructor()
    {
        our $dir = get_script_dir();

        my $player = new MoviePlayer();
        $player.show();
        $.exec();
    }
}
