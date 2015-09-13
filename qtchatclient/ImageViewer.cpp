#include "ImageViewer.h"
#include <QPixmap>
#include <QLabel>

ImageViewer::ImageViewer(QWidget* parent)
	: QWidget(parent)
{
	setupUi(this);
}

ImageViewer::~ImageViewer()
{
}

void ImageViewer::setPixmap(QPixmap& map)
{
	label->setPixmap(map);
}
