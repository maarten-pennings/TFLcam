// TFLcam.h - interface to TensorFlow Lite camera application

// Application version
#define TFLCAM_VERSION "0.5.0"
#define TFLCAM_SHORTNAME "TFLcam"
#define TFLCAM_LONGNAME "TensorFlow Lite camera"


// Application mode (changed by the mode command)
#define TFLCAM_MODE_IDLE         1
#define TFLCAM_MODE_CONTINUOUS   2
#define TFLCAM_MODE_TRAIN        3
extern int tflcam_mode;


// Framebuffer size of the application
#define TFLCAM_MAXPIXELS  10000


// Runs a full capture (crop, transform, image process) and predict cycle. If ascii, dumps image to Serial
void tflcam_capture_predict(int ascii);
