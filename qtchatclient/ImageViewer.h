#pragma once

#include <QWidget>
#include <QPixmap>
#include "ui_imageviewer.h"

class ImageViewer : public QWidget, public Ui::ImageViewer
{
	Q_OBJECT
public:
	ImageViewer(QWidget* parent = nullptr);
	~ImageViewer();
	void setPixmap(QPixmap& map);
};

