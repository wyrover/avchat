#pragma once
#include <QtWidgets/QTextBrowser>

class TextViewer : public QTextBrowser
{
public:
	TextViewer(QWidget* parent = 0);
	~TextViewer();
};
