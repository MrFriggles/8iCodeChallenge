#ifndef BEYERFILTER_H
#define BEYERFILTER_H

#include <QMainWindow>
#include <QImage>
#include <QPainter>
#include <QToolButton>
#include <QPushButton>
#include <QFileDialog>
#include <QGridLayout>
#include <QMessageBox>


class BeyerFilter : public QMainWindow
{
    Q_OBJECT

public:
    BeyerFilter();
    ~BeyerFilter();

// Slots are some Qt meta-programming feature? They seem to work...
private slots:
    void chooseSource();
    void saveOutput();

private:
    QWidget *wdg;

    QToolButton *sourceButton;
    QPushButton *beyerButton;
    QImage sourceImage;
    QImage outputImage;
    QImage cleanPlate;
    QImage fgOutput;

    bool debeyer(QImage *source, QImage *output);
    bool extractForeground(QImage *source, QImage *output);
    void chooseImage(const QString &title, QImage *image, QToolButton *button);
    void loadImage(const QString &fileName, QImage *image, QToolButton *button);
    void saveImage(const QString &fileName, QImage *image);
};

#endif // BEYERFILTER_H
