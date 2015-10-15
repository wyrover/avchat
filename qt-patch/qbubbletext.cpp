#include "QBubbleText.h"
#include "private/qobject_p.h"
#include <qfile.h>
#include <qdebug.h>
#include <qtextobject.h>

static const int kPadding = 12;

QT_BEGIN_NAMESPACE

struct QBubbleTextParam
{
public:
	qint64 timeStamp_;
	int direction_;
	int drawMode_;
	QColor flatColor_;
};
Q_DECLARE_TYPEINFO(QBubbleTextParam, Q_PRIMITIVE_TYPE);

QBubbleText::QBubbleText()
{
	param_ = new QBubbleTextParam;
}

QBubbleText::QBubbleText(const QBubbleText& other)
{
	param_ = new QBubbleTextParam;
	*param_ = *other.param_;
}

QBubbleText::~QBubbleText()
{
	delete param_;
}

void QBubbleText::drawBubble(QPainter* painter, const QRectF &textRect)
{
	if (param_->drawMode_ == kDrawMode_Flat) {
		drawFlatBubble(painter, textRect, param_->flatColor_, param_->direction_);
	}
}

void QBubbleText::setFlat(QColor color)
{
	param_->drawMode_ = kDrawMode_Flat;
	param_->flatColor_ = color;
}

void QBubbleText::setDirection(int dir)
{
	param_->direction_ = dir;
}

void QBubbleText::drawFlatBubble(QPainter* painter, const QRectF &textRect, QColor color, int direction)
{
	painter->save();
	auto borderPadding = kPadding / 2.f;
	QRectF borderRc(textRect.left() - borderPadding, textRect.top() - borderPadding,
		textRect.width() + kPadding, textRect.height() + kPadding);

	QPainterPath path;
	path.addRoundedRect(borderRc, 8, 8);
	painter->setRenderHint(QPainter::HighQualityAntialiasing, true);
	painter->setBrush(color);
	painter->setPen(Qt::NoPen);
	auto y = borderRc.top() + 10;
	if (direction == kDirection_Right) {
		path.moveTo(borderRc.right(), y);
		path.lineTo(borderRc.right() + 7, y);
		path.lineTo(borderRc.right(), y + 4);
	} else {
		path.moveTo(borderRc.left(), y);
		path.lineTo(borderRc.left() - 7, y);
		path.lineTo(borderRc.left(), y + 4);
	}
	painter->drawPath(path);
	painter->restore();
}

QRectF QBubbleText::getTextRect(QTextFrame* frame, const QRectF& frameRect)
{
	QTextFrame::iterator it = frame->begin();
	QRegion textRegion;
	for (; !(it.atEnd()); ++it) {
		QTextBlock bl = it.currentBlock();
		if (bl.isValid()) {
			const QTextLayout *tl = bl.layout();
			auto bf = bl.blockFormat();
			if (bf.hasProperty(QTextFormat::UserProperty)) {
				auto prop = bf.property(QTextFormat::UserProperty);
				auto name = prop.toString();
				if (name == "BubbleText") {
					QRectF r = getNaturalTextRect(tl);
					r.translate(frameRect.left(), frameRect.top());
					qDebug() << "textblock : " << bl.text();
					qDebug("bounding rect (%d, %d, %d, %d)", r.left(), r.top(),
						r.right(), r.bottom());
					textRegion += r.toRect();
				}
			}
		}
	}
	return textRegion.boundingRect();
}

QRectF QBubbleText::getNaturalTextRect(const QTextLayout* tl)
{
	QRegion region;
	for (auto i = 0; i < tl->lineCount(); ++i) {
		QTextLine line = tl->lineAt(i);
		QRectF rc = line.naturalTextRect();
		rc.translate(tl->position());
		region += rc.toRect();
	}
	return region.boundingRect();
}

void QBubbleText::drawDecoration(QPainter* painter, QTextFrame* frame, const QRectF& frameRect)
{
	auto rc = getTextRect(frame, frameRect);
	drawBubble(painter, rc);
}

void QBubbleText::setTimeStamp(qint64 timeStamp)
{
	param_->timeStamp_ = timeStamp;
}

qint64 QBubbleText::getTimeStamp() const
{
	return param_->timeStamp_;
}

QT_END_NAMESPACE
