// cmds.cpp - the commands used by TFLcam
#include <Arduino.h>
#include "core_version.h"   // ARDUINO_ESP32_GIT_VER, ARDUINO_ESP32_GIT_DESC, ARDUINO_ESP32_RELEASE
#include <EloquentTinyML.h> // For version of TensorFlow lite for ESP32, from https://github.com/eloquentarduino/EloquentTinyML

#include "cmd.h"            // command interpreter
#include "cmds.h"           // own interface

// The commands control the app (and some if its libs)
#include "TFLcam.h"         // application
#include "file.h"           // operations on sd card files
#include "cam.h"            // camera configuration
#include "tflu.h"           // TensorFlow lite configuration (of classes)


// cmds_sys =====================================================================================

#include "esp32-hal-cpu.h" // see C:\Users\maarten\AppData\Local\Arduino15\packages\esp32\hardware\esp32\1.0.6\cores\esp32\esp32-hal-cpu.h

const char * cmds_sys_resetreason() {
  esp_reset_reason_t reason = esp_reset_reason();
  switch( reason ) {
    case ESP_RST_UNKNOWN   : return "unknown";
    case ESP_RST_POWERON   : return "power-on";
    case ESP_RST_EXT       : return "reset-external-pin";
    case ESP_RST_SW        : return "reset-by-sw";
    case ESP_RST_PANIC     : return "exception-or-panic";
    case ESP_RST_INT_WDT   : return "watchdog-interrupt";
    case ESP_RST_TASK_WDT  : return "watchdog-task";
    case ESP_RST_WDT       : return "watchdog-other";
    case ESP_RST_DEEPSLEEP : return "from-deepsleep";
    case ESP_RST_BROWNOUT  : return "bownout";
    case ESP_RST_SDIO      : return "SDIO";
    default                : return "<should-not-happen>";
  }
}

void cmds_sys_clk_show() {
  Serial.printf( "clk  : %u MHz (xtal %u MHz)\n",getCpuFrequencyMhz(), getXtalFrequencyMhz() );
}

void cmds_sys_show() {
  cmds_sys_clk_show();
  Serial.printf("reset: %s\n",cmds_sys_resetreason() );
  Serial.printf("chip : model %s (%d cores) rev %d\n",ESP.getChipModel(),ESP.getChipCores(), ESP.getChipRevision() );
  Serial.printf("ftrs :");
  esp_chip_info_t info;
  esp_chip_info(&info);
  if( info.features & BIT(0) ) Serial.printf(" Embedded-Flash");
  if( info.features & BIT(1) ) Serial.printf(" 2.4GHz-WiFi");
  if( info.features & BIT(4) ) Serial.printf(" Bluetooth-LE");
  if( info.features & BIT(5) ) Serial.printf(" Bluetooth-classic");
  Serial.printf("\n");
  Serial.printf( "heap : size %u, free %u\n",ESP.getHeapSize(), ESP.getFreeHeap() );
  Serial.printf( "psram: size %u, free %u\n",ESP.getPsramSize(), ESP.getFreePsram() );
  Serial.printf( "bufs : model %u, tensor %u\n", FILE_LOAD_BUFSIZE ,TFLU_TENSOR_ARENA_SIZE );
  // ESP.getSketchSize(), 
}


// The sys command handler
static void cmds_sys_main( int argc, char * argv[] ) {
  if( argc==1 ) {
    cmds_sys_show();
    return;
  }
  if( argc>=2 && cmd_isprefix(PSTR("clk"),argv[1])) {
    if( argc==2 ) {
      cmds_sys_show();
      return;
    }
    int clk;
    bool ok = cmd_parse_dec(argv[2],&clk) ;
    if( !ok ) { Serial.printf("ERROR: error in frequency\n"); return; }
    setCpuFrequencyMhz(clk); //  240, 160, 80
    if( argv[0][0]!='@') cmds_sys_clk_show();
    return;
  }
  if( argc==2 && cmd_isprefix(PSTR("reboot"),argv[1])) {
    ESP.restart();
  }
  Serial.printf("ERROR: unknown arguments for sys\n" ); return;
}


// Note cmd_register needs all strigs to be PROGMEM strings. For longhelp we do that manually
static const char cmds_sys_longhelp[] PROGMEM = 
  "SYNTAX: sys\n"
  "- without arguments shows some system info\n"
  "SYNTAX: sys clk <freq>\n"
  "- without arguments shows clock frequency\n"
  "- with argument sets clock frequency\n"
  "- valid values are 10, 20, 40, 80, 160, 240, but camera needs >=80\n"
  "SYNTAX: sys reboot\n"
  "- reboot the system\n"
  "NOTES:\n"
  "- supports @-prefix to suppress output\n"
;

// Note cmd_register needs all strings to be PROGMEM strings. For the short string we do that inline with PSTR.
static int cmds_sys_register(void) {
  return cmd_register(cmds_sys_main, PSTR("sys"), PSTR("system info and commands"), cmds_sys_longhelp);
}


// cmds_labels =================================================================================


static void cmds_labels_show() {
  int count= tflu_get_numclasses();
  Serial.printf("labels (%d):", count);
  for( int i=0; i<count; i++ ) Serial.printf(" %s",tflu_get_classname(i) );
  Serial.printf("\n");
}


// The labels command handler
static void cmds_labels_main( int argc, char * argv[] ) {
  if( argc==1 ) {
    cmds_labels_show();
    return;
  }
  tflu_set_numclasses(argc-1);
  for( int i=1; i<argc; i++ ) tflu_set_classname(i-1,argv[i]);
  if( argv[0][0]!='@') cmds_labels_show();
}


// Note cmd_register needs all strigs to be PROGMEM strings. For longhelp we do that manually
static const char cmds_labels_longhelp[] PROGMEM = 
  "SYNTAX: labels <label>...\n"
  "- without arguments, prints the labels for the prediction classes\n"
  "- with arguments, set the labels\n"
  "NOTES:\n"
  "- supports @-prefix to suppress output\n"
;


// Note cmd_register needs all strings to be PROGMEM strings. For the short string we do that inline with PSTR.
static int cmds_labels_register(void) {
  return cmd_register(cmds_labels_main, PSTR("labels"), PSTR("sets labels for the prediction classes"), cmds_labels_longhelp);
}


// cmds_fled =================================================================================


static void cmds_fled_show() {
  int m = tflcam_fledmode_get();
  Serial.printf("fled: ");
  if( m==TFLCAM_FLEDMODE_OFF ) Serial.printf("off");
  if( m==TFLCAM_FLEDMODE_AUTO ) Serial.printf("auto");
  if( m==TFLCAM_FLEDMODE_PERMANENT ) Serial.printf("permanent");
  Serial.printf(" (duty %d)",tflcam_fledmode_get_duty());
  if( tflcam_fledmode_get_duty()==0 ) Serial.printf(" [warn: duty 0 instead of mode off]"); 
  Serial.printf("\n");
}

// The fled command handler
static void cmds_fled_main( int argc, char * argv[] ) {
  if( argc==1 ) {
    cmds_fled_show();
    return;
  }
  int mode = tflcam_fledmode_get();
  int duty = tflcam_fledmode_get_duty();
  int fmode=0;
  int fduty=0;
  int ix=1;
  while( ix<argc ) {
    if( cmd_isprefix(PSTR("off"),argv[ix]) ) {
      if( fmode==1 ) { Serial.printf("ERROR: mode occurs more then once\n"); return; }
      mode=TFLCAM_FLEDMODE_OFF;
      fmode=1;
    } else if( cmd_isprefix(PSTR("auto"),argv[ix]) ) {
      if( fmode==1 ) { Serial.printf("ERROR: mode occurs more then once\n"); return; }
      mode=TFLCAM_FLEDMODE_AUTO;
      fmode=1;
    } else if( cmd_isprefix(PSTR("permanent"),argv[ix]) ) {
      if( fmode==1 ) { Serial.printf("ERROR: mode occurs more then once\n"); return; }
      mode=TFLCAM_FLEDMODE_PERMANENT;
      fmode=1;
    } else {
      bool ok = cmd_parse_dec(argv[ix],&duty) ;
      if( !ok ) { Serial.printf("ERROR: expected mode or duty, not '%s'\n",argv[ix]); return; }
      if( duty<0 || duty>100 ) { Serial.printf("ERROR: duty (%d) must be 0..100\n",duty); return; }
      if( fduty==1 ) { Serial.printf("ERROR: duty occurs more then once\n"); return; }
      fduty=1;
    }
    ix++;      
  }
  tflcam_fledmode_set(mode,duty);
  if( argv[0][0]!='@') cmds_fled_show();
}


// Note cmd_register needs all strigs to be PROGMEM strings. For longhelp we do that manually
static const char cmds_fled_longhelp[] PROGMEM = 
  "SYNTAX: fled ( off | auto | permanent | <duty> )...\n"
  "- without arguments, prints the flash led mode and duty cycle\n"
  "- with mode, sets it to 'off', on when shooting ('auto'), or 'permanently' on\n"
  "- with <duty>, sets the duty cycle (0..100)\n"
  "NOTES:\n"
  "- supports @-prefix to suppress output\n"
;


// Note cmd_register needs all strings to be PROGMEM strings. For the short string we do that inline with PSTR.
static int cmds_fled_register(void) {
  return cmd_register(cmds_fled_main, PSTR("fled"), PSTR("controls the flash led"), cmds_fled_longhelp);
}


// cmds_version =================================================================================


// The version command handler
static void cmds_version_main( int argc, char * argv[] ) {
  if( argc==1 ) {
    Serial.printf( "app     : %s (%s) %s\n", TFLCAM_LONGNAME, TFLCAM_SHORTNAME, TFLCAM_VERSION);
    if( argv[0][0]!='@') Serial.printf( "library : cmd %s\n", CMD_VERSION);
    if( argv[0][0]!='@') Serial.printf( "library : EloquentTinyML %s\n", ELOQUENT_TINYML_VERSION);
    if( argv[0][0]!='@') Serial.printf( "runtime : Arduino ESP32 " ARDUINO_ESP32_RELEASE "\n" );
    if( argv[0][0]!='@') Serial.printf( "compiler: " __VERSION__ "\n" );
    if( argv[0][0]!='@') Serial.printf( "arduino : %d\n",ARDUINO );
    if( argv[0][0]!='@') Serial.printf( "compiled: " __DATE__ ", " __TIME__ "\n" );
    return;
  }
  Serial.printf("ERROR: unknown arguments for version\n" ); return;
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
    Serial.printf("TODO: not yet implemented\n");
    cmds_mode_train_count++;
    char buf[5]; snprintf(buf,sizeof buf, "%04d",cmds_mode_train_count); cmd_set_streamprompt(buf);
  } else {
    Serial.printf("mode: training stopped\n");
    cmd_set_streamfunc(0);
  }
}

static void cmds_mode_show() {
  int m = tflcam_get_opmode();
  if( m==TFLCAM_OPMODE_IDLE ) Serial.printf("mode: idle\n");
  else if( m==TFLCAM_OPMODE_CONTINUOUS ) Serial.printf("mode: continuous (stable %d)\n",tflcam_fledmode_get_count());
  else if( m==TFLCAM_OPMODE_TRAIN ) Serial.printf("mode: train\n");
  else Serial.printf("mode: <unknown>\n");
}

// The mode command handler
static void cmds_mode_main( int argc, char * argv[] ) {
  if( argc==1 ) {
    cmds_mode_show();
    return;
  }
  if( argc>=2 && cmd_isprefix(PSTR("idle"),argv[1]) ) { 
    if( argc!=2 ) { Serial.printf("ERROR: idle does not have argument\n"); return; }
    tflcam_set_opmode(TFLCAM_OPMODE_IDLE);
    cmds_mode_show();
    return;
  }
  if( argc>=2 && cmd_isprefix(PSTR("single"),argv[1]) ) { 
    char * rsave=0;
    char * csave=0;
    int ffull=0;
    int fhex=0;
    int fascii=0;
    int fvector=0;
    int ftime=0;
    int frsave=0;
    int fcsave=0;
    int ix=2;
    while( ix<argc ) {
      if( cmd_isprefix(PSTR("full"),argv[ix]) ) {
        if( ffull==1 ) { Serial.printf("ERROR: full occurs more then once\n"); return; }
        ffull=1;
      } else if( cmd_isprefix(PSTR("hex"),argv[ix]) ) {
        if( fhex==1 ) { Serial.printf("ERROR: hex occurs more then once\n"); return; }
        if( ffull==1 ) { Serial.printf("ERROR: hex occurs with full\n"); return; }
        fhex=1;
      } else if( cmd_isprefix(PSTR("ascii"),argv[ix]) ) {
        if( fascii==1 ) { Serial.printf("ERROR: ascii occurs more then once\n"); return; }
        if( ffull==1 ) { Serial.printf("ERROR: ascii occurs with full\n"); return; }
        fascii=1;
      } else if( cmd_isprefix(PSTR("vector"),argv[ix]) ) {
        if( fvector==1 ) { Serial.printf("ERROR: vector occurs more then once\n"); return; }
        if( ffull==1 ) { Serial.printf("ERROR: vector occurs with full\n"); return; }
        fvector=1;
      } else if( cmd_isprefix(PSTR("time"),argv[ix]) ) {
        if( ftime==1 ) { Serial.printf("ERROR: time occurs more then once\n"); return; }
        if( ffull==1 ) { Serial.printf("ERROR: time occurs with full\n"); return; }
        ftime=1;
      } else if( cmd_isprefix(PSTR("rsave"),argv[ix]) ) {
        if( frsave==1 ) { Serial.printf("ERROR: rsave occurs more then once\n"); return; }
        frsave=1;
        ix++;
        if( ix>=argc ) { Serial.printf("ERROR: rsave needs a filename\n"); return; }
        rsave = argv[ix];
      } else if( cmd_isprefix(PSTR("csave"),argv[ix]) ) {
        if( fcsave==1 ) { Serial.printf("ERROR: csave occurs more then once\n"); return; }
        fcsave=1;
        ix++;
        if( ix>=argc ) { Serial.printf("ERROR: csave needs a filename\n"); return; }
        csave = argv[ix];
      } else {
        Serial.printf("ERROR: unknown flag (%s)\n", argv[ix]); return;
      }
      ix++;      
    }
    int flags = ffull*TFLCAM_SHOOT_FULL | fascii*TFLCAM_SHOOT_ASCII | fhex*TFLCAM_SHOOT_HEX| fvector*TFLCAM_SHOOT_VECTOR | ftime*TFLCAM_SHOOT_TIME;
    tflcam_shoot( flags, rsave, csave );
    if( tflcam_get_opmode() != TFLCAM_OPMODE_IDLE ) { 
      tflcam_set_opmode(TFLCAM_OPMODE_IDLE);
      cmds_mode_show();
    }
    return;
  }
  if( argc>=2 && cmd_isprefix(PSTR("continuous"),argv[1]) ) { 
    int count=0; // default: report all, not changes
    if( argc>2 ) { 
      bool ok = cmd_parse_dec(argv[2],&count);
      if( !ok ) { Serial.printf("ERROR: error in continuous <count>\n"); return; }
      if( count<1 ) { Serial.printf("ERROR: continuous <count> (%d) must be 1..\n",count); return; }
      if( argc>3 ) { Serial.printf("ERROR: unexpected continuous argument '%s'\n", argv[3]); return; }
    }
    tflcam_set_opmode(TFLCAM_OPMODE_CONTINUOUS,count);
    cmds_mode_show();
    return;
  }
  if( argc>=2 && cmd_isprefix(PSTR("train"),argv[1]) ) { 
    if( argc!=3 ) { Serial.printf("ERROR: train must have one directory name\n"); return; }
    tflcam_set_opmode(TFLCAM_OPMODE_TRAIN);
    cmds_mode_show();
    Serial.printf("Press CR to save an image; any non-empty input will abort training mode\n");
    cmds_mode_train_count=0;
    cmd_set_streamprompt("0000");
    cmd_set_streamfunc(cmds_mode_streamfunc_train);
    return;
  }
  Serial.printf("ERROR: unknown sub command (%s) of mode\n", argv[1] ); return;
}


// Note cmd_register needs all strigs to be PROGMEM strings. For longhelp we do that manually
static const char cmds_mode_longhelp[] PROGMEM = 
  "SYNTAX: mode\n"
  "- shows active mode (never shows single)\n"
  "SYNTAX: mode idle\n"
  "- switch camera off, no TensorFlow predictions\n"
  "SYNTAX: mode single <outputflags>...\n"
  "outputflags: full | hex | ascii | vector | time | rsave <name> | csave <name>\n"
  "- takes a single shot, prints prediction, goes to idle mode\n"
  "- 'full' is a shorthand of 'hex ascii vector time'\n"
  "- 'hex' also outputs a hex dump of the image\n"
  "- 'ascii' also outputs a character rendering of the image\n"
  "- 'vector' also outputs the probabilities of all classes\n"
  "- 'time' also outputs elapsed time\n"
  "- 'rsave' saves raw (camera) image on sd card under <name>\n"
  "- 'csave' saves cropped image on sd card under <name>\n"
  "- suggested extension for file <name> is .pgm (portable gray map)\n"
  "SYNTAX: mode continuous [ <count> ]\n"
  "- takes a shot, predicts, and loops\n"
  "- without <count> prints every predictions\n"
  "- with <count>, prints if prediction changed, and stable for <count> loops\n"
  "- typically stopped with command 'mode idle'\n"
  "SYNTAX: mode train <dir>\n"
  "- goes in training mode: after each CR an image is saved in <dir>\n"
  "- any other command then CR returns to idle mode\n"
  "- <dir> must not exist, and will be created\n"
;

// Note cmd_register needs all strings to be PROGMEM strings. For the short string we do that inline with PSTR.
static int cmds_mode_register(void) {
  return cmd_register(cmds_mode_main, PSTR("mode"), PSTR("sensor mode (single, continuous)"), cmds_mode_longhelp);
}


// cmds_file ====================================================================================

// The file command handler
static void cmds_file_main( int argc, char * argv[] ) {
  if( argc==1 ) {
    file_sdprops();
    return;
  }
  if( argc>=2 && cmd_isprefix(PSTR("dir"),argv[1]) ) { 
    if( argc==2 ) { 
      file_dir("/");
    } else if( argc==3 ) { 
      file_dir(argv[2]);
    } else if( argc==4 ) { 
      int levels;
      bool ok = cmd_parse_dec(argv[3],&levels) ;
      if( !ok ) { Serial.printf("ERROR: error in levels\n"); return; }
      if( levels<0 ) { Serial.printf("ERROR: levels (%d) must be 0..\n",levels); return; }
      file_dir(argv[2],levels);
    } else {
      Serial.printf("ERROR: unknown arguments for dir\n" ); return;  
    }
    return;
  }
  if( argc>=2 && cmd_isprefix(PSTR("show"),argv[1]) ) { 
    if( argc!=3 ) { Serial.printf("ERROR: show must have one filename\n"); return; }
    file_show(argv[2]);
    return;
  }
  if( argc>=2 && cmd_isprefix(PSTR("run"),argv[1]) ) { 
    if( argc!=3 ) { Serial.printf("ERROR: run must have one filename\n"); return; }
    file_run(argv[2]);
    // The script ends with printing a prompt. When cmds_file_main() returns another a prompt is printed. We seperate that.
    Serial.printf("\n\n"); 
    return;
  }
  if( argc>=2 && cmd_isprefix(PSTR("load"),argv[1]) ) { 
    if( argc!=3 ) { Serial.printf("ERROR: load must have one filename\n"); return; }
    const uint8_t * model = file_load(argv[2]);
    esp_err_t res = tflu_set_model( model );
    if( argv[0][0]!='@') if( res==ESP_OK ) Serial.printf("model '%s' loaded\n", argv[2]);
    return;
  }
  Serial.printf("ERROR: unknown sub command (%s) of file\n", argv[1] ); return;
}


// Note cmd_register needs all strigs to be PROGMEM strings. For longhelp we do that manually
static const char cmds_file_longhelp[] PROGMEM = 
  "SYNTAX: file\n"
  "- shows SD card poperties\n"
  "SYNTAX: file dir [ <name> [ <levels> ] ]\n"
  "- shows the contents of the directory <name> (default '/')\n"
  "- <levels> is the number of recursive steps (default 0)\n"
  "SYNTAX: file show <name>\n"
  "- shows the content of file with 'name'\n"
  "SYNTAX: file run <name>\n"
  "- runs all commands in file with 'name'\n"
  "SYNTAX: file load <name>\n"
  "- loads the flatbuffer file with 'name' into the TensorFlow interpreter\n"
  "NOTES:\n"
  "- supports subdirectories (separated with '/'), always address from root\n"
  "- script '/boot.cmd' will automatically be run on startup\n"
  "NOTES:\n"
  "- supports @-prefix to suppress output\n"
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
  Serial.printf("crop : left %d top %d width %d height %d xsize %d ysize %d", cam_crop_left, cam_crop_top, cam_crop_width, cam_crop_height,cam_crop_xsize, cam_crop_ysize);
  Serial.print(" (poolx ");
  if( cam_crop_width%cam_crop_xsize==0 ) Serial.printf("%d",cam_crop_width/cam_crop_xsize); else Serial.printf("%.2f",cam_crop_width/(float)cam_crop_xsize); 
  Serial.print(" pooly ");
  if( cam_crop_height%cam_crop_ysize==0 ) Serial.printf("%d",cam_crop_height/cam_crop_ysize); else Serial.printf("%.2f",cam_crop_height/(float)cam_crop_ysize); 
  Serial.print(")");
  if( cam_crop_width%cam_crop_xsize!=0 || cam_crop_height%cam_crop_ysize!=0 ) Serial.printf(" [warn: pool float]"); 
  if( 100*cam_crop_width/cam_crop_xsize != 100*cam_crop_height/cam_crop_ysize ) Serial.printf(" [warn: pool not equal]"); 
  Serial.print("\n");
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
        if( fleft ) { Serial.printf("ERROR: left occurs more then once\n"); return; }
        fleft=1; ix++;
        if( ix>=argc ) { Serial.printf("ERROR: left needs a value\n"); return; }
        ok = cmd_parse_dec(argv[ix],&left) ;
        if( !ok ) { Serial.printf("ERROR: error in left value\n"); return; }
        if( left<0 || left>=CAM_CAPTURE_WIDTH ) { Serial.printf("ERROR: left (%d) must be 0..%d\n",left,CAM_CAPTURE_WIDTH-1); return; }
      } else if( cmd_isprefix(PSTR("top"),argv[ix]) ) {
        if( ftop ) { Serial.printf("ERROR: top occurs more then once\n"); return; }
        ftop=1; ix++;
        if( ix>=argc ) { Serial.printf("ERROR: top needs a value\n"); return; }
        ok = cmd_parse_dec(argv[ix],&top) ;
        if( !ok ) { Serial.printf("ERROR: error in top value\n"); return; }
        if( top<0 || top>=CAM_CAPTURE_HEIGHT ) { Serial.printf("ERROR: top (%d) must be 0..%d\n",top,CAM_CAPTURE_HEIGHT-1); return; }
      } else if( cmd_isprefix(PSTR("width"),argv[ix]) ) {
        if( fwidth ) { Serial.printf("ERROR: width occurs more then once\n"); return; }
        fwidth=1; ix++;
        if( ix>=argc ) { Serial.printf("ERROR: width needs a value\n"); return; }
        ok = cmd_parse_dec(argv[ix],&width) ;
        if( !ok ) { Serial.printf("ERROR: error in width value\n"); return; }
        if( width<1 || width>CAM_CAPTURE_WIDTH ) { Serial.printf("ERROR: width (%d) must be 1..%d\n",width,CAM_CAPTURE_WIDTH); return; }
      } else if( cmd_isprefix(PSTR("height"),argv[ix]) ) {
        if( fheight ) { Serial.printf("ERROR: height occurs more then once\n"); return; }
        fheight=1; ix++;
        if( ix>=argc ) { Serial.printf("ERROR: height needs a value\n"); return; }
        ok = cmd_parse_dec(argv[ix],&height) ;
        if( !ok ) { Serial.printf("ERROR: error in height value\n"); return; }
        if( height<1 || height>CAM_CAPTURE_HEIGHT ) { Serial.printf("ERROR: height (%d) must be 1..%d\n",height,CAM_CAPTURE_HEIGHT); return; }
      } else if( cmd_isprefix(PSTR("xsize"),argv[ix]) ) {
        if( fxsize ) { Serial.printf("ERROR: xsize occurs more then once\n"); return; }
        fxsize=1; ix++;
        if( ix>=argc ) { Serial.printf("ERROR: xsize needs a value\n"); return; }
        ok = cmd_parse_dec(argv[ix],&xsize) ;
        if( !ok ) { Serial.printf("ERROR: error in xsize value\n"); return; }
        if( xsize<1 || xsize>CAM_CAPTURE_WIDTH ) { Serial.printf("ERROR: xsize (%d) must be 1..%d\n",xsize,CAM_CAPTURE_WIDTH); return; }
      } else if( cmd_isprefix(PSTR("ysize"),argv[ix]) ) {
        if( fysize ) { Serial.printf("ERROR: ysize occurs more then once\n"); return; }
        fysize=1; ix++;
        if( ix>=argc ) { Serial.printf("ERROR: ysize needs a value\n"); return; }
        ok = cmd_parse_dec(argv[ix],&ysize) ;
        if( !ok ) { Serial.printf("ERROR: error in ysize value\n"); return; }
        if( ysize<1 || ysize>CAM_CAPTURE_HEIGHT ) { Serial.printf("ERROR: ysize (%d) must be 1..%d\n",ysize,CAM_CAPTURE_HEIGHT); return; }
      } else {
        Serial.printf("ERROR: crop has unknown arg (%s)\n", argv[ix]); return;
      }
      ix++;
    }
    // extra tests
    if( left+width>CAM_CAPTURE_WIDTH ) { Serial.printf("ERROR: left+width (%d+%d) must not exceed cam width (%d)\n",left,width,CAM_CAPTURE_WIDTH); return; }
    if( top+height>CAM_CAPTURE_HEIGHT) { Serial.printf("ERROR: top+height (%d+%d) must not exceed cam height (%d)\n",top,height,CAM_CAPTURE_HEIGHT); return; }
    if( xsize*ysize>TFLCAM_MAXPIXELS ) { Serial.printf("ERROR: resulting size %d*%d exceeds TFL buffer size %d\n",xsize,ysize,TFLCAM_MAXPIXELS); return; }
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
    int fvflip=0;
    int fhmirror=0;
    int frotcw=0;
    int fnone=0;
    int ix=2;
    while( ix<argc ) {
      if( cmd_isprefix(PSTR("none"),argv[ix]) ) {
        if( fnone==1 ) { Serial.printf("ERROR: none occurs more then once\n"); return; }
        fnone=1;
      } else if( cmd_isprefix(PSTR("vflip"),argv[ix]) ) {
        if( fvflip==1 ) { Serial.printf("ERROR: vflip occurs more then once\n"); return; }
        fvflip=1;
      } else if( cmd_isprefix(PSTR("hmirror"),argv[ix]) ) {
        if( fhmirror==1 ) { Serial.printf("ERROR: hmirror occurs more then once\n"); return; }
        fhmirror=1;
      } else if( cmd_isprefix(PSTR("rotcw"),argv[ix]) ) {
        if( frotcw==1 ) { Serial.printf("ERROR: rotcw occurs more then once\n"); return; }
        frotcw=1;
      } else {
        Serial.printf("ERROR: unknown transformation (%s)\n", argv[ix]); return;
      }
      ix++;      
    }
    if( fnone>0 && fvflip+fhmirror+frotcw>0 ) { Serial.printf("ERROR: none can not be combined with others\n"); return; }
    cam_trans_flags = fvflip*CAM_TRANS_VFLIP + fhmirror*CAM_TRANS_HMIRROR + frotcw*CAM_TRANS_ROTCW;
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
        if( histeq==1 ) { Serial.printf("ERROR: histeq occurs more then once\n"); return; }
        histeq=1;
      } else {
        Serial.printf("ERROR: unknown image processing (%s)\n", argv[ix]); return;
      }
      ix++;      
    }
    cam_imgproc_flags = histeq*CAM_IMGPROC_HISTEQ;
    if( argv[0][0]!='@') cmds_cam_imgproc_print();
    return;
  }
  // subcommand <error> --------------------------------
  Serial.printf("ERROR: unknown sub command (%s) of img\n", argv[1] ); return;
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
  "SYNTAX: img trans ( none | vflip | hmirror | rotcw )...\n"
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
  num=cmds_fled_register();
  num=cmdhelp_register();     // built-in help command
  num=cmds_img_register();
  num=cmds_labels_register();
  num=cmds_mode_register();
  num=cmds_sys_register();
  num=cmds_version_register();
  if( num>=0 ) {
    // Serial.printf("cmds: success\n"); 
  } else {
    Serial.printf("cmds: FAIL\n"); // too many commands registered
  }
}
