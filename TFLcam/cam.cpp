// cam.cpp - camera control and image post processing
#include <Arduino.h>
#include "esp_camera.h" // The camera driver,see https://github.com/espressif/esp32-camera/tree/master/driver
#include "cam.h"        // own interface


// Instantiate the externs from the header
int cam_crop_left;
int cam_crop_top;
int cam_crop_width;
int cam_crop_height;
int cam_crop_xsize;
int cam_crop_ysize;
int cam_trans_flags;
int cam_imgproc_flags;


// Flash LED driver =========================================================
// Assumes a LED is attached to pin CAM_FLED_PIN - it will be driven with PWM


// Flash LED settings
#define CAM_FLED_PIN         4    // The GPIO pin for the high-power LED.
#define CAM_FLED_CHANNEL     15   // I just picked first PWM channel (of the 16).
#define CAM_FLED_FREQUENCY   4096 // Some arbitrary PWM frequency, high enough to not see it.
#define CAM_FLED_RESOLUTION  8    // 8 bit resolution for the duty-cycle.

// Configure pins for the flash LED
static void cam_fled_setup() {
  ledcSetup(CAM_FLED_CHANNEL, CAM_FLED_FREQUENCY, CAM_FLED_RESOLUTION); // Setup a PWM channel
  ledcAttachPin(CAM_FLED_PIN, CAM_FLED_CHANNEL); // Attach the PWM channel to the LED pin
  ledcWrite(CAM_FLED_CHANNEL, 0); // Set duty cycle of the PWM channel to 0 (off)
}

// Set flash LED brightness to `duty` (0..100).
void cam_fled_set(int duty) {
  if( duty<0 ) duty= 0;
  if( duty>100 ) duty= 100;
  duty= duty * ((1<<CAM_FLED_RESOLUTION)-1) / 100;
  ledcWrite(CAM_FLED_CHANNEL, duty);
}


// Image processing ================================================================
// Possible image improvement steps.


// Histogram equalization (https://en.wikipedia.org/wiki/Histogram_equalization)
static void cam_imgproc_histeq(uint8_t * img, int imgsize) {
  #define COLS 256       // Number of colors
  static int bins[COLS]; // Histogram bins

  // Histogram bins cleared to 0
  for( int i = 0; i<COLS; i++ ) bins[i]=0;
  // Histogram bins count pixel data from image
  for( int i = 0; i<imgsize; i++ ) bins[ img[i] ]+=1;
  // Cumulate histogram bins
  for( int i = 1; i<COLS; i++ ) bins[i]+=bins[i-1];
  // Find smallest non-zero bin
  int binmin=0;
  for( int i = 0; i<COLS; i++ ) if( bins[i]>0 ) { binmin=bins[i]; break; }
  // Equalize (+0.5 is for rounding)
  for( int i = 0; i<imgsize; i++ ) img[i] = (bins[img[i]]-binmin) * 255.0 / (bins[COLS-1]-binmin) + 0.5;
}


// Camera HW conf ===========================================================

// Define the correct board so that correct pins are used. Pick one from
//   CAMMODEL_AI_THINKER
//   CAMMODEL_WROVER_KIT
//   CAMMODEL_ESP_EYE
//   CAMMODEL_M5STACK_PSRAM
//   CAMMODEL_M5STACK_WIDE
#define  CAMMODEL_AI_THINKER


// Define the pins with which the camera is attached.
#if defined(CAMMODEL_AI_THINKER)
// I believe this has an Omnivision OV2640, see https://www.arducam.com/ov2640/
//   1600x1200@15fps (UXGA)
//   800x600@30fps (SVGA)
//   352x288@60fps (CIF)
  #define CAMMODEL_PWDN     32
  #define CAMMODEL_RESET    -1
  #define CAMMODEL_XCLK      0
  #define CAMMODEL_SIOD     26
  #define CAMMODEL_SIOC     27

  #define CAMMODEL_Y9       35
  #define CAMMODEL_Y8       34
  #define CAMMODEL_Y7       39
  #define CAMMODEL_Y6       36
  #define CAMMODEL_Y5       21
  #define CAMMODEL_Y4       19
  #define CAMMODEL_Y3       18
  #define CAMMODEL_Y2        5

  #define CAMMODEL_VSYNC    25
  #define CAMMODEL_HREF     23
  #define CAMMODEL_PCLK     22

#elif defined(CAMMODEL_WROVER_KIT)

  #define CAMMODEL_PWDN    -1
  #define CAMMODEL_RESET   -1
  #define CAMMODEL_XCLK    21
  #define CAMMODEL_SIOD    26
  #define CAMMODEL_SIOC    27

  #define CAMMODEL_Y9      35
  #define CAMMODEL_Y8      34
  #define CAMMODEL_Y7      39
  #define CAMMODEL_Y6      36
  #define CAMMODEL_Y5      19
  #define CAMMODEL_Y4      18
  #define CAMMODEL_Y3       5
  #define CAMMODEL_Y2       4

  #define CAMMODEL_VSYNC   25
  #define CAMMODEL_HREF    23
  #define CAMMODEL_PCLK    22

#elif defined(CAMMODEL_ESP_EYE)

  #define CAMMODEL_PWDN    -1
  #define CAMMODEL_RESET   -1
  #define CAMMODEL_XCLK    4
  #define CAMMODEL_SIOD    18
  #define CAMMODEL_SIOC    23

  #define CAMMODEL_Y9      36
  #define CAMMODEL_Y8      37
  #define CAMMODEL_Y7      38
  #define CAMMODEL_Y6      39
  #define CAMMODEL_Y5      35
  #define CAMMODEL_Y4      14
  #define CAMMODEL_Y3      13
  #define CAMMODEL_Y2      34

  #define CAMMODEL_VSYNC   5
  #define CAMMODEL_HREF    27
  #define CAMMODEL_PCLK    25

#elif defined(CAMMODEL_M5STACK_PSRAM)

  #define CAMMODEL_PWDN     -1
  #define CAMMODEL_RESET    15
  #define CAMMODEL_XCLK     27
  #define CAMMODEL_SIOD     25
  #define CAMMODEL_SIOC     23

  #define CAMMODEL_Y9       19
  #define CAMMODEL_Y8       36
  #define CAMMODEL_Y7       18
  #define CAMMODEL_Y6       39
  #define CAMMODEL_Y5        5
  #define CAMMODEL_Y4       34
  #define CAMMODEL_Y3       35
  #define CAMMODEL_Y2       32

  #define CAMMODEL_VSYNC    22
  #define CAMMODEL_HREF     26
  #define CAMMODEL_PCLK     21

#elif defined(CAMMODEL_M5STACK_WIDE)

  #define CAMMODEL_PWDN     -1
  #define CAMMODEL_RESET    15
  #define CAMMODEL_XCLK     27
  #define CAMMODEL_SIOD     22
  #define CAMMODEL_SIOC     23

  #define CAMMODEL_Y9       19
  #define CAMMODEL_Y8       36
  #define CAMMODEL_Y7       18
  #define CAMMODEL_Y6       39
  #define CAMMODEL_Y5        5
  #define CAMMODEL_Y4       34
  #define CAMMODEL_Y3       35
  #define CAMMODEL_Y2       32

  #define CAMMODEL_VSYNC    25
  #define CAMMODEL_HREF     26
  #define CAMMODEL_PCLK     21

#else

  #error "Camera model not selected"

#endif


// See https://github.com/espressif/esp32-camera/blob/master/driver/include/sensor.h for enums
#include "esp_camera.h"
static camera_config_t cammodel_config = {
  // Control pins
  .pin_pwdn       = CAMMODEL_PWDN,
  .pin_reset      = CAMMODEL_RESET,
  .pin_xclk       = CAMMODEL_XCLK,
  .pin_sscb_sda   = CAMMODEL_SIOD,
  .pin_sscb_scl   = CAMMODEL_SIOC,
  // Data pins
  .pin_d7         = CAMMODEL_Y9,
  .pin_d6         = CAMMODEL_Y8,
  .pin_d5         = CAMMODEL_Y7,
  .pin_d4         = CAMMODEL_Y6,
  .pin_d3         = CAMMODEL_Y5,
  .pin_d2         = CAMMODEL_Y4,
  .pin_d1         = CAMMODEL_Y3,
  .pin_d0         = CAMMODEL_Y2,
  // Sync pins
  .pin_vsync      = CAMMODEL_VSYNC,
  .pin_href       = CAMMODEL_HREF,
  .pin_pclk       = CAMMODEL_PCLK,
  // 20MHz or 10MHz for OV2640 double FPS (Experimental)
  .xclk_freq_hz   = 20000000,
  .ledc_timer     = LEDC_TIMER_0,
  .ledc_channel   = LEDC_CHANNEL_0,
  // Format of the pixel data: PIXFORMAT_ + RGB565|YUV422|GRAYSCALE|JPEG | RGB888|RAW|RGB444|RGB555
  // Do not use sizes above QVGA when not JPEG
  .pixel_format   = PIXFORMAT_GRAYSCALE,
  // Size of the output image: FRAMESIZE_ + QVGA   |CIF     |VGA    |SVGA   |XGA     |SXGA     |UXGA      || 96X96|QQVGA  |QCIF   |HQVGA  |240X240|HVGA   |HD
  //                                        320x240|352x288?|640x480|800x600|1024x768|1280x1024|1600x1200 || 96x96|160x120|176x144|240x176|240x240|480x320|1280x720
  .frame_size     = FRAMESIZE_QVGA,
  // Quality of JPEG output. 0-63 lower means higher quality
  .jpeg_quality   = 10,
  // Number of frame buffers to be allocated. If more than one, then each frame will be acquired (double speed).
  .fb_count       = 1
};


// Camera driver ============================================================


// Configure the camera. Returns success status. Prints problems also to Serial.
esp_err_t cam_setup() {
  cam_fled_setup();
  // Overwrite some configuration entries (for rest see commodel.h)
  cammodel_config.frame_size = FRAMESIZE_QVGA;
  cammodel_config.pixel_format = PIXFORMAT_GRAYSCALE;
  // Configure the camera on the board
  esp_err_t err = esp_camera_init(&cammodel_config);
  // Defaults for crop
  cam_crop_left = 0;
  cam_crop_top = 0;
  cam_crop_width = (CAM_CAPTURE_WIDTH/8)*5;
  cam_crop_height = (CAM_CAPTURE_HEIGHT/8)*5;
  cam_crop_xsize = 5;
  cam_crop_ysize = 5;
  // Defaults for image processing
  cam_imgproc_flags = 0;
  // Print and return success
  if( err==ESP_OK ) {
    // Serial.printf("cam : success\n");
  } else {
    Serial.printf("cam : FAIL (%d: s)\n",err, esp_err_to_name(err));
  }
  return err;
}


// Capture image with camera, and a pointer to the framebuffer.
// Frame is size CAM_CAPTURE_WIDTH by CAM_CAPTURE_HEIGHT.
// If Capture fails, returns 0 and prints message to Serial.
const uint8_t * cam_capture( ) {
  camera_fb_t *fb = esp_camera_fb_get();

  if( !fb ) {
    Serial.printf("cam : fb_get() failed\n");
    return 0;
  }

  // These are "assert", you may leave them out
  if( fb->width!=CAM_CAPTURE_WIDTH ) {
    Serial.printf("cam : mismatch in configured and actual frame width\n");
    return 0;
  }
  if( fb->height!=CAM_CAPTURE_HEIGHT ) {
    Serial.printf("cam : mismatch in configured and actual frame height\n");
    return 0;
  }
  if( fb->format!=PIXFORMAT_GRAYSCALE ) {
    Serial.printf("cam : mismatch in configured and actual frame format\n");
    return 0;
  }
  return fb->buf;
}

// Reduces image imag ein `inbuf` (of size CAM_CAPTURE_WIDTH by CAM_CAPTURE_HEIGHT)
// as dictated by configuration variables cam_crop_xxx, cam_trans_xxx, cam_imgproc_xxx.
// Result is stored in caller allocated `outbuf` with size `outsize`. Returns ESP_ERR_INVALID_SIZE if outsize is too small.
// Return success status. Prints problems also to Serial.
esp_err_t cam_crop(const uint8_t * inbuf, uint8_t * outbuf, int outsize ) {
  // A run-time check on outsize
  if( cam_crop_xsize*cam_crop_ysize > outsize ) {
    Serial.printf("cam : outsize (%d) too small given crop %d*%d\n", outsize, cam_crop_xsize, cam_crop_ysize);
    return ESP_ERR_INVALID_SIZE;
  }

  // Crop and transform
  for( int yp=0; yp<cam_crop_ysize; yp++ ) {
    for( int xp=0; xp<cam_crop_xsize; xp++ ) {
      // (xp,yp) is the coordinate of the block of input pixels that is averaged
      int sum=0;
      int count = 0;
      for( int yi=cam_crop_top+yp*cam_crop_height/cam_crop_ysize; yi<cam_crop_top+(yp+1)*cam_crop_height/cam_crop_ysize; yi++ ) {
        for( int xi=cam_crop_left+xp*cam_crop_width/cam_crop_xsize; xi<cam_crop_left+(xp+1)*cam_crop_width/cam_crop_xsize; xi++ ) {
          // (xi,yi) is the coordinate of the pixel in the averaging block
          sum+= inbuf[xi+CAM_CAPTURE_WIDTH*yi];
          count++;
        }
      }
      // transform
      int xo = xp;
      int yo = yp;
      int ww = cam_crop_xsize;
      if( cam_trans_flags & CAM_TRANS_VFLIP   ) { yo = cam_crop_ysize-1 - yo; }
      if( cam_trans_flags & CAM_TRANS_HMIRROR ) { xo = cam_crop_xsize-1 - xo; }
      if( cam_trans_flags & CAM_TRANS_ROTCW   ) { int t=yo; yo=xo; xo=cam_crop_ysize-1 - t; ww=cam_crop_ysize; }
      // (xo,yo) is the coordinate of the pixel in the outbuf
      outbuf[xo+ww*yo]= (sum+count/2)/count;
    }
  }

  // Image processing
  if( cam_imgproc_flags & CAM_IMGPROC_HISTEQ ) {
    cam_imgproc_histeq(outbuf, cam_crop_xsize*cam_crop_ysize );
  }

  return ESP_OK;
}

// Returns the width of the outbuf filled by cam_capture()
int cam_outwidth() {
  return cam_trans_flags & CAM_TRANS_ROTCW ? cam_crop_ysize : cam_crop_xsize;
}

// Returns the height of the outbuf filled by cam_capture()
int cam_outheight() {
  return cam_trans_flags & CAM_TRANS_ROTCW ? cam_crop_xsize : cam_crop_ysize;
}

// Print `img` in hex and ASCII to Serial
void cam_printframe(uint8_t * img, int width, int height, bool ascii, bool hex) {
  static const char *level="W@8Oo=- ";
  // print hex and/or ascii header
  if( hex ) {
    Serial.printf("y\\x: 00%*d",width*2-2,width-1);
    if( ascii ) Serial.printf(" "); else Serial.printf("\n");
  }
  if( ascii ) {
    if( !hex ) Serial.printf("y\\x: ");
    Serial.printf("00%*d\n",width,width-1);
  }
  // print framebuffers in hex and/or ascii
  for( int y=0; y<height; y++ ) {
    Serial.printf("%3d: ",y);
    if( hex ) {
      for( int x=0; x<width; x++ ) {
        Serial.printf("%02x",img[x+width*y]);
      }
      Serial.printf(" ");
    }
    if( ascii ) {
      Serial.printf("|");
      // Next print ASCII impression
      for( int x=0; x<width; x++ ) {
        Serial.printf("%c",level[img[x+width*y]/32]);
      }
      Serial.printf("|");
    }
    Serial.printf("\n");
  }
}
