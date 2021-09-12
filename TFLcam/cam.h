// cam.h - interface for camera control and image post processing 


// Main API =================================================================
// Call setup() to initialize the camera and capture() to shoot an image.


// Configure the camera. Returns success status. Prints problems also to Serial.
esp_err_t cam_setup();

// Capture image with camera, crop, transform, and apply image processing,
// as dictated by configuration variables cam_crop_xxx, cam_trans_xxx, cam_imgproc_xxx.
// Result is stored in caller allocated `outbuf` with size `outsize`. 
// Return success status. Prints problems also to Serial.
esp_err_t cam_capture(uint8_t * outbuf, int outsize );

// Returns the width and height of the outbuf filled by cam_capture()
int cam_outwidth();
int cam_outheight();

// Print `img` in hex and ASCII to Serial
void cam_printframe(uint8_t * img, int width, int height);


// Flash LED control ========================================================
#define CAM_FLED_MODE_OFF        0 // Flash LED always off - cam_fled_set_mode()
#define CAM_FLED_MODE_AUTO       1 // Flash LED automatically on during shots
#define CAM_FLED_MODE_PERMANENT  2 // Flash LED permanently on

#define CAM_FLED_DUTY_MIN        0 
#define CAM_FLED_DUTY_MAX        100

// Set the mode of the flash LED: off, only on when shooting images ("auto"), or permanently on.
void cam_fled_set_mode( int mode );
// Get the mode of the flash LED
int cam_fled_get_mode( );
// The brightness of the flash LED when it is used (auto or permanently)
void cam_fled_set_duty( int duty );
// Get the brightness of the flash LED
int cam_fled_get_duty( );


// Configuration ============================================================
// These global variables dictate the processing done by cam_capture()
 
 
// At this moment the cam module is hardwired to QVGA in grayscale, resulting in this camera resolution.
#define CAM_CAPTURE_WIDTH  320
#define CAM_CAPTURE_HEIGHT 240

// Settings for the crop (and average) feature of cam_capture()
extern int cam_crop_left;
extern int cam_crop_top;
extern int cam_crop_width; // width to crop from camera
extern int cam_crop_height;// height to crop from camera
extern int cam_crop_xsize; // output width after averaging
extern int cam_crop_ysize; // output height after averaging

// Flags for image transformation (mirror, rotate) features of cam_capture()
#define CAM_TRANS_VFLIP   4
#define CAM_TRANS_HMIRROR 2
#define CAM_TRANS_ROTCW   1
extern int cam_trans_flags;

// Flags for image processing features of cam_capture()
#define CAM_IMGPROC_HISTEQ   1
extern int cam_imgproc_flags;
