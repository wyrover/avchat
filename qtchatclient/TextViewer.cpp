#include "TextViewer.h"
#include <QTextFrame>
#include <QDebug>
#include <QImageReader>
#include <qbubbletext.h>
#include <QMovie>
#include <QDir>
#include <QFile>
#include "../chatclient/Utils.h"
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

void TextViewer::addMessage(const QString& username, bool self, time_t timestamp, const QString& message, const QString& imageDir)
{
	addBubbleTextFrame(username, message, self, imageDir);
}

void TextViewer::addBubbleTextFrame(const QString& username, const QString& message, bool self, const QString& imageDir)
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
	if (message.indexOf("<img") != -1) {
		std::vector<std::wstring> imageList;
		std::wstring qtMessage;
		client::Utils::XmlMessageToQtMessage(message.toStdWString(), &imageList, &qtMessage);
		int index = 0;
		int pos = -1;
		int imageIndex = 0;
		while ((pos = qtMessage.find(L'\uffec', index)) != -1) {
			auto text = QString::fromStdWString(qtMessage.substr(index, pos - index));
			QDir dir(imageDir);
			c.insertText(text);
			addPicture(c, dir.filePath(QString::fromStdWString(imageList[imageIndex])));
			imageIndex++;
			index = pos+1;
		}
		auto text = QString::fromStdWString(qtMessage.substr(index));
		c.insertText(text);
	} else {
		c.insertText(message);
	}
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

void TextViewer::addPicture(QTextCursor& c, const QString& picFile)
{
	QImage img;
	img.load(picFile);
	document()->addResource(QTextDocument::ImageResource, picFile, QVariant(img));

	QTextImageFormat imageFormat;
	float maxImageWidth = width() / 2.f;
	if (img.width() < maxImageWidth) {
		imageFormat.setWidth(img.width());
		imageFormat.setHeight(img.height());
	} else {
		imageFormat.setWidth(maxImageWidth);
		imageFormat.setHeight(maxImageWidth / img.width() * img.height());
	}
	imageFormat.setName(picFile);
	c.insertImage(imageFormat);

	QFileInfo f(picFile);
	auto suf = f.suffix().toLower();
	if (suf == "gif") {
		QMovie* movie = new QMovie(this);
		movie->setFileName(picFile);
		movie->setCacheMode(QMovie::CacheNone);
		connect(movie, SIGNAL(frameChanged(int)), this, SLOT(onAnimate(int)));
		movie->start();
	}
}

void TextViewer::testGif(QTextCursor& c)
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
	QMovie* movie = qobject_cast<QMovie*>(sender());
	if (movie) {
		document()->addResource(QTextDocument::ImageResource,
			movie->fileName(), movie->currentPixmap());
		setLineWrapColumnOrWidth(lineWrapColumnOrWidth());
	}
}