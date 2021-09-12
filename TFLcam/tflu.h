// tflu.h - interface to TensorFlow lite prediction


#define TFLU_TENSOR_ARENA_SIZE (45*1024)
#define TFLU_MAXCLASSES        10
#define TFLU_MAXCLASSNAMELEN   10


// Initializes the tflu module
void tflu_setup();

// Returns the number of classes
int tflu_get_numclasses();

// Sets the number of classes (maximum is hardwired to TFLU_MAXCLASSES)
void tflu_set_numclasses( int num );

// Get name for class `ix`, must be 0<=ix<tflu_get_numclasses().
const char* tflu_get_classname(int ix);

// Sets name of class `ix` to `name`. Name length should be less than TFLU_MAXCLASSNAMELEN.
void tflu_set_classname(int ix, const char * name);

// Get prediction for class `ix` of last tflu_predict() call, must be 0<=ix<tflu_get_numclasses().
const char* tflu_get_classprediction(int ix);

// todo: load model
esp_err_t tflu_load(const uint8_t* model);

// Prints the prediction of all the classes to Serial
void tflu_print( );


// Runs the TFLu predictor on the input `frame` of `size`.
// Sets the predictions for tflu_get_classprediction().
// Returns index of best fitting class.
int tflu_predict( uint8_t * frame, int size );

// ALERT
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
