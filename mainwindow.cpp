#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "grayscaleImage.h"
#include "numArray.h"
#include "rawImage.h"

#include <QDebug>

const QString MainWindow::projectDirectory = "";

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	stonesHighlighting();
}

void MainWindow::stonesHighlighting() {
	//Stone size = 6
	this->stones = new GrayscaleImage(this->projectDirectory + "stones.jpg");
	this->equalized = stones->histogramEqualization();

	NumArray hist = stones->histogram();
	NumArray equalizedHist = equalized->histogram();

	//stones->displayImage(ui->image);
	equalized->displayImage(ui->image_3);
	stones->displayImage(ui->image_2);

	hist.plotData(ui->plot, 0, 1);
	equalizedHist.plotData(ui->plot_2, 0, 1);

	Image *img = equalized->binarisation(185);

	Image *result = getStonesFromBinaryImage(img, 6);
	result->displayImage(ui->image_4);
	QImage stonesImage(result->getWidth(), result->getHeight(), QImage::Format_RGB16);
	for (int i = 0; i < result->getWidth(); ++i) {
		for (int j = 0; j < result->getHeight(); ++j) {
			if (result->getPixel(i, j) > 0) {
				stonesImage.setPixel(i, j, qRgb(255, 0, 0));
			} else {
				int color = stones->getPixel(i, j);
				stonesImage.setPixel(i, j, qRgb(color, color, color));
			}
		}
	}
	QGraphicsScene *scene = new QGraphicsScene(ui->image);
	scene->addPixmap(QPixmap::fromImage(stonesImage));
	scene->setSceneRect(stonesImage.rect());
	ui->image->setScene(scene);
	delete result;
	delete img;
}

inline uint qHash (const QPoint & key)
{
	return qHash (QPair<int,int>(key.x(), key.y()) );
}

Image *MainWindow::getStonesFromBinaryImage(Image *source, size_t stoneSize) {
	Image *result = new GrayscaleImage(source->getWidth(), source->getHeight());
	Image *stonesImage = new GrayscaleImage(source->getWidth(), source->getHeight());
	for (int i = 0; i < stonesImage->getWidth(); ++i) {
		for (int j = 0; j < stonesImage->getHeight(); ++j) {
			stonesImage->setPixel(i, j, source->getPixel(i, j));
		}
	}

	int stoneNum = 0;
	int acceptableStoneNum = 0;
	for (int i = 0; i < stonesImage->getWidth(); ++i) {
		for (int j = 0; j < stonesImage->getHeight(); ++j) {
			if (stonesImage->getPixel(i, j) > 0) {
				QSet<QPoint> stone;
				QSet<QPoint> currentLayer;
				stone.insert(QPoint(i, j));
				currentLayer.insert(QPoint(i, j));
				while (!currentLayer.isEmpty()) {
					foreach (QPoint point, currentLayer) {
						stonesImage->setPixel(point.rx(), point.ry(), 0);
					}

					QSet<QPoint> newLayer;
					foreach (QPoint point, currentLayer) {
						if (point.rx() > 0) {
							checkAndInsert(stonesImage, newLayer, point.rx() - 1, point.ry());
							if (point.ry() > 0)
								checkAndInsert(stonesImage, newLayer, point.rx() - 1, point.ry() - 1);
							if (point.ry() < stonesImage->getHeight() - 1)
								checkAndInsert(stonesImage, newLayer, point.rx() - 1, point.ry() + 1);
						}
						if (point.rx() < stonesImage->getWidth() - 1) {
							checkAndInsert(stonesImage, newLayer, point.rx() + 1, point.ry());
							if (point.ry() > 0)
								checkAndInsert(stonesImage, newLayer, point.rx() + 1, point.ry() - 1);
							if (point.ry() < stonesImage->getHeight() - 1)
								checkAndInsert(stonesImage, newLayer, point.rx() + 1, point.ry() + 1);
						}
						if (point.ry() > 0)
							checkAndInsert(stonesImage, newLayer, point.rx(), point.ry() - 1);
						if (point.ry() < stonesImage->getHeight() - 1)
							checkAndInsert(stonesImage, newLayer, point.rx(), point.ry() + 1);
					}
					currentLayer = newLayer;
					stone.unite(currentLayer);
				}
				//qDebug() << "Stone number " << stoneNum << " is highlighted.";
				++stoneNum;
				int minX = stonesImage->getWidth() + 1;
				int minY = stonesImage->getHeight() + 1;
				int maxX = -1;
				int maxY = -1;
				foreach (QPoint point, stone) {
					if (point.rx() < minX)
						minX = point.rx();
					if (point.ry() < minY)
						minY = point.ry();
					if (point.rx() > maxX)
						maxX = point.rx();
					if (point.ry() > maxY)
						maxY = point.ry();
				}
				//qDebug() <<"Stone size is " << qMax(qAbs(minX - maxX) + 1, qAbs(minY - maxY) + 1);
				//if (qAbs(minX - maxX) + 1 == stoneSize && qAbs(minY - maxY) + 1 == stoneSize) {
				if (qMax(qAbs(minX - maxX) + 1, qAbs(minY - maxY) + 1) == stoneSize) {
					++acceptableStoneNum;
					foreach (QPoint point, stone) {
						result->setPixel(point.rx(), point.ry(), 255);
					}
				}
			}
		}
	}
	qDebug() << "Number of stones: " << stoneNum;
	qDebug() << "Number of stones with size " << stoneSize << ": " << acceptableStoneNum;

	return result;
}

void MainWindow::checkAndInsert(Image *stonesImage, QSet<QPoint> &set, int rx, int ry) {
	if (stonesImage->getPixel(rx, ry) > 0) {
		set.insert(QPoint(rx, ry));
	}
}

void MainWindow::imageRestoring() {
	int width = 259;
	int height = 185;
	NumArray kernel(this->projectDirectory + "kernL64_f4.dat");
	kernel.plotData(ui->plot, 0, 1);
	Image *normal = new GrayscaleImage(width - kernel.getSize(), height);
	Image *blur = new GrayscaleImage(width, height);
	QFile f(this->projectDirectory + "sblur259x185L.dat");
	f.open(QIODevice::ReadOnly);
	QDataStream in(&f);
	in.setByteOrder(QDataStream::LittleEndian);
	in.setFloatingPointPrecision(QDataStream::SinglePrecision);
	for (int i = 0; i < height; ++i) {
		NumArray line(in, width);
		blur->setLine(height - i - 1, line);
		normal->setLine(height - i - 1, line.deconvolution(kernel));
	}
	normal->histogram().plotData(ui->plot_2, 0, 1);
	Image *binary = normal->binarisation(140);
	blur->displayImage(ui->image);
	normal->displayImage(ui->image_2);
	binary->displayImage(ui->image_3);
	blur->saveImage(this->projectDirectory + "sourceImage.jpg");
	normal->saveImage(this->projectDirectory + "deconvoluted.jpg");
	binary->saveImage(this->projectDirectory + "binarised.jpg");

	delete blur;
	delete normal;
	delete binary;
}


void MainWindow::contourHighlighting() {
	Image *source = new GrayscaleImage(this->projectDirectory + "MODELimage.jpg");
	Image *binary = source->binarisation(200);
	QVector<int> laplasianFilter = {1, 1, 1, 1, -8, 1, 1, 1, 1};
	Image *laplasian = binary->filterImage(laplasianFilter, 1);
	laplasian->histogram().plotData(ui->plot, 0, 1);
	Image *contour = laplasian->binarisation(140);
	contour->displayImage(ui->image);
	source->saveImage(this->projectDirectory + "model.jpg");
	binary->saveImage(this->projectDirectory + "binaryModel.jpg");
	laplasian->saveImage(this->projectDirectory + "laplasian.jpg");
	contour->saveImage(this->projectDirectory + "contour.jpg");

	delete source;
	delete binary;
	delete laplasian;
	delete contour;
}

void MainWindow::contourHighlighting_2() {
	Image *source = new GrayscaleImage(this->projectDirectory + "MODELimage.jpg");
	Image *binary = source->binarisation(200);
	Image *gradient = binary->gradient();
	Image *binary_2 = gradient->binarisation(175);
	Image *negative = binary_2->getNegativeImage();
	Image *binary_3 = gradient->binarisation(185);
	Image *result = negative->binaryAdding(binary_3);
	result->displayImage(ui->image_2);
	gradient->saveImage(this->projectDirectory + "gradient.jpg");
	result->saveImage(this->projectDirectory + "contour_2.jpg");

	delete source;
	delete binary;
	delete gradient;
	delete binary_2;
	delete binary_3;
	delete result;
}

void MainWindow::contourHighlighting_3() {
	Image *source = new GrayscaleImage(this->projectDirectory + "MODELimage.jpg");
	NumArray hist = source->histogram();
	Image *binary = source->binarisation(200);
	NumArray exampleLine = binary->getLine(130);
	exampleLine.plotData(ui->plot, 0, 1);
	exampleLine.amplitudeSpectrum().plotData(ui->plot_2, 0, 1.0 / exampleLine.getSize());
	NumArray filter = NumArray::highPassFilter(exampleLine.getSize(), 1, 0.18);
	exampleLine.applyFilter(filter).plotData(ui->plot_3, 0, 1);

	Image *img = new GrayscaleImage(source->getWidth(), source->getHeight());
	#pragma omp parallel for
	for (int i = 0; i < source->getHeight(); ++i) {
		NumArray line = binary->getLine(i);
		img->setLine(i, line.applyFilter(filter).getBinaryArray(255, 50));
	}

	img->displayImage(ui->image_3);
	img->saveImage(this->projectDirectory + "contour_3.jpg");

	delete source;
	delete binary;
	delete img;
}

void MainWindow::contourHighlighting_4() {
	Image *source = new GrayscaleImage(this->projectDirectory + "MODELimage.jpg");
	Image *binary = source->binarisation(200);
	QVector<int> structElement = {0, 1, 0,
								  1, 1, 1,
								  0, 1, 0};
	Image *dilatated = binary->dilatation(structElement);
	Image *contour = dilatated->substraction(binary);
	contour->displayImage(ui->image_4);
	dilatated->saveImage(this->projectDirectory + "dilatated.jpg");
	contour->saveImage(this->projectDirectory + "contour_4.jpg");

	delete source;
	delete binary;
	delete dilatated;
	delete contour;
}

MainWindow::~MainWindow()
{
	delete this->stones;
	delete this->equalized;
	if (this->binary != nullptr) {
		delete binary;
	}
	if (this->binaryEqualized != nullptr) {
		delete binaryEqualized;
	}
	delete ui;
}

void MainWindow::on_pushButton_clicked()
{
	if (this->binaryEqualized != nullptr) {
		delete binaryEqualized;
	}
	binaryEqualized = equalized->binarisation(ui->lineEdit->text().toInt());
	binaryEqualized->displayImage(ui->image_32);
}

void MainWindow::on_pushButton_2_clicked()
{
	if (this->binary != nullptr) {
		delete binary;
	}
	binary = stones->binarisation(ui->lineEdit_2->text().toInt());
	binary->displayImage(ui->image_22);
}
