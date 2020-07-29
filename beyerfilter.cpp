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
    cleanPlate = QImage( tmp.width()/4, tmp.height()/4, QImage::Format_ARGB32);
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
    fgOutput    = QImage(sourceImage.size(), QImage::Format_ARGB32); // Difference in format so we can set it to be transparent

    if (debeyer(&sourceImage, &outputImage)
            && extractForeground(&outputImage, &fgOutput)
            && fgOutput.save(tr("beans.png", "PNG")))
    {

        sourceButton->setIcon(QPixmap::fromImage(fgOutput));

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
    const int w = source->width();  // Width of source image
    const int h = source->height(); // heigh of source image

    /*
     * setPixel seems to be inefficent according to documentation; could run awfully on very large resolutions...
     *
     * "Warning: This function is expensive due to the call of the internal detach() function called within;
     * if performance is a concern, we recommend the use of scanLine() or bits() to access pixel data directly."
     */
    QRgb pixColRed;
    QRgb pixColBlue;
    QRgb pixColGreen1;
    QRgb pixColGreen2;
    QRgb newPixCol;

    int k = 0;
    int l = 0;
    for (int i = 0; i < h; i += 2)
    {
        for (int j = 0; j < w; j += 2)
        {
//            pixCol = source->pixel(j, i); // Red-1
//            output->setPixel(j, i, pixCol & 0xff0000);

//            pixCol = source->pixel(j+1, i); // Green-2
//            output->setPixel(j+1, i, pixCol & 0x00ff00);

//            pixCol = source->pixel(j, i+1); // Green-3
//            output->setPixel(j, i+1, pixCol & 0x00ff00);

//            pixCol = source->pixel(j+1, i+1); // Blue-4
//            output->setPixel(j+1, i+1, pixCol & 0x0000ff);

            pixColRed = source->pixel(j, i) & 0xffff0000 ; // Red-1
            pixColGreen2 = source->pixel(j+1, i) & 0xff00ff00; // Green-2
            pixColGreen1 = source->pixel(j, i+1) & 0xff00ff00; // Green-3
            pixColBlue = source->pixel(j+1, i+1) & 0xff0000ff; // Blue-4
            newPixCol = qRgba(pixColRed,((pixColGreen1 + pixColGreen2) / 2), pixColBlue, (unsigned int)0xff000000);
            printf("k: %d \t l: %d\n", k, l);
            //            output->setPixel(l, k, newPixCol);

            if (j % 2 == 0)
            {
                l++;
            }
        }

        if (i % 2 == 0)
        {
            k++;
        }
    }

    output->save("test.png", "PNG");

    return true;
}

bool BeyerFilter::extractForeground(QImage *source, QImage *output)
{
    const int w = source->width();  // Width of source image
    const int h = source->height(); // heigh of source image

    for (int i = 0; i < h; i += 2)
    {
        for (int j = 0; j < w; j += 2)
        {
            QRgb sourceGreen = qGreen(source->pixel(j+1, i));
            QRgb sourceRed   = qRed(source->pixel(j, i));
            QRgb sourceBlue  = qBlue(source->pixel(j+1, i+1));
            QRgb cleanGreen  = qGreen(cleanPlate.pixel(j+1, i));

            double tol = 0.15; // Make this change with slider
            double ratio = ((double)sourceGreen / (double)cleanGreen);

            if (sourceGreen > sourceRed && sourceGreen > sourceBlue
                    && (((1 - tol) <= ratio) && (ratio <= (1 + tol))))
            {
                output->setPixel(j,   i,   0x00000000);
                output->setPixel(j+1, i,   0x00000000);
                output->setPixel(j,   i+1, 0x00000000);
                output->setPixel(j+1, i+1, 0x00000000);
            }
            else
            {
                output->setPixel(j,   i,   source->pixel(j,   i));
                output->setPixel(j+1, i,   source->pixel(j+1, i));
                output->setPixel(j,   i+1, source->pixel(j,   i+1));
                output->setPixel(j+1, i+1, source->pixel(j+1, i+1));
            }
        }
    }
    return true;
}
