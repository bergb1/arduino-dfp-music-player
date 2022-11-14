/*
 * File that's not yet documented
 */

#ifndef DFPLAYER_H
#define DFPLAYER_H

#include <stdint.h>

/*
 * A DFP command looks like this:
 * /=========================================================================================================\
 * [ Start || Version || Length || Command ID || Feedback ||      Parameter     ||      Checksum      || End ]
 * \=========================================================================================================/
 * <---0---><----1----><---2----><-----3------><----4-----><----5----><----6----><----7----><----8----><--9-->
 */

// DFP Command Buffer
#define DFP_BUF_ARR_LEN   (10)
#define DFP_EMPTY_ARG     (0)

// DFP Constants
#define DFP_MAX_SONGS     (3000)
#define DFP_STD_VOLUME    (5)

// DFP Command Constants
#define DFP_BUF_START     (0x7E)
#define DFP_BUF_VERSION   (0xFF)
#define DFP_BUF_LEN       (0x06)
#define DFP_BUF_END       (0xEF)

// DFP Setting Command IDs
#define DFP_SPECIFY_TRACK (0x03)
#define DFP_SPECIFY_VOL   (0x06)

// DFP Playback Command IDs
#define DFP_PLAY          (0x0D)
#define DFP_PAUSE         (0x0E)

// DFP Initialization Command ID
#define DFP_INIT          (0x3F)

// DFP Query Command IDs
#define DFP_QUERY_STATUS  (0x42)
#define DFP_QUERY_TOTAL   (0x48)

/*
 * Function that's not yet documented
 */
uint8_t * generateDFPBuffer(uint8_t command_id, uint16_t parameter, uint8_t feedback);

#endif
