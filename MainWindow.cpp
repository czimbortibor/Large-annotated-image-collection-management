#include "MainWindow.hpp"
#include "ui_MainWindow.h"

QDir MainWindow::dir;
int MainWindow::imageHeight;
int MainWindow::imageWidth;
QDir MainWindow::smallImagesDir;

typedef QFutureWatcher<QImage> FutureWatcher;

MainWindow::MainWindow(QWidget* parent) :
	QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    this->frame.setParent(this);
	//QThread::currentThread()->setPriority(QThread::HighPriority);
    //qDebug() << QImageReader::supportedImageFormats();

	connect(ui->btnLoad, &QPushButton::clicked, this, &MainWindow::onLoadImagesClick);
	connect(ui->btnSave, &QPushButton::clicked, this, &MainWindow::onSaveImagesClick);
	connect(ui->btnReverse, &QPushButton::clicked, this, &MainWindow::onReverseButtonClick);

    // http://doc.qt.io/qt-5/graphicsview.html#opengl-rendering
    //QGraphicsView::setViewport(new QGLWidget);
}

MainWindow::~MainWindow() {
	if (futureResult.isRunning()) {
		futureResult.cancel();
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

	MainWindow::dir = QDir(fileName[0]);
	MainWindow::dir.setFilter(QDir::Files);
	imageNames = MainWindow::dir.entryList();
	nrOfImages = imageNames.length();

	QDesktopWidget desktop;
	int screenWidth = desktop.geometry().width();
	int screenHeight = desktop.geometry().height();
	int nrOfPixels = screenHeight * screenWidth - (ui->frame_2->size().height() + ui->frame_2->size().width());

	MainWindow::imageWidth = iconSize;//(int) sqrt(nrOfPixels/nrOfImages) / 2;
	MainWindow::imageHeight = iconSize; //(int) sqrt(nrOfPixels/nrOfImages) / 2;

	int len = imageNames.length();
	qDebug() << "image size =" << MainWindow::imageWidth << "x" << MainWindow::imageHeight;
	qDebug() << "image count =" << len;

	layout = QSharedPointer<FlowLayout>::create();

	scene = new QGraphicsScene;
	form = new QGraphicsWidget;

	//scene->addItem(form);
	//view = new QGraphicsView;
	//viewPort = view->viewport();

	/*view->setScene(scene);
	view->showMaximized();*/

	ui->graphicsView->setScene(scene);
	ui->graphicsView->show();

	timer.start();
	watcher = QSharedPointer<FutureWatcher>::create(this);
	futureResult = QtConcurrent::mapped(imageNames, loadImage);
	//connect(watcher, &FutureWatcher::resultReadyAt, this, &MainWindow::onImageReceive);
	connect(watcher.data(), &FutureWatcher::resultsReadyAt, this, &MainWindow::onImagesReceive);
	connect(watcher.data(), &FutureWatcher::finished, this, &MainWindow::onFinished);

	watcher->setFuture(futureResult);
}

void MainWindow::onSaveImagesClick() {
	dir.cdUp();
	QString currentDir = dir.absolutePath();
	if (!QDir(currentDir + "/smallImages").exists()) {
		QDir().mkdir(currentDir + "/smallImages");
	}
	else {
		smallImagesDir.removeRecursively();
	}
	smallImagesDir = QDir(currentDir + "/smallImages");

	qDebug() << "saving the " << iconSize << "x" << iconSize << "icons to: " + smallImagesDir.absolutePath();
	timer.start();
	for (int i = 0; i < images.length(); ++i) {
		QString newFileName = (smallImagesDir.absolutePath() + "/" + imageNames[i]);
		images[i].save(newFileName);
	}
	logTime("time needed to save the images: ");
}

QImage MainWindow::loadImage(const QString &imageName) {
	QString fileName = dir.absoluteFilePath(imageName);

	//cv::Mat cvImage = cv::imread(fileName.toStdString());
	/*if (cvImage.empty()) {
        return QImage();
	}*/
	//cv::Mat cvResizedImg;
	//cv::resize(cvImage, cvResizedImg, cv::Size(MainWindow::imageWidth, MainWindow::imageHeight));
    //QImage image = QImage((uchar*)cvResizedImg.data, cvResizedImg.cols, cvResizedImg.rows, cvResizedImg.step, QImage::Format_RGB888);
	//QImage image = Mat2QImage(cvImage);
	//return image.scaled(MainWindow::imageWidth, MainWindow::imageHeight);
	//return image;

	QImage image(fileName);
    QImage resizedImg = image.scaled(MainWindow::imageWidth, MainWindow::imageHeight);
	return resizedImg;
}

void MainWindow::onImageReceive(int resultInd) {
    //LayoutItem* item = dynamic_cast<LayoutItem*>(futureResult.resultAt(resultInd));
    images.append(futureResult.resultAt(resultInd));
}

void MainWindow::onImagesReceive(int resultsBeginInd, int resultsEndInd) {
	for (int i = resultsBeginInd; i < resultsEndInd; ++i) {
        QImage image = futureResult.resultAt(i);
        images.append(image);
        //LayoutItem* item = new LayoutItem(NULL, image);
        //layout->addItem(item);
    }
    //ui->graphicsView->update();
    //viewPort->update();
}

void MainWindow::onReverseButtonClick() {
	/** clearing the layout */
	//clearLayout(layout.data());
	layout.clear();
	layout = QSharedPointer<FlowLayout>::create();

	std::reverse(images.begin(), images.end());

	/** display the images in reverse order */
    /*for (const QImage image : images) {
        LayoutItem* item = new LayoutItem(NULL, image);
        layout->addItem(dynamic_cast<QGraphicsLayoutItem*>(item));
    }
	*/
}

void MainWindow::onFinished() {
	logTime("load time: ");

	timer.start();
    for (const QImage image : images) {
        LayoutItem* item = new LayoutItem(NULL, image);
        layout->addItem(dynamic_cast<QGraphicsLayoutItem*>(item));
    }
	form->setLayout(layout.data());
    scene->addItem(form);

	logTime("display time: ");
}

void MainWindow::clearLayout(QGraphicsLayout* layout) {
	/*QGraphicsLayoutItem* item;
	while ((item = layout->itemAt(0)) != NULL) {
		layout->removeAt(0);
	}*/
}

void MainWindow::logTime(QString message) {
	std::ofstream logFile;
	logFile.open("log.txt", std::ios::out | std::ios::app);
	double time;
	time = timer.nsecsElapsed() / 1000000000.0;
	qDebug() << message << time << " seconds";
	logFile << "number of images: " << nrOfImages << "\n";
	logFile << message.toStdString() << nrOfImages << "\n";
	logFile.close();
}

QImage MainWindow::Mat2QImage(const cv::Mat &image) {
	return QImage((uchar*)image.data, image.cols, image.rows, image.step, QImage::Format_RGB888);
}

