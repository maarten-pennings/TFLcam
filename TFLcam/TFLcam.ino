// TFLcam.ino - TensorFlow Lite camera application
// In Arduino, select eg Tools > Board > AI-Thinker ESP32-CAM


#include "TFLcam.h" // has application version #define's, eg TFLCAM_VERSION
#include "file.h"   // operations on sd card files, eg file_run()
#include "cmd.h"    // the command interpreter
#include "cmds.h"   // commands for the command interpreter
#include "cam.h"    // get images from the camera
#include "tflu.h"   // TensorFlow interpreter


#include "rps32model.h" // todo make dynamic

int     tflcam_mode;
uint8_t tflcam_buf[TFLCAM_MAXPIXELS];


// Runs a full shoot (capture, crop, transform, image process, predict) cycle. 
// Set flags to combination of TFLCAM_SHOOT_XXX for extra output.
void tflcam_shoot(int flags ) {
  uint32_t t0=millis();
  // Capture/crop/transform/image process
  cam_capture(tflcam_buf, TFLCAM_MAXPIXELS, 100 );
  if( flags & TFLCAM_SHOOT_ASCII ) cam_printframe(tflcam_buf,cam_outwidth(),cam_outheight());
  // Predict
  int ix = tflu_predict( tflcam_buf, cam_outwidth()*cam_outheight() );
  if( flags & TFLCAM_SHOOT_VECTOR ) tflu_print();
  if( flags & TFLCAM_SHOOT_PREDICT ) Serial.printf("predict: %d/%s\n",ix,tflu_get_classname(ix));
  uint32_t t1=millis();
  if( flags & TFLCAM_SHOOT_TIME ) Serial.printf("time: %lu ms (%.2f FPS)\n", t1-t0, 1000.0/(t1-t0) );
}


void setup() {
  Serial.begin( 115200 );
  while( ! Serial ) delay(250);
  Serial.printf("\n%s - %s - version %s\n", TFLCAM_SHORTNAME, TFLCAM_LONGNAME, TFLCAM_VERSION);

  file_setup();
  cmds_setup();
  cam_setup();
  tflu_setup();
  tflu_load(rps32model_data); // todo: make dynamic
  Serial.print("\ntype 'help' for help\n");
  cmd_begin(); // also prints initial prompt

  // todo: move to boot.cmd
  cmd_addstr("@img crop  left 122  top 36  width 112  height 184  xsize 28  ysize 46\n");
  cmd_addstr("@img trans rotcw\n");  
  cmd_addstr("@img proc histeq\n"); 
  cmd_addstr("@labels none paper rock scissers\n"); 
  
  tflcam_mode = TFLCAM_MODE_IDLE;
  file_run("/boot.cmd");
}


void loop() {
  // Process (characters from serial) commands
  cmd_pollserial();
  
  // Did a command change the mode?
  if( tflcam_mode==TFLCAM_MODE_CONTINUOUS ) tflcam_shoot(TFLCAM_SHOOT_PREDICT);
}









// https://randomnerdtutorials.com/esp32-cam-take-photo-save-microsd-card/
// todo Disable brownout #include "soc/rtc_cntl_reg.h"         WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
// todo if(psramFound()) config.fb_count = 2; else config.fb_count = 1;
// todo Remove img trans to make faster
// todo flash light management
// todo: add mode change (continuous but only print when there is a stable change)
// todo: implement 'save' for the cropped image frame
// todo: implement 'rawsave' for the raw image frame
// todo: where is all my DRAM?

