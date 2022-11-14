/*
 * File that's not yet documented
 */
 
#include <SoftwareSerial.h>
#include <stdint.h>

// DFP communication logic
#include "DFPlayer.h"

// DFP Receive/Transmit Pins
#define rx_pin  (5)
#define tx_pin  (4)

// Pause switch pin
#define sw_pin  (12)

// Motor pin
#define m_pin   (16)

// DFP buffers
uint8_t rBuffer[DFP_BUF_ARR_LEN];
uint8_t rReady[DFP_BUF_ARR_LEN];
uint8_t bufferIndex = 0;

// Song index to shuffle songs played and not repeat them
uint16_t * songArr = (uint16_t *) malloc(0);
uint16_t songsStored = 0;
uint16_t songIndex = 0;

// State of the device
uint8_t isPaused = false;

// Inverted Software Serial
SoftwareSerial serial = SoftwareSerial(rx_pin, tx_pin);

/*
 * Function that's not yet documented
 */
void configDFP(uint8_t volume) {
  // Wait for the device to be unpaused
  while(digitalRead(sw_pin)) delay(16);

  // Get the total amount of songs on the sd card
  songsStored = writeReliableDFP(DFP_QUERY_TOTAL, DFP_EMPTY_ARG, false);
  while(songsStored > DFP_MAX_SONGS || songsStored == 0) {
    Serial.println("[DEBUG] Micro USB Not Ready");
    delay(1000);
    songsStored = writeReliableDFP(DFP_QUERY_TOTAL, DFP_EMPTY_ARG, false);
  }
  songIndex = songsStored - 1;
  Serial.printf("[DEBUG] Available Songs: %d\r\n", songsStored);
  
  // Resize the songArr
  songArr = (uint16_t *) realloc(songArr, songsStored * sizeof(uint16_t));

  // Populate the songArr
  for(uint16_t i = 0; i < songsStored; i++) {
    songArr[i] = i + 1;
  }

  // Use the status to detirmine the DFP's current state
  uint8_t statusDFP = writeReliableDFP(DFP_QUERY_STATUS, DFP_EMPTY_ARG, false) & 0xFF;
  Serial.printf("[DEBUG] Status: %02X\r\n", statusDFP);
  if(statusDFP == 0x00) {
    // Play a random song
    playRandomSongDFP(DFP_STD_VOLUME);
  } else if(statusDFP == 0x02) {
    // Resume the DFP's Music
    setVolumeDFP(DFP_STD_VOLUME);
    writeReliableDFP(DFP_PLAY, DFP_EMPTY_ARG, true);
  }
}

/*
 * Function that's not yet documented
 */
void setup () {
  // Start the Debugging Serial
  Serial.begin(74880);
  
  // Start the Software Serial
  serial.begin(9600);

  // Configure the pins
  pinMode(sw_pin, INPUT);

  // Enable the motor
  analogWrite(m_pin, 127);

  // Send a ping to the DFP
  writeReliableDFP(0x41, DFP_EMPTY_ARG, 0x01);
  
  // Initialize the Music Module
  uint8_t * wBuffer = generateDFPBuffer(DFP_INIT, DFP_EMPTY_ARG, 0x00);
  sendBuffer(wBuffer, DFP_BUF_ARR_LEN);
  
  // Configure the Music Module
  configDFP(DFP_STD_VOLUME);
}

/*
 * Function that's not yet documented
 */
void loop () {
  // Listens for DFP Requests
  listenDFP();

  // Handles DFP Requests
  if(rReady[0] == DFP_BUF_START) {
    uint8_t requestID = rReady[3];
    switch(requestID) {
      // Song Finished
      case 0x3D:
        // Play a random song
        playRandomSongDFP(DFP_STD_VOLUME);
        break;
        
      // Initialization Request
      case 0x3F:
        // Restart the initiation sequence
        configDFP(DFP_STD_VOLUME);
        break;

      // Micro SD Plugged In
      case 0x3A:
        // Restart the initiation sequence
        configDFP(DFP_STD_VOLUME);
        break;

      default:
          Serial.printf("[DEBUG] Unsupported Request: %02X\r\n", rReady[3]);
        break;
    }

    // Clear the DFP Request
    rReady[0] = DFP_EMPTY_ARG;
  }

  // Pause Switch Logic
  if(!digitalRead(sw_pin) && isPaused) {
    // Resume the DFP's Music
    setVolumeDFP(DFP_STD_VOLUME);
    writeReliableDFP(DFP_PLAY, DFP_EMPTY_ARG, true);
    isPaused = false;
  } else if(digitalRead(sw_pin) && !isPaused) {
    // Pause the DFP's Music
    writeReliableDFP(DFP_PAUSE, DFP_EMPTY_ARG, true);
    isPaused = true;
  }

  // Limit the Loops Per Second to 60
  delay(16);
}

/*
 * Function that's not yet documented
 */
uint16_t playRandomSongDFP(uint8_t volume) {
  // Adjust the volume
  setVolumeDFP(volume);
  
  // Plays a random song from songArr[] with a procedural uniform shuffle
  return writeReliableDFP(DFP_SPECIFY_TRACK, shuffleSelectSong(), true);
}

/*
 * Function that's not yet documented
 */
uint16_t shuffleSelectSong(void) {
  // Generate a random song index
  uint32_t seed = micros();
  randomSeed(seed);
  uint16_t rdIndex = random(0, songIndex);

  // Switching the chosen song with the last choosable song
  uint16_t temp = songArr[songIndex];
  songArr[songIndex] = songArr[rdIndex];
  songArr[rdIndex] = temp;

  // Adjust the songIndex
  if(songIndex == 0) {
    songIndex = songsStored - 1;
    return songArr[0];
  }
  songIndex--;

  // Return the song in the now unreachable index
  return songArr[songIndex + 1];
}

/*
 * Function that's not yet documented
 */
uint16_t setVolumeDFP(uint8_t volume) {
  // Updates the volume with an acknowledgement
  return writeReliableDFP(DFP_SPECIFY_VOL, volume, true);
}

/*
 * Function that's not yet documented
 */
bool listenDFP(void) {
  // Loop until the buffer is empty or a message is constructed
  while(serial.available() > 0) {
    uint8_t input = serial.read();

    // Check if the bufferIndex should be reset
    if(input == DFP_BUF_START || bufferIndex == DFP_BUF_ARR_LEN) {
      bufferIndex = 0;
    }

    // Manage the buffer
    rBuffer[bufferIndex] = input;
    bufferIndex++;

    // Checks if the buffer can be returned
    if(input == DFP_BUF_END && bufferIndex == DFP_BUF_ARR_LEN) {
      bufferIndex = 0;
      memmove(rReady, rBuffer, DFP_BUF_ARR_LEN);
      return true;
    }
  }
  return false;
}

/*
 * Function that's not yet documented
 */
uint16_t writeReliableDFP(uint8_t command, uint16_t parameter, uint8_t feedback) {
  flushBuffer();
  Serial.printf("\r\nwriteSecureDFP(%02X, %04X, %02X) {\r\n", command, parameter, feedback);
  
  // Generate the command buffer
  uint8_t * wBuffer = generateDFPBuffer(command, parameter, feedback);
  uint16_t timeout = 1000;

  // Keep sending the buffer until the checkSum is correct
  bool reliable = false;
  while(!reliable) {
    // Send the buffer
    sendBuffer(wBuffer, DFP_BUF_ARR_LEN);
    Serial.println("[MESSAGE] Sending Request");
    
    // Receive the Response
    uint32_t startT = millis();
    while(listenDFP() != true) {
      if(millis() - startT > timeout) {
        sendBuffer(wBuffer, DFP_BUF_ARR_LEN);
        startT = millis();
        Serial.println("[DEBUG] Timeout");
      }
    }
    Serial.println("[MESSAGE] Received Answer");
    reliable = checkReliabilityDFP(rReady);
    flushBuffer();
  }
  parameter = (rReady[5] << 8) | rReady[6];
  Serial.printf("[MESSAGE] Type: %02X, Value: %04X\r\n", rReady[3], parameter);

  Serial.printf("}\r\n");
  flushBuffer();
  return parameter;
}

/*
 * Function that's not yet documented
 */
bool checkReliabilityDFP(uint8_t * buff) {
  // calculate the checksum
  int16_t checksum = (rReady[7] << 8) | rReady[8];
  checksum = checksum +
    rReady[1] + rReady[2] + rReady[3] + 
    rReady[4] + rReady[5] + rReady[6];

  // Doing reliability checks
  if (checksum != 0) {
    Serial.printf("[DEBUG] Checksum Expected: 0, Checksum Received: %d\r\n", checksum);
  } else if (rReady[3] == 0x40) {
    Serial.println("[DEBUG] Retransmission Requested");
  } else {
    return true;
  }
  return false;
}

/*
 * Function that's not yet documented
 */
void sendBuffer(uint8_t * buff, uint16_t buff_len) {
  // Send the buffer over the SoftwareSerial
  for (uint8_t i = 0; i < buff_len; i++) {
    serial.write(buff[i]);
  }
}

/*
 * Function that's not yet documented
 */
void flushBuffer(void) {
  // Clear the response buffers
  while (serial.read() != -1);
  rReady[0] = DFP_EMPTY_ARG;
}
