#include "TitleBar.h"
#include <QStyle>
#include <QIcon>
#include <QPushButton>
#include <QHBoxLayout>
#include <QApplication>
#include <QLabel>

TitleBar::TitleBar(QWidget *parent /*= 0*/) :QWidget(parent)
{
	title_ = new QLabel(this);
	QPushButton *min = new QPushButton(this);
	QPushButton *close = new QPushButton(this);
	QSpacerItem *spacer = new QSpacerItem(100, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);

	QIcon closeIcon(":/Resources/close.png");
	QIcon minIcon(":/Resources/min.png");
	min->setIcon(minIcon);
	close->setIcon(closeIcon);
	min->setFixedSize(QSize(30, 28));
	close->setFixedSize(QSize(30, 28));

	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->addWidget(title_);
	layout->setSpacing(0);
	layout->addSpacerItem(spacer);
	layout->addWidget(min);
	layout->addWidget(close);
	layout->setMargin(0);
	layout->setContentsMargins(QMargins(10, 0, 0, 0));
	setLayout(layout);

	connect(close, SIGNAL(clicked()), window(), SLOT(close()));
	connect(min, SIGNAL(clicked()), window(), SLOT(showMinimized()));
}

void TitleBar::setTitle(const QString& title)
{
	title_->setText(title);
}
