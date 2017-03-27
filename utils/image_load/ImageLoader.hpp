#ifndef IMAGELOADER_HPP
#define IMAGELOADER_HPP

#include <QObject>
#include <QRunnable>
#include <QList>
#include <QDebug>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include "../CBIR.hpp"


class ImageLoader : public QObject, public QRunnable {
    Q_OBJECT
public:
    ImageLoader(const QString dirName, QStringList& imageNames,
                         QList<cv::Mat>& results,
                         const cv::Size& size, const int notifyRate, QObject* parent = 0);
    void run();

    bool isRunning() const { return static_cast<int>(_running); }
    void cancel() { _running.testAndSetOrdered(1, 0); }

private:
    QString _dirName;
    QStringList* _imageNames;
    cv::Size _size;
    QList<cv::Mat>* _results;
    int _notifyRate;

    QAtomicInt _running;

signals:
    void resultsReadyAt(int begin, int end);
    void finished();

public slots:
    void onCancel() { _running.testAndSetOrdered(1, 0); }
};

#endif // IMAGELOADER_HPP
