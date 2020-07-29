#include "beyerfilter.h"

static const QSize windowSize(1000, 700); //Arbritary ... It just looks nice

BayerFilter::BayerFilter()
{
    wdg = new QWidget(this);
    wdg->setFixedSize(windowSize);

    QGridLayout *gridLayout = new QGridLayout(wdg);

    sourceButton = new QToolButton();
    bayerButton  = new QPushButton(QString("Debayer Image"));
    slider       = new QSlider(Qt::Horizontal);
    saveButton   = new QPushButton(QString("Save"));

    sourceButton->setText("Load an image...");

    slider->setRange(0, 100);
    slider->setSingleStep(5);

    // Initialize the clean plate image
    QImage tmp("./CleanPlate.png");
    debayer(&tmp, &cleanPlate);

    gridLayout->addWidget(bayerButton,  0, 0, Qt::AlignHCenter);
    gridLayout->addWidget(saveButton,   1, 0, Qt::AlignHCenter);
    gridLayout->addWidget(sourceButton, 2, 0, Qt::AlignHCenter);
    gridLayout->addWidget(slider,       3, 0);
    //TODO add rotation and zoom buttons

    connect(sourceButton, SIGNAL(clicked()),         this, SLOT(chooseSource()));
    connect(bayerButton,  SIGNAL(clicked()),         this, SLOT(processImage()));
    connect(slider,       SIGNAL(valueChanged(int)), this, SLOT(updateTolerance()));
    connect(saveButton,   SIGNAL(clicked()),         this, SLOT(saveFgImage()));

    setCentralWidget(wdg);
}

BayerFilter::~BayerFilter()
{
    delete wdg;
    delete sourceButton;
    delete bayerButton;
}

void BayerFilter::updateTolerance()
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

void BayerFilter::processImage()
{
    QMessageBox msgBox;

    if (debayer(&sourceImage, &outputImage)
            && extractForeground(&outputImage, &fgOutput))
    {
        sourceButton->setIcon(QPixmap::fromImage(fgOutput));
        msgBox.setText("Foreground image has been extracted.");
        msgBox.setInformativeText("Do you want to save the foreground image?");

        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);

        if (msgBox.exec() == QMessageBox::Save)
        {
            saveFgImage();
        }
    }
    else
    {
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("Can't debeyer image... Did you load an image first?");
        msgBox.exec();
    }
}

void BayerFilter::saveFgImage()
{
    QMessageBox msgBox;

    if (fgOutput.save("foreground.png", "PNG"))
    {
        msgBox.setText("Foreground image has been saved as 'foreground.png'.");
        msgBox.exec();
    }
    else
    {
      msgBox.setText("Unable to save image!");
      msgBox.setIcon(QMessageBox::Critical);
    }
}

//Smells like boilerplate code...
void BayerFilter::chooseSource()
{
    chooseImage(tr("Choose Source Image"), &sourceImage, sourceButton);
}

void BayerFilter::chooseImage(const QString &title, QImage *image, QToolButton *button)
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


bool BayerFilter::debayer(QImage *source, QImage *output)
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

bool BayerFilter::extractForeground(QImage *source, QImage *output)
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
