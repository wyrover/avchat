#pragma once
#include <QWidget>
#include <QLabel>
class TitleBar : public QWidget
{
	Q_OBJECT
public:
	explicit TitleBar(QWidget *parent = 0);
	void setTitle(const QString& title);
private:
	QLabel* title_;
};