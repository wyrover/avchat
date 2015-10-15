#include "Utils.h"
#include <QFileDialog>

Utils::Utils()
{
}

Utils::~Utils()
{
}

std::u16string Utils::textEditToMessageText(QTextEdit* textEdit)
{
	std::u16string result;
	QTextCursor c(textEdit->document());
	auto text = textEdit->toPlainText();
	size_t textStart = 0;
	int pos;
	QChar ch(65532);
	while ((pos = text.indexOf(ch, textStart)) != -1) {
		result += text.mid(textStart, pos - textStart).toStdU16String();
		c.setPosition(pos + 1);
		auto imageFormat = c.charFormat().toImageFormat();
		if (imageFormat.isValid()) {
			auto fileName = imageFormat.name();
            result += u"<img path=\"" + fileName.toStdU16String() + u"\"/>";
		}
		textStart = pos + 1;
	}
	result += text.mid(textStart).toStdU16String();
	return result;
}

void Utils::addImageToTextEdit(const QString& fileName, QTextEdit* textEdit)
{
	if (!fileName.isNull()) {
		QTextCursor c(textEdit->document());
		c.movePosition(QTextCursor::End);

		QImage image;
		image.load(fileName);
		QTextImageFormat imageFormat;
		float maxImageWidth = textEdit->width() / 2.f;
		if (image.width() < maxImageWidth) {
			imageFormat.setWidth(image.width());
			imageFormat.setHeight(image.height());
		} else {
			imageFormat.setWidth(maxImageWidth);
			imageFormat.setHeight(maxImageWidth / image.width() * image.height());
		}
		imageFormat.setName(fileName);
		c.insertImage(imageFormat);
	}
}
