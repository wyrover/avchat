#pragma once

#include "QtWidgets/qstackedwidget.h"
class ResizeableStackedWidget : public QStackedWidget
{
	Q_OBJECT
public:
	ResizeableStackedWidget(QWidget* parent = NULL);
	~ResizeableStackedWidget();
	void addWidget(QWidget* pWidget);

private slots:
	void onCurrentChanged(int index);
};

