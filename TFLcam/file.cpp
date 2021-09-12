// file.cpp - operations on sd card files
#include "SD_MMC.h"
#include "cmd.h" // for run command
#include "file.h"


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


esp_err_t file_setup() {
  bool ok = SD_MMC.begin("/sdcard", true); // true makes sd card not use DATA1 line, which is shared with flash light
  if( !ok ) { Serial.printf("sd  : FAIL to connect\n"); return ESP_FAIL; }
  Serial.printf("sd  : success\n");

  sdcard_type_t cardtype=SD_MMC.cardType();
  ok = cardtype==CARD_MMC || cardtype==CARD_SD || cardtype==CARD_SDHC;
  if( !ok ) { Serial.printf("card: FAIL (no card)\n"); return ESP_FAIL; }

  return ESP_OK;
}


void file_dir(const char * dirname, uint8_t levels, int indent ) {
  File root = SD_MMC.open(dirname);
  if( !root ) { Serial.println("ERROR: could not open (root slash missing?)"); return; }
  if( !root.isDirectory() ){ Serial.println("ERROR: not a directory"); return; }

  if( indent==0 ) Serial.printf("dir: '%s'\n", dirname);
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
  Serial.printf("%*c (%s has %d files, %d dirs)\n", indent*2+1,' ', dirname, filecount, dircount);
  root.close();
}


void file_show(const char * path) {
  File file = SD_MMC.open(path);
  if( !file ) { Serial.println("ERROR: could not open (root slash missing?)"); return; }
  if( file.isDirectory() ){ Serial.println("ERROR: is a directory"); return; }

  while(file.available()){
    Serial.write( file.read() ); // one byte at a time
  }
  file.close();
}


void file_run(const char * path) {
  File file = SD_MMC.open(path);
  if( !file ) { Serial.println("ERROR: could not open (root slash missing?)"); return; }
  if( file.isDirectory() ){ Serial.println("ERROR: is a directory"); return; }

  while(file.available()){
    cmd_add( file.read() );
  }
  if( cmd_pendingschars()>0 ) cmd_add('\n');
  
  Serial.printf("\n");
  file.close();
}
