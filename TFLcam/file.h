// file.h - interface to operations on sd card files


// The load command needs a buffer, this defines the max size
#define FILE_LOAD_BUFSIZE (48*1024)


// Configure the file library (SD card access). Returns success status. Prints problems also to Serial.
esp_err_t file_setup();


// Prints SD card properties to Serial.
void file_sdprops();


// Prints a directory listing to Serial.
// `dirpath` is a _full_ path leading to a directory (there is no current working directory, so start with /). 
// Will recurse for sub-directories up to `levels` deep.
// The indent is used during recursion: the number of spaces (*2) to indent
void file_dir(const char * dirpath, uint8_t levels=0, int indent=0 );


// Print the content of file `filepath` to Serial.
// `filepath` is a full file path leading to a file (there is no current working directory, so start with /). 
void file_show(const char * filepath);


// Feeds the content of file `filepath` to the command interpreter.
// `filepath` is a full file path leading to a file (there is no current working directory, so start with /). 
void file_run(const char * filepath);


// Loads the content of file `filepath` into a local buffer; the pointer to the buffer is returned.
// A next call to this function will free the local buffer and allocate a new buffer for the new file.
// `filepath` is a full file path leading to a file (there is no current working directory, so start with /). 
const uint8_t * file_load(const char * filepath);


// Writes the imag `img` (resolution `width` by `height`) to file `filepath`.
// `filepath` is a full file path leading to a file (there is no current working directory, so start with /). 
esp_err_t file_imgwrite(const char * filepath, uint8_t * img, int width, int height);
