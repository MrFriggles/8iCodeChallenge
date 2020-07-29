#include "beyerfilter.h"

static const QSize windowSize(1000, 700); //Arbritary ... It just looks nice

BeyerFilter::BeyerFilter()
{
    wdg = new QWidget(this);
    wdg->setFixedSize(windowSize);

    QGridLayout *gridLayout = new QGridLayout(wdg);

    sourceButton = new QToolButton();
    beyerButton  = new QPushButton(QString("Debeyer Image"));
    slider       = new QSlider(Qt::Horizontal);

    sourceButton->setText("Load an image...");

    slider->setRange(0, 100);
    slider->setSingleStep(5);
    slider->setTracking(true);

    // Initialize the clean plate image
    QImage tmp("./CleanPlate.png");
    debeyer(&tmp, &cleanPlate);

    cleanPlate.save(tr("hotdog.png"), "PNG");

    gridLayout->addWidget(beyerButton,  0, 0, Qt::AlignHCenter);
    gridLayout->addWidget(sourceButton, 1, 0, Qt::AlignHCenter);
    gridLayout->addWidget(slider,       2, 0);
    //TODO add rotation and zoom buttons

    connect(sourceButton, SIGNAL(clicked()),         this, SLOT(chooseSource()));
    connect(beyerButton,  SIGNAL(clicked()),         this, SLOT(saveOutput()));
    connect(slider,       SIGNAL(valueChanged(int)), this, SLOT(updateTolerance()));


    setCentralWidget(wdg);

    printf("sourceImage is Null? %s\n", sourceImage.isNull() ? "True" : "False");
    printf("sourceImage is Null? %s\n", outputImage.isNull() ? "True" : "False");
    printf("sourceImage is Null? %s\n", fgOutput.isNull() ? "True" : "False");
}

BeyerFilter::~BeyerFilter()
{
    delete wdg;
    delete sourceButton;
    delete beyerButton;
}

void BeyerFilter::updateTolerance()
{
    if (0 == slider->value())
    {
        tolerance = 0;
        return;
    }

    tolerance = slider->value() / 100.0;

    if (!fgOutput.isNull())
    {
        extractForeground(&outputImage, &fgOutput);
        sourceButton->setIcon(QPixmap::fromImage(fgOutput));
    }
}


void BeyerFilter::saveOutput()
{
    QMessageBox msgBox;

    if (debeyer(&sourceImage, &outputImage)
            && extractForeground(&outputImage, &fgOutput)
            && fgOutput.save(tr("beans.png", "PNG")))
    {

        sourceButton->setIcon(QPixmap::fromImage(fgOutput));

        //TODO this should request user if they want to save the debayered image
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setText("Image 'beans.png' saved to build diretory.");
        msgBox.exec();
    }
    else
    {
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("Can't debeyer image... Did you load an image first?");
        msgBox.exec();
    }
}

//Seems like boilerplate code...
void BeyerFilter::chooseSource()
{
    chooseImage(tr("Choose Source Image"), &sourceImage, sourceButton);
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


bool BeyerFilter::debeyer(QImage *source, QImage *output)
{
    const int w = source->width();  // Width of source image
    const int h = source->height(); // heigh of source image

    if (source->isNull())
    {
        return false;
    }

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
            pixColRed    = source->pixel(j, i)     & 0xffff0000; // Red-1
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

//            double tol    = 0.2; // Make this change with slider
            double ratioG = ((double)sourcePixelG  / (double)cleanPixel);


            /*
             * Only check for green dominance over red, as
             * checking for blue could leave blue artifacts around
             * from the debeyering in the background of the extracted image
             */
            if (((1 - tolerance) <= ratioG) && (ratioG <= (1 + tolerance))
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
