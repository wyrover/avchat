#pragma once

#include <QWidget>
#include "ui_filetransfer.h"
struct AvFileInfo;
class FileRequestWidget : public QWidget, public  Ui::fileTransferWidget
{
public:
	FileRequestWidget(QWidget* parent = NULL);
	virtual ~FileRequestWidget();
	void addSendRequest(const QString& filePath, time_t timestamp);
	void addRecvRequest(const QString& filePath, int fileSize, time_t timestamp);
private:
};