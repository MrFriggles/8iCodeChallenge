# 8i Coding Challenge

## Description
This program is a GUI that will take in a raw PNG bayered image, debayer that image and extract the foreground image based on a comparison with a 'clean plate' image. 

Raw PNG bayered image:

![bayered image](https://github.com/MrFriggles/8iCodeChallenge/blob/master/resources/Bottle.png "Bayered image")


Debayered image:

![debayered image](https://github.com/MrFriggles/8iCodeChallenge/blob/master/examples/bottleDebayered.png "Debayered image")


Foreground extracted image (tolerance at ~80%):

![foreground image](https://github.com/MrFriggles/8iCodeChallenge/blob/master/examples/foreground.png "Foreground image")

## How to build
In a directory of your choosing, extract the source code to a new directory:

```
mkdir 8i
unzip 8i.zip -d 8i
```

Create the ninja makefiles necessary to bulid the project using cmake:

```
mkdir build
cd build
cmake -G Ninja ../8i
```

Build the program:

```
cmake --build . -- -v
```

## Running

Simply call the 8i program in the commandline:

`./8i`


## Todo
 * Image rotation
 * Image zoom
 * Make the GUI less awful looking...
 * More extensive error checking

## Bugs
 * When an image is debayered and extracted the first time, the foreground will have a tolerance of 0... So a terrible foreground will be requested to be saved.

