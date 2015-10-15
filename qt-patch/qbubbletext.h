#pragma once

#include <QtCore/qobject.h>
#include <QtCore/qsize.h>
#include <QtCore/qrect.h>
#include <QtCore/qvariant.h>
#include <QtGui/qfont.h>
#include <QtCore/qurl.h>
QT_BEGIN_NAMESPACE

class QTextFormatCollection;
class QTextListFormat;
class QRect;
class QPainter;
class QPagedPaintDevice;
class QAbstractTextDocumentLayout;
class QPoint;
class QTextObject;
class QTextFormat;
class QTextFrame;
class QTextBlock;
class QTextCodec;
class QVariant;
class QRectF;
class QTextLayout;
class QTextOption;
class QTextCursor;
struct QBubbleTextParam;

class Q_GUI_EXPORT QBubbleText : public QObject
{
	Q_OBJECT
public:
	enum DrawMode {
		kDrawMode_Flat,
		kDrawMode_9Path,
	};
	enum Direction {
		kDirection_Left,
		kDirection_Right,
	};
	QBubbleText();
	~QBubbleText();
	QBubbleText(const QBubbleText& other);
	void setFlat(QColor color);
	void setDirection(int dir);
	void drawDecoration(QPainter* painter, QTextFrame* frame, const QRectF& frameRect);
	void setTimeStamp(qint64 timeStamp);
	qint64 getTimeStamp() const;

private:
	void drawBubble(QPainter* painter, const QRectF &textRect);
	void drawErrorIcon();
	static QRectF getTextRect(QTextFrame* frame, const QRectF& frameRect);
	static QRectF getNaturalTextRect(const QTextLayout* tl);
	void drawFlatBubble(QPainter* painter, const QRectF &textRect, QColor color, int direction);
	QBubbleTextParam* param_;
};

Q_DECLARE_METATYPE(QBubbleText);
QT_END_NAMESPACE

