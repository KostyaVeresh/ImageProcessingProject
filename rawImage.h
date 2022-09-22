#ifndef RAWIMAGE_H
#define RAWIMAGE_H

#include "image.h"

#include <QVector>

class RawImage : public Image
{
public:
	RawImage(int width, int height);
	RawImage(const QString& fileName, int width, int height);
	virtual int getPixel(int xCoord, int yCoord);
	virtual void setPixel(int xCoord, int yCoord, int color);

protected:
	virtual Image *getNewImage(int width, int height);
private:
	QVector<uint16_t> data;
};

#endif // RAWIMAGE_H
