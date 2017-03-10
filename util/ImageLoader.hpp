#ifndef IMAGELOADER_HPP
#define IMAGELOADER_HPP

#include <QObject>
#include <QRunnable>
#include <QList>
#include <QDebug>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>


class ImageLoader : public QObject, public QRunnable {
    Q_OBJECT
public:
    explicit ImageLoader(QString dirName, QList<QString>* imageNames, QList<cv::Mat>& results, const cv::Size& size, int notifyRate, QObject* parent = 0);
    void run();

    bool isRunning() { return _running; }
    void cancel() { _cancel = true; }

private:
    QString _dirName;
    QList<QString>* _imageNames;
    cv::Size _size;
    QList<cv::Mat>* _results;
    int _notifyRate;

    bool _running = false;
    bool _cancel = false;

signals:
    void resultsReadyAt(int begin, int end);
    void finished();
};

#endif // IMAGELOADER_HPP
