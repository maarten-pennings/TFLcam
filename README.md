# TFLcam
Firmware for the ESP32-CAM turning it into a TensorFlow-Lite based classification sensor

## Introduction

The TFLcam project started with [getting to know the ESP32-CAM](https://github.com/maarten-pennings/esp32cam).
The [ESP32-CAM](https://www.aliexpress.com/item/1005001818136526.html) is a cost effective and small board 
with an ESP32, an OmniVision OV2640 camera, an SD card, and even a flash LED. 
It does not have a USB-serial converter, I would advise to buy [that](https://www.aliexpress.com/item/1005001810692306.html).

The ESP32-CAM board is powerful enough to shoot images and pass them through a TensorFlow Lite classifier running on the ESP32 itself.
I piloted that with a rock-paper-scissors [classifier](https://github.com/maarten-pennings/MachineLearning/blob/main/rock-paper-scissors/rock-paper-scissors.ipynb).
A Python script trains and generates the (tensorFlow Lite) model, and saves it as a flatbuffer file. 
The flatbuffer is linked in in the ESP32 sketch, together with a TensorFlow Lite interpreter (I took [EloquentTinyML](https://github.com/eloquentarduino/EloquentTinyML)),
resulting in a rock-paper-scissors classifier, see [video](https://www.youtube.com/watch?v=dVIRe2fjQL4).

This project goes one step further.
It puts the flatbuffer on the SD card.
It also puts a configuration file on the SD card: what is the input of the TensorFlow model 
(e.g. which area of the camera to crop, how much to subsample) and what is the output of the TensorFlow model (how many classes, what are their names.

This means that by writing thw two files to the SD card, you can configure the TFLcam to be either a rock-paper-scissors classifier
or a simple Lego brick classifier.

## Architecture

The core componets of the TFLcam project are
 - TensorFlow Lite interpreter
 - command interpreter
 - camera driver
 - SD card file system driver
 
Key commands configure the processing done on the camera output (crop, subsample) before it is fed to the TensorFlow interpreter.
Another key command is to load the model from the SD card into the TensoFlow interpreter.
These commands commands are typically stored on a file `boot.cmd`:

```
// boot.cmd for Rock, paper, scissors
@img crop  left 122  top 36  width 112  height 184  xsize 28  ysize 46
@img trans rotcw
@img proc histeq
@labels none paper rock scissors
file load /rps.tfl
```

The model (`rps.tfl`) is trained on input images of 28×46 (see crop) and expected to output a vector of four (see labels none paper rock scissors).

(end)



