#ifndef IMAGE_H
#define IMAGE_H

#include "numArray.h"

#include <QGraphicsView>
#include <QVector>

class Image
{
public:
	virtual ~Image();
	virtual int getPixel(int xCoord, int yCoord) = 0;
	virtual void setPixel(int xCoord, int yCoord, int color) = 0;

	Image();
	int getWidth();
	int getHeight();
	NumArray getLine(int lineIndex);
	void setLine(int lineIndex, NumArray line);
	QImage convertToQImage();
	void saveImage(const QString &fileName);
	Image *getNegativeImage();
	NumArray histogram();
    Image *binarisation(int threshold);
	Image *binaryAdding(Image *toAdd);
	Image *substraction(Image *img);
	Image *addGaussianNoise(double intensity);
	Image *addImpulseNoise(double intensity);
	Image *filterImage(QVector<int> filter, double coeff);
	Image *dilatation(QVector<int> structElement);
	Image *erosion(QVector<int> structElement);
    Image *gradient();
	Image *makeFrame(int frameWidth);
	Image *deleteFrame(int frameWidth);
	Image *histogramEqualization();
	void displayImage(QGraphicsView *view);

protected:
	virtual Image *getNewImage(int width, int height) = 0;
	int width;
	int height;
	int range;

private:
	double countMean();
	double countDispersion();
	QVector<int> getPixelArray();
	void setPixelArray(const QVector<int> &array);
	void rangePixelArray(QVector<int> &array);
};

#endif // IMAGE_H
