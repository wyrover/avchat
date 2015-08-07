#pragma once

#include <QImage>
#include <QString>
#include <QMap>
#include <QCache>

class ImageLoader
{
public:
	ImageLoader();
	~ImageLoader();
	QImage* getImage(const QString& name);

private:
	QMap<QString, QImage> imageCache_;
};