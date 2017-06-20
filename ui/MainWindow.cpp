#include "MainWindow.hpp"
#include "ui_MainWindow.h"


using CollectionMap = std::map<GraphicsImage, cv::Mat, CBIR::MatCompare>;

std::string Logger::file_name = "log_file.txt";

MainWindow::MainWindow(QWidget* parent) :
    QMainWindow(parent), ui(new Ui::MainWindow) {
	ui->setupUi(this);


	Logger::log("\t ------------ Application started ------------- ");

	initDb();
    initWindow();
    initHashes();
    initView();
}

MainWindow::~MainWindow() {
	Logger::log("\t ----------- Application closed -------------- ");
    if (_loadingHandler) {
        _loadingHandler->onCancel();
    }
	_images.reset();
    delete ui;
}

void MainWindow::initDb() {
	_dbContext.init();
	auto connection = DbContext::MongoAccess::instance().get_connection();
	if (!connection) {
		QMessageBox::warning(this, "Warning", "Database connection error!");
	}
	Logger::log("succesful database connection");

	// initialize the collections
	auto db = connection->database(_dbContext.databaseName);
	_dbContext.feedsCollection = db[_dbContext.feedsCollection_name];
	_dbContext.feedsNameCollection = db[_dbContext.feedsNameCollection_name];
	_dbContext.imageCollection = db[_dbContext.imageCollection_name];
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
	connect(ui->btn_selectedImages, &QPushButton::clicked, this, &MainWindow::on_btn_selectedImages_clicked);

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

    // ---------- filters -----------
    ui->widget_createFilters->hide();
	_filters.insert("date range", new DateFilter(_dbContext));
	_filters.insert("text filters", new TextFilter(_dbContext));
    _filterList = new QListWidget(ui->widget_createFilters);
	for (const auto& filterName : _filters.keys()) {
		_filterList->addItem(filterName);
    }
	//ui->widget_createFilters->setMinimumSize(_filterList->size());
    connect(_filterList, &QListWidget::itemDoubleClicked, this, &MainWindow::onAddNewFilter);

    ui->btn_applyFilters->hide();

	ui->tableWidget_metadata->hide();
}

void MainWindow::initHashes() {
    // store the available hashing algorithms
    ui->comboBox_hashes->addItems(_imageCollection.getHashingAlgorithms());
}

void MainWindow::initView() {
	_view = new GraphicsView;
	ui->centralWidget->layout()->addWidget(_view);
	_view->show();

	connect(this, &MainWindow::addViewItem, _view, &GraphicsView::onAddItem);
	connect(this, &MainWindow::display, [&](const QList<GraphicsImage>& images) {
		_imageDisplayer.reset(new QFuture<void>(
								  QtConcurrent::run(this, &MainWindow::displayImages, images)));
	});
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

    _nrOfImages = _dir.entryList().length();
	_images = std::shared_ptr<QList<GraphicsImage>>(new QList<GraphicsImage>());

    _iconSize = ui->slider_imgSize->value();
    _imgWidth = _iconSize;
    _imgHeight = _iconSize;

    int len = _nrOfImages;
	Logger::log("image size = " + std::to_string(_imgWidth) + "x" + std::to_string(_imgHeight));
	Logger::log("image count = " + std::to_string(len));

	// TODO: recursive dir read http://doc.qt.io/qt-5/qdiriterator.html
	/*_dirSmallImg = _dir;
	_dirSmallImg.cdUp();
	const QString dirName = _dirSmallImg.dirName();
	const QString originalDir = _dirSmallImg.absolutePath() + "/collections";
	// check if the collection directory exists /
	if (!QDir(originalDir).exists()) {
		_dirSmallImg.mkdir("collections");
	}
	_dirSmallImg.cd("collections");
	const QString collectionDir = _dir.dirName() + "_" + QString::number(ui->slider_imgSize->value());
	// check if the collection exists
	if (QDir(collectionDir).exists()) {
		Logger::log("image collection already exists, reading from that...");
		_dirSmallImg.cd(collectionDir);
	}
	else {
		_dirSmallImg.mkdir(collectionDir);
		_dirSmallImg = _dir;
	}

	_dir = _dirSmallImg;*/
    _imageNames = std::unique_ptr<QStringList>(new QStringList);
	*_imageNames.get() = _dir.entryList();

	_timer.start();

    _loadingHandler = std::unique_ptr<LoadingHandler>(new LoadingHandler(_imageCollection));
    _loadingHandler->setWidth(_imgWidth);
	_loadingHandler->setHeight(_imgHeight);

	connect(_loadingHandler.get(), &LoadingHandler::imageReady, this, &MainWindow::onImageReceived);

	/*connect(_loadingHandler.get(), &LoadingHandler::mt_imageReady,
			[this](const cv::Mat& image, const QString& url) {
		_images->append(image);
		LayoutItem* item = new LayoutItem(ImageConverter::Mat2QImage(image), url, "");
		connect(item, &LayoutItem::clicked, this, &MainWindow::onImageClicked);
		connect(item, &LayoutItem::hoverEnter, this, &MainWindow::onImageHoverEnter);
		connect(item, &LayoutItem::doubleClick, this, &MainWindow::onImageDoubleClicked);
		emit addViewItem(item);
		_progressBar->setValue(_progressBar->value() + 1);
	});*/
	connect(_loadingHandler.get(), &LoadingHandler::mt_imageReady, this, &MainWindow::onImageReceivedMT);

    connect(_loadingHandler.get(), &LoadingHandler::finishedLoading, this, &MainWindow::onFinishedLoading);
    connect(ui->btn_cancelLoad, &QPushButton::clicked, _loadingHandler.get(), &LoadingHandler::onCancel);
    connect(ui->btn_cancelLoad, &QPushButton::clicked, this, &MainWindow::onFinishedLoading);

	const QString originalDirPath = _dir.absolutePath();
	auto loaderPtr = _loadingHandler->loadImages_st(_dir.absolutePath(), _imageNames.get(), originalDirPath);
	//_loadingHandler->loadImages_mt(_dir.absolutePath(), *_imageNames.get());
	_images.reset(loaderPtr);

    ui->btn_cancelLoad->setVisible(true);

    showProgressBar(_imageNames->length(), "loading");
}

void MainWindow::onImageReceivedMT(const GraphicsImage& image, const QString& url) {
	_images->append(image);
	//LayoutItem* item = new LayoutItem(ImageConverter::Mat2QImage(image), url, "");
	connect(&image, &GraphicsImage::clicked, this, &MainWindow::onImageClicked);
	connect(&image, &GraphicsImage::hoverEnter, this, &MainWindow::onImageHoverEnter);
	connect(&image, &GraphicsImage::doubleClick, this, &MainWindow::onImageDoubleClicked);
	emit addViewItem(&image);
	_progressBar->setValue(_progressBar->value() + 1);
}

void MainWindow::onImageReceived(int index, const QString& url, const QString& originalUrl) {
	//LayoutItem* item = new LayoutItem(ImageConverter::Mat2QImage(_images->at(index)), url, originalUrl);
	const GraphicsImage* item = &_images->at(index);
	connect(item, &GraphicsImage::clicked, this, &MainWindow::onImageClicked);
	connect(item, &GraphicsImage::hoverEnter, this, &MainWindow::onImageHoverEnter);
	connect(item, &GraphicsImage::doubleClick, this, &MainWindow::onImageDoubleClicked);
    _view->addItem(item);
    _progressBar->setValue(_progressBar->value() + 1);
}

void MainWindow::onFinishedLoading() {
	logTime("load time:");
    _loadingHandler.release();
    ui->btn_cancelLoad->hide();
    _progressBar.reset(nullptr);
    connect(ui->btn_hash, &QPushButton::clicked, this, &MainWindow::onHashImages);

    saveImages(ui->slider_imgSize->value());

	QJsonArray raw_data = _dbContext.queryAll();
	_metadata.reset(&MetadataParser::getMetadata(raw_data));
}

void MainWindow::populateMetadataTable(const QList<Metadata>& metadata) {
	int col;
	int row = 0;
	for (const auto& data : metadata) {
		col = 0;
		ui->tableWidget_metadata->insertRow(ui->tableWidget_metadata->rowCount());
		for (const auto& header : data.keys()) {
			QTableWidgetItem* cell = new QTableWidgetItem(tr("%1").arg(QString::fromStdString(data[header])));
			ui->tableWidget_metadata->setItem(row, col, cell);
			++col;
		}
		++row;
	}
}

void MainWindow::onHashImages() {
    const QString hasherName = ui->comboBox_hashes->currentText();
	 //_hashedImages.release();
	 _images.reset(_imageCollection.getHashedImages(hasherName));
	//std::multimap<cv::Mat, cv::Mat, CBIR::MatCompare>* result = _imageCollection.getHashedImages(hasherName);
	//_hashedImages.reset(result);
    _view->clear();
	displayImages(*_images.get());
}

void MainWindow::imageSaving(int size) {
    for (int i = 0; i < _images->length(); ++i) {
		cv::Mat image = *_images->at(i).mat;
        cv::Mat resizedImg;
        cv::resize(image, resizedImg, cv::Size(size, size));
		QString fileName = (_dir.absolutePath() + QDir::separator() + _imageNames->at(i));
        cv::imwrite(fileName.toStdString(), resizedImg);
        emit saveProgress(i+1);
    }
}

void MainWindow::onSavingChange(int value) {
    _progressBar->setValue(_progressBar->value() + value);
}

void MainWindow::saveImages(int size) {
	QString currentDir(_dir.absolutePath());
	int nr_of_images = QDir(currentDir).entryList().length();
	if (!nr_of_images) {
		Logger::log("saving the " + std::to_string(size) + " icons to: " + currentDir.toStdString() + "...");
        _timer.start();

        showProgressBar(_images->length(), "saving images");
        connect(this, &MainWindow::saveProgress, _progressBar.get(), &QProgressBar::setValue);

        auto saver_fun = std::bind(&MainWindow::imageSaving, this, size);
        _imageSaver = std::unique_ptr<QFuture<void>>(new QFuture<void>(
                                                         QtConcurrent::run(saver_fun)));
        _saverWatcher.setFuture(*_imageSaver.get());
        connect(&_saverWatcher, &QFutureWatcher<void>::finished, this, &MainWindow::onFinishedSaving);
    }
}

void MainWindow::onFinishedSaving() {
	logTime("saving time: ");
    _progressBar.reset();
    _progressBar.release();
}

void MainWindow::onClearLayout() {
    _view->clear();
    //_images->clear();
    // releases any memory not required to store the items
   // _images->squeeze();
}

void MainWindow::displayImages(const QList<GraphicsImage>& images) {
	_view->clear();
	for (const auto& image : images) {
		const GraphicsImage* item = new GraphicsImage(image);
		/*connect(&image, &GraphicsImage::clicked, this, &MainWindow::onImageClicked);
		connect(&image, &GraphicsImage::hoverEnter, this, &MainWindow::onImageHoverEnter);
		connect(&image, &GraphicsImage::doubleClick, this, &MainWindow::onImageDoubleClicked);*/
		_view->addItem(item);
		//emit addViewItem(&image);
	}
}

void MainWindow::logTime(QString message) {
	double time = _timer.nsecsElapsed() / 1000000000.0;
	message = message + QString::number(time) + " seconds \n" + "number of images: " + QString::number(_nrOfImages);
	Logger::log(message.toStdString());
}

void MainWindow::onRadiusChanged(double value) {
	_view->setRadius(value);
	displayImages(*_images.get());
}

void MainWindow::onPetalNrChanged(int value) {
	_view->setNrOfPetals(value);
	displayImages(*_images.get());
}

void MainWindow::onSpiralDistanceChanged(int value) {
	_view->setSpiralDistance(value);
	displayImages(*_images.get());
}

void MainWindow::onSpiralTurnChanged(int value) {
	_view->setSpiralTurn(value);
	displayImages(*_images.get());
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

void MainWindow::onImageClicked(const QString& url) {
	ui->tableWidget_metadata->setVisible(ui->tableWidget_metadata->isVisible());
	ui->tableWidget_metadata->clear();

	QList<Metadata> data{_metadata->at(0)};
	ui->tableWidget_metadata->setVisible(true);
	populateMetadataTable(data);
}

void MainWindow::onImageDoubleClicked(const QString& url) {
	// if the images are still loading
	if (_progressBar) {
		return;
	}
	const QString& hasherName = ui->comboBox_hashes->currentText();
	QList<GraphicsImage>* results = _imageCollection.getSimilarImages(url, hasherName);
	displayImages(*results);
}

QImage MainWindow::loadImage(const QString& url) const {
    cv::Mat originalImage = cv::imread(url.toStdString());
    cv::Size imgSize = originalImage.size.operator()();
    QSize viewSize = _view->size();
    if (imgSize.height > viewSize.height() || imgSize.width > viewSize.width()) {
        cv::Mat resizedImg;
        cv::resize(originalImage, resizedImg, cv::Size(viewSize.width() - Qt::Horizontal, viewSize.height() - Qt::Vertical));
        return ImageConverter::Mat2QImage(resizedImg);
    }
    return ImageConverter::Mat2QImage(originalImage);
}

void MainWindow::onFinishedOneImageLoad() {
    QImage image = _oneImageLoader->result();
    QLabel* imageLabel = new QLabel();
    imageLabel->setScaledContents(false);
    QPixmap pixmap(QPixmap::fromImage(image));
    imageLabel->setPixmap(pixmap);
    imageLabel->setMask(pixmap.mask());
    imageLabel->setWindowFlags(Qt::Popup);
    _view->addPopupImage(imageLabel, _hoveredItem);
}

void MainWindow::onImageHoverEnter(const QString& url, GraphicsImage* item) {
    _hoveredItem = item;
    _oneImageLoader.reset(new QFuture<QImage>(QtConcurrent::run(this, &MainWindow::loadImage, url)));
    _oneImageWatcher.setFuture(*_oneImageLoader.get());
    connect(&_oneImageWatcher, &QFutureWatcher<QImage>::finished, this, &MainWindow::onFinishedOneImageLoad);
}

void MainWindow::onAddFilter() {
	ui->widget_createFilters->setVisible(!ui->widget_createFilters->isVisible());
	_filterList->show();
	ui->btn_applyFilters->setVisible(!ui->btn_applyFilters->isVisible());
}

void MainWindow::onAddNewFilter(QListWidgetItem* item) {
    ui->widget_createFilters->hide();
    // create the selected filter
	AbstractFilter* filter = _filters.value(item->text())->makeFilter(_dbContext);
    // put back the created filter
    _filters.insert(item->text(), filter);
	// remove the selected filter from the available list
	_filterList->takeItem(_filterList->row(item));

    QWidget* filterControl = filter->makeControl();
	QLabel* filterLabel = new QLabel(item->text());
	ui->widget_filters->layout()->addWidget(filterLabel);
    ui->widget_filters->layout()->addWidget(filterControl);

	QPushButton* removeButton = filter->removeButton();
	connect(removeButton, &QPushButton::clicked, [&]() {
		ui->widget_filters->layout()->removeWidget(filterLabel);
		ui->widget_filters->layout()->removeWidget(filterControl);
		//_filterList->addItem(item->text());
		//_filters.take(item->text());
	});

	TextFilter* textFilter = dynamic_cast<TextFilter*>(filter);
	if (textFilter) {
		connect(textFilter, &TextFilter::changed, [&](const QJsonArray& results) {
			if (results.size()) {
				QList<Metadata> metadata = MetadataParser::getMetadata(results);
				QList<GraphicsImage> filtered_images = MetadataParser::getImages(metadata, _imageCollection);
				displayImages(filtered_images);
			}
			/*else {
				emit displayImages(*_images.get());
			}*/
		});
	}
	else {
		DateFilter* dateFilter = dynamic_cast<DateFilter*>(filter);
		connect(dateFilter, &DateFilter::changed, [&](const QJsonArray& results) {

		});
	}
}

void MainWindow::on_btn_applyFilters_clicked() {

}

void MainWindow::on_btn_selectedImages_clicked() {
	QList<GraphicsImage> selectedImages = _view->getSelectedImages();

}
