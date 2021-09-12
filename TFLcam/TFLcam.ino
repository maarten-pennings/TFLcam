
// TFLcam.ino - TensorFlow Lite camera application
// In Arduino, select eg Tools > Board > AI-Thinker ESP32-CAM


#include "TFLcam.h" // has application version #define's
#include "file.h"   // operations on sd card files
#include "cmd.h"    // the command interpreter
#include "cmds.h"   // commands for the command interpreter
#include "cam.h"    // get images from the camera
#include "tflu.h"   // TensorFlow interpreter



#include "rps32model.h" // todo make dynamic

int     tflcam_mode;
uint8_t tflcam_buf[TFLCAM_MAXPIXELS];


// Runs a full capture (crop, transform, image process) and predict cycle. If ascii, dumps image to Serial
void tflcam_capture_predict(int ascii ) {
  cam_capture(tflcam_buf, TFLCAM_MAXPIXELS, 100 );
  if( ascii ) cam_printframe(tflcam_buf,cam_outwidth(),cam_outheight());
  int cls = tflu_predict( tflcam_buf, cam_outwidth()*cam_outheight(), 1 );
  Serial.printf("predict: %d\n",cls);
}


void setup() {
  Serial.begin( 115200 );
  while( ! Serial ) delay(250);
  Serial.printf("\n%s - %s - version %s\n", TFLCAM_SHORTNAME, TFLCAM_LONGNAME, TFLCAM_VERSION);
  file_setup();
  cmds_setup();
  cam_setup();
  tflu_load(rps32model_data); 
  Serial.print("\ntype 'help' for help\n");
  cmd_begin(); // also prints initial prompt
  cmd_addstr("@img proc histeq\n"); // todo: move to boot.cmd
  tflcam_mode = TFLCAM_MODE_IDLE;
  file_run("/boot.cmd");
}


void loop() {
  // Process (characters from serial) commands
  cmd_pollserial();
  
  // Did a command change the mode?
  if( tflcam_mode==TFLCAM_MODE_CONTINUOUS ) { tflcam_capture_predict(0); delay(3000); }
}









// https://randomnerdtutorials.com/esp32-cam-take-photo-save-microsd-card/
//  - Disable brownout #include "soc/rtc_cntl_reg.h"         WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
//  - if(psramFound()) config.fb_count = 2; else config.fb_count = 1;
//  - Remove img trans to make faster
//  - make TFL dynamic
//  - where did all the RAM go (wrt original predict app)?
//  - save image to SD card

// org cmd_addstr("@img crop  left 122  top 36  width 112  height 184  xsize 28  ysize 46\n");
// cmd_addstr("@img crop  left 120  top 40  width 112  height 184  xsize 28  ysize 46\n");
// cmd_addstr("@img trans rotcw\n");
