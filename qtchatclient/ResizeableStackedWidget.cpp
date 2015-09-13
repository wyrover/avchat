#include "ResizeableStackedWidget.h"
#include "../common/trace.h"


ResizeableStackedWidget::ResizeableStackedWidget(QWidget* parent)
	:QStackedWidget(parent)
{
	setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	connect(this, SIGNAL(currentChanged(int)), this, SLOT(onCurrentChanged(int)));
}


ResizeableStackedWidget::~ResizeableStackedWidget()
{
}

void ResizeableStackedWidget::addWidget(QWidget* pWidget)
{
	pWidget->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	QStackedWidget::addWidget(pWidget);
}

void ResizeableStackedWidget::onCurrentChanged(int index)
{
	QWidget* pWidget = widget(index);
	Q_ASSERT(pWidget);
	pWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	pWidget->adjustSize();
	adjustSize();
	updateGeometry();
}
