#include "grayscaleImage.h"

GrayscaleImage::GrayscaleImage(const QString &fileName)
{
	this->image = QImage(fileName);
	this->width = image.width();
	this->height = image.height();
	this->range = 256;
}

GrayscaleImage::GrayscaleImage(int width, int height)
{
	this->image = QImage(width, height, QImage::Format_Grayscale8);
	this->width = image.width();
	this->height = image.height();
	this->range = 256;
	for (int i = 0; i < width; ++i) {
		for (int j = 0; j < height; ++j) {
			image.setPixelColor(i, j, qRgb(0, 0, 0));
		}
	}
}

int GrayscaleImage::getPixel(int xCoord, int yCoord) {
	return qRed(image.pixel(xCoord, yCoord));
}

void GrayscaleImage::setPixel(int xCoord, int yCoord, int color) {
	image.setPixel(xCoord, yCoord, qRgb(color, color, color));
}

Image *GrayscaleImage::getNewImage(int width, int height) {
	return new GrayscaleImage(width, height);
}
