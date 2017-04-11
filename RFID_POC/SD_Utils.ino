/* ********************************************************************************
 *
 * File: SD_Utils.ino
 * 
 * Originator: Matthew Caron
 *
 * Description: Contains functions and variables for handling SD operations.
 * 
 ******************************************************************************** */

#include <SD.h>

#define SD_CS_PIN     4  // SD chip select pin
#define REC_LEN_ID    10 // length of a record's ID
#define REC_LEN_DATA  32 // length of a record's data
#define MSG_LEN       15 // length of an error message

uint8_t MSG_1[] = "SD Card Error! ";
uint8_t MSG_2[] = "File Error!    ";
uint8_t MSG_3[] = "File Corrupted!";
uint8_t MSG_4[] = "Unknown UID!   ";
uint8_t HEX_CHARS[] = "0123456789abcdef";

uint8_t hexID[REC_LEN_ID << 1];
uint8_t data[REC_LEN_DATA];
uint8_t fileBuffer[REC_LEN_ID << 1];
uint8_t id[REC_LEN_ID];
File    fileObj;

// retrieves a name record given an ID, or an error message if none exists
// uses an iterative implementation of binary search
// logs unknown IDs
uint8_t *retrieveRecord(uint8_t *ID, uint8_t lenID){  
  
  if(!SD.begin(SD_CS_PIN)){ // make sure SD card is available
    setData(MSG_1, MSG_LEN);
    return data;
  }
  
  fileObj = SD.open("known.bin", FILE_READ);
  
  if(!fileObj){  // file error
    setData(MSG_2, MSG_LEN);
    return data;
  }
    
  setID(ID, lenID);
  uint32_t high = fileObj.size() / (REC_LEN_ID + REC_LEN_DATA); // get number of records
  
  if(high * (REC_LEN_ID + REC_LEN_DATA) != fileObj.size()){ // file does not contain whole number of records
    fileObj.close();
    setData(MSG_3, MSG_LEN);
    return data;
  }
  
  uint32_t low = 0; // set low to index of first record
  high--; // set high to index of last record
  uint32_t guess;
  int16_t compareResult;
  
  while(low < high + 1){
    guess = (low + high) >> 1;
    fileObj.seek(guess * (REC_LEN_ID + REC_LEN_DATA));
    fileObj.read(fileBuffer, REC_LEN_ID);
    compareResult = memcmp(fileBuffer, id, REC_LEN_ID);
    
    if(compareResult == 0){ // found it
      fileObj.read(data, REC_LEN_DATA);
      fileObj.close();
      return data;
    }
    
    if(compareResult < 0) // not far enough into file
      low = guess + 1;
     
    else // too far into file
      high = guess - 1;
  }
  
  fileObj.close(); // id not in file
  logID(); // log the unknown ID
  setData(MSG_4, MSG_LEN);
  return data;
}

// Helper functions *****************************************************************

// generates a big-endian hexidecimal string 
// representation of a little-endian byte array
// reads from id, writes to hexID
void idToHex(){
  for(uint8_t i = 0; i < REC_LEN_ID; i++){
    hexID[i << 1] = HEX_CHARS[id[REC_LEN_ID - (i + 1)] >> 4];
    hexID[(i << 1) + 1] = HEX_CHARS[id[REC_LEN_ID - (i + 1)] & 0xF];
  }
}

// writes a string to data, padding with spaces
// truncates anything beyond REC_LEN_DATA chars
void setData(uint8_t * dat, uint8_t len){
  uint8_t i;
  for(i = 0; i < smallest(len, REC_LEN_DATA); i++)
    data[i] = dat[i];
  for(i = len; i < REC_LEN_DATA; i++)
    data[i] = ' ';
}

// writes a byte array to id, padding with 0s
// max uid length is 10 chars, so no need to truncate
// NOTE: index 0 is lowest-order byte
void setID(uint8_t * ID, uint8_t len){
  uint8_t i;
  for(i = 0; i < len; i++)
    id[i] = ID[i];
  for(i = len; i < REC_LEN_ID; i++)
    id[i] = 0;
}

// attempts to log a new ID
void logID(){

  idToHex(); // convert current id to hex string
  
  fileObj = SD.open("new.txt", FILE_READ);
  
  if(!fileObj){ // file error; might not exist
    fileObj = SD.open("new.txt", FILE_WRITE); // try to create it
    if(!fileObj) // unable to create file
      return;
    fileObj.close(); // creation successful
    fileObj = SD.open("new.txt", FILE_READ); // try to reopen in read mode
    if(!fileObj) // just in case :)
      return;
  }

  // get number of complete records in file
  uint16_t nRecords = fileObj.size() / ((REC_LEN_ID << 1) + 2);
  
  for(uint16_t i = 0; i < nRecords; i++){ // check to see if ID has already been logged
    fileObj.read(fileBuffer, REC_LEN_ID << 1);
    if(memcmp(hexID, fileBuffer, REC_LEN_ID << 1) == 0)
      return; // id already exists, so has therefore been successfully logged
    fileObj.read(fileBuffer, 2); // increment file read index by 2 (for newline and carriage return)
  }
  
  fileObj.close(); // done searching; id not found
  fileObj = SD.open("new.txt", FILE_WRITE); // reopen file in write mode
  
  if(!fileObj) // error opening file in write mode
    return;
    
  fileObj.write(hexID, REC_LEN_ID << 1);
  fileObj.write("\r\n", 2);
  fileObj.close();
}
