#include "ui/MainWindow.hpp"
#include "ui/ConnectionManager.hpp"
#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication* a = new QApplication(argc, argv);

    /*QFile file(":/stylesheet/resources/stylesheet/stylesheet.qss");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString style = QLatin1String(file.readAll());
        a->setStyleSheet(style);
        file.close();
    }*/

	qRegisterMetaType<cv::Mat>();

	ConnectionManager cm;
	//cm.show();
	MainWindow w;
    w.setStyleSheet("QPushButton { background-color: yellow }");
    w.showMaximized();

    return a->exec();
}
