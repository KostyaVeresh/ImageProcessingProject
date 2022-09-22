#include "image.h"

#include <QDebug>

#include <iostream>
#include <random>

Image::Image() {

}

int Image::getWidth() {
	return this->width;
}

int Image::getHeight() {
	return this->height;
}

NumArray Image::getLine(int lineIndex) {
	QVector<double> result(this->width);
	for (int i = 0; i < this->width; ++i) {
		result[i] = getPixel(i, lineIndex);
	}
	return NumArray(result);
}

void Image::setLine(int lineIndex, NumArray line) {
    for (size_t i = 0; i < line.getSize(); ++i) {
		setPixel(i, lineIndex, int(line.getNum(i)));
	}
}

QImage Image::convertToQImage() {
	QImage img(width, height, QImage::Format_Grayscale8);
	for (int i = 0; i < width; ++i) {
		for (int j = 0; j < height; ++j) {
			int color = getPixel(i, j) / (range - 1.0) * 255;
			img.setPixel(i, j, qRgb(color, color, color));
		}
	}
	return img;
}

void Image::saveImage(const QString &fileName) {
	convertToQImage().save(fileName);
}

Image *Image::getNegativeImage() {
	Image *img = getNewImage(width, height);
	for (int i = 0; i < width; ++i) {
		for (int j = 0; j < height; ++j) {
			int color = getPixel(i, j);
			img->setPixel(i, j, range - color - 1);
		}
	}
	return img;
}

NumArray Image::histogram() {
	QVector<double> hist(range);
	for (int i = 0; i < width; ++i) {
		for (int j = 0; j < height; ++j) {
			++hist[getPixel(i, j)];
		}
	}
	for (int i = 0; i < hist.size(); ++i) {
		hist[i] /= (width * height);
	}
	return NumArray(hist);
}

Image *Image::binarisation(int threshold) {
    Image *img = getNewImage(width, height);\
    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            if (getPixel(i, j) < threshold) {
                img->setPixel(i, j, 0);
            } else {
                img->setPixel(i, j, range - 1);
            }
        }
    }
    return img;
}

Image *Image::binaryAdding(Image *toAdd) {
	Image *result = getNewImage(width, height);
	for (int i = 0; i < result->width; ++i) {
		for (int j = 0; j < result->height; ++j) {
			result->setPixel(i, j, (getPixel(i, j) || toAdd->getPixel(i, j)) * (range - 1));
		}
	}
	return result;
}

Image *Image::substraction(Image *img) {
	QVector<int> result(width * height);
	for (int i = 0; i < width; ++i) {
		for (int j = 0; j < height; ++j) {
			result[j * width + i] = getPixel(i, j) - img->getPixel(i, j);
		}
	}
	rangePixelArray(result);
	Image *res = getNewImage(width, height);
	res->setPixelArray(result);
	return res;
}

Image *Image::addGaussianNoise(double intensity) {
	Image *img = getNewImage(width, height);
	qDebug() << countMean();
	qDebug() << countDispersion();
	double noiseDispersion = countDispersion() * intensity;
	std::default_random_engine generator;
	std::normal_distribution<double> distribution(0, sqrt(noiseDispersion));
	QVector<int> pixelArray = getPixelArray();
	for (int i = 0; i < pixelArray.size(); ++i) {
		pixelArray[i] += int(distribution(generator));
	}
	rangePixelArray(pixelArray);
	img->setPixelArray(pixelArray);
	return img;
}

Image *Image::addImpulseNoise(double intensity) {
	Image *img = getNewImage(width, height);
	for (int i = 0; i < height; ++i) {
		int numberOfSpikes = width * intensity;
		QVector<int> positions(width);
		for (int j = 0; j < positions.size(); ++j) {
			positions[j] = j;
			img->setPixel(j, i, getPixel(j, i));
		}
		for (int j = 0; j < numberOfSpikes; ++j) {
			int pos = rand() % positions.size();
			int color = rand() % int((range - 1) * 0.2);
			if (color > (range - 1) * 0.1) {
				color += (range - 1) * 0.8;
			}
			img->setPixel(positions[pos], i, color);
			positions[pos] = positions[positions.size() - 1];
			positions.removeLast();
		}
	}
	return img;
}

Image *Image::filterImage(QVector<int> filter, double coeff) {
	int filterSize = sqrt(filter.size());
	Image *img = makeFrame(filterSize / 2);
	QVector<int> result(width * height);
	for (int i = 0; i < width; ++i) {
		for (int j = 0; j < height; ++j) {
            int sum = 0;
			for (int k = 0; k < filterSize; ++k) {
				for (int m = 0; m < filterSize; ++m) {
                    sum += img->getPixel(i + k, j + m) * filter[m * filterSize + k];
				}
			}
            result[j * width + i] = int(sum * coeff);
		}
	}
    rangePixelArray(result);
    Image *res = getNewImage(width, height);
    res->setPixelArray(result);
    return res;
}

Image *Image::dilatation(QVector<int> structElement) {
	int elemSize = sqrt(structElement.size());
	Image *img = makeFrame(elemSize / 2);
	for (int i = 0; i < width; ++i) {
		for (int j = 0; j < height; ++j) {
			if (getPixel(i, j) == range - 1) {
				for (int k = 0; k < elemSize; ++k) {
					for (int m = 0; m < elemSize; ++m) {
						img->setPixel(i + k, j + m,
									  (img->getPixel(i + k, j + m) || structElement[m * elemSize + k]) * (range - 1));
					}
				}
			}
		}
	}
	return img->deleteFrame(elemSize / 2);
}

Image *Image::erosion(QVector<int> structElement) {
	Image *negative = getNegativeImage();
	Image *dilatated = negative->dilatation(structElement);
	Image *erosed = dilatated->getNegativeImage();
	delete negative;
	delete dilatated;
	return erosed;
}

Image *Image::gradient() {
	QVector<int> sobelXFilter = {-1, 0, 1, -2, 0, 2, -1, 0, 1};
	QVector<int> sobelYFilter = {-1, -2, -1, 0, 0, 0, 1, 2, 1};
    Image *sobelX = filterImage(sobelXFilter, 1);
    Image *sobelY = filterImage(sobelYFilter, 1);
    Image *grad = getNewImage(width, height);
    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
			grad->setPixel(i, j, sqrt(sobelX->getPixel(i, j) * sobelX->getPixel(i, j) +
									  sobelY->getPixel(i, j) * sobelY->getPixel(i, j)));
        }
    }
    return grad;
}

Image *Image::makeFrame(int frameWidth) {
	Image *img = getNewImage(width + 2 * frameWidth, height + 2 * frameWidth);
	for (int i = 0; i < width; ++i) {
		for (int j = 0; j < height; ++j) {
			img->setPixel(i + frameWidth, j + frameWidth, getPixel(i, j));
		}
	}
	for (int i = 0; i < width; ++i) {
		for (int j = 0; j < frameWidth; ++j) {
			img->setPixel(frameWidth + i, j, getPixel(i, 0));
			img->setPixel(frameWidth + i, frameWidth + height + j, getPixel(i, height - 1));
		}
	}
	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < frameWidth; ++j) {
			img->setPixel(j, frameWidth + i, getPixel(0, i));
			img->setPixel(frameWidth + width + j, frameWidth + i, getPixel(width - 1, i));
		}
	}
	for (int i = 0; i < frameWidth; ++i) {
		for (int j = 0; j < frameWidth; ++j) {
			img->setPixel(i, j, img->getPixel(frameWidth, j));
			img->setPixel(i + frameWidth + width, j, img->getPixel(frameWidth + width - 1, j));
			img->setPixel(i, j + frameWidth + height, img->getPixel(frameWidth, j + frameWidth + height));
			img->setPixel(i + frameWidth + width, j + frameWidth + height, img->getPixel(frameWidth + width - 1, j + frameWidth + height));
		}
	}
	return img;
}

Image *Image::deleteFrame(int frameWidth) {
	Image *img = getNewImage(width - 2 * frameWidth, height - 2 * frameWidth);
	for (int i = 0; i < img->width; ++i) {
		for (int j = 0; j < img->height; ++j) {
			img->setPixel(i, j, getPixel(frameWidth + i, frameWidth + j));
		}
	}
	return img;
}

Image *Image::histogramEqualization() {
	Image *img = getNewImage(width, height);
	NumArray hist = histogram();
	QVector<double> distribution(hist.getSize());
	double elem = 0;
	for (int i = 0; i < distribution.size(); ++i) {
		elem += hist.getNum(i);
		distribution[i] = elem;
	}

	for (int i = 0; i < width; ++i) {
		for (int j = 0; j < height; ++j) {
			img->setPixel(i, j, distribution[getPixel(i, j)] * (range - 1));
		}
	}
	return img;
}

void Image::displayImage(QGraphicsView *view) {
	QImage scr = convertToQImage();
	//view->setFixedSize(scr.width() + 2, scr.height() + 2);
	QGraphicsScene *scene = new QGraphicsScene(view);
	scene->addPixmap(QPixmap::fromImage(scr));
	scene->setSceneRect(scr.rect());
	view->setScene(scene);
}

double Image::countMean() {
	double mean = 0;
	for (int i = 0; i < width; ++i) {
		for (int j = 0; j < height; ++j) {
			mean += getPixel(i, j);
		}
	}
	return mean / (width * height);
}

double Image::countDispersion() {
	double dispersion = 0;
	double mean = countMean();
	for (int i = 0; i < width; ++i) {
		for (int j = 0; j < height; ++j) {
			dispersion += pow(getPixel(i, j) - mean, 2);
		}
	}
	return dispersion / (width * height);
}

QVector<int> Image::getPixelArray() {
	QVector<int> array(width * height);
	for (int i = 0; i < width; ++i) {
		for (int j = 0; j < height; ++j) {
			array[j * width + i] = getPixel(i, j);
		}
	}
	return array;
}

void Image::setPixelArray(const QVector<int> &array) {
	for (int i = 0; i < width; ++i) {
		for (int j = 0; j < height; ++j) {
			setPixel(i, j, array[j * width + i]);
		}
	}
}

void Image::rangePixelArray(QVector<int> &array) {
	int min = *std::min_element(array.begin(), array.end());
	int max = *std::max_element(array.begin(), array.end());
	qDebug() << min << max;
	if (min < 0 || max >= range) {
		min = std::min(0, min);
		max = std::max(range - 1, max);
		qDebug() << min << max;
		for (int i = 0; i < array.size(); ++i) {
			array[i] = int(double(array[i] - min) / (max - min) * (range - 1));
		}
	}
	min = *std::min_element(array.begin(), array.end());
	max = *std::max_element(array.begin(), array.end());
	qDebug() << min << max;
}

Image::~Image() {

}
