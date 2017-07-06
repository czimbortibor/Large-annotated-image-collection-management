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
	auto try_connection = DbContext::MongoAccess::instance().try_get_connection();
	if (!try_connection) {
		Logger::log("database connection error");
		QMessageBox::warning(this, "Warning", "Database connection error!");
		this->close();
		QApplication::exit();
	}
	auto connection = DbContext::MongoAccess::instance().get_connection();
	Logger::log("succesful database connection");

	// initialize the collections
	auto db = connection->database(_dbContext.databaseName);
	_dbContext.feedsCollection = db[_dbContext.feedsCollection_name];
	_dbContext.feedsNameCollection = db[_dbContext.feedsNameCollection_name];
	_dbContext.imageCollection = db[_dbContext.imageCollection_name];
}

void MainWindow::initWindow() {

	// ------------- demo changes ------------------
	ui->btn_hash->setVisible(false);
	ui->btn_selectedImages->setVisible(false);

	// set the supported image formats
	for (const QByteArray& item : QImageReader::supportedImageFormats()) {
		_supportedImgFormats.append("*." + item);
	}

	connect(this, &MainWindow::clearLayout, this, &MainWindow::onClearLayout);

	// buttons
	connect(ui->btn_load, &QPushButton::clicked, this, &MainWindow::onLoadImagesClick);
	connect(ui->btn_clear, &QPushButton::clicked, this, &MainWindow::onClearLayout);
	connect(ui->btn_selectedImages, &QPushButton::clicked, this, &MainWindow::on_btn_selectedImages_clicked);
	connect(ui->btn_clearSelections, &QPushButton::clicked, this, &MainWindow::on_btn_clearSelections_clicked);

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
	_progressBar = std::unique_ptr<QProgressBar>(new QProgressBar());
    _progressBar->setMaximum(maximumValue);
    QLabel* taskLabel = new QLabel(taskName);
	//ui->dockWidgetContents_mainControls->layout()->addWidget(taskLabel);
	//ui->dockWidgetContents_mainControls->layout()->addWidget(_progressBar.get());
	ui->centralWidget->layout()->addWidget(taskLabel);
	ui->centralWidget->layout()->addWidget(_progressBar.get());
    connect(_progressBar.get(), &QProgressBar::destroyed, taskLabel, &QLabel::deleteLater);
}

void MainWindow::onLoadImagesClick() {
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
	_imageNames.reset(new QStringList());
	QDirIterator dirIterator(_dir.absolutePath(), _supportedImgFormats, QDir::Files, QDirIterator::Subdirectories);
	while (dirIterator.hasNext()) {
		_imageNames->append(dirIterator.next());
	}
	_nrOfImages = _imageNames->length();
	if (_nrOfImages == 0) {
		showAlertDialog();
		return;
	}

    _iconSize = ui->slider_imgSize->value();
    _imgWidth = _iconSize;
    _imgHeight = _iconSize;
	ui->tableWidget_metadata->setIconSize(QSize(_imgWidth, _imgHeight));

	Logger::log("icon size = " + std::to_string(_imgWidth) + "x" + std::to_string(_imgHeight));
	Logger::log("file count = " + std::to_string(_nrOfImages));

	_timer.start();

	_images.reset(new QList<GraphicsImage>());

	_loadingHandler = std::unique_ptr<LoadingHandler>(new LoadingHandler(_imageCollection));
    _loadingHandler->setWidth(_imgWidth);
	_loadingHandler->setHeight(_imgHeight);

	connect(_loadingHandler.get(), &LoadingHandler::imageReady_st, this, &MainWindow::onImageReceivedST);

	connect(_loadingHandler.get(), &LoadingHandler::imageReady_mt, this, &MainWindow::onImageReceivedMT);

    connect(_loadingHandler.get(), &LoadingHandler::finishedLoading, this, &MainWindow::onFinishedLoading);
    connect(ui->btn_cancelLoad, &QPushButton::clicked, _loadingHandler.get(), &LoadingHandler::onCancel);
    connect(ui->btn_cancelLoad, &QPushButton::clicked, this, &MainWindow::onFinishedLoading);

	_view->clear();
	if (ui->comboBox_thread->currentText() == "single thread") {
		auto loaderPtr = _loadingHandler->loadImages_st(_imageNames.get());
		_images.reset(loaderPtr);
	}
	else {
		_loadingHandler->loadImages_mt(_imageNames.get());
	}

    ui->btn_cancelLoad->setVisible(true);

    showProgressBar(_imageNames->length(), "loading");
}

void MainWindow::onImageReceivedMT(const GraphicsImage& image) {
	_images->append(image);
	const GraphicsImage* item = new GraphicsImage(image);
	connect(item, &GraphicsImage::clicked, this, &MainWindow::onImageClicked);
	connect(item, &GraphicsImage::hoverEnter, this, &MainWindow::onImageHoverEnter);
	connect(item, &GraphicsImage::doubleClick, this, &MainWindow::onImageDoubleClicked);
	//emit addViewItem(item);
	_view->addItem(item);
	if (_progressBar) {
		_progressBar->setValue(_progressBar->value() + 1);
	}
}

void MainWindow::onImageReceivedST(int index) {
	const GraphicsImage* item = &_images->at(index);
	connect(item, &GraphicsImage::clicked, this, &MainWindow::onImageClicked);
	connect(item, &GraphicsImage::hoverEnter, this, &MainWindow::onImageHoverEnter);
	connect(item, &GraphicsImage::doubleClick, this, &MainWindow::onImageDoubleClicked);
    _view->addItem(item);
	if (_progressBar) {
		_progressBar->setValue(_progressBar->value() + 1);
	}
}

void MainWindow::onFinishedLoading() {
	logTime("load time:");
    ui->btn_cancelLoad->hide();
	_progressBar.reset();
    connect(ui->btn_hash, &QPushButton::clicked, this, &MainWindow::onHashImages);

    saveImages(ui->slider_imgSize->value());

	//QJsonArray raw_data = _dbContext.queryAll();
	//_metadata.reset(&MetadataParser::getMetadata(raw_data));
}

void MainWindow::populateMetadataTable(const QList<Metadata>& metadata, const QList<GraphicsImage>& images) {
	int col;
	int row = 0;
	for (const auto& data : metadata) {
		col = 1;
		ui->tableWidget_metadata->insertRow(ui->tableWidget_metadata->rowCount());
		QTableWidgetItem* icon_cell = new QTableWidgetItem();
		const QString image_path = images[row].getUrl();
		icon_cell->setIcon(QIcon(image_path));
		icon_cell->setToolTip(image_path);
		ui->tableWidget_metadata->setItem(row, 0, icon_cell);
		for (const auto& header : data.keys()) {
			QTableWidgetItem* cell = new QTableWidgetItem(tr("%1").arg(QString::fromStdString(data[header])));
			ui->tableWidget_metadata->setItem(row, col, cell);
			++col;
		}
		++row;
	}
	ui->tableWidget_metadata->resizeRowsToContents();
}

void MainWindow::onHashImages() {
	const QString hasherName = ui->comboBox_hashes->currentText();
	 QList<GraphicsImage>* results = _imageCollection.getHashedImages(hasherName);
	displayImages(*results);
}

void MainWindow::imageSaving(int size) {
	/*for (int i = 0; i < _images->length(); ++i) {
		cv::Mat image = *_images->at(i)->mat;
        cv::Mat resizedImg;
        cv::resize(image, resizedImg, cv::Size(size, size));
		QString fileName = (_dir.absolutePath() + QDir::separator() + _imageNames->at(i));
        cv::imwrite(fileName.toStdString(), resizedImg);
        emit saveProgress(i+1);
	}*/
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
		connect(item, &GraphicsImage::clicked, this, &MainWindow::onImageClicked);
		connect(item, &GraphicsImage::hoverEnter, this, &MainWindow::onImageHoverEnter);
		connect(item, &GraphicsImage::doubleClick, this, &MainWindow::onImageDoubleClicked);
		_view->addItem(item);
		//emit addViewItem(&image);
	}
}

void MainWindow::displayOriginalImages(const QList<GraphicsImage>& images) {
	_view->clear();
	for (const auto& image : images) {
		const GraphicsImage* item = new GraphicsImage(image);
		_view->addItem(item);
		//emit addViewItem(item);
	}
}

void MainWindow::logTime(QString message) {
	double time = _timer.nsecsElapsed() / 1000000000.0;
	message = message + QString::number(time) + " seconds \n" + "number of images: " + QString::number(_nrOfImages);
	Logger::log(message.toStdString());
}

void MainWindow::onRadiusChanged(double value) {
	if (_images) {
		_view->setRadius(value);
		displayImages(*_images.get());
	}
}

void MainWindow::onPetalNrChanged(int value) {
	if (_images) {
		_view->setNrOfPetals(value);
		displayImages(*_images.get());
	}
}

void MainWindow::onSpiralDistanceChanged(int value) {
	if (_images) {
		_view->setSpiralDistance(value);
		displayImages(*_images.get());
	}
}

void MainWindow::onSpiralTurnChanged(int value) {
	if (_images) {
		_view->setSpiralTurn(value);
		displayImages(*_images.get());
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

void MainWindow::onImageClicked(const QString& url) {
	ui->tableWidget_metadata->setVisible(true);
	// deletes all the rows
	ui->tableWidget_metadata->setRowCount(0);

	QList<GraphicsImage> selectedImages = _view->getSelectedImages();
	QStringList selectedPaths;
	for (const auto& image : selectedImages) {
		selectedPaths.append(image.getUrl());
	}
	QJsonArray results;
	if (selectedPaths.length()) {
		if (selectedPaths.length() == 1) {
			results = _dbContext.queryImagePath(selectedPaths[0]);
		}
		else {
			results = _dbContext.queryImagePaths(selectedPaths);
		}
		QList<Metadata> metadata = MetadataParser::getMetadata(results);
		populateMetadataTable(metadata, selectedImages);
	}
}

void MainWindow::onImageDoubleClicked(const QString& url) {
	// if the images are still loading
	if (_progressBar) {
		return;
	}
	const QString& hasherName = ui->comboBox_hashes->currentText();

	// TODO: get similars to only the visible images
	QStringList urls;
	//for (const auto& image : _filteredImages) {

	//}

	QList<GraphicsImage>* results = _imageCollection.getSimilarImages(url, hasherName);

	/* format the display
	 *  grid: selected image goes to the first place (default list order)
	 *  spiral: selected image goes to the center of the spiral (reversed list order)
	 *  petal: selected image stays in place and the similar ones next to it
	*/

	const QString layout_type = ui->comboBox_layout->currentText();
	if (layout_type == "spiral") {
		std::reverse(results->begin(), results->end());
	}
	// TODO: selection in petal layout format
	else if (layout_type == "petal") {
		/*QList<GraphicsImage>* modifiedList = new QList<GraphicsImage>();
		for (int i = 0; i < results->length() - 1; i += 2) {
			modifiedList->append(results->at(i));
			modifiedList->prepend(results->at(i+1));
		}
		results = modifiedList;*/
	}
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

	QPushButton* removeButton = &filter->removeButton();
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
			else {
				displayImages(*_images.get());
			}
		});
	}
	else {
		DateFilter* dateFilter = dynamic_cast<DateFilter*>(filter);
		connect(dateFilter, &DateFilter::datesChanged, [&](const QJsonArray& results) {
			QList<Metadata> metadata = MetadataParser::getMetadata(results);
			QList<GraphicsImage> filtered_images = MetadataParser::getImages(metadata, _imageCollection);
			displayImages(filtered_images);
		});
	}
}

void MainWindow::on_btn_selectedImages_clicked() {
	QList<GraphicsImage> selectedImages = _view->getSelectedImages();

}

void MainWindow::on_btn_clearSelections_clicked() {
	for (auto& image : _view->scene().selectedItems()) {
		image->setSelected(false);
		image->setGraphicsEffect(nullptr);
	}
	ui->tableWidget_metadata->clearContents();
	ui->tableWidget_metadata->setRowCount(0);
}
