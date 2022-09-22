#ifndef NUMARRAY_H
#define NUMARRAY_H

#include "../qcustomplot/qcustomplot.h"

#include <QVector>

class NumArray
{
public:
    NumArray();
    NumArray(QVector<double> data);
    NumArray(const QString &datFileName);
	NumArray(QDataStream &in, size_t size);

    double getNum(int index) const;
    void setNum(int index, double value);
	size_t getSize();
	NumArray reverseArray();
	NumArray scaleArray(size_t newSize);
	NumArray getBinaryArray(int range, double threshrold);
    NumArray convolution(NumArray toConvolute);
	NumArray derivative(double dt);
	NumArray deconvolution(NumArray transferFunc);
	NumArray fourierTransform();
	NumArray inverseFourierTransform();
	NumArray amplitudeSpectrum();
	NumArray applyFilter(NumArray filter);
    void plotData(QCustomPlot *plotWidget, double firstXValue, double discreteStep);

	static NumArray lowPassFilter(int m, double dt, double fcut);
	static NumArray highPassFilter(int m, double dt, double fcut);
	static NumArray bandPassFilter(int m, double dt, double fcut1, double fcut2);
	static NumArray bandStopFilter(int m, double dt, double fcut1, double fcut2);

private:
    QVector<double> data;
};

#endif // NUMARRAY_H
