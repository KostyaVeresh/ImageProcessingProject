#ifndef GRAYSCALEIMAGE_H
#define GRAYSCALEIMAGE_H

#include "image.h"


class GrayscaleImage : public Image
{
public:
	GrayscaleImage(const QString &fileName);
	GrayscaleImage(int width, int height);
	virtual int getPixel(int xCoord, int yCoord);
	virtual void setPixel(int xCoord, int yCoord, int color);

protected:
	Image *getNewImage(int width, int height);

private:
	QImage image;
};

#endif // GRAYSCALEIMAGE_H
