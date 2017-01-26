#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <string>
#include <fstream>
#include <memory>

#include <QMainWindow>
#include <QSharedPointer>
#include <QScopedPointer>
#include <QLayout>
#include <QFileDialog>
#include <QVector>
#include <QFrame>
#include <QThread>
#include <QLabel>
#include <QPushButton>
#include <QDesktopWidget>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent>
#include <QElapsedTimer>
#include <QDebug>
#include <QGraphicsWidget>
#include <QGraphicsProxyWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsLinearLayout>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include "LayoutItem.hpp"
#include "FlowLayout.hpp"
#include "RingLayout.hpp"

typedef QFutureWatcher<QImage> FutureWatcher;
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
	explicit MainWindow(QWidget* parent = 0);
	MainWindow(MainWindow const& otherWindow) = delete;
	MainWindow& operator=(MainWindow const& otherWindow) = delete;
    ~MainWindow();

private:
	void initScene();
	void loadImages();
	void clearLayout();
	void logTime(QString message);

	static QImage loadImage(const QString &fileName);
	static QImage Mat2QImage(const cv::Mat &cvImage);


	Ui::MainWindow* ui;
	int _iconSize;
	int _nrOfImages = 0;
	QList<QString> _imageNames;
	QVector<QImage> _images;
	std::unique_ptr<RingLayout> _layout;
	QFrame _frame;
	QElapsedTimer _timer;
	QFuture<QImage> _futureResult;
	//QSharedPointer<FutureWatcher> _watcher;
	std::unique_ptr<FutureWatcher> _watcher;
	//QGraphicsView* _view;
	//QWidget* _viewPort;
	std::unique_ptr<QGraphicsScene> _scene;
	std::unique_ptr<QGraphicsWidget> _form;

	static QDir DIR;
	static QDir SMALLIMGDIR;
	static int IMGWIDTH;
	static int IMGHEIGHT;

private slots:
	void onLoadImagesClick();
	void onSaveImagesClick();
    void onImageReceive(int resultInd);
    void onImagesReceive(int resultsBeginInd, int resultsEndInd);
	void onReverseButtonClick();
    void onFinished();

signals:
    void start();
};

#endif // MAINWINDOW_H
