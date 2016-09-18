#include <QtGui>

#include "rasterwindow.h"

class AnalogClockWindow : public RasterWindow
{
public:
	AnalogClockWindow(bool go_up) : go_up(go_up) {
		/* Set title */
		QString title_string;
		title_string.sprintf("Analog clock %d", go_up);
		setTitle(title_string);

		/* Add a button */
		QWidget *wdg = new QWidget(NULL, 0);
		QPushButton *train_button = new QPushButton(wdg);
		train_button->setText(tr("something"));

		resize(500, 300);

		m_timerId = startTimer(20);
	}

protected:
    void timerEvent(QTimerEvent *) Q_DECL_OVERRIDE;
    void render(QPainter *p) Q_DECL_OVERRIDE;

private:
	bool go_up;
    int m_timerId;
};

void AnalogClockWindow::timerEvent(QTimerEvent *event)
{
	if (event->timerId() == m_timerId) {
		renderLater();
	}
}

void AnalogClockWindow::render(QPainter *p)
{
	p->setRenderHint(QPainter::Antialiasing);
	p->translate(width() / 2, height() / 2);

	int side = qMin(width(), height());
	p->scale(side / 200.0, side / 200.0);

	//->setPen(Qt::NoPen);
	//p->setBrush();

	QTime time = QTime::currentTime();
	int ms = time.msec();

	if(go_up) {
		p->drawRect(QRect(0, - 1 * ms / 20, 5, 5));
	} else {
		p->drawRect(QRect(0, ms / 20, 5, 5));
	}
}

int main(int argc, char **argv)
{
	QGuiApplication app(argc, argv);

	AnalogClockWindow clock(true);
	clock.show();

	AnalogClockWindow clock2(false);
	clock2.show();

	return app.exec();
}
