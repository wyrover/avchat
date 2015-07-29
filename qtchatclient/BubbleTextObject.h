#pragma once

#include <QObject>
#include <QTextObjectInterface>

//refer: http://stackoverflow.com/a/19425572/193251

class BubbleTextObject : public QObject, public QTextObjectInterface
{
	Q_OBJECT
	Q_INTERFACES(QTextObjectInterface)

public:
	BubbleTextObject();
	virtual ~BubbleTextObject();
	virtual void drawObject(QPainter * painter, const QRectF & rect, QTextDocument * doc, int posInDocument, const QTextFormat & format);
	virtual QSizeF intrinsicSize(QTextDocument * doc, int posInDocument, const QTextFormat & format);
	static int type();
	static int prop();
private:
	void drawBubble(QPainter* painter, const QRectF& rect);
};