#include "rawImage.h"

#include <QFile>

RawImage::RawImage(int width, int height)
{
	this->data = QVector<uint16_t>(width * height);
	this->height = height;
	this->width = width;
	this->range = 1051;
}

RawImage::RawImage(const QString &fileName, int width, int height) :
	RawImage(width, height)
{
	QFile file(fileName);
	file.open(QIODevice::ReadOnly);
	QDataStream in(&file);
	in.setByteOrder(QDataStream::LittleEndian);
	for (int i = 0; i < width; ++i) {
		for (int j = 0; j < height; ++j) {
			uint16_t pixel;
			in >> pixel;
			setPixel(j, height - i - 1, pixel - 1299);
		}
	}
}

void RawImage::setPixel(int xCoord, int yCoord, int color)
{
	data[yCoord * width + xCoord] = color;
}

int RawImage::getPixel(int xCoord, int yCoord)
{
	return data[yCoord * width + xCoord];
}

Image *RawImage::getNewImage(int width, int height)
{
	return new RawImage(width, height);
}
