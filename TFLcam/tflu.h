// tflu.h - interface to TensorFlow lite prediction


#define TFLU_TENSOR_ARENA_SIZE (80*1024)
#define TFLU_MAXCLASSES        10
#define TFLU_MAXCLASSNAMELEN   10


// Initializes the tflu module.
// First call setup(), then in either order set_numclasses() and load(), and from then on predict().
// Every class has a default name ("cls#"), so calling set_classname() is optional and allowed at any time after setup().
void tflu_setup();

// Returns the number of classes
int tflu_get_numclasses();

// Sets the number of classes (maximum is hardwired to TFLU_MAXCLASSES)
void tflu_set_numclasses( int num );

// Get name for class `ix`, must be 0<=ix<tflu_get_numclasses().
const char* tflu_get_classname(int ix);

// Sets name of class `ix` to `name`. Name length should be less than TFLU_MAXCLASSNAMELEN.
void tflu_set_classname(int ix, const char * name);

// Get prediction for class `ix`, must be 0<=ix<tflu_get_numclasses().
// This returns prediction of of last tflu_predict() call, so tflu_predict() _must_ have been called.
float tflu_get_classprediction(int ix);

// Prints all tflu_get_classprediction()'s to Serial.
// This prints prediction of of last tflu_predict() call, so tflu_predict() _must_ have been called.
void tflu_print( );


// The `model` must be a FlatBuffer for TensorFlow Lite.
// The inputs of the model expects must match the frame buffer resolution (in predict()).
// The outputs of the model must match the number of classes set with set_numclasses().
esp_err_t tflu_set_model(const uint8_t * model);


// Runs the TFLu predictor on the input `frame` of `size`.
// Sets the predictions for the classes in `tflu_get_classprediction()`.
// Returns index of best fitting class.
// The number of classes set with set_numclasses() must match the number outputs of the model set with load() - there is no check.
// The resolution of `frame` must match the inputs of the model set with load() - there is no check.
int tflu_predict( uint8_t * frame, int size );


// todo: eloquent as own subclass, not a patch
// PATCH ALERT
// The file EloquentTinyML.h needs a patch.
// The following method needs to be added, eg after the predict()s on line 109 and 135
/*
            float predictx(uint8_t *input, int insize, float *output, int outsize ) {
                // abort if initialization failed
                if( !initialized() ) {
                    error = NOT_INITIALIZED;
                    return sqrt(-1);
                }

                // copy input
                for (size_t i = 0; i < insize; i++)
                    this->input->data.f[i] = 2.0 * input[i] / 255.0 - 1.0;

                if (interpreter->Invoke() != kTfLiteOk) {
                    error = INVOKE_ERROR;
                    reporter->Report("Inference failed");
                    return sqrt(-1);
                }

                // copy output
                for (uint16_t i = 0; i < outsize; i++) {
                    output[i] = this->output->data.f[i];
                }
                
                return this->output->data.f[0];
           }
*/
// I also added
// #define ELOQUENT_TINYML_VERSION "0.0.10"
