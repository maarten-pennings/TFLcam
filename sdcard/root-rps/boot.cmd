// boot.cmd for rock, paper, scissors
@img crop  left 122  top 36  width 112  height 184  xsize 28  ysize 46
@img trans rotcw
@img proc histeq
@labels none paper rock scissors // order as trained
file load /rps.tfl // 28x46->4
