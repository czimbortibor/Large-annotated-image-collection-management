#include "MainWindow.hpp"
#include "ui_MainWindow.h"

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
	connect(ui->btn_save, &QPushButton::clicked, this, &MainWindow::onSaveImagesClick);
	connect(ui->btn_clear, &QPushButton::clicked, this, &MainWindow::onClearLayout);
	connect(ui->btn_reverse, &QPushButton::clicked, this, &MainWindow::onReverseButtonClick);

	// spinboxes
	connect(ui->spinBox_radius, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &MainWindow::onRadiusChanged);
	connect(ui->spinBox_nrOfPetals, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &MainWindow::onPetalNrChanged);
		// overloaded signal -> have to specify the specific function syntax

	// filter fields
	connect(ui->comboBox_layout, static_cast<void(QComboBox::*)(const QString&)>(&QComboBox::currentIndexChanged), this, &MainWindow::onLayoutChanged);
	connect(ui->btn_addFilter, &QPushButton::clicked, this, &MainWindow::onFiltersClicked);
	ui->groupBox_layoutControls->hide();

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
	if (_futureLoaderWatcher.isRunning()) {
		_futureLoaderWatcher.cancel();
	}
	if (_futureLoaderWatcherMT->isRunning()) {
		_futureLoaderWatcherMT->cancel();
	}
	if (_futureResizerWatcherMT.isRunning()) {
		_futureResizerWatcherMT.cancel();
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

void MainWindow::showAlertDialog() {
	QMessageBox msgBox;
	msgBox.setIcon(QMessageBox::Warning);
	msgBox.setText("The selected directory does not contain any supported image files!");

	auto concat = [](QString str1, QString str2) -> QString { return str1 + ", " + str2; };
	QString imageFormats = std::accumulate(_supportedImgFormats.begin(), _supportedImgFormats.end(), _supportedImgFormats[0], concat);
	msgBox.setDetailedText("Supported image formats are:\n" + imageFormats);

	msgBox.exec();
}

void MainWindow::onLoadImagesClick() {
	if (_imageNames) {
		clearLayout();
	}
	QFileDialog dialog;
	dialog.setFileMode(QFileDialog::DirectoryOnly);
	dialog.setOption(QFileDialog::ShowDirsOnly);
	dialog.setDirectory("/home/czimbortibor/images");
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
	_imagesOriginal = std::unique_ptr<QVector<cv::Mat>>(new QVector<cv::Mat>);
	_imagesResized = std::unique_ptr<QVector<cv::Mat>>(new QVector<cv::Mat>);
	_nrOfImages = _imageNames->length();

	// every image is visible on the screen
	/*
	QDesktopWidget desktop;
	int screenWidth = desktop.geometry().width();
	int screenHeight = desktop.geometry().height();
	int nrOfPixels = screenHeight * screenWidth - (ui->frame_2->size().height() + ui->frame_2->size().width());
	_imgWidth = (int) sqrt(nrOfPixels/_nrOfImages) / 2;
	_imgHeight = (int) sqrt(nrOfPixels/_nrOfImages) / 2;
	*/

	// fixed image size
	_iconSize = 150;
	MainWindow::_imgWidth = _iconSize;
	MainWindow::_imgHeight = _iconSize;

	int len = _imageNames->length();
	qDebug() << "image size =" << _imgWidth << "x" << _imgHeight;
	qDebug() << "image count =" << len;

	_timer.start();
	//_layout->setNrOfPetals(ui->spinBox_nrOfPetals->value());
	//_layout->setRadius(ui->spinBox_radius->value());

	// ------------- single-threaded image load --------------------
	/*_futureLoader = QtConcurrent::run(this, &MainWindow::loadImages);
	_futureLoaderWatcher.setFuture(_futureLoader);
	connect(&_futureLoaderWatcher, &QFutureWatcher<void>::finished, this, &MainWindow::onFinishedLoading);
	*/

	// ------------- multi-threaded image load ---------------------
	auto fun = std::bind(&MainWindow::loadImage, this, std::placeholders::_1);
		// this: the hidden this parameter for member functions, placeholders::_1 = <const QString& fileName>
	_futureLoaderMT = std::unique_ptr<QFuture<cv::Mat>>(new QFuture<cv::Mat>(QtConcurrent::mapped(*_imageNames.get(), fun)));
	_futureLoaderWatcherMT = std::unique_ptr<QFutureWatcher<cv::Mat>>(new QFutureWatcher<cv::Mat>);
	_futureLoaderWatcherMT->setFuture(*_futureLoaderMT.get());
	//connect(_futureLoaderWatcherMT.get(), &QFutureWatcher<cv::Mat>::finished, this, &MainWindow::onFinishedLoading);
	connect(_futureLoaderWatcherMT.get(), &QFutureWatcher<cv::Mat>::resultsReadyAt, this, &MainWindow::onImagesReceive);
}

void MainWindow::loadImages() {
	for (const QString& fileName : *_imageNames.get()) {
		QString fullFileName = _dir.absoluteFilePath(fileName);
		// OpenCV read
		cv::Mat cvImage = cv::imread(fullFileName.toStdString());
		_imagesOriginal->append(cvImage);

		// QImage read
		/*QImage image(fullFileName);
		_imagesOriginal->append(image);*/
	}
}

cv::Mat MainWindow::loadImage(const QString &imageName) const {
	QString fileName = _dir.absoluteFilePath(imageName);
	cv::Mat cvImage = cv::imread(fileName.toStdString());
	return cvImage;
}

void MainWindow::onImageReceive(int resultInd) {
	//_imagesOriginal->append(_futureLoaderWatcherMT.resultAt(resultInd));
}

void MainWindow::onImagesReceive(int resultsBeginInd, int resultsEndInd) {
	for (int i = resultsBeginInd; i < resultsEndInd; ++i) {
		cv::Mat image = _futureLoaderWatcherMT->resultAt(i);
		_imagesOriginal->append(image);

		cv::Mat cvResizedImg;
		cv::resize(image, cvResizedImg, cv::Size(_imgWidth, _imgHeight));
		_imagesResized->append(cvResizedImg);

		// ----------- hash ------------

		LayoutItem* item = new LayoutItem(NULL, Mat2QImage(cvResizedImg));
		_view->addItem(item);
	}
}

void MainWindow::onFinishedLoading() {
	logTime("load time:");

	_timer.start();
	/*int newWidth = _imgWidth;
	int newHeight = _imgHeight;
	auto fun = std::bind(&MainWindow::resizeImage, this, std::placeholders::_1, newWidth, newHeight);
		// this: the hidden this parameter for member functions, placeholders::_1 = <const cv::Mat& image>
	_futureResizerMT = std::unique_ptr<QFuture<cv::Mat>>(new QFuture<cv::Mat>(QtConcurrent::mapped(*_imagesOriginal.get(), fun)));
	_futureResizerWatcherMT.setFuture(*_futureResizerMT.get());
	//connect(&_futureResizerWatcherMT, &QFutureWatcher<QImage>::resultsReadyAt, this, &MainWindow::onImagesResized);
	connect(&_futureResizerWatcherMT, &QFutureWatcher<QImage>::finished, this, &MainWindow::onFinishedResizing);
	*/
	//displayImages();
	//logTime("display time:");

	// ------ opencv img_hash ------- : https://github.com/stereomatchingkiss/opencv_contrib/tree/img_hash/modules/img_hash
	 cv::Ptr<cv::img_hash::PHash> hasher = cv::img_hash::PHash::create();
	// cv::Ptr<cv::img_hash::AverageHash> hasher = cv::img_hash::AverageHash::create();
	// cv::Ptr<cv::img_hash::MarrHildrethHash> hasher = cv::img_hash::MarrHildrethHash::create();
	// cv::Ptr<cv::img_hash::RadialVarianceHash> hasher = cv::img_hash::RadialVarianceHash::create();
	auto mapPtr = &imageRetrieval.computeHashes(*_imagesResized.get(), hasher);
	_imagesHashed = std::unique_ptr<std::multimap<double, const cv::Mat>>(mapPtr);
	displayImages<std::multimap<double, const cv::Mat>>(*_imagesHashed.get());
	//_imagesHashed = std::unique_ptr<std::multimap<const cv::Mat, const cv::Mat, CBIR::MatCompare>>(mapPtr);
	//displayImages<std::multimap<const cv::Mat, const cv::Mat, CBIR::MatCompare>>(*_imagesHashed.get());


	// ------ pHash -------
	/*auto mapPtr = &imageRetrieval.computeHashes_pHash(*_imagesResized.get(), _dir.absolutePath(), *_imageNames.get());
	_imagesHashed_pHash = std::unique_ptr<std::multimap<ulong64, const cv::Mat, CBIR::HashCompare>>(mapPtr);
	displayImages<std::multimap<ulong64, const cv::Mat, CBIR::HashCompare>>(*_imagesHashed_pHash.get());
	*/
}

void MainWindow::resizeImages(int size) {
	Q_UNUSED(size)
	for (const cv::Mat& image : *_imagesOriginal.get()) {
		cv::Mat cvResizedImg;
		cv::resize(image, cvResizedImg, cv::Size(_imgWidth, _imgHeight));
		_imagesResized->append(cvResizedImg);
	}
}

cv::Mat MainWindow::resizeImage(const cv::Mat& image, int newWidth, int newHeight) const {
	cv::Mat resizedImg;
	cv::resize(image, resizedImg, cv::Size(newWidth, newHeight));
	return resizedImg;
}

void MainWindow::onImagesResized(int resultsBeginInd, int resultsEndInd) {
	for (int i = resultsBeginInd; i < resultsEndInd; ++i) {
		QImage res = Mat2QImage(_futureResizerWatcherMT.resultAt(i));
		LayoutItem* item = new LayoutItem(NULL, res);
		_view->addItem(static_cast<QGraphicsLayoutItem*>(item));
	}
}

void MainWindow::onFinishedResizing() {
	logTime("resize time:");

	_timer.start();
	for (const cv::Mat& image : *_futureResizerMT.get()) {
		QImage res = Mat2QImage(image);
		LayoutItem* item = new LayoutItem(NULL, res);
		_view->addItem(static_cast<QGraphicsLayoutItem*>(item));
	}
	logTime("display time:");

}

void MainWindow::onSaveImagesClick() {
	_dir.cdUp();
	QString currentDir = _dir.absolutePath();
	if (!QDir(currentDir + "/smallImages").exists()) {
		QDir().mkdir(currentDir + "/smallImages");
	}
	else {
		_dirSmallImg.removeRecursively();
	}
	_dirSmallImg = QDir(currentDir + "/smallImages");

	qDebug() << "saving the " << _iconSize << "x" << _iconSize << "icons to: " + _dirSmallImg.absolutePath();
	_timer.start();
	for (int i = 0; i < _imagesResized->length(); ++i) {
		QString newFileName = (_dirSmallImg.absolutePath() + "/" + _imageNames->at(i));
		//_imagesResized->at(i).save(newFileName);
		cv::imwrite(newFileName.toStdString(), _imagesResized->at(i));
	}
	logTime("time needed to save the images:");
}

void MainWindow::onReverseButtonClick() {
	_view->clear();
	std::reverse(_imagesResized->begin(), _imagesResized->end());

	_timer.start();
	displayImages(*_imagesResized.get());
	logTime("display time:");
}

void MainWindow::onClearLayout() {
	_view->clear();
	_imagesOriginal->clear();
	_imagesResized->clear();
}

void MainWindow::displayImages(const QVector<cv::Mat>& images) const {
	for (const auto& image : images) {
		QImage res = Mat2QImage(image);
		LayoutItem* item = new LayoutItem(NULL, res);
		_view->addItem(static_cast<QGraphicsLayoutItem*>(item));
	}
}

template<typename T>
void MainWindow::displayImages(const T& images) const {
	for (const auto& entry : images) {
		QImage image = Mat2QImage(entry.second);
		//qDebug() << entry.first;
		LayoutItem* item = new LayoutItem(NULL, image);
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

QImage MainWindow::Mat2QImage(const cv::Mat& image) const {
	//return QImage((uchar*)image.data, image.cols, image.rows, image.step, QImage::Format_RGB888).rgbSwapped();

	cv::Mat temp;
	// makes a copy in RGB format
	cvtColor(image, temp, CV_BGR2RGB);
	QImage dest((const uchar*) temp.data, temp.cols, temp.rows, temp.step, QImage::Format_RGB888);
	// enforce deep copy
	dest.bits();
	return dest;
}

void MainWindow::onRadiusChanged(double value) {
	_view->setRadius(value);
	if (_imageNames->length()) {
		_view->clear();
		displayImages(*_imagesResized.get());
	}
}

void MainWindow::onPetalNrChanged(int value) {
	_view->setNrOfPetals(value);
	if (_imageNames->length()) {
		_view->clear();
		displayImages(*_imagesResized.get());
	}
}

void MainWindow::onLayoutChanged(const QString& text) {
	_view->setLayout(text);
	text.compare("petal") == 0 ? ui->groupBox_layoutControls->setVisible(true) : ui->groupBox_layoutControls->hide();
}

void MainWindow::onFiltersClicked() {
	QListWidget* filterList = new QListWidget(ui->widget_filters);
	filterList->addItems(QStringList() << "keyword" << "date range" << "image");
	ui->widget_filters->setMinimumSize(filterList->size());
	//ui->widget_filters->layout()->addWidget(filterList);
	filterList->show();
}
