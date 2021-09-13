// tflu.cpp - TensorFlow lite prediction
// Inspiration from https://eloquentarduino.github.io/2021/05/load-tensorflow-lite-model-from-sd-card-in-arduino/
#include <float.h>          // For FLT_MAX
#include <EloquentTinyML.h> // TensorFlow lite for ESP32, from https://github.com/eloquentarduino/EloquentTinyML
#include "tflcam.h"         // for TFLCAM_MAXPIXELS
#include "tflu.h"           // own header


static Eloquent::TinyML::TfLite<0,0,TFLU_TENSOR_ARENA_SIZE> tflu_interpreter; // See ALRT in tflu.h. We do not use in or out size


#define TFLU_STATE_STARTUP 0
#define TFLU_STATE_SETUP   1
#define TFLU_STATE_MODEL   2
#define TFLU_STATE_PREDICT 3
static int tflu_state;

// Variables to hold the configuration - tflu_set_numclasses() and tflu_set_classname()
static char  tflu_classnames[TFLU_MAXCLASSES][TFLU_MAXCLASSNAMELEN];
static int   tflu_numclasses;

// Variable to hold the results of the last prediction - tflu_predict()
static float tflu_classpredictions[TFLU_MAXCLASSES];


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


// Prints all tflu_get_classprediction()'s to Serial.
// This prints prediction of of last tflu_predict() call, so tflu_predict() _must_ have been called.
void tflu_print( ) {
  if( tflu_state!=TFLU_STATE_PREDICT ) { Serial.printf("ERROR: predict() not yet called\n"); return; }
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


// Initializes the tflu module
void tflu_setup() {
  if( tflu_state!=TFLU_STATE_STARTUP ) { Serial.printf("ERROR: setup() already called\n"); return; }
  for( int i=0; i< TFLU_MAXCLASSES; i++ )
    snprintf(tflu_classnames[i],TFLU_MAXCLASSNAMELEN,"cls%d",i);
  tflu_numclasses= 0;
  Serial.printf("tflu: success\n");
  tflu_state = TFLU_STATE_SETUP;
}


// Returns the number of classes
int tflu_get_numclasses() {
  return tflu_numclasses;
}


// Sets the number of classes (maximum is hardwired to TFLU_MAXCLASSES)
void tflu_set_numclasses( int num ) {
  if( num<=0 || num>TFLU_MAXCLASSES ) { Serial.printf("ERROR: number of classes must be 1..%d\n",TFLU_MAXCLASSES); return; }
  tflu_numclasses= num;
}


// Get name for class `ix`, must be 0<=ix<tflu_get_numclasses().
const char* tflu_get_classname(int ix) {
  if( ix<0 || ix>tflu_numclasses ) return 0;
  return tflu_classnames[ix];
}


// Sets name of class `ix` to `name`. Name length should be less than TFLU_MAXCLASSNAMELEN.
void tflu_set_classname(int ix, const char * name) {
  if( ix<0 || ix>TFLU_MAXCLASSES ) { Serial.printf("ERROR: max number of classes exceeded %d\n",TFLU_MAXCLASSES); return; }
  strncpy(tflu_classnames[ix],name,TFLU_MAXCLASSNAMELEN);
}


// Get prediction for class `ix`, must be 0<=ix<tflu_get_numclasses().
// This returns prediction of of last tflu_predict() call, so tflu_predict() _must_ have been called.
float tflu_get_classprediction(int ix) {
  if( tflu_state!=TFLU_STATE_PREDICT ) { Serial.printf("ERROR: predict() not yet called\n"); return 0; }
  if( ix<0 || ix>tflu_numclasses ) return 0;
  return tflu_classpredictions[ix];
}


// The `model` must be a FlatBuffer for TensorFlow Lite.
// The inputs of the model expects must match the frame buffer resolution (in predict()).
// The outputs of the model must match the number of classes set with set_numclasses().
esp_err_t tflu_set_model(const uint8_t * model) {
  if( tflu_state==TFLU_STATE_STARTUP ) { Serial.printf("ERROR: tflu_setup() not yet called\n"); return ESP_FAIL; }
  if( model==0 ) { tflu_state = TFLU_STATE_SETUP; Serial.printf("ERROR: no model\n"); return ESP_FAIL; }
  bool res = tflu_interpreter.begin(model);
  if( res ) {
    tflu_state = TFLU_STATE_MODEL;
  } else  {
    tflu_state = TFLU_STATE_SETUP;
    Serial.printf("tflu: model load FAIL (%s)\n",tflu_interpreter.errorMessage()); 
  }
  return res ? ESP_OK : ESP_FAIL;
}


// Runs the TFLu predictor on the input `frame` of `size`.
// Sets the predictions for the classes in `tflu_get_classprediction()`, and returns best class.
int tflu_predict( uint8_t * frame, int size ) {
  if( tflu_state<TFLU_STATE_MODEL ) { Serial.printf("ERROR: model not yet loaded\n"); return -1; }
  if( tflu_numclasses==0 ) { Serial.printf("ERROR: no class names defined\n"); return -1; }
  tflu_interpreter.predictx( frame, size, tflu_classpredictions, tflu_numclasses); // Needs patch, see predictx in tflu.h
  int index = tflu_index_of_max(tflu_classpredictions, tflu_numclasses);
  tflu_state = TFLU_STATE_PREDICT;
  return index; 
}
