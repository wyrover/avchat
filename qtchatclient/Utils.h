#pragma once

#include <QTextEdit>


class Utils
{
public:
	Utils();
	~Utils();
	static std::u16string textEditToMessageText(QTextEdit* textEdit);
	static void addImageToTextEdit(const QString& fileName, QTextEdit* textEdit);
};
