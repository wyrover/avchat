#include "TextViewer.h"
#include <QTextFrame>
#include <QDebug>
#include <QImageReader>
#include <qbubbletext.h>
#include <QMovie>
const static QSize kAvatarSize(30, 30);

TextViewer::TextViewer(QWidget* parent)
	: QTextBrowser(parent)
{
	setStyleSheet("background-color: rgb(227, 235, 246)");

	hostUri_.setUrl("def");
	hostImage_.load(":/Resources/left.png");

	remoteUri_.setUrl("abc");
	remoteImage_.load(":/Resources/right.png");

	document()->addResource(QTextDocument::ImageResource, hostUri_, QVariant(hostImage_));
	document()->addResource(QTextDocument::ImageResource, remoteUri_, QVariant(remoteImage_));
}

TextViewer::~TextViewer()
{
}

void TextViewer::addMessage(const QString& username, bool self, time_t timestamp, const QString& message)
{
	addBubbleTextFrame(username, message, self);
}

void TextViewer::addBubbleTextFrame(const QString& username, const QString& message, bool self)
{
	bool left = !self;
	QTextCursor c(document());
	c.movePosition(QTextCursor::End);

	QTextFrameFormat frameFormat;
	QBubbleText bubbleText;
	if (left) {
		bubbleText.setDirection(QBubbleText::kDirection_Left);
	} else {
		bubbleText.setDirection(QBubbleText::kDirection_Right);
	}
	bubbleText.setFlat(QColor(100, 185, 228));
	QVariant var;
	var.setValue(bubbleText);
	frameFormat.setProperty(QTextFormat::UserProperty, var);
	c.insertFrame(frameFormat);
	QTextBlockFormat blockFormat;
	QTextImageFormat imageFormat;
	imageFormat.setWidth(kAvatarSize.width());
	imageFormat.setHeight(kAvatarSize.height());
	if (left) {
		imageFormat.setName(hostUri_.toString());
		c.insertImage(imageFormat, QTextFrameFormat::FloatLeft);
		blockFormat.setAlignment(Qt::AlignLeft);
	} else {
		imageFormat.setName(remoteUri_.toString());
		c.insertImage(imageFormat, QTextFrameFormat::FloatRight);
		blockFormat.setAlignment(Qt::AlignRight);
	}
	blockFormat.setTopMargin(6);
	blockFormat.setRightMargin(kAvatarSize.width() + 15);
	blockFormat.setLeftMargin(kAvatarSize.width() + 15);

	if (!self) {
		QTextBlockFormat bf = blockFormat;
		bf.setTopMargin(0);
		blockFormat.setTopMargin(blockFormat.topMargin() + 3);
		c.insertBlock(bf);
		QTextCharFormat oldFormat = c.charFormat();
		QTextCharFormat format;
		format.setFontWeight(QFont::DemiBold);
		format.setForeground(QBrush(QColor("gray")));
		c.setCharFormat(format);
		c.insertText(username);
		c.setCharFormat(oldFormat);
	}

	blockFormat.setProperty(QTextFormat::UserProperty, "BubbleText");
	c.insertBlock(blockFormat);
	//addPicture(c, "aaa");
	c.insertText(message);
	c.movePosition(QTextCursor::End);
	setTextCursor(c);
}

void TextViewer::paintEvent(QPaintEvent *e)
{
	__super::paintEvent(e);
}

void TextViewer::outputTextBlocks()
{
	auto block = document()->begin();
	while (block.isValid()) {
		qDebug() << 
			QString("block len = %1, block text = %2").arg(QString::number(block.length()), block.text());
		block = block.next();
	}
}

void TextViewer::outputTextFrame(QTextFrame* frame)
{
	QTextFrame::iterator it;
	qDebug() << "\n==find a textframe===\n";
	for (it = frame->begin(); !(it.atEnd()); ++it) {

		QTextFrame *childFrame = it.currentFrame();
		QTextBlock childBlock = it.currentBlock();

		if (childFrame) {
			outputTextFrame(childFrame);
		} else if (childBlock.isValid()) {
			qDebug() << "find a qtextblock:" << childBlock.text();
		}
	}
	qDebug() << "\n==end of a textframe===\n";
}

void TextViewer::addPicture(QTextCursor& c, const QString& picUrl)
{
	QUrl testUri;
	QImage testImage;
	testUri.setUrl("qrc:///Resources/keke.gif");
	testImage.load(":/Resources/keke.gif");
	document()->addResource(QTextDocument::ImageResource, testUri, QVariant(testImage));

	QMovie* movie = new QMovie(this);
	movie->setFileName(":/Resources/keke.gif");
	movie->setCacheMode(QMovie::CacheNone);
	connect(movie, SIGNAL(frameChanged(int)), this, SLOT(onAnimate(int)));
	movie->start();

	QTextImageFormat imageFormat;
	imageFormat.setWidth(testImage.width());
	imageFormat.setHeight(testImage.height());
	imageFormat.setName(testUri.toString());
	c.insertImage(imageFormat);
}

void TextViewer::onAnimate(int a)
{
	QUrl testUri;
	testUri.setUrl("qrc:///Resources/keke.gif");
	if (QMovie* movie = qobject_cast<QMovie*>(sender())) {
		document()->addResource(QTextDocument::ImageResource,
			testUri, movie->currentPixmap());
		setLineWrapColumnOrWidth(lineWrapColumnOrWidth());
	}
}