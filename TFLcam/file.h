// file.h - interface to operations on sd card files


// Configure the file library (SD card access). Returns success status. Prints problems also to Serial.
esp_err_t file_setup();


// Prints SD card properties to Serial.
void file_sdprops();


// Prints a directory listing to Serial.
// `dirname` is a full path leading to a directory (there is no current working directory). 
// Will recurse for sub-directories up to `levels` deep.
// The indent is used during recursion: the number of spaces (*2) to indent
void file_dir(const char * dirname, uint8_t levels=0, int indent=0 );


// Print the content of file `path` to Serial.
// `path` is a full file path leading to a file (there is no current working directory). 
void file_show(const char * path);


// Feeds the content of file `path` to the command interpreter.
// `path` is a full file path leading to a file (there is no current working directory). 
void file_run(const char * path);
