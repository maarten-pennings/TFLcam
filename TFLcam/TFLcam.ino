#define LTFC_VERSION "1.0.0"
#define LTFC_NAME "LTFC"
#include "core_version.h" // ARDUINO_ESP32_GIT_VER, ARDUINO_ESP32_GIT_DESC, ARDUINO_ESP32_RELEASE
#include "cmd.h"

#define CAM_WIDTH  4000
#define CAM_HEIGHT 3000
#define TFL_MAXPIXELS  10000


#define IMG_TRANS_VFLIP   4
#define IMG_TRANS_HMIRROR 2
#define IMG_TRANS_ROTCW   1
int img_trans;

int img_crop_x0 = 0;
int img_crop_y0 = 0;
int img_crop_sx = (CAM_WIDTH/100)*5;
int img_crop_sy = (CAM_HEIGHT/100)*5;
int img_crop_ax = 5;
int img_crop_ay = 5;

#define IMG_PROC_HISTEQ   1
int img_proc;


// cmdversion =================================================================================


// The version command handler
void cmdversion_main( int argc, char * argv[] ) {
  if( argc==1 ) {
    Serial.printf( "app     : %s %s\n", LTFC_NAME, LTFC_VERSION);
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
  "Note: supports @-prefix to suppress output\n"
;

// Note cmd_register needs all strings to be PROGMEM strings. For the short string we do that inline with PSTR.
void cmdversion_register(void) {
  cmd_register(cmdversion_main, PSTR("version"), PSTR("version of this tools, its libs and tools to build it"), cmdversion_longhelp);
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
    cmdfile_run(argv[2]);
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
  "- Note: if boot.cmd exists, it will automatically be run on startup\n"
  "SYNTAX: file load <name>\n"
  "- loads the flatbuffer file with 'name' into the TensorFlow interpreter\n"
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

void cmdimg_crop_print() {
  Serial.printf("crop : (%d,%d) %d*%d average (%d,%d)\n",img_crop_x0,img_crop_y0,img_crop_sx,img_crop_sy,img_crop_ax,img_crop_ay);
}

void cmdimg_proc_print() {
  Serial.printf("proc :");
  if( img_proc & IMG_PROC_HISTEQ   ) Serial.printf(" histeq");
  if( img_proc == 0                ) Serial.printf(" none");
  Serial.printf("\n");
}

void cmdimg_img_print() {
    int w = img_crop_sx/img_crop_ax;
    int h = img_crop_sy/img_crop_ay;
    if( img_trans & IMG_TRANS_ROTCW ) { int t=w; w=h; h=t; }
    Serial.printf("shape: input %d*%d output %d*%d\n", CAM_WIDTH, CAM_HEIGHT,w,h);
}

// The img command handler
void cmdimg_main( int argc, char * argv[] ) {
  if( argc==1 ) {
    if( argv[0][0]!='@') cmdimg_crop_print();
    if( argv[0][0]!='@') cmdimg_proc_print();
    if( argv[0][0]!='@') cmdimg_trans_print(0);
    cmdimg_img_print();
    return;
  }
  if( argc>=2 && cmd_isprefix(PSTR("crop"),argv[1]) ) { 
    if( argc==2 ) { 
      cmdimg_crop_print();
      return;
    }
    if( argc!=8 ) { Serial.printf("Error: 6 numbers needed\n"); return; }
    bool ok;
    int x0,y0,sx,sy,ax,ay;
    ok = cmd_parse_dec(argv[2],&x0) ;
    if( !ok ) { Serial.printf("Error: error in first number (x0)\n"); return; }
    if( x0<0 || x0>=CAM_WIDTH ) { Serial.printf("Error: crop: x0 (%d) must be 0..%d\n",x0,CAM_WIDTH-1); return; }
    ok = cmd_parse_dec(argv[3],&y0) ;
    if( !ok ) { Serial.printf("Error: error in second number (y0)\n"); return; }
    if( y0<0 || y0>=CAM_HEIGHT ) { Serial.printf("Error: crop: y0 (%d) must be 0..%d\n",y0,CAM_HEIGHT-1); return; }
    ok = cmd_parse_dec(argv[4],&sx) ;
    if( !ok ) { Serial.printf("Error: error in third number (sx)\n"); return; }
    if( sx<1 || sx>CAM_WIDTH ) { Serial.printf("Error: crop: sx (%d) must be 1..%d\n",sx,CAM_WIDTH); return; }
    ok = cmd_parse_dec(argv[5],&sy) ;
    if( !ok ) { Serial.printf("Error: error in fourth number (sy)\n"); return; }
    if( sy<1 || sy>CAM_HEIGHT ) { Serial.printf("Error: crop: sy (%d) must be 1..%d\n",sy,CAM_HEIGHT); return; }
    ok = cmd_parse_dec(argv[6],&ax) ;
    if( !ok ) { Serial.printf("Error: error in fifth number (ax)\n"); return; }
    if( ax<1 || ax>CAM_WIDTH ) { Serial.printf("Error: crop: ax (%d) must be 1..%d\n",ax,CAM_WIDTH); return; }
    ok = cmd_parse_dec(argv[7],&ay) ;
    if( !ok ) { Serial.printf("Error: error in sixth number (ay)\n"); return; }
    if( ay<1 || ax>CAM_HEIGHT ) { Serial.printf("Error: crop: ay (%d) must be 1..%d\n",ay,CAM_HEIGHT); return; }
    // extra tests
    if( x0+sx>CAM_WIDTH ) { Serial.printf("Error: x0+sx (%d+%d) must not exceed width (%d)\n",x0,sx,CAM_WIDTH); return; }
    if( y0+sy>CAM_HEIGHT) { Serial.printf("Error: y0+sy (%d+%d) must not exceed height (%d)\n",y0,sy,CAM_HEIGHT); return; }
    if( sx%ax != 0 ) { Serial.printf("Error: width sx (%d) must be divisable by ax (%d)\n",sx,ax); return; }
    if( sy%ay != 0 ) { Serial.printf("Error: height sy (%d) must be divisable by ay (%d)\n",sy,ay); return; }
    if( (sx/ax)*(sy/ay)>TFL_MAXPIXELS ) { Serial.printf("Error: crop: resulting size sx/ax*sy/ay (%d/%d*%d/%d) must not exceed TFL buffer %d\n",sx,ax,sy,ay,TFL_MAXPIXELS); return; }
    // all ok
    img_crop_x0 = x0;
    img_crop_y0 = y0;
    img_crop_sx = sx;
    img_crop_sy = sy;
    img_crop_ax = ax;
    img_crop_ay = ay;
    if( argv[0][0]!='@') cmdimg_crop_print();
    return;
  }
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
  Serial.printf("Error: unknown sub command (%s) of img\n", argv[1] ); return;
}


// Note cmd_register needs all strigs to be PROGMEM strings. For longhelp we do that manually
const char cmdimg_longhelp[] PROGMEM = 
  "SYNTAX: img\n"
  "- shows summary of image processing configuration\n"
  "SYNTAX: img crop [ <x0> <y0> <x1> <y1> <ax> <ay>]\n"
  "- without arguments show current crop configuration (\"reduce size\")\n"
  "- with numbers sets crop configuration\n"
  "- crop rectangle is (x0,y0)-(x1,y1)\n"
  "- crop everaging ax pixels horizontal and ay pixels vertical\n"
  "SYNTAX: img proc ( none | [histeq] )\n"
  "- without arguments show current image processing settings (\"boost quality\")\n"
  "- 'none' removes all image processing settings\n"
  "- use 'histeq' for histogram equalization\n"
  "SYNTAX: img trans ( none | [vflip] [hmirror] [rotcw] )\n"
  "- without arguments show current transformation settings (\"flip image\")\n"
  "- 'none' removes all transformation settings\n"
  "- use one or more of 'vflip', 'hmirror', or 'rotcw'\n"
  "  for vertical flip, horizontal mirror or rotate clockwise (all three for ccw)\n"
  "Note: supports @-prefix to suppress output\n"
;


// Note cmd_register needs all strings to be PROGMEM strings. For the short string we do that inline with PSTR.
void cmdimg_register(void) {
  cmd_register(cmdimg_main, PSTR("img"), PSTR("configure image processing parameters"), cmdimg_longhelp);
}


void setup() {
  Serial.begin( 115200 );
  while( ! Serial ) delay(250);
  Serial.println("\nLTFC - Lego TensorFlow Cam - version " LTFC_VERSION);
  cmd_begin();
  // Registration order is list order
  cmdecho_register();   // Use the built-in echo command
  cmdfile_register();   // Register our own img command
  cmdhelp_register();   // Use the built-in help command
  cmdimg_register();    // Register our own img command
  cmdversion_register();// Register our own img command
}

void loop() {
  cmd_pollserial();
}
