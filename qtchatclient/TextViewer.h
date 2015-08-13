#pragma once

#include <QtWidgets/QTextBrowser>

class TextViewer : public QTextBrowser
{
	Q_OBJECT

public:
	TextViewer(QWidget* parent = 0);
	~TextViewer();
	void addMessage(const QString& username, bool self, time_t timestamp, const QString& message, const QString& imageDir);
	virtual void paintEvent(QPaintEvent *e);

private:
	void addPicture(QTextCursor& c, const QString& picUrl);
	void outputTextBlocks();
	void outputTextFrame(QTextFrame* frame);
	void addBubbleTextFrame(const QString& username, const QString& message, bool self, const QString& imageDir);
	void testGif(QTextCursor& c);

private:	
	QImage hostImage_;
	QImage remoteImage_;
	QUrl hostUri_;
	QUrl remoteUri_;

public slots:
	void onAnimate(int a);
};