#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <string>
#include <fstream>
#include <memory>
#include <functional>
#include <numeric>

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
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
#include <QListWidget>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include "util/LayoutItem.hpp"
#include "util/CBIR.hpp"
#include "views/GraphicsView.hpp"
#include "layouts/FlowLayout.hpp"
#include "layouts/PetalLayout.hpp"
#include "db/MongoAccess.hpp"

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
	void init();
	/** creates the main view for displaying images */
	void initView();

	void loadImages();
	/** no images were selected */
	void showAlertDialog();
	cv::Mat loadImage(const QString &fileName) const;
	void resizeImages(int size);
	cv::Mat resizeImage(const cv::Mat& image, int newWidth, int newHeight) const;
	void displayImages(const QVector<cv::Mat>& images) const;
	/** opencv_img_hash & pHash display */
	template<typename T> void displayImages(const T& images) const;

	void logTime(QString message);
	QImage Mat2QImage(const cv::Mat &cvImage) const;


	Ui::MainWindow* ui;
	int _iconSize;
	int _nrOfImages;
	QDir _dir;
	QDir _dirSmallImg;
	QStringList _supportedImgFormats;
	std::unique_ptr<QList<QString>> _imageNames;
	int _imgWidth;
	int _imgHeight;
	std::unique_ptr<QVector<cv::Mat>> _imagesOriginal;
	std::unique_ptr<QVector<cv::Mat>> _imagesResized;
	QElapsedTimer _timer;

	// ------ single-thread image load -------
	std::unique_ptr<QFuture<void>> _futureLoader;
	QFutureWatcher<void> _futureLoaderWatcher;
	// ------ multi-threaded image load -------
	std::unique_ptr<QFuture<cv::Mat>> _futureLoaderMT;
	std::unique_ptr<QFutureWatcher<cv::Mat>> _futureLoaderWatcherMT;

	// ------ multi-threaded image resize ------
	std::unique_ptr<QFuture<cv::Mat>> _futureResizerMT;
	QFutureWatcher<cv::Mat> _futureResizerWatcherMT;

	GraphicsView* _view;

	CBIR imageRetrieval;
	//std::unique_ptr<std::multimap<const cv::Mat, const cv::Mat, CBIR::MatCompare>> _imagesHashed;
	std::unique_ptr<std::multimap<double, const cv::Mat>> _imagesHashed;
	std::unique_ptr<std::multimap<ulong64, const cv::Mat, CBIR::HashCompare>> _imagesHashed_pHash;

	std::unique_ptr<MongoAccess> _mongoAccess;

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
	/** display the imges in reverse order */
	void onReverseButtonClick();

	// ------------- filters ---------------
	void onRadiusChanged(double value);
	void onPetalNrChanged(int value);
	// TODO: factory method to create the different layouts
	void onLayoutChanged(const QString& text);

	void onFiltersClicked();

signals:
	void clearLayout();
};

#endif // MAINWINDOW_H
