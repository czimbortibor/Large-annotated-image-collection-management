#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <string>
#include <fstream>
#include <memory>
#include <functional>
#include <numeric>
#include <iterator>

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
#include <QProgressBar>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include <libconfig.h++>

#include "utils/LayoutItem.hpp"
#include "utils/CBIR.hpp"
#include "utils/ImageLoader.hpp"
#include "utils/ImageConverter.hpp"
#include "view/GraphicsView.hpp"
#include "layouts/FlowLayout.hpp"
#include "layouts/PetalLayout.hpp"
#include "db/MongoAccess.hpp"
#include "filters/DateFilter.hpp"

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
    /* creates the main view for displaying images */
	void initView();

    //void resizeImages(int newWidth, int newHeight);
    void saveImages(int size);

    /* no images were selected */
    void showAlertDialog() const;
    cv::Mat loadImage(const QString& fileName) const;
    cv::Mat resizeImage(const cv::Mat& image, int newWidth, int newHeight) const;
    void displayImages(const QList<cv::Mat>& images) const;
    /* opencv_img_hash & pHash display */
	template<typename T> void displayImages(const T& images) const;

	void logTime(QString message);

    /* returns the similar images of the target image */
    QList<cv::Mat>& getSimilarImages(const LayoutItem& target) const;


	Ui::MainWindow* ui;
	int _iconSize;
	int _nrOfImages;
	QDir _dir;
    QDir* _dirSmallImg;
	QStringList _supportedImgFormats;
	std::unique_ptr<QList<QString>> _imageNames;
	int _imgWidth;
	int _imgHeight;
    std::unique_ptr<QList<cv::Mat>> _images;
	QElapsedTimer _timer;
    int _notifyRate;
    std::unique_ptr<QProgressBar> _progressBar;
    /* were the images already hashed? */
    bool _wereHashed = false;

    std::unique_ptr<libconfig::Config> _config;
    std::unique_ptr<libconfig::Setting> _collections;

	// ------ multi-threaded image load -------
	std::unique_ptr<QFuture<cv::Mat>> _futureLoaderMT;
	std::unique_ptr<QFutureWatcher<cv::Mat>> _futureLoaderWatcherMT;

    // -------- custom single-threaded image load -----------
    std::unique_ptr<ImageLoader> _loadingWorker;

	// ------ multi-threaded image resize ------
    std::shared_ptr<QFuture<cv::Mat>> _futureResizerMT;
    std::shared_ptr<QFutureWatcher<cv::Mat>> _futureResizerWatcherMT;

	GraphicsView* _view;

	CBIR imageRetrieval;
    //std::unique_ptr<std::multimap<const cv::Mat, const cv::Mat, CBIR::MatCompare>> _imagesHashed;
    std::unique_ptr<std::multimap<double, const cv::Mat>> _imagesHashed;
    std::unique_ptr<std::multimap<ulong64, const cv::Mat, CBIR::HashCompare>> _imagesHashed_pHash;

	std::unique_ptr<MongoAccess> _mongoAccess;

    QMultiMap<QString, AbstractFilter*> _filters;
    QListWidget* _filterList;

private slots:
	void onSceneChanged();
	void onClearLayout();

    void onImagesReceived(int resultsBeginInd, int resultsEndInd);
	void onFinishedLoading();
	void onImagesResized(int resultsBeginInd, int resultsEndInd);
	void onFinishedResizing();

    void onHashImages();

	void onLoadImagesClick();
    void onResizeImages(int newWidth, int newHeight);

	// ------------- filters ---------------
	void onRadiusChanged(double value);
	void onPetalNrChanged(int value);
	void onLayoutChanged(const QString& text);
    void onImageSizeChanged(int size);

    void onAddFilter();

    void onImageClicked(LayoutItem* image);
    void onAddNewFilter(QListWidgetItem* item);

signals:
	void clearLayout();
    void resizeImages(int newWidth, int newHeight);
};

#endif // MAINWINDOW_H
