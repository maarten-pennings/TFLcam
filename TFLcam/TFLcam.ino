// TFLcam.ino - TensorFlow Lite camera application
#include "TFLcam.h" // has application version #define's
#include "cmd.h"    // the command interpreter
#include "cmds.h"   // commands for the command interpreter
#include "cam.h"    // get images from the camera


int     tflcam_mode;
uint8_t tflcam_buf[TFLCAM_MAXPIXELS];


void setup() {
  Serial.begin( 115200 );
  while( ! Serial ) delay(250);
  Serial.printf("\n%s - %s - version %s\n", TFLCAM_SHORTNAME, TFLCAM_LONGNAME, TFLCAM_VERSION);
  cmd_begin();
  cmds_setup();
  cam_setup();
  tflcam_mode = TFLCAM_MODE_IDLE;
}


void loop() {
  // Process (characters from serial) commands
  cmd_pollserial();
  
  // Did a command change the mode?
  if( tflcam_mode!=TFLCAM_MODE_IDLE && tflcam_mode!=TFLCAM_MODE_TRAIN ) {
    cam_capture(tflcam_buf, cam_crop_xsize*cam_crop_xsize, 100 );
    cam_printframe(tflcam_buf, cam_crop_xsize, cam_crop_xsize);
    if( tflcam_mode==TFLCAM_MODE_ONCE ) tflcam_mode=TFLCAM_MODE_IDLE;
    else if( tflcam_mode==TFLCAM_MODE_ASCII ) tflcam_mode=TFLCAM_MODE_IDLE;
    else if( tflcam_mode==TFLCAM_MODE_CONTINUOUS ) /*skip*/;
    // Low framerate for testing
    delay(1000);
  }
  
}
