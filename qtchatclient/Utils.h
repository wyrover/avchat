#pragma once

#include <QTextEdit>


class Utils
{
public:
	Utils();
	~Utils();
	static std::wstring textEditToMessageText(QTextEdit* textEdit);
};
