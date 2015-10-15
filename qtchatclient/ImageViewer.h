#pragma once

#include <QtWidgets/QWidget>
#include <QtGui/QPixmap>
#include "ui_ImageViewer.h"

class ImageViewer : public QWidget, public Ui::ImageViewer
{
	Q_OBJECT
public:
	ImageViewer(QWidget* parent = nullptr);
	~ImageViewer();
	void setPixmap(QPixmap& map);
};

