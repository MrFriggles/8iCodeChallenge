#include "bayerfilter.h"

static const QSize windowSize(1000, 700); //Arbritary ... It just looks nice

BayerFilter::BayerFilter()
{
    // Create the main widget
    wdg = new QWidget(this);
    wdg->setFixedSize(windowSize);

    /*
     * Create a new girdLayout. This will house all our
     * buttons, sliders, and images.
     */
    QGridLayout *gridLayout = new QGridLayout(wdg);

    // initialize all child widgets
    sourceButton = new QToolButton();
    bayerButton  = new QPushButton(QString("Debayer Image"));
    slider       = new QSlider(Qt::Horizontal);
    saveButton   = new QPushButton(QString("Save"));

    sourceButton->setText("Load an image...");

    // Slider should be 0% to 100% for image extraction tolerance
    slider->setRange(0, 100);
    slider->setSingleStep(5);

    // Initialize the clean plate image
    QImage tmp("./CleanPlate.png");
    debayer(&tmp, &cleanPlate);

    /*
     * Setup everything in the same column. Won't win UI/UX awards,
     * but it does the job...
     */
    gridLayout->addWidget(bayerButton,  0, 0, Qt::AlignHCenter);
    gridLayout->addWidget(saveButton,   1, 0, Qt::AlignHCenter);
    gridLayout->addWidget(sourceButton, 2, 0, Qt::AlignHCenter);
    gridLayout->addWidget(slider,       3, 0);
    //TODO add rotation and zoom buttons

    // Connect widgets to associated actions
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
    delete saveButton;
}

/*
 * Slot Function to update the tolerance value from the slider.
 *
 * updateTolerance takes no parameters.
 *
 * returns nothing.
 */
void BayerFilter::updateTolerance()
{
    if (0 == slider->value())
    {
        tolerance = 0;
        return;
    }

    tolerance = slider->value() / 100.0;

    // Just in case the slider is moved when no image is loaded/debayered
    if (!fgOutput.isNull())
    {
        extractForeground(&outputImage, &fgOutput);
        sourceButton->setIcon(QPixmap::fromImage(fgOutput));
    }
}

/*
 * Slot Function debayers and extracts the foreground image
 * from the selected loaded image from the user.
 *
 * processImage takes no parameters.
 *
 * returns nothing.
 */
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

/*
 * Slot Function simply saves the foreground image from fgOutput to file.
 *
 * saveFgImage takes no parameters.
 *
 * returns nothing.
 */
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

/*
 * Slot Function calls chooseImage to load a new picture from the user.
 * Smells like unnecessary boilerplate code in here...
 *
 * chooseSource takes no paremeters.
 *
 * returns nothing.
 */
void BayerFilter::chooseSource()
{
    chooseImage(tr("Choose Source Image"), &sourceImage, sourceButton);
}

/*
 * Function chooseImage creates a new dialog and requests the user to upload a new image file
 * to be debayered and have the foreground extracted.
 *
 * Parameters:
 *  * const QString &title: the title of the dialog box when opened.
 *  * QImage *image: a pointer to a QImage variable to store the selected picture.
 *  * QToolButton *button: the button that called this funtion, which will present
 *                          the loaded picture from the image parameter.
 */
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

/*
 * Function debayer takes in two pointers to QImage variables.
 * The source will be used to extract the light levels of each
 * pixel, then match it against RGGB colouring in a quadrant.
 * This quadrant is then mixed together as (R & B & ((G1+G2)/2)),
 * then set in a new QImage to be set as the oupput. The resulting image
 * will however be smaller as we are taking 4 pixels and making 1.
 *
 * Parameters:
 *  * QImage *source: the image to be debayered.
 *  * QImage *output: the resulting debayered output image.
 *
 * returns bool:
 *  * true on successful debayering (is that a word?).
 *  * false otherwise.
 */
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

    // Resulting image will be halved in x and in y
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

            // only increment every second x
            if (j % 2 == 0)
            {
                l++;
            }
        }

        l = 0;

        // only increment every second y
        if (i % 2 == 0)
        {
            k++;
        }
    }

    *output = tmp;

    return true;
}


/*
 * Function extractForeground takes a debayered image and removes the background
 * greenscreen, leaving a foreground image; which is the resulting output image.
 *
 * Parameters:
 *  * QImage *source: the debayered image that will extract the foreground image.
 *  * QImage *output: the resulting foreground image.
 *
 * returns bool:
 *  * true on successful foreground extraction.
 *  * false otherwise.
 */
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
            // QRgb sourcePixelB    = qBlue(sourcePixelAll); // Unused

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
