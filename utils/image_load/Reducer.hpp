#ifndef REDUCER_HPP
#define REDUCER_HPP

#include <QObject>
#include <QList>

#include <opencv2/core.hpp>

#include "../ImageCollection.hpp"


class Reducer : public QObject {
    Q_OBJECT
public:
	Reducer() = default;
    ~Reducer() = default;
	void operator()(QList<GraphicsImage>& images, const GraphicsImage& image) {
		//images.append(image);
        // TODO: pass URL
		//imageCollection.insert(&image);
		emit imageReady(image);
    }

signals:
	void imageReady(const GraphicsImage& image);
};

Q_DECLARE_METATYPE(GraphicsImage)

#endif // REDUCER_HPP
