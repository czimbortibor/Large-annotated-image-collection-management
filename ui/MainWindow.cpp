#include "MainWindow.hpp"
#include "ui_MainWindow.h"


using ImageMap = std::multimap<const cv::Mat, const cv::Mat, CBIR::MatCompare>;

MainWindow::MainWindow(QWidget* parent) :
    QMainWindow(parent), ui(new Ui::MainWindow) {
	ui->setupUi(this);

    initWindow();
    initDb();
    initHashes();
    initView();
}

MainWindow::~MainWindow() {
    if (_loadingHandler) {
        _loadingHandler->onCancel();
    }
    _configHandler.release();
    delete ui;
}

void MainWindow::initWindow() {
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
    connect(ui->btn_addFilter, &QPushButton::clicked, this, &MainWindow::onAddFilter);
	ui->groupBox_layoutControls->hide();

    QString text = "size: " + QString::number(ui->slider_imgSize->value());
    ui->lbl_size->setText(text);

    ui->btn_cancelLoad->hide();

    // ------- configurations -----------
    _configHandler = std::unique_ptr<ConfigurationsHandler>(new ConfigurationsHandler("configurations.cfg"));

    // ---------- filters -----------
    ui->widget_createFilters->hide();
    _filters.insert("date range", new DateFilter);
    _filterList = new QListWidget(ui->widget_createFilters);
    for (const auto& filter : _filters.keys()) {
        _filterList->addItem(filter);
    }
    ui->widget_createFilters->setMinimumSize(_filterList->size());
    connect(_filterList, &QListWidget::itemDoubleClicked, this, &MainWindow::onAddNewFilter);

    ui->btn_applyFilters->hide();
}

void MainWindow::initDb() {
    std::string host = "mongodb://localhost:27017";
    std::string database = "local";
    std::string collection = "TwitterFDL2015";
    _mongoAccess = std::unique_ptr<MongoAccess>(new MongoAccess(host, database, collection));
    if (_mongoAccess->init()) {
        qInfo() << "succesful database connection";
    }
}

void MainWindow::initHashes() {
    // store the available hashing algorithms
    QStringList hashingAlgorithms;
    hashingAlgorithms << "Average hash" << "Perceptual hash" << "Marr Hildreth hash"
    << "Radial Variance hash" << "Block Mean hash" << "Color Moment hash";
    ui->comboBox_hashes->addItems(hashingAlgorithms);
    _hashes.insert("Average hash", cv::img_hash::AverageHash::create());
    _hashes.insert("Perceptual hash", cv::img_hash::PHash::create());
    _hashes.insert("Marr Hildreth hash", cv::img_hash::MarrHildrethHash::create());
    _hashes.insert("Radial Variance hash", cv::img_hash::RadialVarianceHash::create());
    _hashes.insert("Block Mean hash", cv::img_hash::BlockMeanHash::create());
    _hashes.insert("Color Moment hash", cv::img_hash::ColorMomentHash::create());
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

void MainWindow::showProgressBar(const int maximumValue, const QString& taskName) {
    _progressBar = std::unique_ptr<QProgressBar>(new QProgressBar);
    _progressBar->setMaximum(maximumValue);
    QLabel* taskLabel = new QLabel(taskName);
    ui->dockWidgetContents_mainControls->layout()->addWidget(taskLabel);
    ui->dockWidgetContents_mainControls->layout()->addWidget(_progressBar.get());
    connect(_progressBar.get(), &QProgressBar::destroyed, taskLabel, &QLabel::deleteLater);
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
    _imageNames = std::unique_ptr<QStringList>(new QStringList);
    *_imageNames.get() = _dir.entryList();
	_nrOfImages = _imageNames->length();
    _images = std::unique_ptr<QList<cv::Mat>>(new QList<cv::Mat>);
    _wereHashed = false;

    _iconSize = ui->slider_imgSize->value();
    _imgWidth = _iconSize;
    _imgHeight = _iconSize;

	int len = _imageNames->length();
	qDebug() << "image size =" << _imgWidth << "x" << _imgHeight;
	qDebug() << "image count =" << len;

	_timer.start();

    _loadingHandler = std::unique_ptr<LoadingHandler>(new LoadingHandler);
    _loadingHandler->setWidth(_imgWidth);
    _loadingHandler->setHeight(_imgHeight);
    connect(_loadingHandler.get(), &LoadingHandler::imageReady, this, &MainWindow::onImageReceived);
    connect(_loadingHandler.get(), &LoadingHandler::imagesReady, this, &MainWindow::onImagesReceived);
    connect(_loadingHandler.get(), &LoadingHandler::finishedLoading, this, &MainWindow::onFinishedLoading);
    connect(ui->btn_cancelLoad, &QPushButton::clicked, _loadingHandler.get(), &LoadingHandler::onCancel);
    connect(ui->btn_cancelLoad, &QPushButton::clicked, this, &MainWindow::onFinishedLoading);
    /** start loading */
    _notifyRate = 10;
    auto loaderPtr = _loadingHandler->loadImages_st(_dir.absolutePath(), _imageNames.get(), _notifyRate);
    //auto loaderPtr = _loadingHandler->loadImages_mt(_dir.absolutePath(), *_imageNames.get());
    _images.reset(loaderPtr);

    ui->btn_cancelLoad->setVisible(true);

    showProgressBar(_imageNames->length(), "loading");
}

void MainWindow::onImageReceived(const cv::Mat& image) {
    LayoutItem* item = new LayoutItem(ImageConverter::Mat2QImage(image));
    connect(item, &LayoutItem::clicked, this, &MainWindow::onImageClicked);
    _view->addItem(item);
    _progressBar->setValue(_progressBar->value() + 1);
}

void MainWindow::onImagesReceived(int resultsBeginInd, int resultsEndInd) {
    for (int i = resultsBeginInd; i < resultsEndInd; ++i) {
        LayoutItem* item = new LayoutItem(ImageConverter::Mat2QImage(_images->at(i)));
        connect(item, &LayoutItem::clicked, this, &MainWindow::onImageClicked);
        _view->addItem(item);
    }
    /*ImageMap::const_iterator iter = std::next(_images->begin(), resultsBeginInd);
    ImageMap::const_iterator endIter = std::next(_images->begin(), resultsEndInd);
    for (iter; iter != endIter; ++iter) {
        LayoutItem* item = new LayoutItem(ImageConverter::Mat2QImage((*iter).second));
        connect(item, &LayoutItem::clicked, this, &MainWindow::onImageClicked);
        _view->addItem(item);
    }*/

    _progressBar->setValue(_progressBar->value() + _notifyRate);
}

void MainWindow::onFinishedLoading() {
	logTime("load time:");
    _loadingHandler.release();
    ui->btn_cancelLoad->hide();
    _progressBar->reset();
    _progressBar.reset(nullptr);
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
    /*auto fun = std::bind(&MainWindow::resizeImage, this, std::placeholders::_1, newWidth, newHeight);
    _futureResizerMT = std::make_shared<QFuture<cv::Mat>>(QtConcurrent::mapped(*_images.get(), fun));
    _futureResizerWatcherMT = std::make_shared<QFutureWatcher<cv::Mat>>();
    _futureResizerWatcherMT->setFuture(*_futureResizerMT.get());
    connect(_futureResizerWatcherMT.get(), &QFutureWatcher<cv::Mat>::resultsReadyAt, this, &MainWindow::onImagesResized);
    connect(_futureResizerWatcherMT.get(), &QFutureWatcher<cv::Mat>::finished, this, &MainWindow::onFinishedResizing);
    //connect(&_futureResizerWatcherMT, &QFutureWatcher<cv::Mat>::finished, &QFutureWatcher<cv::Mat>::deleteLater);
    */
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
    ui->btn_clear->setEnabled(true);
}

void MainWindow::onHashImages() {
    _wereHashed = true;

    const QString& algorithm = ui->comboBox_hashes->currentText();
    cv::Ptr<cv::img_hash::ImgHashBase> hasher = _hashes.value(algorithm);
    auto mapPtr = imageRetrieval.computeHashes(*_images.get(), hasher);
    //_imagesHashed = std::unique_ptr<ImageMap>(mapPtr);
    _imagesHashed.reset(mapPtr);
    _view->clear();
    displayImages(*_imagesHashed.get());
    /** release and delete the owned object */
    //_images.reset(nullptr);

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
    QString collectionName(QString::number(_images->size()) + "_" + QString::number(size));
    QString collectionDir(dirSeparator + QString("collections") + dirSeparator + collectionName + dirSeparator);
    QString absPath = currentDir + collectionDir;
    if (!QDir(absPath).exists()) {
        QDir().mkdir(currentDir + dirSeparator + "collections");
        QDir().mkdir(absPath);
        _dirSmallImg = new QDir(absPath);

        _configHandler->addNewCollection(collectionName, absPath);

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
        QImage res = ImageConverter::Mat2QImage(image);
        LayoutItem* item = new LayoutItem(res);
		_view->addItem(static_cast<QGraphicsLayoutItem*>(item));
	}
}

template<typename T>
void MainWindow::displayImages(const T& images) const {
    for (const auto& entry : images) {
        QImage image = ImageConverter::Mat2QImage(entry.second);
        LayoutItem* item = new LayoutItem(image);
        _view->addItem(static_cast<QGraphicsLayoutItem*>(item));
    }
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

void MainWindow::onRadiusChanged(double value) {
    if (_images != nullptr) {
        _view->setRadius(value);
		_view->clear();
        displayImages(*_images.get());
	}
}

void MainWindow::onPetalNrChanged(int value) {
    if (_images != nullptr) {
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

void MainWindow::onAddFilter() {
    ui->widget_createFilters->setVisible(true);
    _filterList->show();
    ui->btn_applyFilters->setVisible(true);
}

void MainWindow::onAddNewFilter(QListWidgetItem* item) {
    ui->widget_createFilters->hide();
    // create the selected filter
    AbstractFilter* filter = _filters.value(item->text())->makeFilter();
    // put back the created filter
    _filters.insert(item->text(), filter);
    QWidget* filterControl = filter->makeControl();
    ui->widget_filters->layout()->addWidget(new QLabel(item->text()));
    ui->widget_filters->layout()->addWidget(filterControl);

    QGroupBox* datePicker = static_cast<QGroupBox*>(filterControl);

    //connect(datePicker, &QDateEdit::dateChanged, this, &MainWindow::testMongo);
}

void MainWindow::on_btn_applyFilters_clicked() {
    // 1449493211046 -> Mon Dec 07 13:00:11 +0000 2015
    const QGroupBox* dateEdits = ui->widget_filters->findChild<QGroupBox*>();
    DateFilter* dateFilter = static_cast<DateFilter*>(_filters.value("date range"));
    QList<std::string> selectedRanges = dateFilter->getDates();
    std::string date1 = selectedRanges[0];
    std::string date2 = selectedRanges[1];
    qInfo() << QString::fromStdString(date1);
    qInfo() << QString::fromStdString(date2);

    //testMongo(date1, date2);
}

void MainWindow::testMongo(const std::string& date1, const std::string& date2) {
    QStringList* results = _mongoAccess->test(date1, date2);

    cv::Size size(_imgWidth, _imgHeight);
    _notifyRate = 10;
    QString dir = "/home/czimbortibor/images/500";
}

void MainWindow::onImageClicked(LayoutItem* image) {
    if (_wereHashed == 0) {
        ui->btn_hash->click();
    }
    QList<cv::Mat> similarImages = getSimilarImages(*image);
    //emit clearLayout();
    _view->clear();
    //displayImages(similarImages);
}

QList<cv::Mat>& MainWindow::getSimilarImages(const LayoutItem& target) const {
    /*int nrOfSimilars;
    if (ui->comboBox_layout->currentText() == "petal") {
        nrOfSimilars = ui->spinBox_nrOfPetals->value();
    }
    else {
        nrOfSimilars = 2;
    }

    QImage image = target.getPixmap()->toImage();
    cv::Mat cvImage = ImageConverter::QImage2Mat(image);
    //cv::Mat targetHash = CBIR::getHash(cvImage);
    double targetHash = CBIR::getHashValue(cvImage);
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
        rightIt = std::next(it, k);
        if (rightIt == _imagesHashed->end()) {
            break;
        }
        leftIt = std::next(it, -k);
        if (leftIt == _imagesHashed->begin()) {
            break;
        }
        /*double rightDist = CBIR::getDistance(it->first, rightIt->first);
        double leftDist = CBIR::getDistance(it->first, leftIt->first);*/
        /*double rightDist = abs(it->first - rightIt->first);
        double leftDist = abs(it->first - leftIt->first);
        (rightDist < leftDist) ? res->push_back(rightIt->second) : res->push_back(leftIt->second);
        ++k;
    }
    return *res;*/
}
