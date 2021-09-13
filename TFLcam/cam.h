// cam.h - interface for camera control and image post processing 


// Main API =================================================================
// Call setup() to initialize the camera and capture() to shoot an image.


// Configure the camera. Returns success status. Prints problems also to Serial.
esp_err_t cam_setup();

// Capture image with camera, and returns a pointer to the framebuffer.
// Frame is size CAM_CAPTURE_WIDTH by CAM_CAPTURE_HEIGHT.
// If capture fails, returns 0 and prints message to Serial.
const uint8_t * cam_capture( );

// Reduces image imag ein `inbuf` (of size CAM_CAPTURE_WIDTH by CAM_CAPTURE_HEIGHT)
// as dictated by configuration variables cam_crop_xxx, cam_trans_xxx, cam_imgproc_xxx.
// Result is stored in caller allocated `outbuf` with size `outsize`. Returns ESP_ERR_INVALID_SIZE if outsize is too small.
// Return success status. Prints problems also to Serial.
esp_err_t cam_crop(const uint8_t * inbuf, uint8_t * outbuf, int outsize );

// Returns the width and height of the outbuf filled by cam_capture()
int cam_outwidth();
int cam_outheight();

// Print `img` in hex and ASCII to Serial
void cam_printframe(uint8_t * img, int width, int height);

// Set flash LED brightness to `duty` (0..100).
void cam_fled_set(int duty);


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
