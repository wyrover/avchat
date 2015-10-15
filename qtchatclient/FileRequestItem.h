#pragma once

#include "ui_filetransitem.h"
#include <QtWidgets/QWidget>

class FileRequestItem : public QWidget, public Ui::fileTransItem
{
public:
	enum Direction {
		kFileRequest_Send,
		kFileRequest_Recv,
	};
public:
	FileRequestItem(int direction);
	~FileRequestItem();
	void setSendInfo(const QString& filePath, time_t timestamp);
	void setRecvInfo(const QString& filePath, int fileSize, time_t timestamp);
	void onSend();

private:
	void loadFileTypeIcon(const QString& filePath, const QString& fileExt);
	int direction_;
	time_t timestamp_;
	QString filePath_;
	int fileSize_;
};

