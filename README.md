# TFLcam
Firmware (arduino sketch) for the ESP32-CAM turning it into a TensorFlow-Lite based classification sensor.

## Introduction

The TFLcam project started with [getting to know the ESP32-CAM](https://github.com/maarten-pennings/esp32cam).
The [ESP32-CAM](https://www.aliexpress.com/item/1005001818136526.html) is a cost effective and small board 
with an ESP32, an OmniVision OV2640 camera, an SD card, and even a flash LED. 
It does not have an on-board USB-serial converter, I would advise to buy [one](https://www.aliexpress.com/item/1005001810692306.html) separately.

The ESP32-CAM board is powerful enough to capture images with its camera and pass them through a TensorFlow Lite interpreter running on the ESP32 itself ("edge AI").
I piloted that with a [rock-paper-scissors](https://github.com/maarten-pennings/MachineLearning/blob/main/rock-paper-scissors/rock-paper-scissors.ipynb) classifier.
A Python script on a PC trains and generates the (TensorFlow Lite) model, and saves it as a flatbuffer file. 
The flatbuffer is linked in in the ESP32 sketch, together with a TensorFlow Lite interpreter (I took [EloquentTinyML](https://github.com/eloquentarduino/EloquentTinyML)),
resulting in a rock-paper-scissors classifier, see [video](https://www.youtube.com/watch?v=dVIRe2fjQL4).

This project goes one step further.
It puts the flatbuffer on the SD card.
It also puts a configuration file on the SD card: what is the input of the TensorFlow model 
(e.g. which area of the camera to crop, how much to subsample) and what is the output of the TensorFlow model (how many classes, what are their names).

This means that by writing the two files to the SD card, one can configure the TFLcam to be either a rock-paper-scissors classifier
or, for example, a simple Lego brick classifier. 

## Architecture

The core modules of the TFLcam project are
 - TensorFlow Lite interpreter (`tflu.cpp` and `tflu.h`)
 - command interpreter (`cmds.cpp` and `cmds.h`)
 - camera driver (`cam.cpp` and `cam.h`)
 - SD card file system driver (`file.cpp` and `file.h`)
 
The command interpreter contains several commands.
Key commands configure the processing done on the camera output (crop, subsample); the reduced frame buffer is fed to the TensorFlow interpreter.
Another key command is to load the model from the SD card into the TensoFlow interpreter.
These commands commands are typically stored on a file `boot.cmd`, which is automatically run on (power-on) reset:

```
// File boot.cmd for rock, paper, scissors
image crop  left 122  top 36  width 112  height 184  xsize 28  ysize 46
labels none paper rock scissors
file load /rps.tfl // from 28x46 to 4
```

The model (`rps.tfl`) is trained on input images of 28×46 (see crop `xsize` and `ysize`) 
and expected to output a vector of four class probabilities (see labels `none`, `paper`, `rock`, and `scissors`).

Find a typical SD card image in this repo, in subdirectory [sdcard](sdcard).

## Sample run

Here is a sample run.
Observe that the ESP32 boots: it shows a banner and successful initialization of the core modules.
Then it runs `boot.cmd`, which configures the input and output for the interpreter, and loads the model.

```
   _______ ______ _
  |__   __|  ____| |
     | |  | |__  | |     ___ __ _ _ __ ___
     | |  |  __| | |    / __/ _` | '_ ` _ \
     | |  | |    | |___| (_| (_| | | | | | |
     |_|  |_|    |______\___\__,_|_| |_| |_|
TFLcam - TensorFlow Lite camera - version 0.8.0

file: success
cmds: success
cam : success
tflu: success

type 'help' for help
>> 
>> // File boot.cmd for rock, paper, scissors
>> img crop  left 122  top 36  width 112  height 184  xsize 28  ysize 46
>> labels none paper rock scissors // order as trained
>> file load /rps.tfl // from 28x46 to 4
tflu: model loaded
>> fled perm
fled: permanent (duty 100)
>> mode cont 2
mode: continuous (stable 2)
>>
predict: 0/none
predict: 2/rock
predict: 3/scissors
predict: 1/paper
predict: 2/rock
predict: 0/none
```

Manually I have switched on the flash led `fled perm` and finally 
I put the sensor in continuous mode, asking for reports, only when the prediction stabilizes after a change.

I have put my rock hand in front of the camera, changed it to scissors, paper and back to rock, and then removed it.
Notice that I indeed got one report for each.


## Limitations

At this moment the biggest limitations are that the sketch uses the camera in
gray scale only (but that could be changed), and the available RAM, a structural problem.

The RAM needs to hold the "Tensors" (the values of the "pixels" in the various layers).
Secondly, because we opted for a dynamic model, the RAM also needs to hold the model (flatbuffer).
These are `#define`s in the code, both below 100 kilo byte.
Arduino currently reports `Global variables use 118588 bytes (36%) of dynamic memory`, but 
I have trouble allocating big consecutive blocks (as needed for the tensors and the flatbuffer).

Personally I do not care too much about speed, but that might be different for your application.
At this moment, the rock paper scissors classifier runs at about 5FPS.
Note that 2/3 of the time is spend on capture and normalize, and 1/3 on prediction (inference).

```
>> mode single time
predict: 0/none
time: 222 ms, 4.50 FPS (prediction 67 ms)
>> mode single time
predict: 0/none
time: 197 ms, 5.08 FPS (prediction 66 ms)
>> mode single time
predict: 0/none
time: 189 ms, 5.29 FPS (prediction 67 ms)
>> mode single time
predict: 0/none
time: 179 ms, 5.59 FPS (prediction 66 ms)
>> mode single time
predict: 1/paper
time: 168 ms, 5.95 FPS (prediction 66 ms)
>> mode single time
predict: 3/scissors
time: 206 ms, 4.85 FPS (prediction 67 ms)
>> mode single time
predict: 2/rock
time: 226 ms, 4.42 FPS (prediction 67 ms)
>> mode single time
predict: 2/rock
time: 184 ms, 5.43 FPS (prediction 67 ms)
>> mode single time
predict: 3/scissors
time: 182 ms, 5.49 FPS (prediction 67 ms)
>> mode single time
predict: 0/none
time: 190 ms, 5.26 FPS (prediction 66 ms)
>>
```

(end)

