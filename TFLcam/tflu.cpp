// tflu.cpp - TensorFlow lite prediction
// Inspiration from https://eloquentarduino.github.io/2021/05/load-tensorflow-lite-model-from-sd-card-in-arduino/
#include <float.h>          // For FLT_MAX
#include <EloquentTinyML.h> // TensorFlow lite for ESP32, from https://github.com/eloquentarduino/EloquentTinyML
#include "tflcam.h"         // for TFLCAM_MAXPIXELS
#include "tflu.h"           // own header


#define NUMBER_OF_INPUTS  (28*46)   // todo make dynamic
#define NUMBER_OF_OUTPUTS 4         // todo make dynamic
#define TENSOR_ARENA_SIZE (45*1024) // biggest that works // todo: where is all my DRAM?


static Eloquent::TinyML::TfLite<
  NUMBER_OF_INPUTS, 
  NUMBER_OF_OUTPUTS, 
  TENSOR_ARENA_SIZE
> tflu_interpreter; // todo: make dynamic


// 
esp_err_t tflu_load(const uint8_t* model) {
  bool res = tflu_interpreter.begin(model); 
  if( res ) {
    Serial.printf("tflu: success\n");  
  } else  {
    Serial.printf("tflu: FAIL (%s)\n",tflu_interpreter.errorMessage()); 
  }
  return res ? ESP_OK : ESP_FAIL;
}


// Convert inframe[] 0..255 unit8_t  to  outframe[] -1.0..+1.0 float
static void tfly_norm( uint8_t * inframe, float * outframe, int size ) {
  for( int i=0; i<size; i++ ) {
    outframe[i] = 2.0 * inframe[i] / 255.0 - 1.0;
  }
}


// For a (float) `vector` of size `size`, returns the index of the greatest.
// todo: check predictClass and probaToClass in EloquentTinyML.h
static int tflu_index_of_max( float * vector, int size ) {
  float max_val = -FLT_MAX;
  int   max_ix;
  for( int ix=0; ix<size; ix++ ) {
    if( vector[ix] > max_val ) {
      max_ix = ix;
      max_val = vector[ix];
    }
  }
  return max_ix;
}


// Prints a (float) `vector` of size `size` to Serial. Item `index` is highlighted (pass -1 if not needed).
static void tflu_print( float * vector, int size, int index ) {
  Serial.printf("tflu: vector:");
  for( int ix=0; ix<size; ix++ ) {
    if( ix==index ) 
      Serial.printf(" [%d:%.4f]",ix,vector[ix]);
    else 
      Serial.printf(" %d:%.4f",ix,vector[ix]);
  }
  Serial.printf("\n");
}


// The frame buffer in float for TFLu
static float tflu_frame[TFLCAM_MAXPIXELS];

int tflu_predict( uint8_t * frame, int size, int showall ) {
  tfly_norm(frame, tflu_frame, size);
  float output[NUMBER_OF_OUTPUTS];
  tflu_interpreter.predict( (float*)tflu_frame, output );
  int index = tflu_index_of_max(output, NUMBER_OF_OUTPUTS);
  if( showall ) tflu_print(output, NUMBER_OF_OUTPUTS, index );
  return index; // returns index of best matching class
}
