// cam.h - interface for camera control and image post processing 


// Main API =================================================================
// Call setup() to initialize the camera and capture() to shoot an image.


// Configure the camera. Returns success status. Prints problems also to Serial.
esp_err_t cam_setup();

// Capture image with camera, crop, transform, and apply image processing,
// as dictated by configuration variables cam_crop_xxx, cam_trans_xxx, cam_imgproc_xxx.
// Result is stored in `outbuf` with size `outsize`. 
// If `fled`<0 the flashlight must be controlled by the caller, using cam_fled_set().
// Otherwise, the flash light will be turned on with brightness `fled` (0..100) before the shot and off afterwards.
// Return success status. Prints problems also to Serial.
esp_err_t cam_capture(uint8_t * outbuf, int outsize, int fled );


// Helpers ==================================================================

// Print `img` in hex and ASCII to Serial
void cam_printframe(uint8_t * img, int xsize, int ysize);

// Set flash LED brightness to `duty` (0..100).
void cam_fled_set(int duty);


// Configuration ============================================================
// These global variables dictate the processing done by cam_capture()
 
 
// At this moment the cam module is hardwired to QVGA in grayscale, resulting in this resolution.
#define CAM_CAPTURE_WIDTH  320
#define CAM_CAPTURE_HEIGHT 240

// Settings for the crop (and average) feature of cam_capture()
extern int cam_crop_left;
extern int cam_crop_top;
extern int cam_crop_width;
extern int cam_crop_height;
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
