#include "FileRequestWidget.h"
#include "FileRequestItem.h"
#include "../chatclient/ChatClient.h"

FileRequestWidget::FileRequestWidget(QWidget* parent)
	: QWidget(parent)
{
	setupUi(this);
	verticalLayout->setAlignment(Qt::AlignTop);
}


FileRequestWidget::~FileRequestWidget()
{
}

void FileRequestWidget::addSendRequest(const QString& filePath, time_t timestamp)
{
	auto widget = new FileRequestItem(FileRequestItem::kFileRequest_Send);
	widget->setSendInfo(filePath, timestamp);
	verticalLayout->addWidget(widget, Qt::AlignTop);
}

void FileRequestWidget::addRecvRequest(const QString& filePath, int fileSize, time_t timestamp)
{
	auto widget = new FileRequestItem(FileRequestItem::kFileRequest_Recv);
	widget->setRecvInfo(filePath, fileSize, timestamp);
	verticalLayout->addWidget(widget, Qt::AlignTop);
}
