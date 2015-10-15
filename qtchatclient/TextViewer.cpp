#include "TextViewer.h"
#include <QTextFrame>
#include <QDebug>
#include <QMenu>
#include <QImageReader>
#include <qbubbletext.h>
#include <QMovie>
#include <QDir>
#include <QFile>
#include <QSignalMapper>
#include "ImageViewer.h"
#include "../common/trace.h"
#include "../chatclient/Utils.h"
const static QSize kAvatarSize(30, 30);

#if _DEBUG
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

#endif

TextViewer::TextViewer(QWidget* parent)
	: QTextBrowser(parent)
{
	setStyleSheet("background-color: rgb(227, 235, 246)");

	hostUri_.setUrl("def");
	hostImage_.load(":/Resources/left.png");

	remoteUri_.setUrl("abc");
	remoteImage_.load(":/Resources/right.png");

	errorUri_.setUrl("error");
	errorImage_.load(":/Resources/error.png");

	TRACE("size = %d\n", sizeof(QBubbleText));
	document()->addResource(QTextDocument::ImageResource, hostUri_, QVariant(hostImage_));
	document()->addResource(QTextDocument::ImageResource, remoteUri_, QVariant(remoteImage_));
	document()->addResource(QTextDocument::ImageResource, errorUri_, QVariant(errorImage_));
}

TextViewer::~TextViewer()
{
}

void TextViewer::addMessage(const QString& username, bool self, time_t timestamp, const QString& message, const QString& imageDir)
{
	addBubbleTextFrame(username, message, self, imageDir, timestamp);
}

void TextViewer::addBubbleTextFrame(const QString& username, const QString& message, bool self, const QString& imageDir, time_t timestamp)
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
	bubbleText.setTimeStamp(timestamp);
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
		std::vector<std::u16string> imageList;
		std::u16string qtMessage;
		avc::Utils::XmlMessageToQtMessage(message.toStdU16String(), &imageList, &qtMessage);
		int index = 0;
		int pos = -1;
		int imageIndex = 0;
		while ((pos = qtMessage.find(L'\uffec', index)) != -1) {
			auto text = QString::fromStdU16String(qtMessage.substr(index, pos - index));
			QDir dir(imageDir);
			c.insertText(text);
			addPicture(c, dir.filePath(QString::fromStdU16String(imageList[imageIndex])));
			imageIndex++;
			index = pos+1;
		}
		auto text = QString::fromStdU16String(qtMessage.substr(index));
		c.insertText(text);
	} else {
		c.insertText(message);
	}
	c.movePosition(QTextCursor::End);
	setTextCursor(c);
}

void TextViewer::paintEvent(QPaintEvent *e)
{
    QTextBrowser::paintEvent(e);
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

void TextViewer::onAnimate(int a)
{
	QMovie* movie = qobject_cast<QMovie*>(sender());
	if (movie) {
		document()->addResource(QTextDocument::ImageResource,
			movie->fileName(), movie->currentPixmap());
		setLineWrapColumnOrWidth(lineWrapColumnOrWidth());
	}
}

bool TextViewer::markSendError(time_t timestamp)
{
	auto rootFrame = document()->rootFrame();
	QTextFrame::iterator it = rootFrame->end();
	while (true) {
		QTextFrame *frame = it.currentFrame();
		if (frame) {
			auto customProp = frame->frameFormat().property(QTextFormat::UserProperty);
			if (!customProp.isNull()) {
				if (customProp.canConvert<QBubbleText>()) {
					auto bubbleText = customProp.value<QBubbleText>();
					if (bubbleText.getTimeStamp() == timestamp) {
						auto it = frame->begin();
						for (; !it.atEnd(); ++it) {
							auto block = it.currentBlock();
							if (block.isValid()) {
								auto var = block.blockFormat().property(QTextFormat::UserProperty);
								if (!var.isNull() && var.toString() == "BubbleText") {
									QTextCursor c(block);
									QTextImageFormat imageFormat;
									imageFormat.setWidth(20);
									imageFormat.setHeight(20);
									imageFormat.setName(errorUri_.toString());
									c.insertImage(imageFormat);
									return true;
								}
							}
						}
					}
				}
			}
		}
		if (it == rootFrame->begin()) {
			break;
		} else {
			--it;
		}
	}
	return false;
}

QImage TextViewer::getImageByPos(const QPoint& eventPos)
{
	QTextCursor cursor = cursorForPosition(eventPos);

	QRect rect = cursorRect();
	if (rect.x() < eventPos.x())
		cursor.movePosition(QTextCursor::Right);

	int type = cursor.charFormat().objectType();
	if (type == QTextFormat::ImageObject)
	{
		auto block = cursor.block();
		if (block.isValid()) {
			auto var = block.blockFormat().property(QTextFormat::UserProperty);
			if (!var.isNull() && var.toString() == "BubbleText") {
				QTextFragment fragment;
				QTextBlock::iterator it;
				for (it = block.begin(); !(it.atEnd()); ++it) {
					fragment = it.fragment();
					if (fragment.contains(cursor.position()))
						break;
				}
				if (fragment.isValid()) {
					QTextImageFormat newImageFormat = fragment.charFormat().toImageFormat();
					if (newImageFormat.isValid()) {
						auto res = document()->resource(QTextDocument::ImageResource, newImageFormat.name());
						if (res.canConvert<QImage>()) {
							return res.value<QImage>();
				
						}
					}
				}
			}
		}
	}
	return QImage();
}

void TextViewer::mouseDoubleClickEvent(QMouseEvent *event)
{
	auto image = getImageByPos(event->pos());
	if (!image.isNull()) {
		ImageViewer* viewer = new ImageViewer();
        //viewer->setPixmap(QPixmap::fromImage(image));
		viewer->show();
	}
}

void TextViewer::contextMenuEvent(QContextMenuEvent * event)
{
	auto image = getImageByPos(event->pos());
	if (!image.isNull()) {
		auto addAction = new QAction(tr("add face"), this);
		connect(addAction, &QAction::triggered, this, [this, &image]{ addFace(image); });
		auto menu = new QMenu(this);
		menu->addAction(addAction);
		menu->exec(event->globalPos());
		event->accept();
	} else {
        QTextBrowser::contextMenuEvent(event);
	}
}

void TextViewer::addFace(const QImage& image)
{

}
