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
 - Command interpreter (`cmds.cpp` and `cmds.h`)
 - Camera driver (`cam.cpp` and `cam.h`)
 - SD card file system driver (`file.cpp` and `file.h`)
 - Top-level application sketch (`TFLcam.ino` and `TFLcam.h`)

The command interpreter contains several commands.
Key are the commands that configure the processing done on the camera output (cropping, sub sampling);
the reduced frame buffer is fed to the TensorFlow interpreter.
Another configuration command loads the TensorFlow model from the SD card into the TensoFlow interpreter.
Finally there is a command that configures the labels of the classes predicted by the model.

All these configuration commands are typically stored in a file `boot.cmd`.
This file is automatically run on (power-on) reset:

```
// File boot.cmd for classifier: rock, paper, scissors
@img crop  left 122  top 36  width 112  height 184  xsize 28  ysize 46
@labels none paper rock scissors // order as in rps.tfl
@file load /rps.tfl // from 28x46 to 4
```

The example model (`rps.tfl`) is trained on input images of 28×46 (see crop `xsize` and `ysize`)
and is expected to output a vector of four class probabilities (see labels `none`, `paper`, `rock`, and `scissors`).

Find a typical SD card image in this repo, in the sub directory [sdcard](sdcard).

## Sample run

Here is a sample run.
Observe that the ESP32 boots: it shows a banner and successful initialization (no error messages).
Then it runs `boot.cmd`, which configures the input and output for the TensorFlow Lite interpreter,
and loads the tensorFlow Lite model `rps.tfl`.

```
   _______ ______ _
  |__   __|  ____| |
     | |  | |__  | |     ___ __ _ _ __ ___
     | |  |  __| | |    / __/ _` | '_ ` _ \
     | |  | |    | |___| (_| (_| | | | | | |
     |_|  |_|    |______\___\__,_|_| |_| |_|
TFLcam - TensorFlow Lite camera - version 0.9.0

type 'help' for help
>> file run /boot.cmd

>> // File boot.cmd for classifier: rock, paper, scissors
>> @img crop  left 122  top 36  width 112  height 184  xsize 28  ysize 46
>> @img trans rotcw
>> @img proc histeq
>> @labels none paper rock scissors // order as in rps.tfl
>> @file load /rps.tfl // from 28x46 to 4
>>

>> mode continuous 2
mode: continuous (stable 2)
>>
predict: 0/none
predict: 2/rock
predict: 3/scissors
predict: 1/paper
predict: 2/rock
predict: 0/none
```

Manually I put the sensor in continuous mode (`mode continuous 2`).
The final `2` asks for reports, only when the prediction changes and when it is stable for 2 measurements.

I have put my rock hand in front of the camera, changed it to scissors, paper and back to rock, and then removed it.
Notice that I indeed got one report for each.

If you want to replay, put your ESP32-CAM 40cm from a white wall and keep your hand at the right position.
You can check that with `mode single ascii`.

```
>> mode single ascii
y\x: 00                                            45
  0: |ooo==-----                         -----=====O|
  1: |oo===-----                         ------====O|
  2: |oo==------                          -----====O|
  3: |o===-----                          ------====o|
  4: |====------                         ------====O|
  5: |====----- -OOOOOoo=-               ------====o|
  6: |====----- WW@@888OOOOOOo-          ------===oo|
  7: |====------WW@@8888OOO8OOOOo-       -----====oo|
  8: |====-----=8WW@@@@88888OOOOO88OOOo=------====oo|
  9: |=====----===OOO8@@@@@@8OOOOO8888888Oo=--====oo|
 10: |=o===-----========oO@@@@88888888888888Oooo=ooo|
 11: |=o===------------==ooO8@@@8@88@888@@@@@@88OOOo|
 12: |=o===----------  --==ooooW888@W8@@@WW@@@@@@@@8|
 13: |=o====-------    -=oOO8@@@@8O8W@@WW@8@@@WW@@@@|
 14: |=oo===----oOO88888@@888@@@@W@8@W@WW8888@@@@@@W|
 15: |=oo====-8WW@@@888888OO888888@WWWWW88OO8888@@@W|
 16: |=oo====OWWW@8@8888@888888@@@@WWWW@8OO8888888@@|
 17: |oooo==oOWWW@@@@88@@@@@@8@W8@WWW@88OO88888888@@|
 18: |ooooo=oo8WW@8OOOOOOOOooO@8@W@W@88OO88@88888@@W|
 19: |ooooo=oooo=o=======ooooO8WW@@@@888@@W@@@@@@WWW|
 20: |oOooooo======-======oooWWW@@@@@8@@WWWWWWWWWWWW|
 21: |OOOoooo======--====ooO8@@8@88@@@WW88@WWWWWWWWW|
 22: |OOOOooooo========oooOOW88888@WW@8888@WWWWWWWWW|
 23: |OOOOooooooo==o=ooooOOO@8O8WWWW@@@88@WW@WWW@WWW|
 24: |OOOOOoooooooooooooOOOOO@WWW@OW@@@@WWWW@@@WWWWW|
 25: |OOOOOOooooooooooooOOOOOOOOOO8@WWWWWWWWWWWWWWWW|
 26: |OOOOOOOoooooooooooOOOOOOOOOOOO8@WWWWWWWWWWWWWW|
 27: |OOOOOOOoooooooooOoOOOOOOOOOOOO88@WWWWWWWWWWWWW|
predict: 3/scissors
```

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
time: 5.10 FPS, 196 ms = 119 (capture) + 11 (crop) + 66 (predict) + 0 (output)  
>> mode single time                                                             
predict: 3/scissors                                                             
time: 6.49 FPS, 154 ms = 76 (capture) + 11 (crop) + 67 (predict) + 0 (output)   
>> mode single time                                                             
predict: 1/paper                                                                
time: 5.21 FPS, 192 ms = 114 (capture) + 11 (crop) + 67 (predict) + 0 (output)  
>> mode single time                                                             
predict: 2/rock                                                                 
time: 4.50 FPS, 222 ms = 144 (capture) + 11 (crop) + 67 (predict) + 0 (output)  
>> mode single time                                                             
predict: 0/none                                                                 
time: 5.56 FPS, 180 ms = 102 (capture) + 11 (crop) + 66 (predict) + 1 (output)  
>> mode single time                                                             
predict: 2/rock                                                                 
time: 6.13 FPS, 163 ms = 84 (capture) + 12 (crop) + 66 (predict) + 1 (output)   
>> mode single time                                                             
predict: 3/scissors                                                             
time: 4.76 FPS, 210 ms = 132 (capture) + 12 (crop) + 66 (predict) + 0 (output)  
>> mode single time                                                             
predict: 1/paper                                                                
time: 4.81 FPS, 208 ms = 130 (capture) + 11 (crop) + 67 (predict) + 0 (output)  
>>                                                                              
```

(end)

