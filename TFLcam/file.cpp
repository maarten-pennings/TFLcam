// file.cpp - operations on sd card files
#include "SD_MMC.h"
#include "cmd.h" // for run command
#include "file.h"


// Prints SD card properties to Serial.
void file_sdprops() {
  Serial.printf("card: type ");
  sdcard_type_t cardtype=SD_MMC.cardType();
  switch( cardtype ) {
    case CARD_NONE    : Serial.printf("none"); break;
    case CARD_MMC     : Serial.printf("MMC"); break;
    case CARD_SD      : Serial.printf("SD"); break;
    case CARD_SDHC    : Serial.printf("SDHC"); break;
    case CARD_UNKNOWN : Serial.printf("unknown"); break;
    default           : Serial.printf("error"); break;
  }
  if( cardtype!=CARD_NONE ) Serial.printf(" size %lu", SD_MMC.cardSize()); 
  Serial.printf("\n");
}


// The buffer to load the TFLu model in
static uint8_t * file_load_buf; 


// Configure the file library (SD card access). Returns success status. Prints problems also to Serial.
esp_err_t file_setup() {
  bool ok = SD_MMC.begin("/sdcard", true); // true makes sd card not use DATA1 line, which is shared with flash light
  if( !ok ) { Serial.printf("sd  : FAIL to connect\n"); return ESP_FAIL; }

  sdcard_type_t cardtype=SD_MMC.cardType();
  ok = cardtype==CARD_MMC || cardtype==CARD_SD || cardtype==CARD_SDHC;
  if( !ok ) { Serial.printf("sd  : FAIL (no card)\n"); return ESP_FAIL; }

  file_load_buf = (uint8_t*)malloc(FILE_LOAD_BUFSIZE); // we could maybe use ps_malloc(FILE_LOAD_BUFSIZE);
  if( file_load_buf==0 ) { Serial.printf("sd  : FAIL (no load mem)\n"); return ESP_FAIL; }
  // Serial.printf("heap free=%d\n",ESP.getFreeHeap()); // Hack alert: somehow is helps to allocate the memory on startup (then it is stail avail)

  Serial.printf("sd  : success\n");
  return ESP_OK;
}


// Prints a directory listing to Serial.
// `dirpath` is a _full_ path leading to a directory (there is no current working directory, so start with /). 
// Will recurse for sub-directories up to `levels` deep.
// The indent is used during recursion: the number of spaces (*2) to indent
void file_dir(const char * dirpath, uint8_t levels, int indent ) {
  File root = SD_MMC.open(dirpath);
  if( !root ) { Serial.printf("ERROR: could not open '%s' (root slash missing?)\n",dirpath); return; }
  if( !root.isDirectory() ){ Serial.println("ERROR: not a directory"); return; }

  if( indent==0 ) Serial.printf("dir: '%s'\n", dirpath);
  File file = root.openNextFile();
  int filecount=0;
  int dircount=0;
  while( file ){
    if( file.isDirectory() ){
      dircount++;
      Serial.printf("%*c %s\n", indent*2+1,' ', file.name() );
      if( levels ) file_dir(file.name(), levels-1, indent+1);
    } else {
      filecount++;
      Serial.printf("%*c %s (%ul)\n", indent*2+1,' ', file.name(), file.size() );
    }
    file = root.openNextFile();
  }
  Serial.printf("%*c (%s has %d files, %d dirs)\n", indent*2+1,' ', dirpath, filecount, dircount);
  root.close();
}



// Print the content of file `filepath` to Serial.
// `filepath` is a full file path leading to a file (there is no current working directory, so start with /). 
void file_show(const char * filepath) {
  File file = SD_MMC.open(filepath);
  if( !file ) { Serial.printf("ERROR: could not open '%s' (root slash missing?)\n", filepath); return; }
  if( file.isDirectory() ){ Serial.println("ERROR: is a directory"); return; }

  while(file.available()){
    Serial.write( file.read() ); // one byte at a time
  }
  file.close();
}


// Feeds the content of file `filepath` to the command interpreter.
// `filepath` is a full file path leading to a file (there is no current working directory, so start with /). 
void file_run(const char * filepath) {
  File file = SD_MMC.open(filepath);
  if( !file ) { Serial.printf("ERROR: could not open '%s' (root slash missing?)\n", filepath); return; }
  if( file.isDirectory() ){ Serial.printf("ERROR: is a directory\n"); return; }

  while(file.available()){
    cmd_add( file.read() );
  }
  if( cmd_pendingschars()>0 ) cmd_add('\n');
  
  Serial.printf("\n");
  file.close();
}


// Loads the content of file `filepath` into a local buffer; the pointer to the buffer is returned.
// `filepath` is a full file path leading to a file (there is no current working directory, so start with /). 
const uint8_t * file_load(const char * filepath) {
  File file = SD_MMC.open(filepath);
  if( !file ) { Serial.printf("ERROR: could not open '%s' (root slash missing?)\n", filepath); return 0; }
  if( file.isDirectory() ){ Serial.printf("ERROR: is a directory\n"); return 0; }
  
  // Check buffer with the file size
  size_t size = file.size();
  if( size>FILE_LOAD_BUFSIZE ) { Serial.printf("ERROR: model (%d) to big for buffer (%d)\n",size,FILE_LOAD_BUFSIZE); return 0; }

  for( int i=0; i<size; i++ ) file_load_buf[i] = file.read(); 
  file.close();
  
  return file_load_buf;
}
