#!/usr/bin/env qore

# This is basically a direct port of the QT widget example
# "analogclock" to Qore using Qore's "qt4" module.  

# use the "qt4" module
%requires qt4

# this is an object-oriented program, the application class is "analog_clock_example"
%exec-class analog_clock_example
# require all variables to be explicitly  declared
%require-our
# enable all parse warnings
%enable-all-warnings

class AnalogClock inherits QWidget {
    constructor(any $parent) : QWidget($parent) {
        my QTimer $timer($self);
        $.connect($timer, SIGNAL("timeout()"), SLOT("update()"));
        $timer.start(1000);
        
        $.setWindowTitle($.tr("Analog Clock"));
        $.resize(200, 200);
    }

    paintEvent(QPaintEvent $event) {        
        my int $side = min($.width(), $.height());
        my date $time = now();

        my QPainter $painter = new QPainter($self);

        $painter.setRenderHint(QPainter::Antialiasing);
        $painter.translate($.width() / 2, $.height() / 2);
        $painter.scale($side / 200.0, $side / 200.0);        
        $painter.setPen(Qt::NoPen);
        $painter.setBrush(new QBrush($hourColor));
        
        $painter.save();        
        $painter.rotate(30.0 * ((get_hours($time) + get_minutes($time) / 60.0)));
        $painter.drawConvexPolygon($hourHand);
        $painter.restore();
        
        $painter.setPen($hourColor);
        
        for (my int $i = 0; $i < 12; ++$i) {
            $painter.drawLine(88, 0, 96, 0);
            $painter.rotate(30.0);
        }
        
        $painter.setPen(Qt::NoPen);
        $painter.setBrush(new QBrush($minuteColor));
        
        $painter.save();
        $painter.rotate(6.0 * (get_minutes($time) + get_seconds($time) / 60.0));
        $painter.drawConvexPolygon($minuteHand);
        $painter.restore();
        
        $painter.setPen($minuteColor);
        
        for (my int $j = 0; $j < 60; ++$j) {
            if (($j % 5) != 0)
                $painter.drawLine(92, 0, 96, 0);
            $painter.rotate(6.0);
        }
    }
}

class analog_clock_example inherits QApplication {
    constructor() {
        # declare global variables
        our QPolygon $hourHand((new QPoint(7, 8), 
				new QPoint(-7, 8), 
				new QPoint(0, -40)));
        our QPolygon $minuteHand((new QPoint(7, 8),
				  new QPoint(-7, 8),
				  new QPoint(0, -70)));

        our QColor $hourColor(127, 0, 127);
        our QColor $minuteColor(0, 127, 127, 191);

        my AnalogClock $clock();
        $clock.show();
        $.exec();
    }
}
