#include "numArray.h"

#include <QDataStream>
#include <QDebug>
#include <QFile>

NumArray::NumArray() {

}

NumArray::NumArray(QVector<double> data) {
    this->data = QVector<double>(data.size());
    for (int i = 0; i < data.size(); ++i) {
        this->data[i] = data[i];
    }
}

NumArray::NumArray(const QString &datFileName) {
    QFile f(datFileName);
    f.open(QIODevice::ReadOnly);
    QDataStream in(&f);
	in.setByteOrder(QDataStream::LittleEndian);
	in.setFloatingPointPrecision(QDataStream::SinglePrecision);
    while (!in.atEnd()) {
        float v;
        in >> v;
        this->data.append(v);
    }
}

NumArray::NumArray(QDataStream &in, size_t size) {
	this->data = QVector<double>(size);
	for (size_t i = 0; i < size; ++i) {
		float v;
		in >>  v;
		this->data[i] = v;
	}
}

double NumArray::getNum(int index) const {
    return this->data[index];
}

void NumArray::setNum(int index, double value) {
    this->data[index] = value;
}

size_t NumArray::getSize() {
	return this->data.size();
}

NumArray NumArray::reverseArray() {
	QVector<double> result(getSize());
	for (int i = 0; i < result.size(); ++i) {
		result[i] = getNum(result.size() - 1 - i);
	}
	return NumArray(result);
}

NumArray NumArray::scaleArray(size_t newSize) {
	QVector<double> result(newSize);
	size_t size = newSize < getSize() ? newSize : getSize();
	for (size_t i = 0; i < size; ++i) {
		result[i] = data[i];
	}
	return NumArray(result);
}

NumArray NumArray::getBinaryArray(int range, double threshrold) {
	QVector<double> result(getSize());
	for (int i = 0; i < result.size(); ++i) {
		result[i] = data[i] < threshrold ? 0 : range;
	}
	return NumArray(result);
}

NumArray NumArray::convolution(NumArray toConvolute) {
    int N = data.size();
    int M = toConvolute.data.size();
    QVector<double> result(M + N);
    for (int k = 0; k < M + N; ++k) {
        for (int i = 0; i < M; ++i) {
            if (k - i < 0 || k - i > N - 1)
                continue;
            result[k] += data[k - i] * toConvolute.getNum(i);
        }
    }
    return NumArray(result);
}

NumArray NumArray::derivative(double dt) {
	QVector<double> result(getSize());
	for (int i = 0; i < result.size() - 1; ++i) {
		result[i] = abs(getNum(i + 1) - getNum(i)) / dt;
	}
	result[getSize() - 1] = 0;
	return NumArray(result);
}

NumArray NumArray::deconvolution(NumArray transferFunc) {
	size_t N = getSize();
	NumArray scaledTransferFunc = transferFunc.scaleArray(N);
	NumArray fourier = fourierTransform();
	NumArray fourierTransferFunc = scaledTransferFunc.fourierTransform();
	QVector<double> resultFourier(N * 2);
	for (size_t i = 0; i < N; ++i) {
		double a = fourier.getNum(i);
		double b = fourier.getNum(i + N);
		double c = fourierTransferFunc.getNum(i);
		double d = fourierTransferFunc.getNum(i + N);
		resultFourier[i] = (a * c + b * d) / (c * c + d * d);
		resultFourier[i + N] = (b * c - a * d) / (c * c + d * d);
	}
	return NumArray(resultFourier).inverseFourierTransform().scaleArray(N - transferFunc.getSize());
}

NumArray NumArray::fourierTransform() {
	size_t N = this->data.size();
	QVector<double> result(N * 2);
	for (size_t k = 0; k < N; ++k) {
		double re = 0;
		double im = 0;
		for (size_t n = 0; n < N; ++n) {
			re += (data[n] * cos(2 * M_PI * k * n / double(N)));
			im += (data[n] * sin(2 * M_PI * k * n / double(N)));
		}
		re /= N;
		im /= N;
		result[k] = re;
		result[N + k] = - im;
	}
	return NumArray(result);
}

NumArray NumArray::inverseFourierTransform() {
	size_t N = this->getSize() / 2;
	QVector<double> result(N);
	for (size_t k = 0; k < N; ++k) {
		double re = 0;
		for (size_t n = 0; n < N; ++n) {
			re += (data[n] * cos(2 * M_PI * k * n / double(N)) - data[n + N] * sin(2 * M_PI * k * n / double(N)));
		}
		result[k] = re;
	}
	return NumArray(result);
}

NumArray NumArray::amplitudeSpectrum() {
	NumArray fourier = fourierTransform();
	size_t N = fourier.data.size() / 2;
	QVector<double> result(N / 2);
	for (size_t i = 0; i < N / 2; ++i) {
		result[i] = 2 * sqrt(fourier.getNum(i) * fourier.getNum(i) + fourier.getNum(i + N) * fourier.getNum(i + N));
		if (result[i] == 0) {
			qDebug() << "Potential ZERO DIVIDED ERROR.";
		}
	}
	return NumArray(result);
}

NumArray NumArray::applyFilter(NumArray filter) {
	int shift = filter.getSize() / 2;
	NumArray filtered = convolution(filter);
	QVector<double> result(getSize());
	for (int i = 0; i < result.size(); ++i) {
		result[i] = filtered.getNum(shift + i);
	}
	return NumArray(result);
}

void NumArray::plotData(QCustomPlot *plotWidget, double firstXValue, double discreteStep) {
    double size = data.size();
    double max = *std::max_element(data.begin(), data.end());
    double min = *std::min_element(data.begin(), data.end());
    double range = max - min;
    QVector<double> xs(size);
    for (int i = 0; i < size; ++i) {
        xs[i] = firstXValue + i * discreteStep;
    }
    plotWidget->xAxis->setRange(firstXValue, firstXValue + size * discreteStep);
    plotWidget->yAxis->setRange(min - range / 3, max + range / 3);
    if (plotWidget->graph() == 0) {
        plotWidget->addGraph();
    }
    plotWidget->graph(0)->setData(xs, data);
    plotWidget->replot();
}

NumArray NumArray::lowPassFilter(int m, double dt, double fcut) {
	double d1 = 0.35577019;
	double d2 = 0.2436983;
	double d3 = 0.07211497;
	double d4 = 0.00630165;
	double lpw[m + 1];
	double d[4];
	double arg, sumg, sum;
	d[0] = d1;
	d[1] = d2;
	d[2] = d3;
	d[3] = d4;
	arg = 2 * fcut * dt;
	lpw[0] = arg;
	arg = arg * M_PI;
	for (int i = 1; i <= m; i++) {
		lpw[i] = sin(arg * i) / (M_PI * i);
	}
	lpw[m] = lpw[m] / 2;
	sumg = lpw[0];
	for (int i = 1; i <= m; i++ ) {
		sum = d[0];
		arg = (M_PI * i) / m;
		for (int k = 1; k <= 3; k++) {
			sum = sum + 2 * d[k] * cos(arg * k);
		}
		lpw[i] = lpw[i] * sum;
		sumg = sumg + 2 * lpw[i];
	}
	for (int i = 0; i <= m; i++) {
		lpw[i] = lpw[i] / sumg;
	}
	/*for (int i = 0; i <= m; ++i) {
		lpw[i] = lpw[i] * m;
	}*/
	QVector<double> result(m * 2 + 1);
	for (int i = 0; i <= m; ++i) {
		result[i + m] = lpw[i];
		result[m - i] = lpw[i];
	}

	return NumArray(result);
}

NumArray NumArray::highPassFilter(int m, double dt, double fcut) {
	double hpw[2 * m + 1];
	//double[] lpw[2 * m + 1];
	NumArray lp = lowPassFilter(m, dt, fcut);

	//lpw = lowPassFilter(fcut, m, dt);

	for (int k = 0; k < 2 * m + 1; k++) {
		if (k == m) {
			hpw[k] = 1 - lp.getNum(k);
		}
		else {
			hpw[k] = -lp.getNum(k);
		}
	}
	QVector<double> result(2 * m + 1);
	for (int i = 0; i < 2 * m + 1; ++i) {
		result[i] = hpw[i];
	}
	return NumArray(result);
}

NumArray NumArray::bandPassFilter(int m, double dt, double fcut1, double fcut2) {
	QVector<double> bpw(2 * m + 1);
	//double[] lpw1 = new double[2 * m + 1];
	//double[] lpw2 = new double[2 * m + 1];

	NumArray lpw1 = lowPassFilter(m, dt, fcut1);
	NumArray lpw2 = lowPassFilter(m, dt, fcut2);

	for (int k = 0; k < 2 * m + 1; k++) {
		bpw[k] = lpw2.getNum(k) - lpw1.getNum(k);
	}

	return NumArray(bpw);
}

NumArray NumArray::bandStopFilter(int m, double dt, double fcut1, double fcut2) {
	QVector<double> bsw(2 * m + 1);
	NumArray lpw1 = lowPassFilter(m, dt, fcut1);
	NumArray lpw2 = lowPassFilter(m, dt, fcut2);

	//lpw1 = lowPassFilter(fcut1, m, dt);
	//lpw2 = lowPassFilter(fcut2, m, dt);

	for (int k = 0; k < 2 * m + 1; k++) {
		if (k == m) {
			bsw[k] = 1 + lpw1.getNum(k) - lpw2.getNum(k);
		}
		else {
			bsw[k] = lpw1.getNum(k) - lpw2.getNum(k);
		}
	}

	return NumArray(bsw);
}
