// TFLcam.h - interface to TensorFlow Lite camera application

// Application version
#define TFLCAM_VERSION "0.9.1"
#define TFLCAM_SHORTNAME "TFLcam"
#define TFLCAM_LONGNAME "TensorFlow Lite camera"
#define TFLCAM_BANNER "\n\n"\
  "   _______ ______ _\n"\
  "  |__   __|  ____| |\n"\
  "     | |  | |__  | |     ___ __ _ _ __ ___\n"\
  "     | |  |  __| | |    / __/ _` | '_ ` _ \\\n"\
  "     | |  | |    | |___| (_| (_| | | | | | |\n"\
  "     |_|  |_|    |______\\___\\__,_|_| |_| |_|\n"\
// https://patorjk.com/software/taag/#p=display&v=2&f=Big&t=TFLcam


// Framebuffer size of the application
#define TFLCAM_MAXPIXELS  10000


// Operation mode (changed by the mode command)
#define TFLCAM_OPMODE_IDLE         11
#define TFLCAM_OPMODE_TRAIN        12
#define TFLCAM_OPMODE_CONTINUOUS   13
// `count` is only used in continous:
//  =0: print every prediction
//  >0: print prediction only if it is different from previous print, and stable for `count` predictions
// Sets the operation mode (when to shoot)
void tflcam_set_opmode( int opmode, int count=0 );
// Returns the opmode set with tflcam_set_opmode()
int tflcam_get_opmode( );
// Returns the count of the opmode
int tflcam_fledmode_get_count( );


// Flash LED mode (changed by the fled command)
#define TFLCAM_FLEDMODE_OFF        21 // Flash LED always off - cam_fled_set_mode()
#define TFLCAM_FLEDMODE_AUTO       22 // Flash LED automatically on during shots
#define TFLCAM_FLEDMODE_PERMANENT  23 // Flash LED permanently on
// Set the mode of the flash LED and the duty cycle (dark=0..100=bright)
void tflcam_fledmode_set( int fledmode, int duty );
// Get the mode of the flash LED
int tflcam_fledmode_get( );
// The brightness of the flash LED when it is used (auto or permanently)
int tflcam_fledmode_get_duty( );


// Activating the sensor for one reading. called by loop() or by a 'mode single' command.
#define TFLCAM_SHOOT_ASCII   1
#define TFLCAM_SHOOT_HEX     2
#define TFLCAM_SHOOT_VECTOR  4
#define TFLCAM_SHOOT_TIME    8
#define TFLCAM_SHOOT_FULL    (TFLCAM_SHOOT_ASCII|TFLCAM_SHOOT_HEX|TFLCAM_SHOOT_VECTOR|TFLCAM_SHOOT_TIME) 
// Runs a full shoot (capture, crop, prediction, report) cycle 
// Pass a combination of TFLCAM_SHOOT_XXX flags for extra output.
// If filename_raw!=0, the raw camera image is saved under `filename_raw` on SD card.
// If filename_crop!=0, the cropped image (for training inference) is saved under `filename_crop` on SD card.
void tflcam_shoot(int flags=0, const char * filename_raw=0, const char * filename_crop=0 );
