// TFLcam.ino - TensorFlow Lite camera application
// In Arduino, select eg Tools > Board > AI-Thinker ESP32-CAM


#include "TFLcam.h" // has application version #define's
#include "cmd.h"    // the command interpreter
#include "cmds.h"   // commands for the command interpreter
#include "cam.h"    // get images from the camera


int     tflcam_mode;
uint8_t tflcam_buf[TFLCAM_MAXPIXELS];


// Runs a full capture (crop, transform, image process) and predict cycle. If ascii, dumps image to Serial
void tflcam_capture_predict(int ascii ) {
  cam_capture(tflcam_buf, TFLCAM_MAXPIXELS, 100 );
  if( ascii ) cam_printframe(tflcam_buf,cam_outwidth(),cam_outheight());
  Serial.printf("predict: 3\n");
}


void setup() {
  Serial.begin( 115200 );
  while( ! Serial ) delay(250);
  Serial.printf("\n%s - %s - version %s\n", TFLCAM_SHORTNAME, TFLCAM_LONGNAME, TFLCAM_VERSION);
  cmds_setup();
  cam_setup();
  Serial.print("\ntype 'help' for help\n");
  cmd_begin(); // also prints initial prompt
  cmd_addstr("@img crop  left 122  top 36  width 112  height 184  xsize 28  ysize 46\n");
  cmd_addstr("@img trans rotcw\n");
  tflcam_mode = TFLCAM_MODE_IDLE;
}


void loop() {
  // Process (characters from serial) commands
  cmd_pollserial();
  
  // Did a command change the mode?
  if( tflcam_mode==TFLCAM_MODE_CONTINUOUS ) { tflcam_capture_predict(0); delay(3000); }
}
