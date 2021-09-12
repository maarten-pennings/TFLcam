// TFLcam.h - interface to TensorFlow Lite camera application

// Application version
#define TFLCAM_VERSION "0.8.0"
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


// Application mode (changed by the mode command)
#define TFLCAM_MODE_IDLE         1
#define TFLCAM_MODE_CONTINUOUS   2
#define TFLCAM_MODE_TRAIN        3
extern int tflcam_mode;

// Only used in continous:
//  =0: report every prediction
//  >0: report prediction if is id different from previous report, and stable for tflcam_mode_sub predictions
extern int tflcam_mode_sub;

#define TFLCAM_SHOOT_ALL     (1|2|4)
#define TFLCAM_SHOOT_NONE    0
#define TFLCAM_SHOOT_IMAGE   1
#define TFLCAM_SHOOT_VECTOR  2
#define TFLCAM_SHOOT_TIME    4
// Runs a full shoot (capture, crop, transform, image process, report prediction) cycle 
// Pass a combination of TFLCAM_SHOOT_XXX flags for extra output.
// If savename!=0, the image is saved under `savename` on SD card.
void tflcam_shoot(int flags, const char * savename=0 );

// Reset the predictions reporting history (needed when starting continuous mode)
void tflcam_reset_predictions_reporting( );
