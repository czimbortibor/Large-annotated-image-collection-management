#ifndef IMAGELOADERST_HPP
#define IMAGELOADERST_HPP

#include <QObject>
#include <QRunnable>
#include <QList>
#include <QDebug>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include "../CBIR.hpp"
#include "../ImageCollection.hpp"


/**
 * @brief handles image loading in a single-threaded asyncronous way
 */
class ImageLoaderST : public QObject, public QRunnable {
    Q_OBJECT
public:
	ImageLoaderST(QStringList& imageNames,
						 QList<const GraphicsImage*>& results,
						 const cv::Size& size,
						 ImageCollection& imageCollection,
						 QObject* parent = 0);
	~ImageLoaderST() = default;

    void run();

    bool isRunning() const { return static_cast<int>(_running); }
    void cancel() { _running.testAndSetOrdered(1, 0); }

private:
    ImageCollection* _imageCollection;
    QString _dirName;
    QString _originalDirPath;
    QStringList* _imageNames;
    cv::Size _size;
	QList<const GraphicsImage*>* _results;
	int _index = 0;

    QAtomicInt _running;

signals:
	void resultReady(int index);
    void finished();

public slots:
    void onCancel() { _running.testAndSetOrdered(1, 0); }
};

#endif // IMAGELOADERST_HPP
