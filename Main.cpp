#include "ui/MainWindow.hpp"
#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication* a = new QApplication(argc, argv);

    /*QFile file(":/stylesheet/resources/stylesheet/stylesheet.qss");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString style = QLatin1String(file.readAll());
        a->setStyleSheet(style);
        file.close();
    }*/

	qRegisterMetaType<GraphicsImage>();

	MainWindow w;
	w.setStyleSheet("QPushButton { background-color: yellow }");
	//qApp->setStyleSheet("QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }");
    w.showMaximized();

    return a->exec();
}
