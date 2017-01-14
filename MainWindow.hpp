#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSharedPointer>
#include <QScopedPointer>
#include <QLayout>
#include <QFileDialog>
#include <QVector>
#include <QFrame>
#include <QThread>
#include <QLabel>
#include <QPushButton>
#include <QDesktopWidget>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent>
#include <QElapsedTimer>
#include <QDebug>
#include <string>
#include <fstream>
#include <QGraphicsWidget>
#include <QGraphicsProxyWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsLinearLayout>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include "LayoutItem.hpp"
#include "FlowLayout.hpp"

typedef QFutureWatcher<QImage> FutureWatcher;
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
	Ui::MainWindow* ui;

	const int iconSize = 10;
	static QDir dir;
    static QDir smallImagesDir;
    int nrOfImages = 0;
	QList<QString> imageNames;
    QVector<QImage> images;
	QSharedPointer<FlowLayout> layout;
    QFrame frame;
	static int imageWidth;
	static int imageHeight;
	QElapsedTimer timer;

    QFuture<QImage> futureResult;
	QSharedPointer<FutureWatcher> watcher;
    QGraphicsView* view;
    QWidget* viewPort;
    QGraphicsScene* scene;
    QGraphicsWidget* form;

    static QImage loadImage(const QString &fileName);
    static QImage Mat2QImage(const cv::Mat &cvImage);

	void clearLayout(QGraphicsLayout* layout);
	void logTime(QString message);

private slots:
	void onLoadImagesClick();
	void onSaveImagesClick();
    void onImageReceive(int resultInd);
    void onImagesReceive(int resultsBeginInd, int resultsEndInd);
	void onReverseButtonClick();
    void onFinished();

signals:
    void start();
};

#endif // MAINWINDOW_H
