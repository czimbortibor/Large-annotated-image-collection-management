#include "MainWindow.hpp"
#include "ui_MainWindow.h"


using CollectionMap = std::map<cv::Mat, cv::Mat, CBIR::MatCompare>;

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
    _hashedImages.release();
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
    connect(ui->spinBox_a, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &MainWindow::onSpiralTurnChanged);
    connect(ui->spinBox_b, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &MainWindow::onSpiralDistanceChanged);

    connect(ui->slider_imgSize, &QSlider::valueChanged ,this, &MainWindow::onImageSizeChanged);

	// filter fields
	connect(ui->comboBox_layout, static_cast<void(QComboBox::*)(const QString&)>(&QComboBox::currentIndexChanged), this, &MainWindow::onLayoutChanged);
    connect(ui->btn_addFilter, &QPushButton::clicked, this, &MainWindow::onAddFilter);
    ui->tabWidget->hide();

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
    ui->comboBox_hashes->addItems(_imageCollection.getHashingAlgorithms());
}

void MainWindow::initView() {
	_view = new GraphicsView;
	ui->centralWidget->layout()->addWidget(_view);
	_view->show();
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

    _iconSize = ui->slider_imgSize->value();
    _imgWidth = _iconSize;
    _imgHeight = _iconSize;

	int len = _imageNames->length();
	qDebug() << "image size =" << _imgWidth << "x" << _imgHeight;
	qDebug() << "image count =" << len;

	_timer.start();

    _loadingHandler = std::unique_ptr<LoadingHandler>(new LoadingHandler(_imageCollection));
    _loadingHandler->setWidth(_imgWidth);
    _loadingHandler->setHeight(_imgHeight);
    connect(_loadingHandler.get(), &LoadingHandler::imageReady, this, &MainWindow::onImageReceived);
    connect(_loadingHandler.get(), &LoadingHandler::finishedLoading, this, &MainWindow::onFinishedLoading);
    connect(ui->btn_cancelLoad, &QPushButton::clicked, _loadingHandler.get(), &LoadingHandler::onCancel);
    connect(ui->btn_cancelLoad, &QPushButton::clicked, this, &MainWindow::onFinishedLoading);

    auto loaderPtr = _loadingHandler->loadImages_st(_dir.absolutePath(), _imageNames.get());
    //auto loaderPtr = _loadingHandler->loadImages_mt(_dir.absolutePath(), *_imageNames.get());
    _images.reset(loaderPtr);

    ui->btn_cancelLoad->setVisible(true);

    showProgressBar(_imageNames->length(), "loading");
}

void MainWindow::onImageReceived(int index, const QString& url) {
    LayoutItem* item = new LayoutItem(ImageConverter::Mat2QImage(_images->at(index)), url);
    connect(item, &LayoutItem::clicked, this, &MainWindow::onImageClicked);
    _view->addItem(item);
    _progressBar->setValue(_progressBar->value() + 1);
}

void MainWindow::onFinishedLoading() {
	logTime("load time:");
    _loadingHandler.release();
    ui->btn_cancelLoad->hide();
    _progressBar.reset(nullptr);
    connect(ui->btn_hash, &QPushButton::clicked, this, &MainWindow::onHashImages);

    // shuffle the images
    /*
    auto listPtr = *_images.get();
    std::random_shuffle(listPtr.begin(), listPtr.end());
    displayImages(listPtr);
    logTime("display time:");
    */

    // TODO: fix saving 100's
    saveImages(100);
    saveImages(10);
}

void MainWindow::onHashImages() {
    const QString hasherName = ui->comboBox_hashes->currentText();
     _hashedImages.release();
    std::map<cv::Mat, cv::Mat, CBIR::MatCompare> result = _imageCollection.getHashedImages(hasherName);
    _hashedImages.reset(&result);
    _view->clear();
    displayImages(*_hashedImages.get(), *_imageNames.get());

    //displayImages<std::multimap<const cv::Mat, const cv::Mat, CBIR::MatCompare>>(*_imagesHashed.get());


    // ------ pHash -------
    /*auto mapPtr = &imageRetrieval.computeHashes_pHash(*_images.get(), _dir.absolutePath(), *_imageNames.get());
    _imagesHashed_pHash = std::unique_ptr<std::multimap<ulong64, const cv::Mat, CBIR::HashCompare>>(mapPtr);
    displayImages(*_imagesHashed_pHash.get());
    */
}

void MainWindow::imageSaving(int size) {
    for (int i = 0; i < _images->length(); ++i) {
        QString fileName = (_dirSmallImg->absolutePath() + QDir::separator() + _imageNames->at(i));
        cv::imwrite(fileName.toStdString(), _images->at(i));
    }
}

void MainWindow::onSavingChange(int value) {
    _progressBar->setValue(_progressBar->value() + value);
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

        showProgressBar(_images->length(), "saving images");
        connect(_progressBar.get(), &QProgressBar::valueChanged, this, &MainWindow::onSavingChange);

        auto saver_fun = std::bind(&MainWindow::imageSaving, this, size);
        _imageSaver = std::unique_ptr<QFuture<void>>(new QFuture<void>(
                                                         QtConcurrent::run(saver_fun)));
        _saverWatcher.setFuture(*_imageSaver.get());
        connect(&_saverWatcher, &QFutureWatcher<void>::finished, this, &MainWindow::onFinishedSaving);
    }
}

void MainWindow::onFinishedSaving() {
    logTime("saving time:");
    _progressBar.release();
}

void MainWindow::onClearLayout() {
    _view->clear();
    //_images->clear();
    // releases any memory not required to store the items
   // _images->squeeze();
}

void MainWindow::displayImages(const QList<cv::Mat>& images, const QStringList& urls) const {
    int i = 0;
    for (const auto& image : images) {
        QImage res = ImageConverter::Mat2QImage(image);
        LayoutItem* item = new LayoutItem(res, urls.at(i));
        ++i;
        connect(item, &LayoutItem::clicked, this, &MainWindow::onImageClicked);
		_view->addItem(static_cast<QGraphicsLayoutItem*>(item));
	}
    // _view->setMinSceneSize(_view->size());
}

template<typename T>
void MainWindow::displayImages(const T& images, const QStringList& urls) const {
    int i = 0;
    for (const auto& entry : images) {
        QImage image = ImageConverter::Mat2QImage(entry.second);
        LayoutItem* item = new LayoutItem(image, urls.at(i));
        ++i;
        connect(item, &LayoutItem::clicked, this, &MainWindow::onImageClicked);
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
    if (_hashedImages != nullptr) {
        _view->setRadius(value);
		_view->clear();
        displayImages(*_hashedImages.get(), *_imageNames.get());
    } else {
        _view->setNrOfPetals(value);
        _view->clear();
        displayImages(*_images.get(), *_imageNames.get());
    }
}

void MainWindow::onPetalNrChanged(int value) {
    if (_hashedImages != nullptr) {
        _view->setNrOfPetals(value);
		_view->clear();
        displayImages(*_hashedImages.get(), *_imageNames.get());
    } else {
        _view->setNrOfPetals(value);
        _view->clear();
        displayImages(*_images.get(), *_imageNames.get());
    }
}

void MainWindow::onSpiralDistanceChanged(int value) {
    if (_hashedImages != nullptr) {
        _view->setSpiralDistance(value);
        _view->clear();
        displayImages(*_hashedImages.get(), *_imageNames.get());
    } else {
        _view->setNrOfPetals(value);
        _view->clear();
        displayImages(*_images.get(), *_imageNames.get());
    }
}

void MainWindow::onSpiralTurnChanged(int value) {
    if (_hashedImages != nullptr) {
        _view->setSpiralTurn(value);
        _view->clear();
        displayImages(*_hashedImages.get(), *_imageNames.get());
    } else {
        _view->setNrOfPetals(value);
        _view->clear();
        displayImages(*_images.get(), *_imageNames.get());
    }
}

void MainWindow::onLayoutChanged(const QString& text) {
    _view->setLayout(text);
    if (text.compare("petal") == 0) {
        ui->tabWidget->setVisible(true);
        ui->tabWidget->setTabEnabled(1, true);
        ui->tabWidget->setTabEnabled(0, false);
    }
    else {
        if (text.compare("spiral") == 0) {
            ui->tabWidget->setVisible(true);
            ui->tabWidget->setTabEnabled(0, true);
            ui->tabWidget->setTabEnabled(1, false);
        }
        else {
            ui->tabWidget->hide();
        }
    }
}

void MainWindow::onImageSizeChanged(int size) {
    QString newText = "size: " + QString::number(size);
    ui->lbl_size->setText(newText);
}

void MainWindow::onImageClicked(const LayoutItem& image) {

    /*if (_wereHashed == false) {
        auto resPtr = hashImages(*_images.get());
        _imagesHashed.reset(resPtr);
        _wereHashed = true;
    }
    QList<cv::Mat>* similarImages = getSimilarImages(image);
    _view->clear();
    displayImages(*similarImages);*/
}



    /*QImage image = target.getPixmap().toImage();
    cv::Mat targetImage = ImageConverter::QImage2Mat(image);
    cv::imshow("target", targetImage);
    cv::waitKey(-1);
    cv::Mat targetHash = _imageRetrieval.getHash(targetImage);
    cv::imshow("hash", targetHash);
    cv::waitKey(-1);
    auto it = _imagesHashed->find(targetHash);
    cv::imshow("found", _imagesHashed->at(targetHash));
    cv::waitKey(-1);
    QList<cv::Mat>* res = new QList<cv::Mat>;
    /*res->push_front(it->second);
    return res;*/

    /*if (it == _imagesHashed->end()) {
        return *res;
    }*/

    /*int k = 1;
    auto rightIt = it;
    std::advance(rightIt, 1);
    auto leftIt = it;
    std::advance(leftIt, -1);
    while (k < 3) {
        if (rightIt != _imagesHashed->end()) {
            rightIt = std::next(rightIt, 1);
            cv::imshow("right", (*it).second);
            cv::waitKey(-1);
        }
        if (leftIt != _imagesHashed->begin()) {
            leftIt = std::prev(leftIt, 1);
            cv::imshow("left", (*it).second);
            cv::waitKey(-1);
            //std::advance(leftIt, -k);
        }
        double rightDist = _imageRetrieval.getDistance(targetHash, rightIt->first);
        double leftDist = _imageRetrieval.getDistance(targetHash, leftIt->first);
        (rightDist < leftDist) ? res->push_front(rightIt->second) : res->push_front(leftIt->second);
        ++k;
    }
    res->push_front(targetImage);
    return res;*/

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

