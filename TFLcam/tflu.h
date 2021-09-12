// tflu.h - interface to TensorFlow lite prediction


// 
esp_err_t tflu_load(const uint8_t* model);

// 
int tflu_predict( uint8_t * frame, int size, int showall );
