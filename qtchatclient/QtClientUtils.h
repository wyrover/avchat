#pragma once

#include <QTextEdit>

class QtClientUtils
{
public:
	QtClientUtils();
	~QtClientUtils();
	static std::wstring textEditToMessageText(QTextEdit* textEdit);
};