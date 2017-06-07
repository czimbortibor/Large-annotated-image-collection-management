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
#include <QElapsedTimer>
#include <QDebug>
#include <QListWidget>
#include <QProgressBar>

#include "view/GraphicsView.hpp"
#include "utils/LayoutItem.hpp"
#include "utils/CBIR.hpp"
#include "utils/ConfigurationsHandler.hpp"
#include "utils/image_load/LoadingHandler.hpp"
#include "utils/image_load/ImageLoader.hpp"
#include "utils/ImageCollection.hpp"
#include "layouts/FlowLayout.hpp"
#include "layouts/PetalLayout.hpp"
#include "db/DbContext.hpp"
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
    /**
     * @brief initializes the main window's elements
     */
    void initWindow();

    /**
     * @brief initiiaizes the MongoDB connection
     */
    void initDb();

    /**
     * @brief creates the available hashing functions
     */
    void initHashes();

    /** creates the main view for displaying images */
	void initView();

    void saveImages(int size);
    /**
     * @brief imageSaving
     * @param size
     */
    void imageSaving(int size);

    /** no images were selected */
    void showAlertDialog() const;

    /**
     * @brief showProgressBar displays a progress bar to the given task
     * @param maximumValue the completion point
     * @param taskName the name of the ongoing task
     */
    void showProgressBar(const int maximumValue, const QString& taskName);

    cv::Mat resizeImage(const cv::Mat& image, int newWidth, int newHeight) const;
    void displayImages(const QList<cv::Mat>& images) const;
    /** opencv_img_hash & pHash display */
    template<typename T> void displayImages(const T& images) const;

    /**
     * @brief loads one image
     * @param the image's absolute path
     * @return the loaded image
     */
    QImage loadImage(const QString& url) const;

	void logTime(QString message);

	Ui::MainWindow* ui;
	int _iconSize;
	int _nrOfImages;
	QDir _dir;
    QDir* _dirSmallImg;
	QStringList _supportedImgFormats;
    std::unique_ptr<QStringList> _imageNames;
	int _imgWidth;
	int _imgHeight;
	QElapsedTimer _timer;
    int _notifyRate;
    std::unique_ptr<QProgressBar> _progressBar;
    LayoutItem* _hoveredItem;

    std::unique_ptr<ConfigurationsHandler> _configHandler;

    /**
     * @brief _imageSaver saves the images to the disk
     */
    std::unique_ptr<QFuture<void>> _imageSaver;
    QFutureWatcher<void> _saverWatcher;

	GraphicsView* _view;

    /**
     * @brief _loadingHandler handles the image loading, in single/multi-threaded ways
     */
    std::unique_ptr<LoadingHandler> _loadingHandler;

    std::unique_ptr<QFuture<QImage>> _oneImageLoader;
    QFutureWatcher<QImage> _oneImageWatcher;

    std::unique_ptr<QList<cv::Mat>> _images;
    std::unique_ptr<std::multimap<cv::Mat, cv::Mat, CBIR::MatCompare>> _hashedImages;

    ImageCollection _imageCollection;

    CBIR _imageRetrieval;

    std::unique_ptr<std::multimap<ulong64, const cv::Mat, CBIR::HashCompare>> _imagesHashed_pHash;

	std::unique_ptr<DbContext> _dbContext;

    QMap<QString, AbstractFilter*> _filters;
    QListWidget* _filterList;

private slots:
	void onClearLayout();

    void onImageReceived(int index, const QString& url, const QString& originalUrl);
    void onFinishedLoading();

    void onSavingChange(int value);
    void onFinishedSaving();

    void onHashImages();

	void onLoadImagesClick();

	// ------------- filters ---------------
	void onRadiusChanged(double value);
	void onPetalNrChanged(int value);
	void onLayoutChanged(const QString& text);
    void onImageSizeChanged(int size);
    void onSpiralTurnChanged(int value);
    void onSpiralDistanceChanged(int value);

    void onAddFilter();

    void onImageClicked(const QString& url);
    void onImageDoubleClicked(const QString& url);
    void onImageHoverEnter(const QString& url, LayoutItem* item);
    void onFinishedOneImageLoad();

    void onAddNewFilter(QListWidgetItem* item);
    void on_btn_applyFilters_clicked();

    void testMongo(const std::string& date1, const std::string& date2);

signals:
	void clearLayout();
    void resizeImages(int newWidth, int newHeight);
    void saveProgress(int value);
};

#endif // MAINWINDOW_H
