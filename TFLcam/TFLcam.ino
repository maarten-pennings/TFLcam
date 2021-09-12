// TFLcam.ino - TensorFlow Lite camera application
// In Arduino, select eg Tools > Board > AI-Thinker ESP32-CAM


#include "TFLcam.h" // has application version #define's, eg TFLCAM_VERSION
#include "file.h"   // operations on sd card files, eg file_run()
#include "cmd.h"    // the command interpreter
#include "cmds.h"   // commands for the command interpreter
#include "cam.h"    // get images from the camera
#include "tflu.h"   // TensorFlow interpreter


int     tflcam_mode;
int     tflcam_mode_sub;
uint8_t tflcam_buf[TFLCAM_MAXPIXELS];


static int tflcam_ix_prev;       // The previous classification (ix is the current classification)
static int tflcam_ix_reported;   // The last reported classification
static int tflcam_ix_count_same; // The count of classifications equal to the current one
// Reset the predictions reporting history (needed when starting continuous mode)
void tflcam_reset_predictions_reporting( ) {
  tflcam_ix_prev = -1;
  tflcam_ix_reported = -1;
}


// Runs a full shoot (capture, crop, transform, image process, report prediction) cycle 
// Pass a combination of TFLCAM_SHOOT_XXX flags for extra output.
// If savename!=0, the image is saved under `savename` on SD card.
void tflcam_shoot(int flags, const char * savename ) {
  uint32_t t0=millis();
  
  // Capture/crop/transform/image process
  cam_capture(tflcam_buf, TFLCAM_MAXPIXELS );

  // Output framebuffer
  if( flags & TFLCAM_SHOOT_IMAGE ) cam_printframe(tflcam_buf,cam_outwidth(),cam_outheight());
  if( savename  ) {
    esp_err_t err= file_imgwrite(savename, tflcam_buf, cam_outwidth(), cam_outheight());
    Serial.printf("save: %s %s\n",savename, err==ESP_OK?"success":"FAIL");
  }
  
  // Predict
  uint32_t t0p=millis();
  int ix = tflu_predict( tflcam_buf, cam_outwidth()*cam_outheight() );
  uint32_t t1p=millis();
  if( flags & TFLCAM_SHOOT_VECTOR ) tflu_print();

  // Print prediction
  bool freport_prediction = true; // in principle yes, except when we are in teh special submode of continuous
  if( tflcam_mode==TFLCAM_MODE_CONTINUOUS && tflcam_mode_sub>0 ) {\
    // Record stability, e.g. the amount of equal predictions in sequence (in tflcam_ix_count_same)
    if( ix!=tflcam_ix_prev ) {
      tflcam_ix_count_same = 1;
    } else {
      if( tflcam_ix_count_same<INT_MAX) tflcam_ix_count_same += 1; // increment with clipping
    }
    tflcam_ix_prev = ix;
    // Report if stable (and different from previous report)
    if( tflcam_ix_count_same>=tflcam_mode_sub && ix!=tflcam_ix_reported ) {
      tflcam_ix_reported = ix;
      freport_prediction = true;
    } else {
      freport_prediction = false;      
    }
  }
  if( freport_prediction ) Serial.printf("predict: %d/%s\n",ix,tflu_get_classname(ix));
  
  uint32_t t1=millis();
  if( flags & TFLCAM_SHOOT_TIME ) Serial.printf("time: %u ms, %.2f FPS (prediction %u ms)\n", t1-t0, 1000.0/(t1-t0), t1p-t0p );
}


void setup() {
  Serial.begin( 115200 );
  while( ! Serial ) delay(250);
  Serial.printf(TFLCAM_BANNER);
  Serial.printf("%s - %s - version %s\n\n", TFLCAM_SHORTNAME, TFLCAM_LONGNAME, TFLCAM_VERSION);

  file_setup();
  cmds_setup();
  cam_setup();
  tflu_setup();
  Serial.print("\ntype 'help' for help\n");
  cmd_begin(); // also prints initial prompt

  tflcam_mode = TFLCAM_MODE_IDLE;
  file_run("/boot.cmd");
}


void loop() {
  // Process (characters from serial) commands
  cmd_pollserial();
  
  // Did a command change the mode?
  if( tflcam_mode==TFLCAM_MODE_CONTINUOUS ) tflcam_shoot(TFLCAM_SHOOT_NONE);
}


// Typical sd card has 
//   boot.cmd
//   rps.tfl
// and boot.cmd has contents
//   // boot.cmd for Rock, paper, scissors
//   @img crop  left 122  top 36  width 112  height 184  xsize 28  ysize 46
//   @img trans rotcw
//   @img proc histeq
//   @labels none paper rock scissors
//   file load /rps.tfl // 28x46->4
// rps.tfl is a TFL flatbuffer that must match
//   input   28*46
//   output  4 (none paper rock scissors)




// https://randomnerdtutorials.com/esp32-cam-take-photo-save-microsd-card/
// todo Disable brownout #include "soc/rtc_cntl_reg.h"         WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
// todo replace img trans to fixed rot (to make faster/simpler)
// todo: implement 'rawsave' for the raw image frame
// todo: split cam get_fb and process, so that we can print time in three: fbget, process, predict
// todo: eloquent as own subclass, not a patch
