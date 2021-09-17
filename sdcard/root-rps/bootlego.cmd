// File bootlego.cmd for classifier: rock, paper, scissors in Lego Sensor
@fled auto 25
@img crop  left 120  top 20  width 128 height 208  xsize 28  ysize 46
@img trans rotcw
@img proc histeq
@labels none paper rock scissors // order as in rps.tfl
@file load /rps.tfl // from 28x46 to 4
