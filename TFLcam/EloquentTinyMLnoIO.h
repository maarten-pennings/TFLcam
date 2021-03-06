// EloquentTinyMLnoIO.h - a wrapper class for EloquentTinyML introducing dynamic input and output size
#include <EloquentTinyML.h> // TensorFlow lite for ESP32, from https://github.com/eloquentarduino/EloquentTinyML


// Version id is missing in underlying library
#define ELOQUENT_TINYML_VERSION "0.0.10"


// Wrapper class implementing predict_io() replacing predict().
namespace Eloquent {
    namespace TinyML {
        template<size_t tensorArenaSize>
        class TfLiteNoIO : public TfLite<0,0,tensorArenaSize> {
        public:

            // Block existing predict()s, since they use the inputSize and outputSize
            uint8_t predict(uint8_t *input, uint8_t *output = NULL) { this->error=INVOKE_ERROR; this->reporter->Report("Inference failed"); return sqrt(-1); }
            float   predict(float *input, float *output = NULL) { this->error=INVOKE_ERROR; this->reporter->Report("Inference failed"); return sqrt(-1); }

            // New predict(), input and output size passed dynamically. 
            // This method can also scale input while copying; this saves an intermediate buffer (uint8->float).
            // Typically use scale=2.0/255.0 and offset=-1.0 to normalize an image.
            float predict_io(uint8_t *input, int insize, float *output, int outsize, float scale=1.0, float offset=0.0 ) {
                // Abort if initialization failed
                if( !this->initialized() ) {
                    this->error = NOT_INITIALIZED;
                    return sqrt(-1);
                }

                // Copy input with scaling
                for( size_t i = 0; i < insize; i++ )
                    this->input->data.f[i] = input[i]*scale+offset; // scale while copying

                if( this->interpreter->Invoke() != kTfLiteOk ) {
                    this->error = INVOKE_ERROR;
                    this->reporter->Report("Inference failed");
                    return sqrt(-1);
                }

                // Copy output
                for( uint16_t i = 0; i < outsize; i++ ) {
                    output[i] = this->output->data.f[i];
                }
                
                return this->output->data.f[0];
            }

        };
    }
}

// Instantiate with only the TENSOR_ARENA_SIZE
//    Eloquent::TinyML::TfLiteNoIO<TENSOR_ARENA_SIZE> ml;
