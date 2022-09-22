#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "image.h"

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

private slots:
	void on_pushButton_clicked();

	void on_pushButton_2_clicked();

private:
	Ui::MainWindow *ui;
	Image *stones = nullptr;
	Image *equalized = nullptr;
	Image *binary = nullptr;
	Image *binaryEqualized = nullptr;
	void stonesHighlighting();
	void checkAndInsert(Image *stonesImage, QSet<QPoint> &set, int rx, int ry);
	Image *getStonesFromBinaryImage(Image *source, size_t stoneSize);
	void imageRestoring();
	void contourHighlighting();
	void contourHighlighting_2();
	void contourHighlighting_3();
	void contourHighlighting_4();

	static const QString projectDirectory;
};

#endif // MAINWINDOW_H
