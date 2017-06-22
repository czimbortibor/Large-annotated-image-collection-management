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
#include "utils/GraphicsImage.hpp"
#include "utils/CBIR.hpp"
#include "utils/image_load/LoadingHandler.hpp"
#include "utils/image_load/ImageLoaderST.hpp"
#include "utils/ImageCollection.hpp"
#include "utils/metadata/MetadataParser.hpp"
#include "utils/Logger.hpp"
#include "layouts/FlowLayout.hpp"
#include "layouts/PetalLayout.hpp"
#include "db/DbContext.hpp"
#include "filters/DateFilter.hpp"
#include "filters/TextFilter.hpp"


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
	void displayImages(const QList<GraphicsImage>& images);
	void displayOriginalImages(const QList<GraphicsImage>& images);

    /**
     * @brief loads one image
     * @param the image's absolute path
     * @return the loaded image
     */
    QImage loadImage(const QString& url) const;

	void logTime(QString message);

	QJsonArray* getAllMetadata();
	void populateMetadataTable(const QList<Metadata>& metadata, const QList<GraphicsImage>& images);

	Ui::MainWindow* ui;
	int _iconSize;
	int _nrOfImages;
	QDir _dir;
	QDir _dirSmallImg;
	QStringList _supportedImgFormats;
    std::unique_ptr<QStringList> _imageNames;
	int _imgWidth;
	int _imgHeight;
	QElapsedTimer _timer;
    int _notifyRate;
    std::unique_ptr<QProgressBar> _progressBar;
	GraphicsImage* _hoveredItem;

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

	std::unique_ptr<QFuture<void>> _imageDisplayer;

	std::shared_ptr<QList<GraphicsImage>> _images;
	std::unique_ptr<std::multimap<GraphicsImage, cv::Mat, CBIR::MatCompare>> _hashedImages;

    ImageCollection _imageCollection;

    CBIR _imageRetrieval;

	DbContext _dbContext;

    QMap<QString, AbstractFilter*> _filters;
    QListWidget* _filterList;
	std::unique_ptr<QList<Metadata>> _metadata;

private slots:
	void onClearLayout();

	void onImageReceivedST(int index);
	void onImageReceivedMT(const GraphicsImage& image);
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
	void onImageHoverEnter(const QString& url, GraphicsImage* item);
    void onFinishedOneImageLoad();

	void onAddNewFilter(QListWidgetItem* item);
	void on_btn_selectedImages_clicked();
	void on_btn_clearSelections_clicked();

signals:
	void addViewItem(const QGraphicsLayoutItem* item);
	void clearLayout();
    void resizeImages(int newWidth, int newHeight);
    void saveProgress(int value);
	void display(const QList<GraphicsImage>& images);
};

#endif // MAINWINDOW_H
