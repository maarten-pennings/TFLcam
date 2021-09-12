#define TFLCAM_VERSION "1.0.0"
#define TFLCAM_SHORTNAME "TFLcam"
#define TFLCAM_LONGNAME "TensorFlow Lite camera"
#include "core_version.h" // ARDUINO_ESP32_GIT_VER, ARDUINO_ESP32_GIT_DESC, ARDUINO_ESP32_RELEASE
#include "cmd.h"

#define CAM_WIDTH  480
#define CAM_HEIGHT 320
#define TFL_MAXPIXELS  10000


#define IMG_TRANS_VFLIP   4
#define IMG_TRANS_HMIRROR 2
#define IMG_TRANS_ROTCW   1
int img_trans;

int img_crop_x0 = 0;
int img_crop_y0 = 0;
int img_crop_width = (CAM_WIDTH/8)*5;
int img_crop_height = (CAM_HEIGHT/8)*5;
int img_crop_poolx = 5;
int img_crop_pooly = 5;

#define IMG_PROC_HISTEQ   1
int img_proc;



#define MODE_VAL_IDLE         1
#define MODE_VAL_ONCE         2
#define MODE_VAL_ASCII        3
#define MODE_VAL_CONTINUOUS   4
#define MODE_VAL_TRAIN        5
int mode_val = MODE_VAL_IDLE;


// cmdversion =================================================================================


// The version command handler
void cmdversion_main( int argc, char * argv[] ) {
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
const char cmdversion_longhelp[] PROGMEM = 
  "SYNTAX: version\n"
  "- lists version of this tools, its libs and tools to build it\n"
  "NOTES:\n"
  "- supports @-prefix to suppress output\n"
;

// Note cmd_register needs all strings to be PROGMEM strings. For the short string we do that inline with PSTR.
void cmdversion_register(void) {
  cmd_register(cmdversion_main, PSTR("version"), PSTR("version of this tools, its libs and tools to build it"), cmdversion_longhelp);
}


// cmdmode ====================================================================================

int cmdmode_train_count;
void cmdmode_streamfunc_train( int argc, char * argv[] ) {
  if( argc==0 ) {
    cmdmode_train_count++;
    char buf[5]; snprintf(buf,sizeof buf, "%04d",cmdmode_train_count); cmd_set_streamprompt(buf);
  } else {
    Serial.printf("mode: training stopped\n");
    cmd_set_streamfunc(0);
  }
}

// The mode command handler
void cmdmode_main( int argc, char * argv[] ) {
  if( argc==1 ) {
    Serial.printf("TODO: mode %d\n",mode_val);
    return;
  }
  if( argc>=2 && cmd_isprefix(PSTR("idle"),argv[1]) ) { 
    if( argc!=2 ) { Serial.printf("Error: idle does not have argument\n"); return; }
    mode_val = MODE_VAL_IDLE;
    Serial.printf("TODO: mode idle\n");
    return;
  }
  if( argc>=2 && cmd_isprefix(PSTR("once"),argv[1]) ) { 
    if( argc!=2 ) { Serial.printf("Error: once does not have argument\n"); return; }
    mode_val = MODE_VAL_ONCE;
    Serial.printf("TODO: mode once\n");
    return;
  }
  if( argc>=2 && cmd_isprefix(PSTR("ascii"),argv[1]) ) { 
    if( argc!=2 ) { Serial.printf("Error: ascii does not have argument\n"); return; }
    mode_val = MODE_VAL_ASCII;
    Serial.printf("TODO: mode ascii\n");
    return;
  }
  if( argc>=2 && cmd_isprefix(PSTR("continuous"),argv[1]) ) { 
    if( argc!=2 ) { Serial.printf("Error: continuous does not have argument\n"); return; }
    mode_val = MODE_VAL_CONTINUOUS;
    Serial.printf("TODO: mode continuous\n");
    return;
  }
  if( argc>=2 && cmd_isprefix(PSTR("train"),argv[1]) ) { 
    if( argc!=3 ) { Serial.printf("Error: train must have one directory name\n"); return; }
    mode_val = MODE_VAL_TRAIN;
    Serial.printf("Press CR to save an image; any non-empty input will abort training mode\n");
    cmdmode_train_count=0;
    cmd_set_streamprompt("0000");
    cmd_set_streamfunc(cmdmode_streamfunc_train);
    return;
  }
  Serial.printf("Error: unknown sub command (%s) of mode\n", argv[1] ); return;
}


// Note cmd_register needs all strigs to be PROGMEM strings. For longhelp we do that manually
const char cmdmode_longhelp[] PROGMEM = 
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
void cmdmode_register(void) {
  cmd_register(cmdmode_main, PSTR("mode"), PSTR("access to the modes on the SD card"), cmdmode_longhelp);
}


// cmdfile ====================================================================================

void cmdfile_dir() {
    Serial.printf("TODO: dir\n");
}

void cmdfile_show(char *name) {
    Serial.printf("TODO: show %s\n",name);
}

void cmdfile_run(char *name) {
    Serial.printf("TODO: run %s\n",name);
}

void cmdfile_load(char *name) {
    Serial.printf("TODO: load %s\n",name);
}

// The file command handler
void cmdfile_main( int argc, char * argv[] ) {
  if( argc==1 ) {
    cmdfile_dir();
    return;
  }
  if( argc>=2 && cmd_isprefix(PSTR("dir"),argv[1]) ) { 
    if( argc!=2 ) { Serial.printf("Error: dir does not have argument\n"); return; }
    cmdfile_dir();
    return;
  }
  if( argc>=2 && cmd_isprefix(PSTR("show"),argv[1]) ) { 
    if( argc!=3 ) { Serial.printf("Error: show must have one filename\n"); return; }
    cmdfile_show(argv[2]);
    return;
  }
  if( argc>=2 && cmd_isprefix(PSTR("run"),argv[1]) ) { 
    if( argc!=3 ) { Serial.printf("Error: run must have one filename\n"); return; }
    cmdfile_run(argv[2]);
    return;
  }
  if( argc>=2 && cmd_isprefix(PSTR("load"),argv[1]) ) { 
    if( argc!=3 ) { Serial.printf("Error: run must have one filename\n"); return; }
    cmdfile_load(argv[2]);
    return;
  }
  Serial.printf("Error: unknown sub command (%s) of file\n", argv[1] ); return;
}


// Note cmd_register needs all strigs to be PROGMEM strings. For longhelp we do that manually
const char cmdfile_longhelp[] PROGMEM = 
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
void cmdfile_register(void) {
  cmd_register(cmdfile_main, PSTR("file"), PSTR("access to the files on the SD card"), cmdfile_longhelp);
}


// cmdimg =====================================================================================


void cmdimg_trans_print(int help) {
  Serial.printf("trans:");
  if( img_trans & IMG_TRANS_VFLIP   ) Serial.printf(" vflip");
  if( img_trans & IMG_TRANS_HMIRROR ) Serial.printf(" hmirror");
  if( img_trans & IMG_TRANS_ROTCW   ) Serial.printf(" rotcw");
  if( img_trans == 0                ) Serial.printf(" none");
  Serial.printf("\n");
  if( help ) {
    char buf[] = "o*++";
    #define SWAP(i,j) do { char t=buf[i]; buf[i]=buf[j]; buf[j]= t; } while(0)
    if( img_trans & IMG_TRANS_VFLIP   ) { SWAP(0,2); SWAP(1,3); }
    if( img_trans & IMG_TRANS_HMIRROR ) { SWAP(0,1); SWAP(2,3); }
    if( img_trans & IMG_TRANS_ROTCW   ) { char t=buf[0]; buf[0]=buf[2]; buf[2]=buf[3]; buf[3]=buf[1]; buf[1]=t; }
    // Only the last transformation switches landscape to portrait
    if( img_trans & IMG_TRANS_ROTCW   ) {
      Serial.printf("trans: o+*      %c%c\n", buf[0],buf[1] );
      Serial.printf("trans: +++  ->  ++\n" );
      Serial.printf("trans:          %c%c\n", buf[2],buf[3] );
    } else {
      Serial.printf("trans: o+*      %c+%c\n", buf[0],buf[1] );
      Serial.printf("trans: +++  ->  %c+%c\n", buf[2],buf[3] );
    }
  }
}

void cmdimg_proc_print() {
  Serial.printf("proc :");
  if( img_proc & IMG_PROC_HISTEQ   ) Serial.printf(" histeq");
  if( img_proc == 0                ) Serial.printf(" none");
  Serial.printf("\n");
}

void cmdimg_crop_print() {
  Serial.printf("crop : (%d,%d) %d*%d, pool (%d,%d)\n",img_crop_x0,img_crop_y0,img_crop_width,img_crop_height,img_crop_poolx,img_crop_pooly);
}

void cmdimg_img_print() {
    int w = img_crop_width/img_crop_poolx;
    int h = img_crop_height/img_crop_pooly;
    if( img_trans & IMG_TRANS_ROTCW ) { int t=w; w=h; h=t; }
    Serial.printf("shape: input %d*%d output %d*%d\n", CAM_WIDTH, CAM_HEIGHT,w,h);
}

// The img command handler
void cmdimg_main( int argc, char * argv[] ) {
  // subcommand <none> ---------------------------------
  if( argc==1 ) {
    if( argv[0][0]!='@') cmdimg_crop_print();
    if( argv[0][0]!='@') cmdimg_trans_print(0);
    if( argv[0][0]!='@') cmdimg_proc_print();
    cmdimg_img_print();
    return;
  }
  // subcommand crop -----------------------------------
  if( argc>=2 && cmd_isprefix(PSTR("crop"),argv[1]) ) { 
    if( argc==2 ) { 
      cmdimg_crop_print();
      return;
    }
    int x0=img_crop_x0;
    int y0=img_crop_y0;
    int width=img_crop_width;
    int height=img_crop_height;
    int poolx=img_crop_poolx;
    int pooly=pooly;
    int fx0,fy0,fwidth,fheight,fpool; fx0=fy0=fwidth=fheight=fpool=0; // not (yet) set be user
    bool ok;
    int ix=2;
    while( ix<argc ) {
      if( cmd_isprefix(PSTR("x0"),argv[ix]) ) { 
        if( fx0 ) { Serial.printf("Error: x0 occurs more then once\n"); return; }
        fx0=1; ix++;
        if( ix>=argc ) { Serial.printf("Error: x0 needs a value\n"); return; }
        ok = cmd_parse_dec(argv[ix],&x0) ;
        if( !ok ) { Serial.printf("Error: error in x0 value\n"); return; }
        if( x0<0 || x0>=CAM_WIDTH ) { Serial.printf("Error: x0 (%d) must be 0..%d\n",x0,CAM_WIDTH-1); return; }
      } else if( cmd_isprefix(PSTR("y0"),argv[ix]) ) {
        if( fy0 ) { Serial.printf("Error: y0 occurs more then once\n"); return; }
        fy0=1; ix++;
        if( ix>=argc ) { Serial.printf("Error: y0 needs a value\n"); return; }
        ok = cmd_parse_dec(argv[ix],&y0) ;
        if( !ok ) { Serial.printf("Error: error in y0 value\n"); return; }
        if( y0<0 || y0>=CAM_HEIGHT ) { Serial.printf("Error: y0 (%d) must be 0..%d\n",y0,CAM_HEIGHT-1); return; }
      } else if( cmd_isprefix(PSTR("width"),argv[ix]) ) {
        if( fwidth ) { Serial.printf("Error: width occurs more then once\n"); return; }
        fwidth=1; ix++;
        if( ix>=argc ) { Serial.printf("Error: width needs a value\n"); return; }
        ok = cmd_parse_dec(argv[ix],&width) ;
        if( !ok ) { Serial.printf("Error: error in width value\n"); return; }
        if( width<1 || width>CAM_WIDTH ) { Serial.printf("Error: width (%d) must be 1..%d\n",width,CAM_WIDTH); return; }
      } else if( cmd_isprefix(PSTR("height"),argv[ix]) ) {
        if( fheight ) { Serial.printf("Error: height occurs more then once\n"); return; }
        fheight=1; ix++;
        if( ix>=argc ) { Serial.printf("Error: height needs a value\n"); return; }
        ok = cmd_parse_dec(argv[ix],&height) ;
        if( !ok ) { Serial.printf("Error: error in height value\n"); return; }
        if( height<1 || height>CAM_HEIGHT ) { Serial.printf("Error: height (%d) must be 1..%d\n",height,CAM_HEIGHT); return; }
        if( height<0 ) { Serial.printf("Error: height value is negative\n"); return; }
      } else if( cmd_isprefix(PSTR("pool"),argv[ix]) ) {
        if( fpool ) { Serial.printf("Error: pool occurs more then once\n"); return; }
        fpool=1; ix++;
        if( ix>=argc ) { Serial.printf("Error: pool needs a value\n"); return; }
        ok = cmd_parse_dec(argv[ix],&poolx) ;
        if( !ok ) { Serial.printf("Error: error in pool value\n"); return; }
        if( poolx<0 ) { Serial.printf("Error: pool value is negative\n"); return; }
        if( poolx<1 || poolx>CAM_WIDTH ) { Serial.printf("Error: pool (%d) must be 1..%d\n",poolx,CAM_WIDTH); return; }
        ok = ix+1<argc && cmd_parse_dec(argv[ix+1],&pooly) ;
        if( ok ) { ix++; /* pool has two args */ } else { /* pool has one arg */ pooly=poolx; }
        if( pooly<1 || pooly>CAM_HEIGHT ) { Serial.printf("Error: pooly (%d) must be 1..%d\n",pooly,CAM_HEIGHT); return; }
      } else {
        Serial.printf("Error: crop has unknown arg (%s)\n", argv[ix]); return;
      }
      ix++;
    }
    // extra tests
    if( x0+width>CAM_WIDTH ) { Serial.printf("Error: x0+width (%d+%d) must not exceed width (%d)\n",x0,width,CAM_WIDTH); return; }
    if( y0+height>CAM_HEIGHT) { Serial.printf("Error: y0+heigth (%d+%d) must not exceed height (%d)\n",y0,height,CAM_HEIGHT); return; }
    if( width%poolx != 0 ) { Serial.printf("Error: width (%d) must be divisable by poolx (%d)\n",width,poolx); return; }
    if( height%pooly != 0 ) { Serial.printf("Error: height (%d) must be divisable by pooly (%d)\n",height,pooly); return; }
    if( (width/poolx)*(height/pooly)>TFL_MAXPIXELS ) { Serial.printf("Error: resulting size width/poolx*height/pooly (%d/%d*%d/%d) must not exceed TFL buffer %d\n",width,poolx,height,pooly,TFL_MAXPIXELS); return; }
    // all ok
    img_crop_x0 = x0;
    img_crop_y0 = y0;
    img_crop_width = width;
    img_crop_height = height;
    img_crop_poolx = poolx;
    img_crop_pooly = pooly;
    if( argv[0][0]!='@') cmdimg_crop_print();
    return;
  }
  // subcommand trans ----------------------------------
  if( argc>=2 && cmd_isprefix(PSTR("trans"),argv[1]) ) { 
    if( argc==2 ) {
      cmdimg_trans_print(1);
      return;
    }
    if( argc==3 && cmd_isprefix(PSTR("none"),argv[2]) ) { 
      img_trans=0;
      if( argv[0][0]!='@') cmdimg_trans_print(1);
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
    img_trans = vflip*IMG_TRANS_VFLIP + hmirror*IMG_TRANS_HMIRROR + rotcw*IMG_TRANS_ROTCW;
    if( argv[0][0]!='@') cmdimg_trans_print(1);
    return;
  }
  // subcommand proc -----------------------------------
  if( argc>=2 && cmd_isprefix(PSTR("proc"),argv[1]) ) { 
    if( argc==2 ) {
      cmdimg_proc_print();
      return;
    }
    if( argc==3 && cmd_isprefix(PSTR("none"),argv[2]) ) { 
      img_proc=0;
      if( argv[0][0]!='@') cmdimg_proc_print();
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
    img_proc = histeq*IMG_PROC_HISTEQ;
    if( argv[0][0]!='@') cmdimg_proc_print();
    return;
  }
  // subcommand <error> --------------------------------
  Serial.printf("Error: unknown sub command (%s) of img\n", argv[1] ); return;
}


// Note cmd_register needs all strigs to be PROGMEM strings. For longhelp we do that manually
const char cmdimg_longhelp[] PROGMEM = 
  "SYNTAX: img\n"
  "- shows summary of image processing configuration\n"
  "SYNTAX: img crop ( x0 <#> | y0 <#> | width <#> | height <#> | pool <#> [<#>] )*\n"
  "- without arguments show current crop configuration (\"reduce size\")\n"
  "- with arguments sets crop configuration\n"
  "- crop rectangle has upper left (x0,y0) and size width*height)\n"
  "- crop averages blocks of poolx*pooly pixels to 1\n"
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
void cmdimg_register(void) {
  cmd_register(cmdimg_main, PSTR("img"), PSTR("configure image processing parameters"), cmdimg_longhelp);
}


void setup() {
  Serial.begin( 115200 );
  while( ! Serial ) delay(250);
  Serial.printf("\n%s - %s - version %s\n", TFLCAM_SHORTNAME, TFLCAM_LONGNAME, TFLCAM_VERSION);
  cmd_begin();
  // Registration order is list order
  cmdecho_register();   // Register the built-in echo command
  cmdfile_register();   // Register our own file command
  cmdhelp_register();   // Register the built-in help command
  cmdimg_register();    // Register our own img command
  cmdmode_register();   // Register our own mode command
  cmdversion_register();// Register our own version command
}

uint32_t mode_time;
int      mode_spoof=0;
void loop() {
  cmd_pollserial();
  if( mode_val!=MODE_VAL_IDLE && mode_val!=MODE_VAL_TRAIN ) {
    if( !mode_spoof ) {
      mode_spoof=1;
      mode_time=millis();
    } else {
      if( millis()-mode_time>5000 ) {
        // spoof a shoot-predict
        Serial.print("spoof: predict=3\n");
        mode_time = millis();
        // state transition
        if( mode_val==MODE_VAL_ONCE ) { mode_spoof=0; mode_val=MODE_VAL_IDLE; }
        else if( mode_val==MODE_VAL_ASCII ) { Serial.printf("spoof: ASCII\n"); mode_spoof=0; mode_val=MODE_VAL_IDLE; }
        else if( mode_val==MODE_VAL_CONTINUOUS ) { /*skip*/ }
      }
    }
  }
}
