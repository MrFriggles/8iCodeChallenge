#ifndef BAYERFILTER_H
#define BAYERFILTER_H

#include <QColor>
#include <QFileDialog>
#include <QGridLayout>
#include <QImage>
#include <QMainWindow>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QSlider>
#include <QToolButton>


class BayerFilter : public QMainWindow
{
    Q_OBJECT

public:
    BayerFilter();
    ~BayerFilter();
    static bool debayer(QImage *source, QImage *output);


// Slots are some Qt meta-programming feature? They seem to work...
private slots:
    void chooseSource();
    void updateTolerance();
    void processImage();
    void saveFgImage();

private:
    QWidget *wdg;

    QToolButton *sourceButton;
    QPushButton *bayerButton;
    QPushButton *saveButton;
    QSlider     *slider;

    QImage sourceImage;
    QImage outputImage;
    QImage cleanPlate;
    QImage fgOutput;
    double tolerance;

    bool extractForeground(QImage *source, QImage *output);
    void chooseImage(const QString &title, QImage *image, QToolButton *button);
    void loadImage(const QString &fileName, QImage *image, QToolButton *button);
};

#endif // BEYERFILTER_H
