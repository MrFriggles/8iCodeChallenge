#include "beyerfilter.h"

static const QSize windowSize(1000, 700); //Arbritary ... It just looks nice

BeyerFilter::BeyerFilter()
{
    wdg = new QWidget(this);
    wdg->setFixedSize(windowSize);

    QGridLayout *gridLayout = new QGridLayout(wdg);

    sourceButton = new QToolButton();
    sourceButton->setText("Load an image...");

    beyerButton = new QPushButton(QString("Debeyer Image"));

    // Initialize the clean plate image
    QImage tmp("./CleanPlate.png");
    cleanPlate = QImage(tmp.size(), tmp.format());
    debeyer(&tmp, &cleanPlate);
    cleanPlate.save(tr("hotdog.png"), "PNG");


    gridLayout->addWidget(sourceButton, 1, 0, 3, 1);
    gridLayout->addWidget(beyerButton,  0, 0);
    //TODO add rotation and zoom buttons

    connect(sourceButton, SIGNAL(clicked()), this, SLOT(chooseSource()));
    connect(beyerButton,  SIGNAL(clicked()), this, SLOT(saveOutput()));

    setCentralWidget(wdg);
}

BeyerFilter::~BeyerFilter()
{
    delete wdg;
    delete sourceButton;
    delete beyerButton;
}

void BeyerFilter::chooseSource()
{
    chooseImage(tr("Choose Source Image"), &sourceImage, sourceButton);
}

void BeyerFilter::saveOutput()
{
    // Init the output image
    outputImage = QImage(sourceImage.size(), sourceImage.format());

    if (debeyer(&sourceImage, &outputImage)
            && outputImage.save(tr("beans.png", "PNG")))
    {
        sourceButton->setIcon(QPixmap::fromImage(outputImage));

        //TODO this should request user if they want to save the debayered image
        QMessageBox msgBox;
        msgBox.setText("Image 'beans.png' saved to build diretory.");
        msgBox.exec();
    }
    else
    {
        // TODO omg make an actual error
        printf("get beaned\n");
        //error
    }
}

void BeyerFilter::chooseImage(const QString &title, QImage *image, QToolButton *button)
{
    QString fileName = QFileDialog::getOpenFileName(this, title);

    if (!fileName.isEmpty())
    {
        image->load(fileName);

        if (nullptr != button)
        {
            button->setIconSize(image->size());
            button->setIcon(QPixmap::fromImage(*image));
        }
    }
}

// Probably don't need this function
void BeyerFilter::saveImage(const QString &fileName, QImage *image)
{
    image->save(fileName, "PNG");
}


bool BeyerFilter::debeyer(QImage *source, QImage *output)
{
    assert(nullptr != source || nullptr != output);

    const int w = source->width();  // Width of source image
    const int h = source->height(); // heigh of source image

    /*
     * setPixel seems to be inefficent according to documentation; could run awfully on very large resolutions...
     *
     * "Warning: This function is expensive due to the call of the internal detach() function called within;
     * if performance is a concern, we recommend the use of scanLine() or bits() to access pixel data directly."
     */
    QRgb pixCol;
    for (int i = 0; i < h; i += 2)
    {
        for (int j = 0; j < w; j += 2)
        {
            pixCol = source->pixel(j, i); // Red-1
            output->setPixel(j, i, pixCol & 0xff0000);

            pixCol = source->pixel(j+1, i); // Green-2
            output->setPixel(j+1, i, pixCol & 0x00ff00);

            pixCol = source->pixel(j, i+1); // Green-3
            output->setPixel(j, i+1, pixCol & 0x00ff00);

            pixCol = source->pixel(j+1, i+1); // Blue-4
            output->setPixel(j+1, i+1, pixCol & 0x0000ff);
        }
    }

    return true;
}

bool BeyerFilter::extractForeground(QImage *source, QImage *output)
{
    source = nullptr;
    output = nullptr;

    /*
     * TODO
     * compare pixels of each of source and clean slate image
     * if one pixel is bang on green. Turn it transparent/black (and try it's surrounding pixels?)
     * point the output to the resulting forground image.
     */

    return true;
}
