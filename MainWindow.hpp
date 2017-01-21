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
#include "RingLayout.hpp"

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

    static QImage loadImage(const QString &fileName);
    static QImage Mat2QImage(const cv::Mat &cvImage);

	void loadImages();
	void clearLayout(QGraphicsLayout* layout);
	void logTime(QString message);

	static QDir DIR;
	static QDir SMALLIMGDIR;
	static int IMGWIDTH;
	static int IMGHEIGHT;

	int _iconSize;
	int _nrOfImages = 0;
	QList<QString> _imageNames;
	QVector<QImage> _images;
	//QScopedPointer<RingLayout> layout;
	QSharedPointer<RingLayout> _layout;
	QFrame _frame;
	QElapsedTimer _timer;
	QFuture<QImage> _futureResult;
	QSharedPointer<FutureWatcher> _watcher;
	QGraphicsView* _view;
	QWidget* _viewPort;
	QGraphicsScene* _scene;
	QGraphicsWidget* _form;

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
