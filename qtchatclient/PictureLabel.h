#include "QImage.h"
#include "QPixmap.h"
#include "QLabel.h"

class PictureLabel : public QLabel
{
private:
	QPixmap _qpSource; //preserve the original, so multiple resize events won't break the quality
	QPixmap _qpCurrent;

	void _displayImage();

public:
	PictureLabel(QWidget *aParent) : QLabel(aParent) { }
	void setPixmap(QPixmap aPicture);
	void paintEvent(QPaintEvent *aEvent);
};