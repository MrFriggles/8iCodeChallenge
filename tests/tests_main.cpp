#define CATCH_CONFIG_MAIN

#include <iostream>
#include <cstdlib>

#include "include/catch.hpp"
#include "../bayerfilter.h"

#define TEST_IMAGE_SIZE 1024
#define RESULT_SIZE     512


QImage *testImage = new QImage(TEST_IMAGE_SIZE, TEST_IMAGE_SIZE, QImage::Format_ARGB32);


TEST_CASE( "Blank image is debayered", "[BayerFilter::debayer]" )
{
    srand(time(NULL));

    testImage->fill(Qt::gray);

    int i;
    for (i = 0; i < TEST_IMAGE_SIZE; i++)
    {
        int j;
        for (j = 0; j < TEST_IMAGE_SIZE; j++)
        {
            testImage->setPixelColor(i, j, QColor((rand() % 255), (rand() % 255), (rand() % 255), 255));
        }
    }



    QImage tmp = *testImage;

    SECTION( "Pixles Should match " )
    {
        printf("Image should be all grey (all pixels 0x000000)\n");
        printf("0x%x\n", tmp.pixel(0, 0));
        printf("0x%x\n", tmp.pixel(1, 0));
        printf("0x%x\n", tmp.pixel(0, 1));
        printf("0x%x\n", tmp.pixel(1, 1));
        REQUIRE ( tmp.pixel(0, 0) == testImage->pixel(0, 0) );
        REQUIRE ( tmp.pixel(1, 0) == testImage->pixel(1, 0) );
        REQUIRE ( tmp.pixel(0, 1) == testImage->pixel(0, 1) );
        REQUIRE ( tmp.pixel(1, 1) == testImage->pixel(1, 1) );


        SECTION( "Pixles and dimensions should change")
        {
            REQUIRE( BayerFilter::debayer(testImage, &tmp) == true );

            printf("Image should be debayered...\n");
            printf("0x%x\n", tmp.pixel(0, 0));
            printf("0x%x\n", tmp.pixel(1, 0));
            printf("0x%x\n", tmp.pixel(0, 1));
            printf("0x%x\n", tmp.pixel(1, 1));

            REQUIRE ( tmp.pixel(0, 0) != testImage->pixel(0, 0) );
            REQUIRE ( tmp.pixel(1, 0) != testImage->pixel(1, 0) );
            REQUIRE ( tmp.pixel(0, 1) != testImage->pixel(0, 1) );
            REQUIRE ( tmp.pixel(1, 1) != testImage->pixel(1, 1) );

            REQUIRE( tmp.width()  == RESULT_SIZE );
            REQUIRE( tmp.height() == RESULT_SIZE );
        }
    }



}
