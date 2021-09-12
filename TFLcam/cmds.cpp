// cmds.cpp - the commands used by TFLcam
#include <Arduino.h>
#include "core_version.h" // ARDUINO_ESP32_GIT_VER, ARDUINO_ESP32_GIT_DESC, ARDUINO_ESP32_RELEASE

#include "cmd.h"          // command interpreter
#include "cmds.h"         // own interface

// The commands control the app (and some if its libs)
#include "TFLcam.h"       // application
#include "cam.h"          // camera configuration


// cmds_sys =====================================================================================


// The sys command handler
static void cmds_sys_main( int argc, char * argv[] ) {
  if( argc==2 && cmd_isprefix(PSTR("reboot"),argv[1])) {
    ESP.restart();
  }
  Serial.printf("Error: unknown arguments for sys\n" ); return;
}


// Note cmd_register needs all strigs to be PROGMEM strings. For longhelp we do that manually
static const char cmds_sys_longhelp[] PROGMEM = 
  "SYNTAX: sys reboot\n"
  "- reboot the system\n"
;

// TODO: sys clk - to get and set clock
// TODO: WiFi off?

// Note cmd_register needs all strings to be PROGMEM strings. For the short string we do that inline with PSTR.
static int cmds_sys_register(void) {
  return cmd_register(cmds_sys_main, PSTR("sys"), PSTR("system commands (like reboot)"), cmds_sys_longhelp);
}


// cmds_version =================================================================================


// The version command handler
static void cmds_version_main( int argc, char * argv[] ) {
  if( argc==1 ) {
    Serial.printf( "app     : %s (%s) %s\n", TFLCAM_LONGNAME, TFLCAM_SHORTNAME, TFLCAM_VERSION);
    if( argv[0][0]!='@') Serial.printf( "library : cmd %s\n", CMD_VERSION);
    if( argv[0][0]!='@') Serial.printf( "runtime : " ARDUINO_ESP32_RELEASE "\n" );
    if( argv[0][0]!='@') Serial.printf( "compiler: " __VERSION__ "\n" );
    if( argv[0][0]!='@') Serial.printf( "arduino : %d\n",ARDUINO );
    if( argv[0][0]!='@') Serial.printf( "compiled: " __DATE__ ", " __TIME__ "\n" );
    return;
  }
  Serial.printf("Error: unknown arguments for version\n" ); return;
}


// Note cmd_register needs all strigs to be PROGMEM strings. For longhelp we do that manually
static const char cmds_version_longhelp[] PROGMEM = 
  "SYNTAX: version\n"
  "- lists version of this tools, its libs and tools to build it\n"
  "NOTES:\n"
  "- supports @-prefix to suppress output\n"
;

// Note cmd_register needs all strings to be PROGMEM strings. For the short string we do that inline with PSTR.
static int cmds_version_register(void) {
  return cmd_register(cmds_version_main, PSTR("version"), PSTR("version of this tools, its libs and tools to build it"), cmds_version_longhelp);
}


// cmds_mode ====================================================================================

static int cmds_mode_train_count;

static void cmds_mode_streamfunc_train( int argc, char * argv[] ) {
  if( argc==0 ) {
    cmds_mode_train_count++;
    char buf[5]; snprintf(buf,sizeof buf, "%04d",cmds_mode_train_count); cmd_set_streamprompt(buf);
  } else {
    Serial.printf("mode: training stopped\n");
    cmd_set_streamfunc(0);
  }
}

static void cmds_mode_show() {
  if( tflcam_mode==TFLCAM_MODE_IDLE ) Serial.printf("mode: idle\n");
  else if( tflcam_mode==TFLCAM_MODE_CONTINUOUS ) Serial.printf("mode: continuous\n");
  else if( tflcam_mode==TFLCAM_MODE_TRAIN ) Serial.printf("mode: train\n");
  else Serial.printf("mode: <unknown>\n");
}

// The mode command handler
static void cmds_mode_main( int argc, char * argv[] ) {
  if( argc==1 ) {
    cmds_mode_show();
    return;
  }
  if( argc>=2 && cmd_isprefix(PSTR("idle"),argv[1]) ) { 
    if( argc!=2 ) { Serial.printf("Error: idle does not have argument\n"); return; }
    tflcam_mode = TFLCAM_MODE_IDLE;
    cmds_mode_show();
    return;
  }
  if( argc>=2 && cmd_isprefix(PSTR("once"),argv[1]) ) { 
    if( argc!=2 ) { Serial.printf("Error: once does not have argument\n"); return; }
    tflcam_capture_predict(0);
    tflcam_mode = TFLCAM_MODE_IDLE;
    cmds_mode_show();
    return;
  }
  if( argc>=2 && cmd_isprefix(PSTR("ascii"),argv[1]) ) { 
    if( argc!=2 ) { Serial.printf("Error: ascii does not have argument\n"); return; }
    tflcam_capture_predict(1);
    tflcam_mode = TFLCAM_MODE_IDLE;
    cmds_mode_show();
    return;
  }
  if( argc>=2 && cmd_isprefix(PSTR("continuous"),argv[1]) ) { 
    if( argc!=2 ) { Serial.printf("Error: continuous does not have argument\n"); return; }
    tflcam_mode = TFLCAM_MODE_CONTINUOUS;
    cmds_mode_show();
    return;
  }
  if( argc>=2 && cmd_isprefix(PSTR("train"),argv[1]) ) { 
    if( argc!=3 ) { Serial.printf("Error: train must have one directory name\n"); return; }
    tflcam_mode = TFLCAM_MODE_TRAIN;
    cmds_mode_show();
    Serial.printf("Press CR to save an image; any non-empty input will abort training mode\n");
    cmds_mode_train_count=0;
    cmd_set_streamprompt("0000");
    cmd_set_streamfunc(cmds_mode_streamfunc_train);
    return;
  }
  Serial.printf("Error: unknown sub command (%s) of mode\n", argv[1] ); return;
}


// Note cmd_register needs all strigs to be PROGMEM strings. For longhelp we do that manually
static const char cmds_mode_longhelp[] PROGMEM = 
  "SYNTAX: mode\n"
  "- shows active mode\n"
  "SYNTAX: mode idle\n"
  "- switch camera off, no TensorFlow predictions\n"
  "SYNTAX: mode once\n"
  "- takes a single shot, and prints TensorFlow prediction, goes to idle mode\n"
  "SYNTAX: mode ascii\n"
  "- same as 'once' but also prints an ASCII rendering of the (cropped) camera image\n"
  "- typically used during configuration for training\n"
  "SYNTAX: mode continuous\n"
  "- same as once but does not go to idle\n"
  "- typically stopped with command 'mode idle'\n"
  "SYNTAX: mode train <dir>\n"
  "- goes in training mode: after each CR an image is saved in <dir>\n"
  "- any other command then CR returns to idle mode\n"
  "- <dir> must not exist, and will be created\n"
;

// Note cmd_register needs all strings to be PROGMEM strings. For the short string we do that inline with PSTR.
static int cmds_mode_register(void) {
  return cmd_register(cmds_mode_main, PSTR("mode"), PSTR("access to the modes on the SD card"), cmds_mode_longhelp);
}


// cmds_file ====================================================================================

static void cmds_file_dir() {
    Serial.printf("TODO: dir\n");
}

static void cmds_file_show(char *name) {
    Serial.printf("TODO: show %s\n",name);
}

static void cmds_file_run(char *name) {
    Serial.printf("TODO: run %s\n",name);
}

static void cmds_file_load(char *name) {
    Serial.printf("TODO: load %s\n",name);
}

// The file command handler
static void cmds_file_main( int argc, char * argv[] ) {
  if( argc==1 ) {
    cmds_file_dir();
    return;
  }
  if( argc>=2 && cmd_isprefix(PSTR("dir"),argv[1]) ) { 
    if( argc!=2 ) { Serial.printf("Error: dir does not have argument\n"); return; }
    cmds_file_dir();
    return;
  }
  if( argc>=2 && cmd_isprefix(PSTR("show"),argv[1]) ) { 
    if( argc!=3 ) { Serial.printf("Error: show must have one filename\n"); return; }
    cmds_file_show(argv[2]);
    return;
  }
  if( argc>=2 && cmd_isprefix(PSTR("run"),argv[1]) ) { 
    if( argc!=3 ) { Serial.printf("Error: run must have one filename\n"); return; }
    cmds_file_run(argv[2]);
    return;
  }
  if( argc>=2 && cmd_isprefix(PSTR("load"),argv[1]) ) { 
    if( argc!=3 ) { Serial.printf("Error: run must have one filename\n"); return; }
    cmds_file_load(argv[2]);
    return;
  }
  Serial.printf("Error: unknown sub command (%s) of file\n", argv[1] ); return;
}


// Note cmd_register needs all strigs to be PROGMEM strings. For longhelp we do that manually
static const char cmds_file_longhelp[] PROGMEM = 
  "SYNTAX: file [ dir ]\n"
  "- lists all files\n"
  "SYNTAX: file show <name>\n"
  "- shows the content of file with 'name'\n"
  "SYNTAX: file run <name>\n"
  "- runs all commands in file with 'name'\n"
  "SYNTAX: file load <name>\n"
  "- loads the flatbuffer file with 'name' into the TensorFlow interpreter\n"
  "NOTES:\n"
  "- supports subdirectories (separated with '/'), always address from root\n"
  "- if '/boot.cmd' exists, it will automatically be run on startup\n"
;

// Note cmd_register needs all strings to be PROGMEM strings. For the short string we do that inline with PSTR.
static int cmds_file_register(void) {
  return cmd_register(cmds_file_main, PSTR("file"), PSTR("access to the files on the SD card"), cmds_file_longhelp);
}


// cmds_img =====================================================================================


static void cmds_img_trans_print(int help) {
  Serial.printf("trans:");
  if( cam_trans_flags & CAM_TRANS_VFLIP   ) Serial.printf(" vflip");
  if( cam_trans_flags & CAM_TRANS_HMIRROR ) Serial.printf(" hmirror");
  if( cam_trans_flags & CAM_TRANS_ROTCW   ) Serial.printf(" rotcw");
  if( cam_trans_flags == 0                ) Serial.printf(" none");
  Serial.printf("\n");
  if( help ) {
    char buf[] = "o*++";
    #define SWAP(i,j) do { char t=buf[i]; buf[i]=buf[j]; buf[j]= t; } while(0)
    if( cam_trans_flags & CAM_TRANS_VFLIP   ) { SWAP(0,2); SWAP(1,3); }
    if( cam_trans_flags & CAM_TRANS_HMIRROR ) { SWAP(0,1); SWAP(2,3); }
    if( cam_trans_flags & CAM_TRANS_ROTCW   ) { char t=buf[0]; buf[0]=buf[2]; buf[2]=buf[3]; buf[3]=buf[1]; buf[1]=t; }
    // Only the last transformation switches landscape to portrait
    if( cam_trans_flags & CAM_TRANS_ROTCW   ) {
      Serial.printf("trans: o+*      %c%c\n", buf[0],buf[1] );
      Serial.printf("trans: +++  ->  ++\n" );
      Serial.printf("trans:          %c%c\n", buf[2],buf[3] );
    } else {
      Serial.printf("trans: o+*      %c+%c\n", buf[0],buf[1] );
      Serial.printf("trans: +++  ->  %c+%c\n", buf[2],buf[3] );
    }
  }
}

static void cmds_cam_imgproc_print() {
  Serial.printf("proc :");
  if( cam_imgproc_flags & CAM_IMGPROC_HISTEQ   ) Serial.printf(" histeq");
  if( cam_imgproc_flags == 0                   ) Serial.printf(" none");
  Serial.printf("\n");
}

static void cmds_img_crop_print() {
  Serial.printf("crop : left %d  top %d  width %d  height %d  xsize %d ysize %d ", cam_crop_left, cam_crop_top, cam_crop_width, cam_crop_height,cam_crop_xsize, cam_crop_ysize);
  if( cam_crop_width%cam_crop_xsize==0 ) Serial.printf("(poolx %d ",cam_crop_width/cam_crop_xsize); else Serial.printf("(poolx %.2f",cam_crop_width/(float)cam_crop_xsize); 
  if( cam_crop_height%cam_crop_ysize==0 ) Serial.printf(" pooly %d)\n",cam_crop_height/cam_crop_ysize); else Serial.printf(" pooly %.2f)\n",cam_crop_height/(float)cam_crop_ysize); 
}

static void cmds_img_img_print() {
  Serial.printf("shape: input %d*%d output %d*%d\n", CAM_CAPTURE_WIDTH, CAM_CAPTURE_HEIGHT,cam_outwidth(),cam_outheight());
}

// The img command handler
static void cmds_img_main( int argc, char * argv[] ) {
  // subcommand <none> ---------------------------------
  if( argc==1 ) {
    if( argv[0][0]!='@') cmds_img_crop_print();
    if( argv[0][0]!='@') cmds_img_trans_print(0);
    if( argv[0][0]!='@') cmds_cam_imgproc_print();
    cmds_img_img_print();
    return;
  }
  // subcommand crop -----------------------------------
  if( argc>=2 && cmd_isprefix(PSTR("crop"),argv[1]) ) { 
    if( argc==2 ) { 
      cmds_img_crop_print();
      return;
    }
    int left=cam_crop_left;
    int top=cam_crop_top;
    int width=cam_crop_width;
    int height=cam_crop_height;
    int xsize=cam_crop_xsize;
    int ysize=cam_crop_ysize;
    int fleft,ftop,fwidth,fheight,fxsize,fysize; fleft=ftop=fwidth=fheight=fxsize=fysize=0; // not (yet) set be user
    bool ok;
    int ix=2;
    while( ix<argc ) {
      if( cmd_isprefix(PSTR("left"),argv[ix]) ) { 
        if( fleft ) { Serial.printf("Error: left occurs more then once\n"); return; }
        fleft=1; ix++;
        if( ix>=argc ) { Serial.printf("Error: left needs a value\n"); return; }
        ok = cmd_parse_dec(argv[ix],&left) ;
        if( !ok ) { Serial.printf("Error: error in left value\n"); return; }
        if( left<0 || left>=CAM_CAPTURE_WIDTH ) { Serial.printf("Error: left (%d) must be 0..%d\n",left,CAM_CAPTURE_WIDTH-1); return; }
      } else if( cmd_isprefix(PSTR("top"),argv[ix]) ) {
        if( ftop ) { Serial.printf("Error: top occurs more then once\n"); return; }
        ftop=1; ix++;
        if( ix>=argc ) { Serial.printf("Error: top needs a value\n"); return; }
        ok = cmd_parse_dec(argv[ix],&top) ;
        if( !ok ) { Serial.printf("Error: error in top value\n"); return; }
        if( top<0 || top>=CAM_CAPTURE_HEIGHT ) { Serial.printf("Error: top (%d) must be 0..%d\n",top,CAM_CAPTURE_HEIGHT-1); return; }
      } else if( cmd_isprefix(PSTR("width"),argv[ix]) ) {
        if( fwidth ) { Serial.printf("Error: width occurs more then once\n"); return; }
        fwidth=1; ix++;
        if( ix>=argc ) { Serial.printf("Error: width needs a value\n"); return; }
        ok = cmd_parse_dec(argv[ix],&width) ;
        if( !ok ) { Serial.printf("Error: error in width value\n"); return; }
        if( width<1 || width>CAM_CAPTURE_WIDTH ) { Serial.printf("Error: width (%d) must be 1..%d\n",width,CAM_CAPTURE_WIDTH); return; }
      } else if( cmd_isprefix(PSTR("height"),argv[ix]) ) {
        if( fheight ) { Serial.printf("Error: height occurs more then once\n"); return; }
        fheight=1; ix++;
        if( ix>=argc ) { Serial.printf("Error: height needs a value\n"); return; }
        ok = cmd_parse_dec(argv[ix],&height) ;
        if( !ok ) { Serial.printf("Error: error in height value\n"); return; }
        if( height<1 || height>CAM_CAPTURE_HEIGHT ) { Serial.printf("Error: height (%d) must be 1..%d\n",height,CAM_CAPTURE_HEIGHT); return; }
      } else if( cmd_isprefix(PSTR("xsize"),argv[ix]) ) {
        if( fxsize ) { Serial.printf("Error: xsize occurs more then once\n"); return; }
        fxsize=1; ix++;
        if( ix>=argc ) { Serial.printf("Error: xsize needs a value\n"); return; }
        ok = cmd_parse_dec(argv[ix],&xsize) ;
        if( !ok ) { Serial.printf("Error: error in xsize value\n"); return; }
        if( xsize<1 || xsize>CAM_CAPTURE_WIDTH ) { Serial.printf("Error: xsize (%d) must be 1..%d\n",xsize,CAM_CAPTURE_WIDTH); return; }
      } else if( cmd_isprefix(PSTR("ysize"),argv[ix]) ) {
        if( fysize ) { Serial.printf("Error: ysize occurs more then once\n"); return; }
        fysize=1; ix++;
        if( ix>=argc ) { Serial.printf("Error: ysize needs a value\n"); return; }
        ok = cmd_parse_dec(argv[ix],&ysize) ;
        if( !ok ) { Serial.printf("Error: error in ysize value\n"); return; }
        if( ysize<1 || ysize>CAM_CAPTURE_HEIGHT ) { Serial.printf("Error: ysize (%d) must be 1..%d\n",ysize,CAM_CAPTURE_HEIGHT); return; }
      } else {
        Serial.printf("Error: crop has unknown arg (%s)\n", argv[ix]); return;
      }
      ix++;
    }
    // extra tests
    if( left+width>CAM_CAPTURE_WIDTH ) { Serial.printf("Error: left+width (%d+%d) must not exceed cam width (%d)\n",left,width,CAM_CAPTURE_WIDTH); return; }
    if( top+height>CAM_CAPTURE_HEIGHT) { Serial.printf("Error: top+height (%d+%d) must not exceed cam height (%d)\n",top,height,CAM_CAPTURE_HEIGHT); return; }
    if( xsize*ysize>TFLCAM_MAXPIXELS ) { Serial.printf("Error: resulting size %d*%d exceeds TFL buffer size %d\n",xsize,ysize,TFLCAM_MAXPIXELS); return; }
    // all ok
    cam_crop_left = left;
    cam_crop_top = top;
    cam_crop_width = width;
    cam_crop_height = height;
    cam_crop_xsize = xsize;
    cam_crop_ysize = ysize;
    if( argv[0][0]!='@') cmds_img_crop_print();
    return;
  }
  // subcommand trans ----------------------------------
  if( argc>=2 && cmd_isprefix(PSTR("trans"),argv[1]) ) { 
    if( argc==2 ) {
      cmds_img_trans_print(1);
      return;
    }
    if( argc==3 && cmd_isprefix(PSTR("none"),argv[2]) ) { 
      cam_trans_flags=0;
      if( argv[0][0]!='@') cmds_img_trans_print(1);
      return;
    }
    int vflip=0;
    int hmirror=0;
    int rotcw=0;
    int ix=2;
    while( ix<argc ) {
      if( cmd_isprefix(PSTR("vflip"),argv[ix]) ) {
        if( vflip==1 ) { Serial.printf("Error: vflip occurs more then once\n"); return; }
        vflip=1;
      } else if( cmd_isprefix(PSTR("hmirror"),argv[ix]) ) {
        if( hmirror==1 ) { Serial.printf("Error: hmirror occurs more then once\n"); return; }
        hmirror=1;
      } else if( cmd_isprefix(PSTR("rotcw"),argv[ix]) ) {
        if( rotcw==1 ) { Serial.printf("Error: rotcw occurs more then once\n"); return; }
        rotcw=1;
      } else {
        Serial.printf("Error: unknown transformation (%s)\n", argv[ix]); return;
      }
      ix++;      
    }
    cam_trans_flags = vflip*CAM_TRANS_VFLIP + hmirror*CAM_TRANS_HMIRROR + rotcw*CAM_TRANS_ROTCW;
    if( argv[0][0]!='@') cmds_img_trans_print(1);
    return;
  }
  // subcommand proc -----------------------------------
  if( argc>=2 && cmd_isprefix(PSTR("proc"),argv[1]) ) { 
    if( argc==2 ) {
      cmds_cam_imgproc_print();
      return;
    }
    if( argc==3 && cmd_isprefix(PSTR("none"),argv[2]) ) { 
      cam_imgproc_flags=0;
      if( argv[0][0]!='@') cmds_cam_imgproc_print();
      return;
    }
    int histeq=0;
    int ix=2;
    while( ix<argc ) {
      if( cmd_isprefix(PSTR("histeq"),argv[ix]) ) {
        if( histeq==1 ) { Serial.printf("Error: histeq occurs more then once\n"); return; }
        histeq=1;
      } else {
        Serial.printf("Error: unknown image processing (%s)\n", argv[ix]); return;
      }
      ix++;      
    }
    cam_imgproc_flags = histeq*CAM_IMGPROC_HISTEQ;
    if( argv[0][0]!='@') cmds_cam_imgproc_print();
    return;
  }
  // subcommand <error> --------------------------------
  Serial.printf("Error: unknown sub command (%s) of img\n", argv[1] ); return;
}


// Note cmd_register needs all strigs to be PROGMEM strings. For longhelp we do that manually
static const char cmds_img_longhelp[] PROGMEM = 
  "SYNTAX: img\n"
  "- shows summary of image processing configuration\n"
  "SYNTAX: img crop ((top|left|width|height|xsize|ysize) <num> )...\n"
  "- without arguments show current crop configuration (\"size reduction\")\n"
  "- with arguments sets crop configuration\n"
  "- crop rectangle starts at top,left and has size width*height\n"
  "- crop averages so that output has size xsize,ysize\n"
  "SYNTAX: img trans ( none | [vflip] [hmirror] [rotcw] )\n"
  "- without arguments show current transformation settings (\"flip image\")\n"
  "- 'none' removes all transformation settings\n"
  "- use one or more of 'vflip', 'hmirror', or 'rotcw'\n"
  "  for vertical flip, horizontal mirror or rotate clockwise (all three for ccw)\n"
  "SYNTAX: img proc ( none | [histeq] )\n"
  "- without arguments show current image processing settings (\"boost quality\")\n"
  "- 'none' removes all image processing settings\n"
  "- use 'histeq' for histogram equalization\n"
  "NOTES:\n"
  "- supports @-prefix to suppress output\n"
;


// Note cmd_register needs all strings to be PROGMEM strings. For the short string we do that inline with PSTR.
static int cmds_img_register(void) {
  return cmd_register(cmds_img_main, PSTR("img"), PSTR("configure image processing parameters"), cmds_img_longhelp);
}


// Registers commands (may be called before cmd_begin). Prints problems also to Serial.
void cmds_setup() {
  // Registration order is list order
  int num;
  num=cmdecho_register();     // built-in echo command
  num=cmds_file_register();
  num=cmdhelp_register();     // built-in help command
  num=cmds_img_register();
  num=cmds_mode_register();
  num=cmds_sys_register();
  num=cmds_version_register();
 if( num>=0 ) Serial.printf("cmds: success\n"); else Serial.printf("cmds: FAIL\n"); // too many commands registered
}
