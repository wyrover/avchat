#include "BubbleTextObject.h"
#include <QPainter>

static const int kPadding = 6;

BubbleTextObject::BubbleTextObject()
{
}

BubbleTextObject::~BubbleTextObject()
{

}

void BubbleTextObject::drawObject(QPainter * painter, const QRectF & rect, QTextDocument * doc, int posInDocument, const QTextFormat & format)
{
	Q_ASSERT(format.type() == format.CharFormat);
	drawBubble(painter, rect);
	auto textRc = rect;
	QString s = format.property(prop()).toString();
	textRc.adjust(kPadding, kPadding, 0, 0);
	painter->drawText(textRc, s);
}

QSizeF BubbleTextObject::intrinsicSize(QTextDocument * doc, int posInDocument, const QTextFormat & format)
{
	Q_ASSERT(format.type() == format.CharFormat);
	const QTextCharFormat &tf = *(const QTextCharFormat*)(&format);
	QString s = format.property(prop()).toString();
	QFont fn = tf.font();
	QFontMetrics fm(fn);
	QSize size = fm.boundingRect(s).size();
	return QSizeF(size.width() + kPadding * 2, size.height() + kPadding * 2);
}

int BubbleTextObject::type()
{
	return QTextFormat::UserObject + 1;
}

int BubbleTextObject::prop()
{
	return QTextFormat::UserProperty + 1;
}

void BubbleTextObject::drawBubble(QPainter* painter, const QRectF& rect)
{
	painter->save();
	auto borderRc = rect;
	auto borderPadding = kPadding / 2.f;
	borderRc.adjust(borderPadding, borderPadding, 0, 0);

	float _width = borderRc.width();
	float _height = borderRc.height();
	QPen _outterborderPen;

	int shadowThickness = 3;
	QLinearGradient gradient;
	gradient.setStart(0, 0);
	gradient.setFinalStop(_width, 0);
	QColor grey1(150, 150, 150, 125);

	QColor grey2(225, 225, 225, 125);

	gradient.setColorAt((qreal)0, grey1);
	gradient.setColorAt((qreal)1, grey2);

	QBrush brush(gradient);

	painter->setBrush(brush);

	_outterborderPen.setStyle(Qt::NoPen);
	painter->setPen(_outterborderPen);

	QPointF topLeft(shadowThickness + borderRc.left(), shadowThickness + borderRc.top());
	QPointF bottomRight(borderRc.left() + _width, borderRc.top() + _height);
	QRectF rc1(topLeft, bottomRight);

	painter->drawRoundRect(rc1, 25, 25); // corner radius of 25 pixels

	// draw the top box, the visible one
	QBrush brush2(QColor(255, 250, 200, 255), Qt::SolidPattern);

	painter->setBrush(brush2);

	QPointF topLeft2(borderRc.left(), borderRc.top());
	QPointF bottomRight2(borderRc.left() + _width - shadowThickness, borderRc.top() + _height - shadowThickness);

	QRectF rc12(topLeft2, bottomRight2);

	painter->drawRoundRect(rc12, 25, 25);
	painter->restore();
}