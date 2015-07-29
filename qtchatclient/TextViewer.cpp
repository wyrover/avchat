#include "TextViewer.h"

TextViewer::TextViewer(QWidget* parent)
	: QTextBrowser(parent)
{
	setStyleSheet("background-color: rgb(227, 235, 246)");
}

TextViewer::~TextViewer()
{
}
