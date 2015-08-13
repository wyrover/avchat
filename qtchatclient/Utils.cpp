#include "Utils.h"

Utils::Utils()
{
}

Utils::~Utils()
{
}

std::wstring Utils::textEditToMessageText(QTextEdit* textEdit)
{
	std::wstring result;
	QTextCursor c(textEdit->document());
	auto text = textEdit->toPlainText();
	size_t textStart = 0;
	int pos;
	QChar ch(65532);
	while ((pos = text.indexOf(ch, textStart)) != -1) {
		result += text.mid(textStart, pos - textStart).toStdWString();
		c.setPosition(pos + 1);
		auto imageFormat = c.charFormat().toImageFormat();
		if (imageFormat.isValid()) {
			auto fileName = imageFormat.name();
			result += L"<img path=\"" + fileName.toStdWString() + L"\"/>";
		}
		textStart = pos + 1;
	}
	result += text.mid(textStart).toStdWString();
	return result;
}