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

//Seems like boilerplate code...
void BeyerFilter::chooseSource()
{
    chooseImage(tr("Choose Source Image"), &sourceImage, sourceButton);
}

void BeyerFilter::saveOutput()
{
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

    QImage tmp = QImage(source->scaled(w/2, h/2, Qt::KeepAspectRatio));

    int k = 0;  // Scaled height index
    int l = 0;  // Scaled width index
    for (int i = 0; i < h; i += 2)
    {
        for (int j = 0; j < w; j += 2)
        {
            pixColRed    = source->pixel(j, i)     & 0xffff0000 ; // Red-1
            pixColGreen2 = source->pixel(j+1, i)   & 0xff00ff00; // Green-2
            pixColGreen1 = source->pixel(j, i+1)   & 0xff00ff00; // Green-3
            pixColBlue   = source->pixel(j+1, i+1) & 0xff0000ff; // Blue-4

            newPixCol    = 0xff000000 | pixColRed | pixColBlue | ((pixColGreen1 + pixColGreen2) / 2);

            tmp.setPixel(l, k, newPixCol);

            if (j % 2 == 0)
            {
                l++;
            }
        }

        l = 0;

        if (i % 2 == 0)
        {
            k++;
        }
    }

    *output = tmp;

    return true;
}

bool BeyerFilter::extractForeground(QImage *source, QImage *output)
{
    QImage tmp = QImage(source->size(), QImage::Format_ARGB32);

    const int w = source->width();  // Width of source image
    const int h = source->height(); // heigh of source image

    for (int i = 0; i < h; i++)
    {
        for (int j = 0; j < w; j++)
        {
            QRgb sourcePixelAll  = source->pixel(j, i);
            QRgb sourcePixelR    = qRed(sourcePixelAll);
            QRgb sourcePixelG    = qGreen(sourcePixelAll);
            QRgb sourcePixelB    = qBlue(sourcePixelAll); // Unused

            QRgb cleanPixel      = qGreen(cleanPlate.pixel(j, i));

            double tol    = 0.2; // Make this change with slider
            double ratioG = ((double)sourcePixelG  / (double)cleanPixel);


            /*
             * Only check for green dominance over red, as
             * checking for blue could leave blue artifacts around
             * from the debeyering in the background of the extracted image
             */
            if (((1 - tol) <= ratioG) && (ratioG <= (1 + tol))
                    && (sourcePixelG > sourcePixelR))

            {
                tmp.setPixel(j, i, 0x00000000);
            }
            else
            {
                tmp.setPixel(j, i, source->pixel(j, i));
            }
        }
    }

    *output = tmp;

    return true;
}
