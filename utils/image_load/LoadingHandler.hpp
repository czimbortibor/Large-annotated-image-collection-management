#ifndef LOADINGHANDLER_HPP
#define LOADINGHANDLER_HPP

#include <memory>

#include <QObject>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent>

#include "utils/image_load/Mapper.hpp"
#include "utils/image_load/Reducer.hpp"
#include "utils/image_load/ImageLoader.hpp"
#include "utils/ImageConverter.hpp"


class LoadingHandler : public QObject {
    Q_OBJECT
public:
    LoadingHandler(ImageCollection& imageCollection) {
        _imageCollection = &imageCollection;
    }

    /**
     * @brief loadImages_mt loads the images located at the chosen path
     * using multiple threads. The loaded images will be accessible via a signal
     * @param path a directory containing image files
     * @param imageNames the file names
     */
	void loadImages_mt(const QStringList& imageNames);

    /**
     * @brief loadImages_st loads the images located at the chosen path
     * using a single thread
     * @param path a directory containing image files
     * @param imageNames the file names
     * @param notifyRate after how many loaded images should the observers be notified
     * of available results
     * @return the loaded images
     */
	QList<GraphicsImage>* loadImages_st(QStringList* imageNames);

    cv::Mat loadImage(const QString& fileName) const;

    void setWidth(int width) { _width = width; }
    void setHeight(int height) { _height = height; }

private:
    ImageCollection* _imageCollection;
	std::unique_ptr<QList<GraphicsImage>> _images;
    /**
     * @brief the given width of the image to be loaded
     */
    int _width = 100;
    /**
     * @brief the given height of the image to be loaded
     */
    int _height = 100;

    // ----------- multi-threaded image load ---------------
	std::unique_ptr<QFuture<QList<GraphicsImage>>> _loaderMT;
	std::unique_ptr<QFutureWatcher<QList<GraphicsImage>>> _loaderWatcherMT;
    Mapper _mapper;
    Reducer _reducer;

    // ---------- custom single-threaded image load -----------
    std::unique_ptr<ImageLoader> _loaderST;


signals:
    void imageReady(int index, const QString& url, const QString& originalUrl);
	void mt_imageReady(const GraphicsImage& image);
    void finishedLoading();

public slots:
    void onImageReady(int index, const QString& url, const QString& originalUrl);
	//void onImageReadyMT(const cv::Mat& result);
    void onFinishedLoading();
    void onCancel();
};

#endif // LOADINGHANDLER_HPP
