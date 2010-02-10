#!/usr/bin/env qore

# This is basically a direct port of the QT tutorial to Qore 
# using Qore's "qt4" module.  

# use the "qt4" module
%requires qt4

# this is an object-oriented program, the application class is "qt_example"
%exec-class qt_example
# require all variables to be explicitly  declared
%require-our
# enable all parse warnings
%enable-all-warnings

class LCDRange inherits QWidget {
    public {
	QSlider $.slider = new QSlider(Qt::Horizontal);
	QLabel $.label = new QLabel();
    }
    constructor(string $text, $parent) : QWidget($parent) {
	my $lcd = new QLCDNumber(2);
	$lcd.setSegmentStyle(QLCDNumber::Filled);

	# signals must be declared before used - note that the signature should be c/c++ style
	$.createSignal("valueChanged(int)");

	$.slider.setRange(0, 99);
	$.slider.setValue(0);
	
	$.label.setAlignment(Qt::AlignHCenter | Qt::AlignTop);

	QObject::connect($.slider, SIGNAL("valueChanged(int)"), $lcd, SLOT("display(int)"));
	QObject::connect($.slider, SIGNAL("valueChanged(int)"), $self, SIGNAL("valueChanged(int)"));

	my QVBoxLayout $layout = new QVBoxLayout();
	$layout.addWidget($lcd);
	$layout.addWidget($.slider);
	$layout.addWidget($.label);
	$.setLayout($layout);
	$.setFocusProxy($.slider);
	$.setText($text);
    }

    value() {
	return $.slider.value();
    }

    setValue(int $val) {
	$.slider.setValue($val);
    }

    setRange(int $minValue, int $maxValue) {
	if ($minValue < 0 || $maxValue > 99 || $minValue > $maxValue) {
	    qWarning("LCDRange::setRange(%d, %d)
\tRange must be 0..99
\tand minValue must not be greater than maxValue",
		     $minValue, $maxValue);
	    return;
	}
	$.slider.setRange($minValue, $maxValue);
    }

    text() {
	return $.label.text();
    }

    setText(string $text) {
	$.label.setText($text);
    }
}

class CannonField inherits QWidget {
    private {
	int $.currentAngle = 45;
	int $.currentForce;
	int $.timerCount;
	QTimer $.autoShootTimer = new QTimer($self);
	int $.shootAngle; 
	int $.shootForce; 
	QPoint $.target = new QPoint(0, 0);
	bool $.gameEnded;
	bool $.barrelPressed;
    }

    constructor($parent) : QWidget($parent) {
	# declare dynamic signals
	$.createSignal("angleChanged(int)");
	$.createSignal("forceChanged(int)");
	$.createSignal("hit()");
	$.createSignal("missed()");
	$.createSignal("canShoot(bool)");

	$.autoShootTimer = new QTimer($self);
	$.connect($.autoShootTimer, SIGNAL("timeout()"), SLOT("moveShot()"));
	$.setPalette(new QPalette(new QColor(250, 250, 200)));
	$.setAutoFillBackground(True);
	$.newTarget();
    }

    angle() {
	return $.currentAngle;
    }

    force() {
	return $.currentForce;
    }

    gameOver() {
	return $.gameEnded;
    }

    newTarget() {
	if ($firstTime) {
	    $firstTime = False;
	    my date $now = now();
	    qsrand($now - get_midnight($now));
	}
	$.target = new QPoint(200 + qrand() % 190, 10 + qrand() % 255);
	$.update();
    }

    setGameOver() {
	if ($.gameEnded)
	    return;
	if ($.isShooting())
	    $.autoShootTimer.stop();
	$.gameEnded = True;
	$.update();
    }

    restartGame() {
	if ($.isShooting())
	    $.autoShootTimer.stop();
	$.gameEnded = False;
	$.update();
	$.emit("canShoot(bool)", True);
    }

    setAngle($angle) {
	#printf("CannonField::setAngle(%N) called\n", $angle);
	if ($angle < 5)
	    $angle = 5;
	else if ($angle > 70)
	    $angle = 70;
	if ($.currentAngle == $angle)
	    return;
	$.currentAngle = $angle;
	$.update($.cannonRect());
	$.emit("angleChanged(int)", $.currentAngle);
    }

    setForce($force) {
	if ($force < 0)
	    $force = 0;
	if ($.currentForce == $force)
	    return;
	$.currentForce = $force;
	$.emit("forceChanged(int)", $.currentForce);
    }

    paintEvent() {
	my QPainter $painter = new QPainter($self);

	if ($.gameEnded) {
	    $painter.setPen(Qt::black);
	    $painter.setFont(new QFont("Countier", 48, QFont::Bold));
	    $painter.drawText($.rect(), Qt::AlignCenter, $.tr("Game Over"));
	}

	$.paintCannon($painter);
	$.paintBarrier($painter);
	if ($.isShooting())
	    $.paintShot($painter);
	if (!$.gameEnded)
	    $.paintTarget($painter);
    }

    paintTarget(QPainter $painter) {
	$painter.setPen(Qt::black);
	$painter.setBrush(Qt::red);
	$painter.drawRect($.targetRect());
    }

    paintBarrier(QPainter $painter) {
	$painter.setPen(Qt::black);
	$painter.setBrush(Qt::yellow);
	$painter.drawRect($.barrierRect());
    }

    barrierRect() {
	return new QRect(145, $.height() - 100, 15, 99);
    }

    barrelHit($pos) {
	my QMatrix $matrix = new QMatrix();
	$matrix.translate(0.0, $.height());
	$matrix.rotate(-$.currentAngle);
	$matrix = $matrix.inverted();
	return $barrelRect.contains($matrix.map($pos));
    }

    paintCannon(QPainter $painter) {
	$painter.setPen(Qt::NoPen);
	$painter.setBrush(Qt::blue);

	$painter.save();
	$painter.translate(0.0, $.height());
	$painter.drawPie(new QRect(-35, -35, 70, 70), 0, 90 * 16);
	$painter.rotate(-$.currentAngle);
	$painter.drawRect($barrelRect);
	$painter.restore();
    }

    paintShot(QPainter $painter) {
	$painter.setPen(Qt::NoPen);
	$painter.setBrush(Qt::black);
	$painter.drawRect($.shotRect());
    }

    cannonRect() {
	my QRect $result = new QRect(0, 0, 50, 50);
	$result.moveBottomLeft($.rect().bottomLeft());
	return $result;
    }

    isShooting() {
	return $.autoShootTimer.isActive();
    }

    shoot() {
	if ($.isShooting())
	    return;
	$.timerCount = 0;
	$.shootAngle = $.currentAngle;
	$.shootForce = $.currentForce;
	$.autoShootTimer.start(5);
	# emit a signal - the arguments are given as qore expressions after the signal signature
	$.emit("canShoot(bool)", False);
    }

    moveShot() {
	my QRegion $region = $.shotRect();
	++$.timerCount;
	
	my QRect $shotR = $.shotRect();
	
	if ($shotR.intersects($.targetRect())) {
	    $.autoShootTimer.stop();
	    # emit a signal - arguments (if any) are given as qore expressions 
	    # after the signal signature
	    $.emit("hit()");
	    $.emit("canShoot(bool)", True);
	} else if ($shotR.x() > $.width() || $shotR.y
		   () > $.height() || $shotR.intersects($.barrierRect())) {
	    $.autoShootTimer.stop();
	    $.emit("missed()");
	    $.emit("canShoot(bool)", True);
	} else
	    $region = $region.united($shotR);

	$.update($region);
    }

    mousePressEvent($event) {
	if ($event.button() != Qt::LeftButton)
	    return;
	if ($.barrelHit($event.pos()))
	    $.barrelPressed = True;
    }

    mouseMoveEvent($event) {
	if (!$.barrelPressed)
	    return;
	my QPoint $pos = $event.pos();
	if ($pos.x() <= 0)
	    $pos.setX(1);
	if ($pos.y
	    () >= $.height())
	    $pos.setY($.height() - 1);
	my float $rad = atan((float($.rect().bottom()) - $pos.y
			   ()) / $pos.x());
	$.setAngle(qRound($rad * 180 / 3.14159265));
    }

    mouseReleaseEvent($event) {
	printf("mouseReleaseEvent() arg=%N\n", $event);
	if ($event.button() == Qt::LeftButton)
	    $.barrelPressed = False;
    }

    shotRect() {
	my float $gravity = 4.0;

	my float $time = $.timerCount / 20.0;
	my float $velocity = $.shootForce;
	my float $radians = $.shootAngle * 3.14159265 / 180;

	my float $velx = $velocity * cos($radians);
	my float $vely = $velocity * sin($radians);
	my float $x0 = ($barrelRect.right() + 5) * cos($radians);
	my float $y0 = ($barrelRect.right() + 5) * sin($radians);
	my float $x = $x0 + $velx * $time;
	my float $y = $y0 + $vely * $time - 0.5 * $gravity * $time * $time;

	my QRect $result = new QRect(0, 0, 6, 6);
	$result.moveCenter(new QPoint(qRound($x), $.height() - 1 - qRound($y)));
	return $result;
    }

    targetRect() {
	my QRect $result = new QRect(0, 0, 20, 10);
	$result.moveCenter(new QPoint($.target.x(), $.height() - 1 - $.target.y
				      ()));
	return $result;
    }

    sizeHint() {
	return new QSize(400, 300);
    }
}

class GameBoard inherits QWidget {
    private {
	CannonField $.cannonField = new CannonField();
	QLCDNumber $.hits;
	QLCDNumber $.shotsLeft;
    }
    constructor($parent) : QWidget($parent) {
	my QPushButton $quit = new QPushButton($.tr("&Quit"));
	$quit.setFont(new QFont("Times", 18, QFont::Bold));

	QObject::connect($quit, SIGNAL("clicked()"), qApp(), SLOT("quit()"));

	my LCDRange $angle = new LCDRange($.tr("ANGLE"));
	$angle.setRange(5, 70);

	my LCDRange $force = new LCDRange($.tr("FORCE"));
	$force.setRange(10, 50);

	my QFrame $cannonBox = new QFrame();
	$cannonBox.setFrameStyle(QFrame::WinPanel | QFrame::Sunken);

	#$.cannonField = new CannonField();

	QObject::connect($angle,       SIGNAL("valueChanged(int)"), $.cannonField, SLOT("setAngle(int)"));
	QObject::connect($.cannonField, SIGNAL("angleChanged(int)"), $angle,       SLOT("setValue(int)"));

	$.cannonField.connect($force, SIGNAL("valueChanged(int)"), SLOT("setForce(int)"));
	$force.connect($.cannonField, SIGNAL("forceChanged(int)"), SLOT("setValue(int)"));
	
	$.connect($.cannonField, SIGNAL("hit()"),    SLOT("hit()"));
	$.connect($.cannonField, SIGNAL("missed()"), SLOT("missed()"));

	my QPushButton $shoot = new QPushButton($.tr("&Shoot"));
	$shoot.setFont(new QFont("Times", 18, QFont::Bold));

	$.connect($shoot, SIGNAL("clicked()"), SLOT("fire()"));
	$shoot.connect($.cannonField, SIGNAL("canShoot(bool)"), SLOT("setEnabled(bool)"));

	my QPushButton $restart = new QPushButton($.tr("&New Game"));
	$restart.setFont(new QFont("Times", 18, QFont::Bold));

	$.connect($restart, SIGNAL("clicked()"), SLOT("newGame()"));

	$.hits = new QLCDNumber(2);
	$.hits.setSegmentStyle(QLCDNumber::Filled);

	$.shotsLeft = new QLCDNumber(2);
	$.shotsLeft.setSegmentStyle(QLCDNumber::Filled);

	my QLabel $hitsLabel = new QLabel($.tr("HITS"));
	my QLabel $shotsLeftLabel = new QLabel($.tr("SHOTS LEFT"));

        #new QShortcut(Qt::Key_Enter, $self, SLOT("fire()"));
	#new QShortcut(Qt::Key_Return, $self, SLOT("fire()"));
	#new QShortcut(Qt::CTRL | Qt::Key_Q, $self, SLOT("close()"));
	#new QShortcut(new QKeySequence(Qt::Key_Enter), $self, SLOT("fire()"));
	#new QShortcut(new QKeySequence(Qt::Key_Return), $self, SLOT("fire()"));
	#printf("Qt::CTRL + Qt::Key_Q = %N\n", Qt::CTRL + Qt::Key_Q);
	#new QShortcut(new QKeySequence(Qt::CTRL | Qt::Key_Q), $self, SLOT("close()"));

	my QHBoxLayout $topLayout = new QHBoxLayout();
	$topLayout.addWidget($shoot);
	$topLayout.addWidget($.hits);
	$topLayout.addWidget($hitsLabel);
	$topLayout.addWidget($.shotsLeft);
	$topLayout.addWidget($shotsLeftLabel);
	$topLayout.addStretch(1);
	$topLayout.addWidget($restart);

	my QVBoxLayout $leftLayout = new QVBoxLayout();
	$leftLayout.addWidget($angle);
	$leftLayout.addWidget($force);

	my QVBoxLayout $cannonLayout = new QVBoxLayout();
	$cannonLayout.addWidget($.cannonField);
	$cannonBox.setLayout($cannonLayout);

	my QGridLayout $gridLayout = new QGridLayout();
	$gridLayout.addWidget($quit, 0, 0);
	$gridLayout.addLayout($topLayout, 0, 1);
	$gridLayout.addLayout($leftLayout, 1, 0);
	$gridLayout.addWidget($cannonBox, 1, 1, 2, 1);
	$gridLayout.setColumnStretch(1, 10);
	$.setLayout($gridLayout);

	$angle.setValue(60);
	$force.setValue(25);
	$angle.setFocus();

	$.newGame();
    }

    fire() {
	if ($.cannonField.gameOver() || $.cannonField.isShooting())
	    return;
	$.shotsLeft.display($.shotsLeft.intValue() - 1);
	$.cannonField.shoot();
    }

    hit() {
	$.hits.display($.hits.intValue() + 1);
	if ($.shotsLeft.intValue() == 0)
	    $.cannonField.setGameOver();
	else
	    $.cannonField.newTarget();
    }

    missed() {
	if ($.shotsLeft.intValue() == 0)
	    $.cannonField.setGameOver();
    }

    newGame() {
	$.shotsLeft.display(15);
	$.hits.display(0);
	$.cannonField.restartGame();
	$.cannonField.newTarget();
    }
}

class qt_example inherits QApplication {
    constructor() {
	# in qore programs using "exec-class", global variables must be
	# initialized in the application's constructor
	our bool $firstTime = True;
	our QRect $barrelRect = new QRect(30, -5, 20, 10);

	my GameBoard $board = new GameBoard();
	$board.setGeometry(100, 100, 500, 355);
	$board.show();

	$.exec();
    }
}
