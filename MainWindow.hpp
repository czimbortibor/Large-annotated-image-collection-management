#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <string>
#include <fstream>
#include <memory>

#include <QMainWindow>
#include <QScopedPointer>
#include <QLayout>
#include <QFileDialog>
#include <QDesktopWidget>
#include <QVector>
#include <QFrame>
#include <QThread>
#include <QLabel>
#include <QPushButton>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent>
#include <QElapsedTimer>
#include <QDebug>
#include <QGraphicsWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsLinearLayout>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include "LayoutItem.hpp"
#include "FlowLayout.hpp"
#include "RingLayout.hpp"
#include "LayoutCanvas.hpp"

typedef QFutureWatcher<QImage> FutureWatcher;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
	explicit MainWindow(QWidget* parent = 0);
	MainWindow(MainWindow const& otherWindow) = delete;
	MainWindow& operator=(MainWindow const& otherWindow) = delete;
    ~MainWindow();

private:
	void initScene();
	void loadImages();
	void clearLayout();
	void displayImages();
	void logTime(QString message);

	static QImage loadImage(const QString &fileName);
	static QImage Mat2QImage(const cv::Mat &cvImage);


	Ui::MainWindow* ui;
	int _iconSize;
	int _nrOfImages = 0;
	QList<QString> _imageNames;
	std::unique_ptr<QVector<QImage>> _images;
	QFrame _frame;
	QElapsedTimer _timer;
	QFuture<QImage> _futureResult;
	std::unique_ptr<FutureWatcher> _watcher;
	QFuture<void> _future;
	QFutureWatcher<void> _futureWatcher;
	RingLayout* _layout;
	QGraphicsView* _view;
	QGraphicsScene* _scene;
	QGraphicsWidget* _layoutWidget;

	static QDir DIR;
	static QDir SMALLIMGDIR;
	static int IMGWIDTH;
	static int IMGHEIGHT;

private slots:
    void onImageReceive(int resultInd);
    void onImagesReceive(int resultsBeginInd, int resultsEndInd);
    void onFinished();

	void onSceneChanged();

	void onLoadImagesClick();
	void onSaveImagesClick();
	void onReverseButtonClick();
	void onRadiusChanged(double value);
	void onPetalNrChanged(int value);

signals:
    void start();
};

#endif // MAINWINDOW_H
