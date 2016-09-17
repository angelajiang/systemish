#include <QApplication>
#include <QLabel>
#include <QWidget>

int main(int argc, char *argv[ ])
{
	QApplication app(argc, argv);
	QLabel hello("<center>Hello world</center>");
	hello.resize(400, 400);
	hello.show();
	return app.exec();
}
