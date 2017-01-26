#include "MainWindow.hpp"
#include "ui_MainWindow.h"

QDir MainWindow::DIR;
int MainWindow::IMGHEIGHT;
int MainWindow::IMGWIDTH;
QDir MainWindow::SMALLIMGDIR;

typedef QFutureWatcher<QImage> FutureWatcher;

MainWindow::MainWindow(QWidget* parent) :
	QMainWindow(parent), ui(new Ui::MainWindow) {
	ui->setupUi(this);
	_frame.setParent(this);
	//QThread::currentThread()->setPriority(QThread::HighPriority);
    //qDebug() << QImageReader::supportedImageFormats();

	connect(ui->btnLoad, &QPushButton::clicked, this, &MainWindow::onLoadImagesClick);
	connect(ui->btnSave, &QPushButton::clicked, this, &MainWindow::onSaveImagesClick);
	connect(ui->btnReverse, &QPushButton::clicked, this, &MainWindow::onReverseButtonClick);

    // http://doc.qt.io/qt-5/graphicsview.html#opengl-rendering
    //QGraphicsView::setViewport(new QGLWidget);

	initScene();
}

MainWindow::~MainWindow() {
	/*if (_futureResult.isRunning()) {
		_futureResult.cancel();
	}*/
    delete ui;
}

void MainWindow::initScene() {
	_layout = std::unique_ptr<RingLayout>(new RingLayout);
	_layout->setNrOfPetals(ui->spinBox_nrOfPetals->value());

	_scene = std::unique_ptr<QGraphicsScene>(new QGraphicsScene);
	_form = std::unique_ptr<QGraphicsWidget>(new QGraphicsWidget);

	ui->graphicsView->setScene(_scene.get());
	ui->graphicsView->show();
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

	MainWindow::DIR = QDir(fileName[0]);
	MainWindow::DIR.setFilter(QDir::Files);
	_imageNames = MainWindow::DIR.entryList();
	_nrOfImages = _imageNames.length();

	QDesktopWidget desktop;
	int screenWidth = desktop.geometry().width();
	int screenHeight = desktop.geometry().height();
	int nrOfPixels = screenHeight * screenWidth - (ui->frame_2->size().height() + ui->frame_2->size().width());

	// every image is visible on the screen
	/*
	MainWindow::IMGWIDTH = (int) sqrt(nrOfPixels/_nrOfImages) / 2;
	MainWindow::IMGHEIGHT = (int) sqrt(nrOfPixels/_nrOfImages) / 2;
	*/

	// fixed image size
	_iconSize = 50;
	MainWindow::IMGWIDTH = _iconSize;
	MainWindow::IMGHEIGHT = _iconSize;

	int len = _imageNames.length();
	qDebug() << "image size =" << MainWindow::IMGWIDTH << "x" << MainWindow::IMGHEIGHT;
	qDebug() << "image count =" << len;

	clearLayout();

	_timer.start();
	/*
	//_watcher = QSharedPointer<FutureWatcher>::create(this);
	_watcher = std::unique_ptr<FutureWatcher>(new FutureWatcher);
	_futureResult = QtConcurrent::mapped(_imageNames, loadImage);
	//connect(_watcher, &FutureWatcher::resultReadyAt, this, &MainWindow::onImageReceive);
	connect(_watcher.get(), &FutureWatcher::resultsReadyAt, this, &MainWindow::onImagesReceive);
	connect(_watcher.get(), &FutureWatcher::finished, this, &MainWindow::onFinished);
	_watcher->setFuture(_futureResult);
	*/

	_layout->setNrOfPetals(ui->spinBox_nrOfPetals->value());
	loadImages();
}

void MainWindow::loadImages() {
	for (const QString& fileName : _imageNames) {
		QString fullFileName = DIR.absoluteFilePath(fileName);
		// OpenCV read and OpenCV resize
		cv::Mat cvImage = cv::imread(fullFileName.toStdString());
		cv::Mat cvResizedImg;
		cv::resize(cvImage, cvResizedImg, cv::Size(MainWindow::IMGWIDTH, MainWindow::IMGHEIGHT));
		QImage image = Mat2QImage(cvResizedImg);
		_images.append(image);

		// OpenCV read and QImage resize
		/*cv::Mat cvImage = cv::imread(fullFileName.toStdString());
		QImage image = Mat2QImage(cvImage);
		_images.append(image.scaled(MainWindow::IMGWIDTH, MainWindow::IMGHEIGHT));*/

		// QImage read and QImage resize
		/*QImage image(fullFileName);
		_images.append(image.scaled(MainWindow::IMGWIDTH, MainWindow::IMGHEIGHT));*/
	}
	logTime("load time:");

	_timer.start();
	for (const QImage image : _images) {
		LayoutItem* item = new LayoutItem(NULL, image);
		_layout->addItem(dynamic_cast<QGraphicsLayoutItem*>(item));
	}
	_form->setLayout(_layout.get());
	_scene->addItem(_form.get());

	logTime("display time:");
}

void MainWindow::onSaveImagesClick() {
	DIR.cdUp();
	QString currentDir = DIR.absolutePath();
	if (!QDir(currentDir + "/smallImages").exists()) {
		QDir().mkdir(currentDir + "/smallImages");
	}
	else {
		SMALLIMGDIR.removeRecursively();
	}
	SMALLIMGDIR = QDir(currentDir + "/smallImages");

	qDebug() << "saving the " << _iconSize << "x" << _iconSize << "icons to: " + SMALLIMGDIR.absolutePath();
	_timer.start();
	for (int i = 0; i < _images.length(); ++i) {
		QString newFileName = (SMALLIMGDIR.absolutePath() + "/" + _imageNames[i]);
		_images[i].save(newFileName);
	}
	logTime("time needed to save the images:");
}

QImage MainWindow::loadImage(const QString &imageName) {
	QString fileName = DIR.absoluteFilePath(imageName);

	// OpenCV read and OpenCV resize
	cv::Mat cvImage = cv::imread(fileName.toStdString());
	cv::Mat cvResizedImg;
	cv::resize(cvImage, cvResizedImg, cv::Size(MainWindow::IMGWIDTH, MainWindow::IMGHEIGHT));
	QImage image = Mat2QImage(cvResizedImg);
	return image;

	// OpenCV read and QImage resize
	/*cv::Mat cvImage = cv::imread(fullFileName.toStdString());
	QImage image = Mat2QImage(cvImage);
	return image;*/

	// QImage read and QImage resize
	/*QImage image(fullFileName);
	return image; */
}

void MainWindow::onImageReceive(int resultInd) {
    _images.append(_futureResult.resultAt(resultInd));
}

void MainWindow::onImagesReceive(int resultsBeginInd, int resultsEndInd) {
	for (int i = resultsBeginInd; i < resultsEndInd; ++i) {
        QImage image = _futureResult.resultAt(i);
        _images.append(image);
        //LayoutItem* item = new LayoutItem(NULL, image);
        //layout->addItem(item);
    }
}

void MainWindow::onReverseButtonClick() {
	clearLayout();
	std::reverse(_images.begin(), _images.end());

	// display the images in reverse order
	_timer.start();
	for (const QImage image : _images) {
		LayoutItem* item = new LayoutItem(NULL, image);
		_layout->addItem(dynamic_cast<QGraphicsLayoutItem*>(item));
	}

	logTime("display time:");
}

void MainWindow::onFinished() {
	logTime("load time:");

	_timer.start();
    for (const QImage image : _images) {
        LayoutItem* item = new LayoutItem(NULL, image);
        _layout->addItem(dynamic_cast<QGraphicsLayoutItem*>(item));
    }
	_form->setLayout(_layout.get());
	_scene->addItem(_form.get());

	logTime("display time:");
}

void MainWindow::clearLayout() {
	_layout->clearAll();
}

void MainWindow::logTime(QString message) {
	std::ofstream logFile;
	logFile.open("log.txt", std::ios::out | std::ios::app);
	double time;
	time = _timer.nsecsElapsed() / 1000000000.0;
	qDebug() << message << time << " seconds";
	logFile << "number of images: " << _nrOfImages << "\n";
	logFile << message.toStdString() << _nrOfImages << "\n";
	logFile.close();
}

QImage MainWindow::Mat2QImage(const cv::Mat& image) {
	//return QImage((uchar*)image.data, image.cols, image.rows, image.step, QImage::Format_RGB888).rgbSwapped();

	cv::Mat temp;
	// makes a copy in RGB format
	cvtColor(image, temp, CV_BGR2RGB);
	QImage dest((const uchar*) temp.data, temp.cols, temp.rows, temp.step, QImage::Format_RGB888);
	// enforce deep copy
	dest.bits();
	return dest;
}
