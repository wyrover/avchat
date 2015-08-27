#pragma once

#include <QtWidgets/QTextBrowser>

class TextViewer : public QTextBrowser
{
	Q_OBJECT

public:
	TextViewer(QWidget* parent = 0);
	~TextViewer();
	void addMessage(const QString& username, bool self, time_t timestamp, const QString& message, const QString& imageDir);
	bool markError(time_t timestamp);

protected:
	virtual void paintEvent(QPaintEvent *e);
	virtual void mouseDoubleClickEvent(QMouseEvent *event);
	virtual void contextMenuEvent(QContextMenuEvent * event);

private:
	void addPicture(QTextCursor& c, const QString& picUrl);
	void addBubbleTextFrame(const QString& username, const QString& message, bool self, const QString& imageDir, time_t timestamp);
	QImage getImageByPos(const QPoint& pos);

#if _DEBUG
	void outputTextBlocks();
	void outputTextFrame(QTextFrame* frame);
	void testGif(QTextCursor& c);
#endif

private:
	QImage hostImage_;
	QImage remoteImage_;
	QImage errorImage_;
	QUrl hostUri_;
	QUrl remoteUri_;
	QUrl errorUri_;

public slots:
	void onAnimate(int a);
private slots:
	void addFace(const QImage& image);
};