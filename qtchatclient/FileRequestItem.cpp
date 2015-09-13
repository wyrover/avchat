#include "FileRequestItem.h"
#include "../common/FileUtils.h"
#include <QFileInfo>

FileRequestItem::FileRequestItem(int direction)
{
	direction_ = direction;
	setupUi(this);
	imageLabel->setFixedSize(30, 30);
	button1->setCursor(Qt::PointingHandCursor);
	button2->setCursor(Qt::PointingHandCursor);
	button3->setCursor(Qt::PointingHandCursor);
}

FileRequestItem::~FileRequestItem()
{
}

void FileRequestItem::setSendInfo(const QString& filePath, time_t timestamp)
{
	Q_ASSERT(direction_ == kFileRequest_Send);
	filePath_ = filePath;
	timestamp_ = timestamp;

	QFileInfo fi(filePath);
	loadFileTypeIcon(filePath, fi.suffix());
	nameLabel->setText(fi.fileName());
	sizeLabel->setText(QString::fromStdWString(FileUtils::FileSizeToReadable(fi.size())));
	button1->setVisible(false);
	button2->setVisible(false);
	button3->setText(tr("Cancel"));
}

void FileRequestItem::setRecvInfo(const QString& filePath, int fileSize, time_t timestamp)
{
	Q_ASSERT(direction_ == kFileRequest_Recv);
	fileSize_ = fileSize;
	filePath_ = filePath;
	timestamp_ = timestamp;

	QFileInfo fi(filePath);
	loadFileTypeIcon(filePath, fi.suffix());
	nameLabel->setText(fi.fileName());
	sizeLabel->setText(QString::fromStdWString(FileUtils::FileSizeToReadable(fi.size())));
	button1->setText(tr("Recv"));
	button2->setText(tr("Save as"));
	button3->setText(tr("Cancel"));
}

void FileRequestItem::loadFileTypeIcon(const QString& filePath, const QString& fileExt)
{
	auto url = QString(":Resources/filetypes/%1.png").arg(fileExt);
	QFile f(url);
	if (!f.exists()) {
		url = ":Resources/filetypes/genericRed.png";
	}
	imageLabel->setPixmap(QPixmap(url));
}