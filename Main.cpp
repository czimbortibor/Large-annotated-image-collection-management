#include "forms/MainWindow.hpp"
#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication* a = new QApplication(argc, argv);

    QFile file(":/stylesheet/resources/stylesheet/stylesheet.qss");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        a->setStyleSheet(file.readAll());
        file.close();
    }

    MainWindow w;
	w.showMaximized();

    return a->exec();
}
