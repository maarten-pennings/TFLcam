// TFLcam.ino - TensorFlow Lite camera application
// In Arduino, select eg Tools > Board > AI-Thinker ESP32-CAM


#include "TFLcam.h" // has application version #define's, eg TFLCAM_VERSION
#include "file.h"   // operations on sd card files, eg file_run()
#include "cmd.h"    // the command interpreter
#include "cmds.h"   // commands for the command interpreter
#include "cam.h"    // get images from the camera
#include "tflu.h"   // TensorFlow interpreter



// prediction printing history (feature: print when changed and stable)
static int tflcam_ix_prev;       // The previous classification (ix is the current classification)
static int tflcam_ix_printed;    // The last printed classification
static int tflcam_ix_count_same; // The count of classifications equal to the current one


// Mode management ===========================================================


// The operation mode of the application; will be written by the 'mode' command
static int tflcam_opmode;
static int tflcam_opmode_count;

// The flash LED mode of the application; will be written by the 'fled' command
static int tflcam_fledmode;
static int tflcam_fledmode_duty;

// Update the flash LED when fledmode or opmode changes
static void tflcam_fled_update() {
  if( tflcam_fledmode==TFLCAM_FLEDMODE_PERMANENT ) {
    cam_fled_set(tflcam_fledmode_duty);
  } else if( tflcam_fledmode==TFLCAM_FLEDMODE_OFF ) {
    cam_fled_set(0);
  } else /*TFLCAM_FLEDMODE_AUTO*/ if( tflcam_opmode==TFLCAM_OPMODE_CONTINUOUS ) {
    cam_fled_set(tflcam_fledmode_duty); // no pulsing during continuous opmode
  } else {
    cam_fled_set(0);
  }
}


// Operation mode UI ==========================================================


// Sets the operation mode (when to shoot)
void tflcam_set_opmode( int opmode, int count ) {
  if( opmode==TFLCAM_OPMODE_IDLE || opmode==TFLCAM_OPMODE_CONTINUOUS || opmode==TFLCAM_OPMODE_TRAIN ) {
    if( count<0 ) count=0;
    // record new mode
    tflcam_opmode = opmode;
    tflcam_opmode_count = count;
    // reset print history
    tflcam_ix_prev = -1;
    tflcam_ix_printed = -1;
    tflcam_fled_update();
  }
}

// Returns the opmode set with tflcam_set_opmode() - count is not available
int tflcam_get_opmode( ) {
  return tflcam_opmode;
}

// Returns the count of the opmode
int tflcam_fledmode_get_count( ) {
  return tflcam_opmode_count;
}

// Flash LED mode UI ===========================================================


// Set the mode of the flash LED and the duty cycle (dark=0..100=bright)
void tflcam_fledmode_set( int fledmode, int duty ) {
  if( fledmode==TFLCAM_FLEDMODE_OFF || fledmode==TFLCAM_FLEDMODE_AUTO || fledmode==TFLCAM_FLEDMODE_PERMANENT ) {
    tflcam_fledmode = fledmode;
    tflcam_fledmode_duty = duty; 
    tflcam_fled_update();
  }
}

// Get the mode of the flash LED
int tflcam_fledmode_get( ) {
  return tflcam_fledmode;
}

// The brightness of the flash LED when it is used (auto or permanently)
int tflcam_fledmode_get_duty( ) {
  return tflcam_fledmode_duty;
}


// Main application =============================================================


// Runs a full shoot (capture, crop, prediction, report) cycle 
// Pass a combination of TFLCAM_SHOOT_XXX flags for extra output.
// If filename_raw!=0, the raw camera image is saved under `filename_raw` on SD card.
// If filename_crop!=0, the cropped image (for training inference) is saved under `filename_crop` on SD card.
static uint8_t tflcam_buf[TFLCAM_MAXPIXELS]; 
void tflcam_shoot(int flags, const char * filename_raw, const char * filename_crop ) {
  // Indented to show timing brackets
  uint32_t t0_all=millis();
  
    // Capture with flash LED control
    uint32_t t0_cap=millis();
      if( tflcam_fledmode==TFLCAM_FLEDMODE_AUTO && tflcam_opmode!=TFLCAM_OPMODE_CONTINUOUS ) cam_fled_set(tflcam_fledmode_duty);
      const uint8_t * cambuf = cam_capture();
      if( tflcam_fledmode==TFLCAM_FLEDMODE_AUTO && tflcam_opmode!=TFLCAM_OPMODE_CONTINUOUS ) cam_fled_set(0);
      if( cambuf==0 ) { Serial.printf("ERROR: aborted"); return; }
    uint32_t t1_cap=millis();
  
    // Crop (includes transform and imageprocess)
    uint32_t t0_crop=millis();
      cam_crop(cambuf, tflcam_buf, TFLCAM_MAXPIXELS );
    uint32_t t1_crop=millis();
  
    // Predict
    uint32_t t0_pred=millis();
      int ix = tflu_predict( tflcam_buf, cam_outwidth()*cam_outheight() );
    uint32_t t1_pred=millis();
  
    // Output 
    uint32_t t0_out=millis();
      // 1. Save raw frame
      if( filename_raw  ) {
        esp_err_t err= file_imgwrite(filename_raw, cambuf, CAM_CAPTURE_WIDTH, CAM_CAPTURE_HEIGHT );
        Serial.printf("save: raw frame %s %s\n",filename_raw, err==ESP_OK?"success":"FAIL");
      }
      // 2. Save cropped frame
      if( filename_crop  ) {
        esp_err_t err= file_imgwrite(filename_crop, tflcam_buf, cam_outwidth(), cam_outheight());
        Serial.printf("save: cropped frame %s %s\n",filename_crop, err==ESP_OK?"success":"FAIL");
      }
      // 3. Print cropped frame
      if( flags & (TFLCAM_SHOOT_ASCII|TFLCAM_SHOOT_HEX) ) cam_printframe(tflcam_buf,cam_outwidth(),cam_outheight(), flags&TFLCAM_SHOOT_ASCII, flags&TFLCAM_SHOOT_HEX);
      // 4. Print prediction vector
      if( flags & TFLCAM_SHOOT_VECTOR ) tflu_print();
      // 5. Print prediction class
      bool fprint_prediction = true; // in principle yes, except when we are in the special submode of continuous
      if( tflcam_opmode==TFLCAM_OPMODE_CONTINUOUS && tflcam_opmode_count>0 ) {
        // Record stability, e.g. the amount of equal predictions in sequence (in tflcam_ix_count_same)
        if( ix!=tflcam_ix_prev ) {
          tflcam_ix_count_same = 1;
        } else {
          if( tflcam_ix_count_same<INT_MAX) tflcam_ix_count_same += 1; // increment with clipping
        }
        tflcam_ix_prev = ix;
        // Report if stable (and different from previous report)
        if( tflcam_ix_count_same>=tflcam_opmode_count && ix!=tflcam_ix_printed ) {
          tflcam_ix_printed = ix;
          fprint_prediction = true;
        } else {
          fprint_prediction = false;      
        }
      }
      if( fprint_prediction ) Serial.printf("predict: %d/%s\n",ix,tflu_get_classname(ix));
    uint32_t t1_out=millis();
  
  uint32_t t1_all=millis();
  if( flags & TFLCAM_SHOOT_TIME ) {
    Serial.printf( "time: %.2f FPS, %u ms = ", 1000.0/(t1_all-t0_all), t1_all-t0_all );
    Serial.printf( "%u (capture) + %u (crop) + %u (predict) + %u (output)\n", t1_cap-t0_cap, t1_crop-t0_crop, t1_pred-t0_pred, t1_out-t0_out );
  }
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
  Serial.print("type 'help' for help\n");
  cmd_begin(); // also prints initial prompt

  tflcam_opmode = TFLCAM_OPMODE_IDLE;
  tflcam_opmode_count = 0; // don't care
  tflcam_fledmode = TFLCAM_FLEDMODE_AUTO;
  tflcam_fledmode_duty = 50;
  tflcam_fled_update();
  
  cmd_addstr("file run /boot.cmd\n");
}


void loop() {
  // Process (characters from serial) commands
  cmd_pollserial();
  
  // Did a command change the mode?
  if( tflcam_opmode==TFLCAM_OPMODE_CONTINUOUS ) tflcam_shoot();
}


// https://randomnerdtutorials.com/esp32-cam-take-photo-save-microsd-card/
// todo Disable brownout #include "soc/rtc_cntl_reg.h"         WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
// todo: eloquent as own subclass, not a patch
// todo: load twice does not work
