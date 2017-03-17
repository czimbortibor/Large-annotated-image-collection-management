#include "MainWindow.hpp"
#include "ui_MainWindow.h"

typedef libconfig::Config Config;
typedef libconfig::Setting Setting;

MainWindow::MainWindow(QWidget* parent) :
	QMainWindow(parent), ui(new Ui::MainWindow) {
	ui->setupUi(this);

	init();
}

void MainWindow::init() {
	// set the supported image formats
	for (const QByteArray& item : QImageReader::supportedImageFormats()) {
		_supportedImgFormats.append("*." + item);
	}

	connect(this, &MainWindow::clearLayout, this, &MainWindow::onClearLayout);

	// buttons
	connect(ui->btn_load, &QPushButton::clicked, this, &MainWindow::onLoadImagesClick);
	connect(ui->btn_clear, &QPushButton::clicked, this, &MainWindow::onClearLayout);

	// spinboxes
	connect(ui->spinBox_radius, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &MainWindow::onRadiusChanged);
	connect(ui->spinBox_nrOfPetals, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &MainWindow::onPetalNrChanged);
		// overloaded signal -> have to specify the specific function syntax

    connect(ui->slider_imgSize, &QSlider::valueChanged ,this, &MainWindow::onImageSizeChanged);

	// filter fields
	connect(ui->comboBox_layout, static_cast<void(QComboBox::*)(const QString&)>(&QComboBox::currentIndexChanged), this, &MainWindow::onLayoutChanged);
	connect(ui->btn_addFilter, &QPushButton::clicked, this, &MainWindow::onFiltersClicked);
	ui->groupBox_layoutControls->hide();

    QString text = "size: " + QString::number(ui->slider_imgSize->value());
    ui->lbl_size->setText(text);

    ui->btn_cancelLoad->hide();

    //  configuration file
    _config = std::unique_ptr<Config>(new Config());
    try {
        _config->readFile("configurations.cfg");
    }
    catch (const libconfig::FileIOException& ex) {
        qWarning() << "Configuration file I/O error!";
        qInfo() << "creating new configuration file...";
        _config->writeFile("configurations.cfg");
    }

    Setting& root = _config->getRoot();
    // create the collection if it doesnt exists
    if (root.exists("collections") == 0) {
        root.add("collections", Setting::TypeList);
    }
    // load the collections
    _collections = std::unique_ptr<Setting>(&root["collections"]);
    int count = _collections->getLength();
    qInfo() << "collection count" << count;
    for (const auto& collection : *_collections.get()) {
        std::string name;
        collection.lookupValue("name", name);
        std::string URL;
        collection.lookupValue("URL", URL);
        qInfo() << "name:" << QString::fromStdString(name) << "URL:" << QString::fromStdString(URL);
    }

	initView();

	std::string host = "mongodb://localhost:27017";
	std::string database = "local";
	std::string collection = "TwitterFDL2015";
	_mongoAccess = std::unique_ptr<MongoAccess>(new MongoAccess(host, database, collection));
	if (_mongoAccess->init()) {
		_mongoAccess->test();
	}
}

MainWindow::~MainWindow() {
    if (_loadingWorker != nullptr) {
        if (_loadingWorker->isRunning()) {
            _loadingWorker->cancel();
        }
    }
    if (_futureLoaderWatcherMT != nullptr) {
        if (_futureLoaderWatcherMT->isRunning()) {
            _futureLoaderWatcherMT->cancel();
        }
    }
    delete ui;
}

void MainWindow::initView() {
	_view = new GraphicsView;
	connect(&_view->scene(), &QGraphicsScene::changed, this, &MainWindow::onSceneChanged);
	ui->centralWidget->layout()->addWidget(_view);
	_view->show();
}

void MainWindow::onSceneChanged() {
	QSizeF viewSize = _view->size();
	qreal radius = ui->spinBox_radius->value();
	QSizeF newSize(viewSize.width() - radius, viewSize.height());
	_view->setMinSceneSize(newSize);
}

void MainWindow::showAlertDialog() const {
	QMessageBox msgBox;
	msgBox.setIcon(QMessageBox::Warning);
	msgBox.setText("The selected directory does not contain any supported image files!");

	auto concat = [](QString str1, QString str2) -> QString { return str1 + ", " + str2; };
	QString imageFormats = std::accumulate(_supportedImgFormats.begin(), _supportedImgFormats.end(), _supportedImgFormats[0], concat);
	msgBox.setDetailedText("Supported image formats are:\n" + imageFormats);

	msgBox.exec();
}

void MainWindow::onLoadImagesClick() {
    if (_images) {
		clearLayout();
	}
	QFileDialog dialog;
	dialog.setFileMode(QFileDialog::DirectoryOnly);
	dialog.setOption(QFileDialog::ShowDirsOnly);
    //dialog.setDirectory("/home/czimbortibor/images");
	QStringList fileNames;
	if (dialog.exec()) {
		fileNames = dialog.selectedFiles();
	}
	// nothing was selected
	if (fileNames.count() == 0) {
		return;
	}

	_dir = QDir(fileNames[0]);
	_dir.setFilter(QDir::Files);
	if (_dir.entryList().count() == 0) {
		showAlertDialog();
		return;
	}
	_imageNames = std::unique_ptr<QList<QString>>(new QList<QString>);
	*_imageNames.get() = _dir.entryList();
    _images = std::unique_ptr<QList<cv::Mat>>(new QList<cv::Mat>);
	_nrOfImages = _imageNames->length();
    _wereHashed = false;

    _iconSize = ui->slider_imgSize->value();
    _imgWidth = _iconSize;
    _imgHeight = _iconSize;

	int len = _imageNames->length();
	qDebug() << "image size =" << _imgWidth << "x" << _imgHeight;
	qDebug() << "image count =" << len;

	_timer.start();

	// ------------- multi-threaded image load ---------------------
    /*auto fun = std::bind(&MainWindow::loadImage, this, std::placeholders::_1);
		// this: the hidden this parameter for member functions, placeholders::_1 = <const QString& fileName>
	_futureLoaderMT = std::unique_ptr<QFuture<cv::Mat>>(new QFuture<cv::Mat>(QtConcurrent::mapped(*_imageNames.get(), fun)));
	_futureLoaderWatcherMT = std::unique_ptr<QFutureWatcher<cv::Mat>>(new QFutureWatcher<cv::Mat>);
	_futureLoaderWatcherMT->setFuture(*_futureLoaderMT.get());
    connect(_futureLoaderWatcherMT.get(), &QFutureWatcher<cv::Mat>::finished, this, &MainWindow::onFinishedLoading);
	connect(_futureLoaderWatcherMT.get(), &QFutureWatcher<cv::Mat>::resultsReadyAt, this, &MainWindow::onImagesReceive);
    */

    cv::Size size(_imgWidth, _imgHeight);
    _notifyRate = 10;
    _loadingWorker = std::unique_ptr<ImageLoader>(new ImageLoader(_dir.absolutePath(), _imageNames.get(), *_images.get(), size, _notifyRate));
    connect(_loadingWorker.get(), &ImageLoader::resultsReadyAt, this, &MainWindow::onImagesReceived);
    connect(_loadingWorker.get(), &ImageLoader::finished, this, &MainWindow::onFinishedLoading);
    QThreadPool::globalInstance()->start(_loadingWorker.get());

    connect(ui->btn_cancelLoad, &QPushButton::clicked, _loadingWorker.get(), &ImageLoader::onCancel);
    connect(ui->btn_cancelLoad, &QPushButton::clicked, this, &MainWindow::onFinishedLoading);
    ui->btn_cancelLoad->setVisible(true);

    _progressBar = std::unique_ptr<QProgressBar>(new QProgressBar);
    _progressBar->setMaximum(_imageNames->length());
    ui->frame_mainControls->layout()->addWidget(_progressBar.get());
}

cv::Mat MainWindow::loadImage(const QString& imageName) const {
    cv::Mat cvImage = cv::imread(_dir.absoluteFilePath(imageName).toStdString());
    if (cvImage.data == 0) {
        return cv::Mat();
    }
    return cvImage;
}

void MainWindow::onImagesReceived(int resultsBeginInd, int resultsEndInd) {
	for (int i = resultsBeginInd; i < resultsEndInd; ++i) {
        LayoutItem* item = new LayoutItem(Mat2QImage(_images->at(i)));
        connect(item, &LayoutItem::clicked, this, &MainWindow::onImageClicked);
        _view->addItem(item);
    }

    _progressBar->setValue(_progressBar->value() + _notifyRate);
}

void MainWindow::onFinishedLoading() {
	logTime("load time:");
    _loadingWorker.release();
    ui->btn_cancelLoad->hide();
    connect(ui->btn_hash, &QPushButton::clicked, this, &MainWindow::onHashImages);
    connect(this, &MainWindow::resizeImages, this, &MainWindow::onResizeImages);
    // shuffle the images
    /*
    auto listPtr = *_images.get();
    std::random_shuffle(listPtr.begin(), listPtr.end());
    displayImages(listPtr);
    logTime("display time:");
    */
    //_dir.cdUp();
    saveImages(100);
    saveImages(10);
}

void MainWindow::onResizeImages(int newWidth, int newHeight) {
    ui->btn_clear->setEnabled(false);
    _timer.start();
    auto fun = std::bind(&MainWindow::resizeImage, this, std::placeholders::_1, newWidth, newHeight);
    _futureResizerMT = std::make_shared<QFuture<cv::Mat>>(QtConcurrent::mapped(*_images.get(), fun));
    _futureResizerWatcherMT = std::make_shared<QFutureWatcher<cv::Mat>>();
    _futureResizerWatcherMT->setFuture(*_futureResizerMT.get());
    connect(_futureResizerWatcherMT.get(), &QFutureWatcher<cv::Mat>::resultsReadyAt, this, &MainWindow::onImagesResized);
    connect(_futureResizerWatcherMT.get(), &QFutureWatcher<cv::Mat>::finished, this, &MainWindow::onFinishedResizing);
    //connect(&_futureResizerWatcherMT, &QFutureWatcher<cv::Mat>::finished, &QFutureWatcher<cv::Mat>::deleteLater);
}

cv::Mat MainWindow::resizeImage(const cv::Mat& image, int newWidth, int newHeight) const {
	cv::Mat resizedImg;
    cv::resize(image, resizedImg, cv::Size(newWidth, newHeight));
	return resizedImg;
}

void MainWindow::onImagesResized(int resultsBeginInd, int resultsEndInd) {
    for (int i = resultsBeginInd; i < resultsEndInd; ++i) {
        QString newFileName = (_dirSmallImg->absolutePath() + QDir::separator() + _imageNames->at(i));
        cv::imwrite(newFileName.toStdString(), _futureResizerMT->resultAt(i));
    }
}

void MainWindow::onFinishedResizing() {
    logTime("time needed to save the images:");
    // TODO: disable buttons during resizing
    ui->btn_clear->setEnabled(true);
}

void MainWindow::onHashImages() {
    _wereHashed = true;
    // ------ opencv img_hash ------- : https://github.com/stereomatchingkiss/opencv_contrib/tree/img_hash/modules/img_hash
    cv::Ptr<cv::img_hash::PHash> hasher = cv::img_hash::PHash::create();
    // cv::Ptr<cv::img_hash::AverageHash> hasher = cv::img_hash::AverageHash::create();
    // cv::Ptr<cv::img_hash::MarrHildrethHash> hasher = cv::img_hash::MarrHildrethHash::create();
    // cv::Ptr<cv::img_hash::RadialVarianceHash> hasher = cv::img_hash::RadialVarianceHash::create();
    auto mapPtr = &imageRetrieval.computeHashes(*_images.get(), hasher);
    //_imagesHashed = std::unique_ptr<std::multimap<double, const cv::Mat>>(mapPtr);
    _imagesHashed = std::unique_ptr<std::multimap<const cv::Mat, const cv::Mat, CBIR::MatCompare>>(mapPtr);
    _view->clear();
    displayImages(*_imagesHashed.get());
    //_imagesHashed = std::unique_ptr<std::multimap<const cv::Mat, const cv::Mat, CBIR::MatCompare>>(mapPtr);
    //displayImages<std::multimap<const cv::Mat, const cv::Mat, CBIR::MatCompare>>(*_imagesHashed.get());


    // ------ pHash -------
    /*auto mapPtr = &imageRetrieval.computeHashes_pHash(*_images.get(), _dir.absolutePath(), *_imageNames.get());
    _imagesHashed_pHash = std::unique_ptr<std::multimap<ulong64, const cv::Mat, CBIR::HashCompare>>(mapPtr);
    displayImages(*_imagesHashed_pHash.get());
    */
}

void MainWindow::saveImages(int size) {
    const char dirSeparator = QDir::separator().toLatin1();
    QString currentDir(_dir.absolutePath());
    QString collectionName(QString::number(_images->length()) + "_" + QString::number(size));
    QString collectionDir(dirSeparator + QString("collections") + dirSeparator + collectionName + dirSeparator);
    QString absPath = currentDir + collectionDir;
    if (!QDir(absPath).exists()) {
        QDir().mkdir(currentDir + dirSeparator + "collections");
        QDir().mkdir(absPath);
        _dirSmallImg = new QDir(absPath);
        qInfo() << "creating collection index...";
        // create new entry
        Setting& collection = _collections->add(Setting::TypeGroup);
        collection.add("name", Setting::TypeString) = collectionName.toStdString();
        collection.add("URL", Setting::TypeString) = absPath.toStdString();

        qInfo() << "saving the configurations...";
        // write out the updated configuration
        _config->writeFile("configurations.cfg");

        qInfo() << "saving the " << size << "x" << size << "icons to: " + _dirSmallImg->absolutePath() << "...";
        _timer.start();
        emit resizeImages(size, size);
    }
}

void MainWindow::onClearLayout() {
    _view->clear();
    //_images->clear();
    // releases any memory not required to store the items
   // _images->squeeze();
}

void MainWindow::displayImages(const QList<cv::Mat>& images) const {
	for (const auto& image : images) {
		QImage res = Mat2QImage(image);
        LayoutItem* item = new LayoutItem(res);
		_view->addItem(static_cast<QGraphicsLayoutItem*>(item));
	}
}

template<typename T>
void MainWindow::displayImages(const T& images) const {
    //typedef std::multimap<double, const cv::Mat> map;
    //if (std::is_same<map, T>::value) {
    for (const auto& entry : images) {
        QImage image = Mat2QImage(entry.second);
        //qDebug() << entry.first;
        LayoutItem* item = new LayoutItem(image);
        _view->addItem(static_cast<QGraphicsLayoutItem*>(item));
    }
   // }
}

void MainWindow::logTime(QString message) {
	double time = _timer.nsecsElapsed() / 1000000000.0;
	std::ofstream logFile;
	logFile.open("log.txt", std::ios::out | std::ios::app);
	qDebug() << message << time << " seconds";
	logFile << "number of images: " << _nrOfImages << "\n";
	logFile << message.toStdString() << _nrOfImages << "\n";
	logFile.close();
}

QImage MainWindow::Mat2QImage(const cv::Mat& cvimage) const {
    switch (cvimage.type()) {
         // 8-bit, 4 channel
         case CV_8UC4: {
            QImage qimage(cvimage.data, cvimage.cols, cvimage.rows,
                          static_cast<int>(cvimage.step),
                          QImage::Format_ARGB32);
            return qimage;
         }

         // 8-bit, 3 channel
         case CV_8UC3: {
            QImage qimage(cvimage.data, cvimage.cols, cvimage.rows,
                          static_cast<int>(cvimage.step),
                          QImage::Format_RGB888);
            return qimage.rgbSwapped();
         }

         // 8-bit, 1 channel
         case CV_8UC1: {
            QImage qimage(cvimage.data, cvimage.cols, cvimage.rows,
                          static_cast<int>(cvimage.step),
                          QImage::Format_Grayscale8);
            return qimage;
         }

         default: {
            qWarning() << "cv::Mat type not handled:" << cvimage.type();
            return QImage();
         }
      }
}

cv::Mat MainWindow::QImage2Mat(const QImage& qimage) const {
    switch (qimage.format()) {
         // 8-bit, 4 channel
         case QImage::Format_ARGB32:
         case QImage::Format_ARGB32_Premultiplied: {
            cv::Mat cvimage(qimage.height(), qimage.width(), CV_8UC4,
                          const_cast<uchar*>(qimage.bits()),
                          static_cast<size_t>(qimage.bytesPerLine()));
            return cvimage;
         }

         // 8-bit, 3 channel
         case QImage::Format_RGB32:
         case QImage::Format_RGB888: {
            QImage swapped = qimage;
            if (qimage.format() == QImage::Format_RGB32) {
               swapped = swapped.convertToFormat(QImage::Format_RGB888);
            }
            swapped = swapped.rgbSwapped();
            return cv::Mat(swapped.height(), swapped.width(), CV_8UC3,
                            const_cast<uchar*>(swapped.bits()),
                            static_cast<size_t>(swapped.bytesPerLine())).clone();
         }

         // 8-bit, 1 channel
         case QImage::Format_Indexed8: {
            cv::Mat cvimage(qimage.height(), qimage.width(), CV_8UC1,
                          const_cast<uchar*>(qimage.bits()),
                          static_cast<size_t>(qimage.bytesPerLine()));
            return cvimage;
         }

         default: {
            qWarning() << "QImage format not handled:" << qimage.format();
            return cv::Mat();
         }
    }
}

void MainWindow::onRadiusChanged(double value) {
    if (_images->length()) {
        _view->setRadius(value);
		_view->clear();
        displayImages(*_images.get());
	}
}

void MainWindow::onPetalNrChanged(int value) {
    if (_images->length()) {
        _view->setNrOfPetals(value);
		_view->clear();
        displayImages(*_images.get());
	}
}

void MainWindow::onLayoutChanged(const QString& text) {
    _view->setLayout(text);
	text.compare("petal") == 0 ? ui->groupBox_layoutControls->setVisible(true) : ui->groupBox_layoutControls->hide();
}

void MainWindow::onImageSizeChanged(int size) {
    QString text = "size: " + QString::number(size);
    ui->lbl_size->setText(text);
    if (_images) {
        ui->btn_clear->setEnabled(false);
        // smaller size is requested, no need to reload the images
        //if (size <= _iconSize) {
            _iconSize = size;
           //resizeImages(size, size);
 // }
        // have to reload the images to not damage their quality during resizing
       /* else {
            _iconSize = size;
            _imgWidth = size;
            _imgHeight = size;
            auto fun = std::bind(&MainWindow::loadImage, this, std::placeholders::_1);
            _futureLoaderMT = std::unique_ptr<QFuture<cv::Mat>>(new QFuture<cv::Mat>(QtConcurrent::mapped(*_imageNames.get(), fun)));
            _futureLoaderWatcherMT = std::unique_ptr<QFutureWatcher<cv::Mat>>(new QFutureWatcher<cv::Mat>);
            _futureLoaderWatcherMT->setFuture(*_futureLoaderMT.get());
            connect(_futureLoaderWatcherMT.get(), &QFutureWatcher<cv::Mat>::resultsReadyAt, this, &MainWindow::onImagesReceive);
        }*/
    }
}

void MainWindow::onFiltersClicked() {
	QListWidget* filterList = new QListWidget(ui->widget_filters);

	filterList->addItems(QStringList() << "keyword" << "date range" << "image");
	ui->widget_filters->setMinimumSize(filterList->size());
    //ui->widget_filters->layout()->addWidget(filterList);
	filterList->show();
}

void MainWindow::onImageClicked(LayoutItem* image) {
    if (_wereHashed == 0) {
        ui->btn_hash->click();
    }
    QList<cv::Mat> similarImages = getSimilarImages(*image);
    //emit clearLayout();
    _view->clear();
    displayImages(similarImages);
}

QList<cv::Mat>& MainWindow::getSimilarImages(const LayoutItem& target) const {
    int nrOfSimilars;
    if (ui->comboBox_layout->currentText() == "petal") {
        nrOfSimilars = ui->spinBox_nrOfPetals->value();
    }
    else {
        nrOfSimilars = 0;
    }

    QImage image = target.getPixmap()->toImage();
    cv::Mat cvImage = QImage2Mat(image);
    cv::Mat targetHash = CBIR::getHash(cvImage);
    auto it = _imagesHashed->find(targetHash);
    QList<cv::Mat>* res = new QList<cv::Mat>;
    res->push_front(cvImage);
    if (it == _imagesHashed->end()) {
        return *res;
    }

    int k = 1;
    auto rightIt = it;
    auto leftIt = it;
    while (k <= nrOfSimilars) {
        std::advance(rightIt, k);
        std::advance(leftIt, -k);
        double rightDist = CBIR::getDistance(it->first, rightIt->first);
        double leftDist = CBIR::getDistance(it->first, leftIt->first);
        (rightDist < leftDist) ? res->push_front(rightIt->second) : res->push_front(leftIt->second);
        ++k;
    }
    return *res;
}
