// TFLcam.h - interface to TensorFlow Lite camera application

// Application version
#define TFLCAM_VERSION "0.6.0"
#define TFLCAM_SHORTNAME "TFLcam"
#define TFLCAM_LONGNAME "TensorFlow Lite camera"


// Application mode (changed by the mode command)
#define TFLCAM_MODE_IDLE         1
#define TFLCAM_MODE_CONTINUOUS   2
#define TFLCAM_MODE_TRAIN        3
extern int tflcam_mode;


// Framebuffer size of the application
#define TFLCAM_MAXPIXELS  10000


#define TFLCAM_SHOOT_ASCII   1
#define TFLCAM_SHOOT_VECTOR  2
#define TFLCAM_SHOOT_TIME    4
#define TFLCAM_SHOOT_PREDICT 8
// Runs a full shoot (capture, crop, transform, image process, predict) cycle. 
// Set flags to combination of TFLCAM_SHOOT_XXX for extra output
void tflcam_shoot(int flags);
