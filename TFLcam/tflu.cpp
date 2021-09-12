// tflu.cpp - TensorFlow lite prediction
// Inspiration from https://eloquentarduino.github.io/2021/05/load-tensorflow-lite-model-from-sd-card-in-arduino/
#include <float.h>          // For FLT_MAX
#include <EloquentTinyML.h> // TensorFlow lite for ESP32, from https://github.com/eloquentarduino/EloquentTinyML
#include "tflcam.h"         // for TFLCAM_MAXPIXELS
#include "tflu.h"           // own header


static Eloquent::TinyML::TfLite<0,0,TFLU_TENSOR_ARENA_SIZE> tflu_interpreter; // See ALRT in tflu.h. We do not use in or out size


static char  tflu_classnames[TFLU_MAXCLASSES][TFLU_MAXCLASSNAMELEN];
static int   tflu_numclasses;
static float tflu_classpredictions[TFLU_MAXCLASSES];


// Initializes the tflu module
void tflu_setup() {
  for( int i=0; i< TFLU_MAXCLASSES; i++ )
    snprintf(tflu_classnames[i],TFLU_MAXCLASSNAMELEN,"cls%d",i);
  tflu_numclasses= 0;
  Serial.printf("tflu: success\n");
}


// Returns the number of classes
int tflu_get_numclasses() {
  return tflu_numclasses;
}


// Sets the number of classes (maximum is hardwired to TFLU_MAXCLASSES)
void tflu_set_numclasses( int num ) {
  if( num<0 || num>TFLU_MAXCLASSES ) { Serial.printf("ERROR: max number of classes exceeded %d\n",TFLU_MAXCLASSES); return; }
  tflu_numclasses= num;
}


// Get name for class `ix`, must be 0<=ix<tflu_get_numclasses().
const char* tflu_get_classname(int ix) {
  if( ix<0 || ix>tflu_numclasses ) return "<outof range>";
  return tflu_classnames[ix];
}


// Sets name of class `ix` to `name`. Name length should be less than TFLU_MAXCLASSNAMELEN.
void tflu_set_classname(int ix, const char * name) {
  if( ix<0 || ix>TFLU_MAXCLASSES ) { Serial.printf("ERROR: max number of classes exceeded %d\n",TFLU_MAXCLASSES); return; }
  strncpy(tflu_classnames[ix],name,TFLU_MAXCLASSNAMELEN);
}


// Get prediction for class `ix` of last tflu_predict() call, must be 0<=ix<tflu_get_numclasses().
const char* tflu_get_classprediction(int ix) {
  if( ix<0 || ix>tflu_numclasses ) return "<outof range>";
  return tflu_classnames[ix];
}


// todo: load model
esp_err_t tflu_load(const uint8_t* model) {
  bool res = tflu_interpreter.begin(model); 
  if( res ) {
    Serial.printf("tflu: success\n");  
  } else  {
    Serial.printf("tflu: FAIL (%s)\n",tflu_interpreter.errorMessage()); 
  }
  return res ? ESP_OK : ESP_FAIL;
}


// For a (float) `vector` of size `size`, returns the index of the greatest.
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
void tflu_print( ) {
  int index = tflu_index_of_max(tflu_classpredictions, tflu_numclasses);
  Serial.printf("tflu: vector:");
  for( int ix=0; ix<tflu_numclasses; ix++ ) {
    if( ix==index ) 
      Serial.printf(" [%d/%s:%.4f]",ix,tflu_classnames[ix],tflu_classpredictions[ix]);
    else 
      Serial.printf(" %d/%s:%.4f",ix,tflu_classnames[ix],tflu_classpredictions[ix]);
  }
  Serial.printf("\n");
}


// Runs the TFLu predictor on the input `frame` of `size`.
// Sets the predictions for the classes in `tflu_classpredictions[]` and returns best class
int tflu_predict( uint8_t * frame, int size ) {
  if( tflu_numclasses==0 ) { Serial.printf("ERROR: no class names defined\n"); return -1; }
  tflu_interpreter.predictx( frame, size, tflu_classpredictions, tflu_numclasses);
  int index = tflu_index_of_max(tflu_classpredictions, tflu_numclasses);
  return index; 
}
