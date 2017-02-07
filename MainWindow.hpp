#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <string>
#include <fstream>
#include <memory>
#include <functional>

#include <QMainWindow>
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

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include "LayoutItem.hpp"
#include "FlowLayout.hpp"
#include "RingLayout.hpp"
#include "CBIR.hpp"

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
	cv::Mat loadImage(const QString &fileName) const;
	void resizeImages(int size);
	QImage resizeImage(const cv::Mat& image, int newWidth, int newHeight) const;
	void displayImages();

	void logTime(QString message);
	QImage Mat2QImage(const cv::Mat &cvImage) const;


	Ui::MainWindow* ui;
	QFrame _frame;
	int _iconSize;
	int _nrOfImages;
	QDir _dir;
	QDir _dirSmallImg;
	//QList<QString> _imageNames;
	std::unique_ptr<QList<QString>> _imageNames;
	int _imgWidth;
	int _imgHeight;
	std::unique_ptr<QVector<cv::Mat>> _imagesOriginal;
	std::unique_ptr<QVector<QImage>> _imagesResized;
	QElapsedTimer _timer;

	// ------ single-thread image load -------
	QFuture<void> _futureLoader;
	QFutureWatcher<void> _futureLoaderWatcher;
	// ------ multi-threaded image load -------
	QFuture<cv::Mat> _futureLoaderMT;
	QFutureWatcher<cv::Mat> _futureLoaderWatcherMT;

	// ------ multi-threaded image resize ------
	QFuture<QImage> _futureResizerMT;
	QFutureWatcher<QImage> _futureResizerWatcherMT;

	FlowLayout* _layout;
	QGraphicsWidget* _layoutWidget;
	QGraphicsView* _view;
	QGraphicsScene* _scene;

	CBIR imageRetrieval;
	std::unique_ptr<std::multimap<double, const cv::Mat>> _imagesHashed;

private slots:
	void onSceneChanged();
	void onClearLayout();

    void onImageReceive(int resultInd);
    void onImagesReceive(int resultsBeginInd, int resultsEndInd);
	void onFinishedLoading();
	void onImagesResized(int resultsBeginInd, int resultsEndInd);
	void onFinishedResizing();

	void onLoadImagesClick();
	void onSaveImagesClick();
	void onReverseButtonClick();
	void onRadiusChanged(double value);
	void onPetalNrChanged(int value);

signals:
	void clearLayout();
};

#endif // MAINWINDOW_H
