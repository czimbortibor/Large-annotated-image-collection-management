#include "MainWindow.hpp"
#include "ui_MainWindow.h"

MainWindow::MainWindow(QWidget* parent) :
	QMainWindow(parent), ui(new Ui::MainWindow) {
	ui->setupUi(this);
	_frame.setParent(this);
	//QThread::currentThread()->setPriority(QThread::HighPriority);

    //qDebug() << QImageReader::supportedImageFormats();

	connect(this, &MainWindow::clearLayout, this, &MainWindow::onClearLayout);

	// buttons
	connect(ui->btnLoad, &QPushButton::clicked, this, &MainWindow::onLoadImagesClick);
	connect(ui->btnSave, &QPushButton::clicked, this, &MainWindow::onSaveImagesClick);
	connect(ui->btnClear, &QPushButton::clicked, this, &MainWindow::onClearLayout);
	connect(ui->btnReverse, &QPushButton::clicked, this, &MainWindow::onReverseButtonClick);

	// spinboxes
	connect(ui->spinBox_radius, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &MainWindow::onRadiusChanged);
	connect(ui->spinBox_nrOfPetals, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &MainWindow::onPetalNrChanged);
		// overloaded signal -> have to specify the specific function syntax

    // http://doc.qt.io/qt-5/graphicsview.html#opengl-rendering
    //QGraphicsView::setViewport(new QGLWidget);

	initScene();
}

MainWindow::~MainWindow() {
	if (_futureLoaderWatcher.isRunning()) {
		_futureLoaderWatcher.cancel();
	}
	if (_futureLoaderWatcherMT.isRunning()) {
		_futureLoaderWatcherMT.cancel();
	}
	if (_futureResizerWatcherMT.isRunning()) {
		_futureResizerWatcherMT.cancel();
	}
    delete ui;
}

void MainWindow::initScene() {
	_scene = new QGraphicsScene;
	connect(_scene, &QGraphicsScene::changed, this, &MainWindow::onSceneChanged);
	_view = new QGraphicsView(_scene);
	ui->centralWidget->layout()->addWidget(_view);
	// align from the top-left corner rather then from the center
	_view->setAlignment(Qt::AlignLeft | Qt::AlignTop);

	_layoutWidget = new QGraphicsWidget;
	_layoutWidget->setMinimumSize(_view->size());
	_layout = new FlowLayout;
	_layoutWidget->setLayout(_layout);

	_scene->addItem(_layoutWidget);
	_view->show();
}

void MainWindow::onSceneChanged() {
	QSizeF viewSize = _view->size();
	qreal radius = ui->spinBox_radius->value();
	QSizeF newSize(viewSize.width() - radius, viewSize.height());
	_layoutWidget->setMinimumSize(newSize);
	/*if (_imageNames.length()) {
		clearLayout();
		displayImages();
	}*/
}

void MainWindow::onLoadImagesClick() {
	QFileDialog dialog;
	dialog.setFileMode(QFileDialog::DirectoryOnly);
	dialog.setOption(QFileDialog::ShowDirsOnly);
	//dialog.setNameFilter(tr("Images (*.png *.jpg *.tiff *.bmp)"));
	dialog.setDirectory("/home/czimbortibor/images");
	QStringList fileName;
	if (dialog.exec()) {
		fileName = dialog.selectedFiles();
	}
	if (!fileName.count()) {
		return;
	}

	_dir = QDir(fileName[0]);
	_dir.setFilter(QDir::Files);
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
	_futureLoaderMT = QtConcurrent::mapped(*_imageNames.get(), fun);
	_futureLoaderWatcherMT.setFuture(_futureLoaderMT);
	connect(&_futureLoaderWatcherMT, &QFutureWatcher<cv::Mat>::finished, this, &MainWindow::onFinishedLoading);
	connect(&_futureLoaderWatcherMT, &QFutureWatcher<cv::Mat>::resultsReadyAt, this, &MainWindow::onImagesReceive);
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
		cv::Mat image = _futureLoaderWatcherMT.resultAt(i);
		_imagesOriginal->append(image);

		cv::Mat cvResizedImg;
		cv::resize(image, cvResizedImg, cv::Size(_imgWidth, _imgHeight));
		_imagesResized->append(cvResizedImg);

		// ----------- hash ------------

		//LayoutItem* item = new LayoutItem(NULL, image);
		//layout->addItem(item);
	}
}

void MainWindow::onFinishedLoading() {
	logTime("load time:");

	_timer.start();
	int newWidth = _imgWidth;
	int newHeight = _imgHeight;
	auto fun = std::bind(&MainWindow::resizeImage, this, std::placeholders::_1, newWidth, newHeight);
		// this: the hidden this parameter for member functions, placeholders::_1 = <const cv::Mat& image>
	/*_futureResizerMT = QtConcurrent::mapped(*_imagesOriginal.get(), fun);
	_futureResizerWatcherMT.setFuture(_futureResizerMT);
	//connect(&_futureResizerWatcherMT, &QFutureWatcher<QImage>::resultsReadyAt, this, &MainWindow::onImagesResized);
	connect(&_futureResizerWatcherMT, &QFutureWatcher<QImage>::finished, this, &MainWindow::onFinishedResizing);
	*/
	//displayImages();
	//logTime("display time:");

	// ------ opencv img_hash -------
	auto mapPtr = &imageRetrieval.computeHashes(*_imagesResized.get());
	//_imagesHashed = std::unique_ptr<std::multimap<const cv::Mat, const cv::Mat, CBIR::MatCompare>>(mapPtr);
	//displayImages<std::multimap<const cv::Mat, const cv::Mat, CBIR::MatCompare>>(*_imagesHashed.get());
	_imagesHashed = std::unique_ptr<std::multimap<double, const cv::Mat>>(mapPtr);
	displayImages<std::multimap<double, const cv::Mat>>(*_imagesHashed.get());


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
		_layout->addItem(static_cast<QGraphicsLayoutItem*>(item));
	}
}

void MainWindow::onFinishedResizing() {
	logTime("resize time:");

	_timer.start();
	for (const cv::Mat& image : _futureResizerMT) {
		QImage res = Mat2QImage(image);
		LayoutItem* item = new LayoutItem(NULL, res);
		_layout->addItem(static_cast<QGraphicsLayoutItem*>(item));
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
	emit clearLayout();
	std::reverse(_imagesResized->begin(), _imagesResized->end());

	// display the images in reverse order
	_timer.start();
	//displayImages();
	logTime("display time:");
}

void MainWindow::onClearLayout() {
	_layout->clearAll();
	_imagesOriginal->clear();
	_imagesResized->clear();
}

void MainWindow::displayImages(const QVector<cv::Mat>& images) const {
	for (const auto& image : images) {
		QImage res = Mat2QImage(image);
		LayoutItem* item = new LayoutItem(NULL, res);
		_layout->addItem(static_cast<QGraphicsLayoutItem*>(item));
	}
}

template<typename T>
void MainWindow::displayImages(const T& images) const {
	for (const auto& entry : images) {
		QImage image = Mat2QImage(entry.second);
		//qDebug() << entry.first;
		LayoutItem* item = new LayoutItem(NULL, image);
		_layout->addItem(static_cast<QGraphicsLayoutItem*>(item));
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
	//_layout->setRadius(value);
	if (_imageNames->length()) {
		emit clearLayout();
		//displayImages();
	}
}

void MainWindow::onPetalNrChanged(int value) {
	//_layout->setNrOfPetals(value);
	if (_imageNames->length()) {
		emit clearLayout();
		//displayImages();
	}
}
