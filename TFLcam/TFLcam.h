// TFLcam.h - interface to TensorFlow Lite camera application

#define TFLCAM_VERSION "1.0.0"
#define TFLCAM_SHORTNAME "TFLcam"
#define TFLCAM_LONGNAME "TensorFlow Lite camera"


#define TFLCAM_MAXPIXELS  10000


#define TFLCAM_MODE_IDLE         1
#define TFLCAM_MODE_ONCE         2
#define TFLCAM_MODE_ASCII        3
#define TFLCAM_MODE_CONTINUOUS   4
#define TFLCAM_MODE_TRAIN        5
extern int tflcam_mode;
