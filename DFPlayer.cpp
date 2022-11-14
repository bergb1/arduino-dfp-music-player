/*
 * File that's not yet documented
 */
 
#include "DFPlayer.h"

// Serial Buffer Initialization
uint8_t sBuffer[DFP_BUF_ARR_LEN] = {
  DFP_BUF_START, DFP_BUF_VERSION, DFP_BUF_LEN, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, DFP_BUF_END
};

/*
 * Function that's not yet documented
 */
uint8_t * generateDFPBuffer(uint8_t command_id, uint16_t parameter, uint8_t feedback) {
  // Insert the instructions inside of the buffer
  sBuffer[3] = command_id;
  sBuffer[4] = feedback;
  sBuffer[5] = (parameter >> 8) & 0xFF;
  sBuffer[6] = (parameter) & 0xFF;
  
  // Calculate the checksum
  int16_t checksum = 0;
  checksum = checksum - 
    sBuffer[1] - sBuffer[2] - sBuffer[3] - 
    sBuffer[4] - sBuffer[5] - sBuffer[6];

  // Insert the checksum inside of the command
  sBuffer[7] = (checksum >> 8) & 0xFF;
  sBuffer[8] = (checksum) & 0xFF;

  return sBuffer;
}
