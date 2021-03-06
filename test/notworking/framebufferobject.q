#!/usr/bin/env qore

# This is basically a direct port of a QT example program to Qore
# using Qore's "qt4" module.

# Note that Qore's "qt4" module requires QT 4.3 or above with OpenGL support

# use the "qt4" module
%requires qt4
# use the "opengl" module
%requires opengl

# this is an object-oriented program, the application class is "framebufferobject"
%exec-class framebufferobject
# require all variables to be explicitly declared
%require-our
# enable all parse warnings
%enable-all-warnings

const AMP = 5;

class GLWidget inherits QGLWidget {
    private {
	QPoint $.anchor(); 
	float $.scale = 0.1;
	float $.rot_x = 0.0; 
	float $.rot_y = 0.0; 
	float $.rot_z = 0.0;
	any $.tile_list;
	binary $.wave;
	QImage $.logo;
	QTimeLine $.anim; 
	QSvgRenderer $.svg_renderer;
	QGLFramebufferObject $.fbo;
	#QGLFramebufferObject $.texture_fbo;
	bool $.scale_in = True;
    }
    # no public members
    public {}

    constructor() : QGLWidget(new QGLFormat(QGL::SampleBuffers|QGL::AlphaChannel)) {
        $.setWindowTitle($.tr("OpenGL framebuffer objects"));
        $.makeCurrent();
        $.fbo = new QGLFramebufferObject(512, 512);
        $.anim = new QTimeLine(750, $self);
        $.anim.setUpdateInterval(20);
        $.connect($.anim, SIGNAL("valueChanged(qreal)"), SLOT("animate(qreal)"));
        $.connect($.anim, SIGNAL("finished()"), SLOT("animFinished()"));

        $.svg_renderer = new QSvgRenderer($dir + "images/bubbles.svg", $self);
        #printf("svg_renderer valid=%N\n", $.svg_renderer.isValid());
        $.connect($.svg_renderer, SIGNAL("repaintNeeded()"), SLOT("draw()"));

        $.logo = new QImage($dir + "images/qt4-logo.png");
        #printf("logo valid=%N\n", !$.logo.isNull());
        $.logo = $.logo.convertToFormat(QImage::Format_ARGB32);
        #printf("logo valid=%N\n", !$.logo.isNull());

        $.tile_list = glGenLists(1);

        glNewList($.tile_list, GL_COMPILE);
        glBegin(GL_QUADS);
        {
            glTexCoord2f(0.0, 0.0); glVertex3f(-1.0, -1.0,  1.0);
            glTexCoord2f(1.0, 0.0); glVertex3f( 1.0, -1.0,  1.0);
            glTexCoord2f(1.0, 1.0); glVertex3f( 1.0,  1.0,  1.0);
            glTexCoord2f(0.0, 1.0); glVertex3f(-1.0,  1.0,  1.0);

            glTexCoord2f(1.0, 0.0); glVertex3f(-1.0, -1.0, -1.0);
            glTexCoord2f(1.0, 1.0); glVertex3f(-1.0,  1.0, -1.0);
            glTexCoord2f(0.0, 1.0); glVertex3f( 1.0,  1.0, -1.0);
            glTexCoord2f(0.0, 0.0); glVertex3f( 1.0, -1.0, -1.0);

            glTexCoord2f(0.0, 1.0); glVertex3f(-1.0,  1.0, -1.0);
            glTexCoord2f(0.0, 0.0); glVertex3f(-1.0,  1.0,  1.0);
            glTexCoord2f(1.0, 0.0); glVertex3f( 1.0,  1.0,  1.0);
            glTexCoord2f(1.0, 1.0); glVertex3f( 1.0,  1.0, -1.0);

            glTexCoord2f(1.0, 1.0); glVertex3f(-1.0, -1.0, -1.0);
            glTexCoord2f(0.0, 1.0); glVertex3f( 1.0, -1.0, -1.0);
            glTexCoord2f(0.0, 0.0); glVertex3f( 1.0, -1.0,  1.0);
            glTexCoord2f(1.0, 0.0); glVertex3f(-1.0, -1.0,  1.0);

            glTexCoord2f(1.0, 0.0); glVertex3f( 1.0, -1.0, -1.0);
            glTexCoord2f(1.0, 1.0); glVertex3f( 1.0,  1.0, -1.0);
            glTexCoord2f(0.0, 1.0); glVertex3f( 1.0,  1.0,  1.0);
            glTexCoord2f(0.0, 0.0); glVertex3f( 1.0, -1.0,  1.0);

            glTexCoord2f(0.0, 0.0); glVertex3f(-1.0, -1.0, -1.0);
            glTexCoord2f(1.0, 0.0); glVertex3f(-1.0, -1.0,  1.0);
            glTexCoord2f(1.0, 1.0); glVertex3f(-1.0,  1.0,  1.0);
            glTexCoord2f(0.0, 1.0); glVertex3f(-1.0,  1.0, -1.0);
        }
        glEnd();
        glEndList();

        my int $size = $.logo.width() * $.logo.height();
        my string $str;
        for (my $i = 0; $i < $size; ++$i)
            $str += chr(0);

        $.wave = binary($str);

        $.startTimer(30); # $.wave timer
    }

    destructor() {
        glDeleteLists($.tile_list, 1);
    }

    paintEvent() {
        $.draw();
    }

    draw() {
        my QPainter $p($self); # used for text overlay
        
        # save the GL state set for QPainter
        $.saveGLState();

        # render the 'bubbles.svg' file into our framebuffer object
        my QPainter $fbo_painter($.fbo);
        $.svg_renderer.render($fbo_painter);
        $fbo_painter.end();

        # draw into the GL widget
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glFrustum(-1, 1, -1, 1, 10, 100);
        glTranslatef(0.0, 0.0, -15.0);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glViewport(0, 0, $.width(), $.height());
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glBindTexture(GL_TEXTURE_2D, $.fbo.texture());
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_MULTISAMPLE);
        glEnable(GL_CULL_FACE);

        # draw background
        glPushMatrix();
        glScalef(1.7, 1.7, 1.7);
        glColor4f(1.0, 1.0, 1.0, 1.0);
        glCallList($.tile_list);
        glPopMatrix();

        my int $w = $.logo.width();
        my int $h = $.logo.height();
        #printf("logo h=%n, w=%n\n", $h, $w);

        glRotatef($.rot_x, 1.0, 0.0, 0.0);
        glRotatef($.rot_y, 0.0, 1.0, 0.0);
        glRotatef($.rot_z, 0.0, 0.0, 1.0);
        glScalef($.scale/$w, $.scale/$w, $.scale/$w);

        glDepthFunc(GL_LESS);
        glEnable(GL_DEPTH_TEST);
        # draw the Qt icon
        glTranslatef(-$w+1, -$h+1, 0.0);
        for (my int $y=$h-1; $y>=0; --$y) {
            my $line = $.logo.scanLine($y);
            #printf("y=%n line=%n (%s)\n", $y, $line, makeBase64String($line));
            my int $end = elements $line / 4;
            my $x = 0;
            while ($x < $end) {
                my int $word = get_word_32($line, $x);
                #printf("line=%N (%d), x=%n word=%n wave[%d]=%n\n", $line, elements $line, $x, $word, $y*$w+$x, $.wave[$y*$w+$x]);
                glColor4ub(QGlobalSpace::qRed($word), QGlobalSpace::qGreen($word), QGlobalSpace::qBlue($word), QGlobalSpace::qAlpha($word)* 0.9);
                glTranslatef(0.0, 0.0, $.wave[$y*$w+$x]);
                if (QGlobalSpace::qAlpha($word) > 128)
                    glCallList($.tile_list);
                glTranslatef(0.0, 0.0, -$.wave[$y*$w+$x]);
                glTranslatef(2.0, 0.0, 0.0);
                ++$x;
            }
            glTranslatef(-$w * 2.0, 2.0, 0.0);
        }
        # restore the GL state that QPainter expects
        $.restoreGLState();

        # draw the overlayed text using QPainter
        $p.setPen(new QColor(197, 197, 197, 157));
        $p.setBrush(new QColor(197, 197, 197, 127));
        $p.drawRect(new QRect(0, 0, $.width(), 50));
        $p.setPen(Qt::black);
        $p.setBrush(Qt::NoBrush);
        my $str1 = $.tr("A simple OpenGL framebuffer object example.");
        my $str2 = $.tr("Use the mouse wheel to zoom, press buttons and move mouse to rotate, double-click to flip.");
        my QFontMetrics $fm($p.font());
        $p.drawText($.width()/2 - $fm.width($str1)/2, 20, $str1);
        $p.drawText($.width()/2 - $fm.width($str2)/2, 20 + $fm.lineSpacing(), $str2);
        #$.show();
    }

    mousePressEvent(QMouseEvent $e) {
        $.anchor = $e.pos();
    }

    mouseMoveEvent(QMouseEvent $e) {
	my int $diffx = $e.pos().x() - $.anchor.x();
	my int $diffy = $e.pos().y() - $.anchor.y();

        if ($e.buttons() & Qt::LeftButton) {
            $.rot_x += $diffy/5.0;
            $.rot_y += $diffx/5.0;
        } else if ($e.buttons() & Qt::RightButton) {
            $.rot_z += $diffx/5.0;
        }

        $.anchor = $e.pos();
        $.draw();
    }

    wheelEvent(QWheelEvent $e) {
        if ($e.delta() > 0)
            $.scale += $.scale * 0.1;
        else
            $.scale -= $.scale * 0.1;
        $.draw();
    }

    mouseDoubleClickEvent() {
        $.anim.start();
    }

    animate($val) {
        $.rot_y = $val * 180;
        $.draw();
    }

    animFinished() {
        if ($.anim.direction() == QTimeLine::Forward)
            $.anim.setDirection(QTimeLine::Backward);
        else
            $.anim.setDirection(QTimeLine::Forward);
    }

    saveGLState() {
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
    }

    restoreGLState() {
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
        glPopAttrib();
    }

    timerEvent() {
        if (QApplication::mouseButtons() != 0)
            return;

        if ($.scale_in && $.scale > 35.0)
            $.scale_in = False;
        else if (!$.scale_in && $.scale < 0.5)
            $.scale_in = True;

        $.scale = $.scale_in ? $.scale + $.scale * 0.01 : $.scale-$.scale * 0.01;
        $.rot_z += 0.3;
        $.rot_x += 0.1;

        my (int $dx, int $dy); # disturbance point
        my (float $s, float $v, float $W, float $t);
        our list $wt;
        my int $width = $.logo.width();

        $dx = $dy = $width >> 1;

        $W = 0.3;
        $v = -4; # wave speed

        for (my int $i = 0; $i < $width; ++$i) {
            for (my int $j = 0; $j < $width; ++$j) {
                $s = sqrt((($j - $dx) * ($j - $dx) + ($i - $dy) * ($i - $dy)));
                $wt[$i][$j] += 0.1;
                $t = $s / $v;
                if ($s != 0)
                    $.wave[$i*$width + $j] = AMP * sin(2 * M_PI * $W * ($wt[$i][$j] + $t)) / (0.2*($s + 2));
                else
                    $.wave[$i*$width + $j] = AMP * sin(2 * M_PI * $W * ($wt[$i][$j] + $t));
            }
        }
    }
}

class framebufferobject inherits QApplication {
    constructor() {
        our string $dir = get_script_dir();

        if (!QGLFormat::hasOpenGL()) {
            QMessageBox::information(NOTHING, "OpenGL framebuffer objects",
                                     "this system does not support OpenGL");
            return;
        }
        if (!QGLFramebufferObject::hasOpenGLFramebufferObjects()) {
            QMessageBox::information(NOTHING, "OpenGL framebuffer objects",
                                     "this system does not support framebuffer objects.");
            return;
        }

        my GLWidget $widget();
        $widget.resize(640, 480);
        $widget.show();
        $.exec();
    }
}
