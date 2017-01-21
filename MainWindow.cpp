#include "MainWindow.hpp"
#include "ui_MainWindow.h"

QDir MainWindow::DIR;
int MainWindow::IMGHEIGHT;
int MainWindow::IMGWIDTH;
QDir MainWindow::SMALLIMGDIR;

typedef QFutureWatcher<QImage> FutureWatcher;

MainWindow::MainWindow(QWidget* parent) :
	QMainWindow(parent), ui(new Ui::MainWindow), _layout(new RingLayout) {
	ui->setupUi(this);
    this->_frame.setParent(this);
	//QThread::currentThread()->setPriority(QThread::HighPriority);
    //qDebug() << QImageReader::supportedImageFormats();

	connect(ui->btnLoad, &QPushButton::clicked, this, &MainWindow::onLoadImagesClick);
	connect(ui->btnSave, &QPushButton::clicked, this, &MainWindow::onSaveImagesClick);
	connect(ui->btnReverse, &QPushButton::clicked, this, &MainWindow::onReverseButtonClick);

    // http://doc.qt.io/qt-5/graphicsview.html#opengl-rendering
    //QGraphicsView::setViewport(new QGLWidget);
}

MainWindow::~MainWindow() {
	if (_futureResult.isRunning()) {
		_futureResult.cancel();
	}
    delete ui;
}

void MainWindow::onLoadImagesClick() {
	QFileDialog dialog;
	dialog.setFileMode(QFileDialog::DirectoryOnly);
	dialog.setOption(QFileDialog::ShowDirsOnly);
	dialog.setNameFilter(tr("Images (*.png *.jpg *.tiff *.bmp)"));
	dialog.setDirectory("/home/czimbortibor/images");
	QStringList fileName;
	if (dialog.exec()) {
		fileName = dialog.selectedFiles();
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
	/*MainWindow::imageWidth = (int) sqrt(nrOfPixels/nrOfImages) / 2;
	MainWindow::imageHeight = (int) sqrt(nrOfPixels/nrOfImages) / 2;*/

	// fixed image size
	_iconSize = 50;
	MainWindow::IMGWIDTH = _iconSize;
	MainWindow::IMGHEIGHT = _iconSize;

	int len = _imageNames.length();
	qDebug() << "image size =" << MainWindow::IMGWIDTH << "x" << MainWindow::IMGHEIGHT;
	qDebug() << "image count =" << len;

	_layout = QSharedPointer<RingLayout>::create();
	clearLayout(_layout.data());

	_scene = new QGraphicsScene;
	_form = new QGraphicsWidget;

	//scene->addItem(form);
	//view = new QGraphicsView;
	//viewPort = view->viewport();

	/*view->setScene(scene);
	view->showMaximized();*/

	ui->graphicsView->setScene(_scene);
	ui->graphicsView->show();

	_timer.start();
	/*
	_watcher = QSharedPointer<FutureWatcher>::create(this);
	_futureResult = QtConcurrent::mapped(_imageNames, loadImage);
	//connect(watcher, &FutureWatcher::resultReadyAt, this, &MainWindow::onImageReceive);
	connect(_watcher.data(), &FutureWatcher::resultsReadyAt, this, &MainWindow::onImagesReceive);
	connect(_watcher.data(), &FutureWatcher::finished, this, &MainWindow::onFinished);
	_watcher->setFuture(_futureResult);
	*/

	loadImages();
}

void MainWindow::loadImages() {
	for (const QString& fileName : _imageNames) {
		QString fullFileName = DIR.absoluteFilePath(fileName);
		cv::Mat cvImage = cv::imread(fullFileName.toStdString());
		QImage image = QImage((uchar*)cvImage.data, cvImage.cols, cvImage.rows, cvImage.step, QImage::Format_RGB888);
		_images.append(image.scaled(MainWindow::IMGWIDTH, MainWindow::IMGHEIGHT));
	}
	logTime("load time: ");

	_timer.start();
	for (const QImage image : _images) {
		LayoutItem* item = new LayoutItem(NULL, image);
		_layout->addItem(dynamic_cast<QGraphicsLayoutItem*>(item));
	}
	_form->setLayout(_layout.data());
	_scene->addItem(_form);

	logTime("display time: ");
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
	logTime("time needed to save the images: ");
}

QImage MainWindow::loadImage(const QString &imageName) {
	QString fileName = DIR.absoluteFilePath(imageName);

	cv::Mat cvImage = cv::imread(fileName.toStdString());
	/*if (cvImage.empty()) {
        return QImage();
	}*/
	//cv::Mat cvResizedImg;
	//cv::resize(cvImage, cvResizedImg, cv::Size(MainWindow::imageWidth, MainWindow::imageHeight));
    //QImage image = QImage((uchar*)cvResizedImg.data, cvResizedImg.cols, cvResizedImg.rows, cvResizedImg.step, QImage::Format_RGB888);
	QImage image = QImage((uchar*)cvImage.data, cvImage.cols, cvImage.rows, cvImage.step, QImage::Format_RGB888);
	return image.scaled(MainWindow::IMGWIDTH, MainWindow::IMGHEIGHT);
	//return image;

	/*QImage image(fileName);
	QImage resizedImg = image.scaled(MainWindow::imageWidth, MainWindow::imageHeight);
	return resizedImg;*/
}

void MainWindow::onImageReceive(int resultInd) {
    //LayoutItem* item = dynamic_cast<LayoutItem*>(futureResult.resultAt(resultInd));
    _images.append(_futureResult.resultAt(resultInd));
}

void MainWindow::onImagesReceive(int resultsBeginInd, int resultsEndInd) {
	for (int i = resultsBeginInd; i < resultsEndInd; ++i) {
        QImage image = _futureResult.resultAt(i);
        _images.append(image);
        //LayoutItem* item = new LayoutItem(NULL, image);
        //layout->addItem(item);
    }
    //ui->graphicsView->update();
    //viewPort->update();
}

void MainWindow::onReverseButtonClick() {
	//clearLayout(layout.data());
	//layout.clear();
	//layout = QSharedPointer<FlowLayout>::create();

	std::reverse(_images.begin(), _images.end());

	// display the images in reverse order
    /*for (const QImage image : images) {
        LayoutItem* item = new LayoutItem(NULL, image);
        layout->addItem(dynamic_cast<QGraphicsLayoutItem*>(item));
    }
	*/
}

void MainWindow::onFinished() {
	logTime("load time: ");

	_timer.start();
    for (const QImage image : _images) {
        LayoutItem* item = new LayoutItem(NULL, image);
        _layout->addItem(dynamic_cast<QGraphicsLayoutItem*>(item));
    }
	_form->setLayout(_layout.data());
    _scene->addItem(_form);

	logTime("display time: ");
}

void MainWindow::clearLayout(QGraphicsLayout* layout) {
	/*QGraphicsLayoutItem* item;
	while ((item = layout->itemAt(0)) != NULL) {
		layout->removeAt(0);
	}*/
	for (int i = 0; i < layout->count(); ++i) {
		layout->removeAt(i);
	}
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

QImage MainWindow::Mat2QImage(const cv::Mat &image) {
	return QImage((uchar*)image.data, image.cols, image.rows, image.step, QImage::Format_RGB888);
}

