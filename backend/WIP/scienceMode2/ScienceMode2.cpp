/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *   This software comes with absolutely NO WARRANTY. Use it at your own   *
 *   risk.                                                                 *
 *                                                                         *
 ***************************************************************************/
/*
 *
 * ScienceMode2.cpp
 *
 *  Created on: 17.08.2012
 *       eMail: mendicus@gmx.de
 *      Author: Ralph Stephan
 *
 */

/*
 * This is a driver software to operate a RehaStim2-device made by Hasomed GmbH
 * using the specially for research designed sciencemode2-protocol
 * This implementation is based on official protocol definition by Hasomed GmbH
 */

#include "ScienceMode2.h"

/*******************************************************************************
 \fn ScienceMode2::ScienceMode2 ()
 *
 * \brief standard constructor
 *
 * \param
 */
ScienceMode2::ScienceMode2(  )
{
    printf( "ScienceMode2::ScienceMode2() create Object\n" );

    int i;
    isConnected = false;
    isInitialised = false;
    isRunning = false;
    isNewData = false;

    for ( i = 0; i < 7; i++)
    {
    	sent[i] = 0;
    	acks[i] = 0;
    	errors[i] = 0;
    }

    errstat = 0;

    for( i = 0; i < STIM_CHANNELS; ++i )
        Kanalkonfiguration[i] = i;

    for (i = 0; i < BUF_SIZE; ++i)
    {
        tmpbuf[i] = 0;
        framebuf[i] = 0;
    }

    for (i = 0; i < MAX_FRAME_LENGTH; ++i)
    {
        last_message_data[i] = 0;
        last_req_message_data[i] = 0;
    }

    last_messagedata_length = 0;
    last_req_message_length = 0;

    framebuflen = 0;
    tmpbuflen = 0;

    paketnummer = 0;
    possible_Mode = SINGLES_ALLOWED;

    /*t = 0;*/
    /*delta_t = 0;*/

    unsigned char tmpCRCTable[256] = {
        0x00, 0x07, 0x0E, 0x09, 0x1C, 0x1B, 0x12, 0x15, // 00..07
        0x38, 0x3F, 0x36, 0x31, 0x24, 0x23, 0x2A, 0x2D, // 08..15
        0x70, 0x77, 0x7E, 0x79, 0x6C, 0x6B, 0x62, 0x65, // 16..23
        0x48, 0x4F, 0x46, 0x41, 0x54, 0x53, 0x5A, 0x5D, // 24..31
        0xE0, 0xE7, 0xEE, 0xE9, 0xFC, 0xFB, 0xF2, 0xF5, // 32..39
        0xD8, 0xDF, 0xD6, 0xD1, 0xC4, 0xC3, 0xCA, 0xCD, // 40..47
        0x90, 0x97, 0x9E, 0x99, 0x8C, 0x8B, 0x82, 0x85, // 48..55
        0xA8, 0xAF, 0xA6, 0xA1, 0xB4, 0xB3, 0xBA, 0xBD, // 56..63
        0xC7, 0xC0, 0xC9, 0xCE, 0xDB, 0xDC, 0xD5, 0xD2, // 64..71
        0xFF, 0xF8, 0xF1, 0xF6, 0xE3, 0xE4, 0xED, 0xEA, // 72..79
        0xB7, 0xB0, 0xB9, 0xBE, 0xAB, 0xAC, 0xA5, 0xA2, // 80..87
        0x8F, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9D, 0x9A, // 88..95
        0x27, 0x20, 0x29, 0x2E, 0x3B, 0x3C, 0x35, 0x32, // 96..103
        0x1F, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0D, 0x0A, // 104..111
        0x57, 0x50, 0x59, 0x5E, 0x4B, 0x4C, 0x45, 0x42, // 112..119
        0x6F, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7D, 0x7A, // 120..127
        0x89, 0x8E, 0x87, 0x80, 0x95, 0x92, 0x9B, 0x9C, // 128..135
        0xB1, 0xB6, 0xBF, 0xB8, 0xAD, 0xAA, 0xA3, 0xA4, // 136..143
        0xF9, 0xFE, 0xF7, 0xF0, 0xE5, 0xE2, 0xEB, 0xEC, // 144..151
        0xC1, 0xC6, 0xCF, 0xC8, 0xDD, 0xDA, 0xD3, 0xD4, // 152..159
        0x69, 0x6E, 0x67, 0x60, 0x75, 0x72, 0x7B, 0x7C, // 160..167
        0x51, 0x56, 0x5F, 0x58, 0x4D, 0x4A, 0x43, 0x44, // 168..175
        0x19, 0x1E, 0x17, 0x10, 0x05, 0x02, 0x0B, 0x0C, // 176..183
        0x21, 0x26, 0x2F, 0x28, 0x3D, 0x3A, 0x33, 0x34, // 184..191
        0x4E, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5C, 0x5B, // 192..199
        0x76, 0x71, 0x78, 0x7F, 0x6A, 0x6D, 0x64, 0x63, // 200..207
        0x3E, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2C, 0x2B, // 208..215
        0x06, 0x01, 0x08, 0x0F, 0x1A, 0x1D, 0x14, 0x13, // 216..223
        0xAE, 0xA9, 0xA0, 0xA7, 0xB2, 0xB5, 0xBC, 0xBB, // 224..231
        0x96, 0x91, 0x98, 0x9F, 0x8A, 0x8D, 0x84, 0x83, // 232..239
        0xDE, 0xD9, 0xD0, 0xD7, 0xC2, 0xC5, 0xCC, 0xCB, // 240..247
        0xE6, 0xE1, 0xE8, 0xEF, 0xFA, 0xFD, 0xF4, 0xF3  // 248..255
    };

    for( i = 0; i < 256; i++ )
        this->crctable[i] = tmpCRCTable[i];

}

/*******************************************************************************
 \fn ScienceMode2::~ScienceMode2 ()
 *
 * \brief standard destructor
 *
 * \param
 */
ScienceMode2::~ScienceMode2(  )
{
    delete sPort;
}

/*******************************************************************************
 \fn ScienceMode2::Close_serial ()
 *
 * \brief
 *
 * \param
 */
int ScienceMode2::Close_serial(  )
{
    int retval = 0;

    Send_Stop_Signal(  );

#ifdef __unix
    sPort->serial_flush(  );
#endif

#ifdef _WIN32
    retval = sPort->serial_readstring(tmpbuf, 1);
#endif

    retval = sPort->serial_closeport(  );

    if( retval < 0 )
        printf( "ScienceMode2::Close_serial() RehaStim2 not disconnected\n" );
    else
    {
        printf( "ScienceMode2::Close_serial() RehaStim2 disconnected\n" );
        isInitialised = false;
    }

    return retval;
}

/*******************************************************************************
 \fn void ScienceMode2::setDeviceName ( char * port)
 *
 * \brief sets the devicename for serial communication as a class attribute
 *
 * \param
 */
void ScienceMode2::setDeviceName( char * port )
{
    devicename = port;
}

/*******************************************************************************
 \fn int ScienceMode2::Open_serial ()
 *
 * \brief open serial communication port named by class attribute devicename
 *
 * \param
 */
int ScienceMode2::Open_serial(  )
{
    int retval;

    sPort = new SerialPort(  );

    //open and setup stimulator device
    retval = sPort->serial_openport( devicename );

    if( retval < 0 )
        printf( "ScienceMode2::Open_serial() RehaStim2 not connected\n" );
    else
    {
#ifdef __unix
        retval = sPort->serial_setupport( BAUD_460800, NOHANDSHAKE, ONESTOPBIT );
#endif

#ifdef _WIN32
        retval = sPort->serial_setupport( BAUD_460800, NOHANDSHAKE, ONESTOPBIT, EVEN );
#endif
        if( retval < 0 )
        {
            printf
                ( "ScienceMode2::Open_serial() Error setting up RehaStim2" );

        }
        else
        {
#ifdef __unix
            retval = sPort->serial_set_parity(EVEN);
    		sPort->serial_flush(  );
#endif     
       
#ifdef _WIN32
    		retval = sPort->serial_readstring(tmpbuf, 1);
#endif
            printf( "ScienceMode2::Open_serial() RehaStim2 connected\n" );
        }
    }

    return retval;
}

/*******************************************************************************
 \fn unsigned char ScienceMode2::CalcCRC8( const void *data, unsigned int length )
 *
 * \brief calculates checksum from given buffer
 *
 * \verbatim
 *
 * local  unsigned char                 crc             rw
 * local  const unsigned char           *buf            r
 * local  const void                    *data           r
 * local  unsigned int                        length          rw
 * global unsigned char                 crctable[]      r
 *
 * \endverbatim
 *
 * \param databuffer containing bytes for calculating checksum
 * \param length of given databuffer
 */
unsigned char ScienceMode2::CalcCRC8( const void *data,
                                      unsigned int length )
{
    unsigned char crc = 0x00;

    const unsigned char *buf = (const unsigned char *) data;

    while( length-- )
    {
        crc = crctable[crc ^ *buf++];
    }

    crc ^= 0x00;
#if (DEBUG_SCM2 == 1)
    printf("Calculated Checksum: %u\n", (unsigned char)crc);
#endif
    return crc;
}

/*******************************************************************************
 \fn int ScienceMode2::getData()
 *
 * \brief gets data from serial port and appends it to databuffer
 *
 * local  int                           retval          rw
 * local  int                           counter         rw
 * local  int                           index           rw
 * global char                          tmpbuf[]        rw
 * global char                          framebuf[]      rw
 * global const int                     BUFSIZE         r
 * global int                       framebuflen     rw
 * global int                       tmpbuflen       rw
 *
 * \param
 */
int ScienceMode2::getData(  )
{
    int retval = 0, counter = 0, index = 0;

#if (DEBUG_SCM2 == 1)
	printf("ScienceMode2::getData() TIMER: %lu\n", current_time());
#endif

    //get data from port
#ifdef __unix
    retval = sPort->serial_read_nonblocking(tmpbuf, BUF_SIZE);
#endif

#ifdef _WIN32
    retval = sPort->serial_readstring(tmpbuf, BUF_SIZE);
#endif
    tmpbuflen = retval;

    if (tmpbuflen < 0)
    {
        //no data or error
        return -1;
    } else
    {
        while (counter < tmpbuflen)
        {
            if ((index + framebuflen) >= (BUF_SIZE - 1))
            {
                perror("Buffer full");
                break;

            } else
            {
                framebuf[index + framebuflen] = tmpbuf[counter];
#if (DEBUG_SCM2 == 1)
				printf( "%02x ", (unsigned char)framebuf[index + framebuflen] );
#endif
                tmpbuf[counter] = '\0';
                index++;
            }

            counter++;
        }
    }

    if (retval != index)
    {
        // error
        perror("getData(): error sorting data");
        return -1;

    } else
    {
        framebuflen += index;
        retval = framebuflen;
#if (DEBUG_SCM2 == 1)
		printf("\n");
#endif
    }

    return retval;
}

/*******************************************************************************
 \fn void ScienceMode2::adjustBuffer( int startindex )
 *
 * \brief moves data between given index and the end of buffer to index 0
 *
 * local  int                           startindex      r
 * global char                          framebuf[]      rw
 * global int                       framebuflen     rw
 *
 * \param startindex in buffer where to start moving data
 */
void ScienceMode2::adjustBuffer( int startindex )
{

    // if 10 bytes in buffer: framebuflen = 10
    // then startindex is allowed from 1...9
    if ( (framebuflen > 0) && ( startindex > 0 ) && ( startindex <= (framebuflen - 1) ) )
    {
        // remove read data from buffer, shift left
        for (int i = startindex; i < framebuflen; ++i)
        {
            framebuf[i - startindex] = framebuf[i];
        }

        // change length of valid data for next read operation on the buffer
        framebuflen = framebuflen - startindex;
    } else
    {
        framebuflen = 0;
    }
}

/*******************************************************************************
 \fn bool ScienceMode2::getFrame()
 *
 * \brief processes the data buffer. fingers out a valid data frame.
 *
 * local  int                           pufferindex                 rw
 * local  int                           recv_paketnummer            rw
 * local  int                           kommando                    rw
 * local  int                           retval                      rw
 * local  int                           counter                     rw
 * local  int                           i                           rw
 * local  int                           ffi                         rw
 * local  int                           fli                         rw
 * local  int                           chksum                      rw
 * local  int                           mychecksum                  rw
 * local  int                           datalength                  rw
 * local  char                          datenpuffer[]               rw
 * local  bool                          isStuffed                   rw
 * local  bool                          isValid                     rw
 * global const int                     MAX_FRAME_LENGTH            r
 * global const int                     MIN_FRAME_LENGTH            r
 * global char                          framebuf[]                  rw
 * global int                       framebuflen                 rw
 * global char                          last_message_data[]         rw
 * global int                       last_messagedata_length     rw
 *
 */
bool ScienceMode2::getFrame(  )
{
    int pufferindex = 0, recv_paketnummer, kommando;
    char datenpuffer[MAX_FRAME_LENGTH];

    int retval = 0, counter = 0, i = 0, ffi = 0,        //frame first index
        fli = 0;                //frame last index

    int chksum = 0, mychecksum = 0, datalength = 0;

    retval = framebuflen;

    bool isStuffed = false;
    bool isValid = false;

#if (DEBUG_SCM2 == 1) && (__unix || _TTY_POSIX_)
	printf("ScienceMode2::getFrame() TIMER: %lu\n", current_time());
#endif

    if (retval < 0)
    {
        perror("error getting data buffer");
        return -1;
    }
    //----------------------------------

    if (retval >= MIN_FRAME_LENGTH)
    {

        // search for startbyte
        while ( (counter < retval) && !((framebuf[counter] & 0xFF) == 0xF0) )
        {
            counter++;
        }
    }

    // detected startbyte before buffer ends?
    if (counter < retval)
    {
        // Yes!
        ffi = counter;          // frame first index

        // search for stopbyte
        while ( (counter < retval) && !((framebuf[counter] & 0xFF) == 0x0F) )
        {
            if ((framebuf[counter] & 0xFF) == 0x81)
                // Es kann vorkommen, dass durch das Stuffing der Checksumme
                // ein zu stuffender Wert (zB 0x0f) entsteht. Dann wuerde ein
                // falsches Frameende detektiert werden. Taucht ein StuffingByte
                // auf, so wird das folgende Byte ignoriert um das tatsaechliche
                // Frameende zu finden
                //
                // jump over stuffed data
                counter += 2;
            else
                counter++;
        }

        // detected stopbyte before buffer ends?
        if (counter < retval)
        {
            // Yes!
            fli = counter; // frame last index

            if( (fli - ffi + 1) > MAX_FRAME_LENGTH)
            {
                // length from F0 to 0F too long!
                isValid = false;

                adjustBuffer(fli + 1);
            }
            else
            {

                //----------------------------------

                chksum = (framebuf[ffi + 2] & 0xFF) ^ 0x55; // unstuff checksum
                // Durch das Stuffing der Datenlaenge kann ein zu stuffender
                // Wert entstehen. Das spielt hier aber keine Rolle mehr, da
                // die Framelaenge durch entsprechende Korrektur (s.o.) bereits
                // ermittelt worden ist.
                datalength = (framebuf[ffi + 4] & 0xFF) ^ 0x55; // unstuff datalength

                //----------------------------------

                pufferindex = 0;

                // actual data length corresponds to the desired (calculated) length
                if ( datalength == (fli - (ffi + 5)) )
                {

                    // extract data
                    for ( i = (ffi + 5); i <= (fli - 1); i++ )
                    {
                        if ( (framebuf[i] & 0xFF) == 0x81 )
                        {   // unstuff next byte
                            isStuffed = true;
                            continue;
                        }
                        else
                        {
                            if (isStuffed)
                            {
                                datenpuffer[pufferindex] = (framebuf[i] ^ 0x55) & 0xFF;
                                last_message_data[pufferindex] = (framebuf[i] ^ 0x55) & 0xFF;
                                isStuffed = false;
                            }
                            else
                            {
                                datenpuffer[pufferindex] = framebuf[i] & 0xFF;
                                last_message_data[pufferindex] = framebuf[i] & 0xFF;
                            }
                            pufferindex++;
                        }
                    }

                    // Hier ist der pufferindex gleich der Anzahl der Elemente im datenpuffer
                    last_messagedata_length = pufferindex;

                    recv_paketnummer = datenpuffer[0] & 0xFF;
                    kommando = datenpuffer[1] & 0xFF;

                    // checksum check over all stuffed bytes
                    mychecksum = CalcCRC8( &framebuf[ffi+5], datalength );

                    //----------------------------------

                    if (chksum == mychecksum)
                    {
                        isValid = true;

                        adjustBuffer(fli + 1);

                    }
                    else
                    {
                        printf("checksum failed for command %u, packageno %u, chksum %u, mychecksum %u\n", kommando, recv_paketnummer, chksum, mychecksum);
                        adjustBuffer(ffi);
                    }

                }
                else
                {
                    //
                    // actual length of data in current frame differs to the desired (calculated) no of bytes
                    printf( "datalength failure desired value %u actual value %i\n", datalength, (fli - (ffi + 5)) );
                    isValid = false;
                    /*
                    printf("Data: ");
                    for (i = ffi; i <= fli+8; i++)
                    printf("%02x ", framebuf[i] & 0xFF);
                    printf("\n");
                    */
                    adjustBuffer(ffi);
                }
            }
        }
        else
        {
            // No!
            framebuflen = 0;
            isValid = false;
        }
    }
    else
    {
        // No!
        framebuflen = 0;
        isValid = false;
    }

    //if (isValid)
    //  talk(&last_message_data);

#if (DEBUG_SCM2 == 1)
	debug_received_frame( last_message_data, last_messagedata_length );
#endif

    return isValid;
}

/*******************************************************************************
 \fn ScienceMode2::getFrameBufLen()
 *
 * \brief delivers the current framebuflen
 *
 * global int                       framebuflen                 r
 *
 */
int ScienceMode2::getFrameBufLen(  )
{
    return framebuflen;
}

/*******************************************************************************
 \fn ScienceMode2::talk ()
 *
 * \brief answers received commands
 *
 * \param
 */
void ScienceMode2::talk(  )
{
    talk(&last_message_data);
}

/*******************************************************************************
 \fn ScienceMode2::talk (void * datenpuffer)
 *
 * \brief answers received commands
 *
 * \param
 */
void ScienceMode2::talk( void * datenpuffer )
{
    char * buffer = (char *) datenpuffer;

    switch(buffer[1])
    {
        case 0x01:
#if (DEBUG_SCM2 == 1)
            printf("talk(): RehaStim2 said Init %i: package %u\n", buffer[1], (unsigned char)buffer[0]);
#endif
            acks[ACK]++;
            isConnected = false;
            isInitialised = false; // Die Verbindung war unterbrochen, daher erneute Initialisierung erforderlich
            isRunning = false;
            if( !errstat )
            	Send_Init_Ack();
        break;

        case 0x02:
#if (DEBUG_SCM2 == 1)
            printf("talk(): RehaStim2 said InitAck %i: package %u, ErrorCode %i\n", buffer[1], (unsigned char)buffer[0], buffer[2]);
#endif
        break;

        case 0x03:
#if (DEBUG_SCM2 == 1)
            printf("talk(): RehaStim2 said UnknownCommand %i: package %u\n", buffer[1], (unsigned char)buffer[0]);
#endif
        break;

        case 0x04:
#if (DEBUG_SCM2 == 1)
            printf("talk(): RehaStim2 said Watchdog %i: package %u\n", buffer[1], (unsigned char)buffer[0]);
#endif
        break;

        case 10:
#if (DEBUG_SCM2 == 1)
            printf("talk(): RehaStim2 said GetStimulationMode %i: package %u\n", buffer[1], (unsigned char)buffer[0]);
#endif
        break;

        case 11:
#if (DEBUG_SCM2 == 1)
            printf("talk(): RehaStim2 said GetStimulationModeAck %i: package %u, ErrorCode %i, Mode %i: ", buffer[1], (unsigned char)buffer[0], buffer[2], buffer[3]);
#endif
            acks[MODE]++;

            switch(buffer[2])
            {
                case 0:
                    switch(buffer[3])
                    {
                        case 0:
                            // SINGLE_PULSE possible in this mode only
                            isConnected = true;
                            isInitialised = false;
                            isRunning = false;
#if (DEBUG_SCM2 == 1)
                            printf("stimulator set to startmode\n");
#endif
                        break;

                        case 1:
                            isConnected = true;
                            isInitialised = true;
                            isRunning = false;
#if (DEBUG_SCM2 == 1)
                            printf("stimulation initialized\n");
#endif
                        break;

                        case 2:
                            isConnected = true;
                            isRunning = true;
                            isInitialised = true;
#if (DEBUG_SCM2 == 1)
                            printf("stimulation running\n");
#endif
                        break;

                        default:
                        break;
                    }
                break;

                case -1:
#if (DEBUG_SCM2 == 1)
                    printf("transfer error\n");
#endif
                    errors[MODE]++;
                break;

                case -8:
#if (DEBUG_SCM2 == 1)
                    printf("busy error\n");
#endif
                    errors[MODE]++;
                break;

                default:
#if (DEBUG_SCM2 == 1)
                    printf("\n");
#endif
                    errors[MODE]++;
                break;
            }
        break;

        case 30:
#if (DEBUG_SCM2 == 1)
            printf("talk(): RehaStim2 said InitChannelListMode %i: package %u\n", buffer[1], (unsigned char)buffer[0]);
#endif
        break;

        case 31:
#if (DEBUG_SCM2 == 1)
            printf("talk(): RehaStim2 said InitChannelListModeAck %i: package %u, ErrorCode %i\n", buffer[1], (unsigned char)buffer[0], buffer[2]);
#endif
            acks[INIT]++;
            switch(buffer[2])
            {
                case 0:
                    isConnected = true;
                    isInitialised = true;
                    isRunning = false;
#if (DEBUG_SCM2 == 1)
                    printf("Stimulation initialized\n");
#endif
                break;

                case -1:
#if (DEBUG_SCM2 == 1)
                    printf("Transfer Error\n");
#endif
                    errors[INIT]++;
                break;

                case -2:
#if (DEBUG_SCM2 == 1)
                    printf("Parameter Error\n");
#endif
                    errors[INIT]++;
                break;

                case -3:
#if (DEBUG_SCM2 == 1)
                    printf("Wrong Mode Error\n");
#endif
                    errors[INIT]++;
                break;

                case -8:
#if (DEBUG_SCM2 == 1)
                    printf("Busy Error\n");
#endif
                    errors[INIT]++;
                break;

                default:
#if (DEBUG_SCM2 == 1)
                    printf("\n");
#endif
                    errors[INIT]++;
                break;
            }
        break;

        case 32:
#if (DEBUG_SCM2 == 1)
            printf("talk(): RehaStim2 said StartChannelListMode %i: package %u\n", buffer[1], (unsigned char)buffer[0]);
#endif
        break;

        case 33:
#if (DEBUG_SCM2 == 1)
            printf("talk(): RehaStim2 said StartChannelListModeAck %i: package %u, ErrorCode %i\n", buffer[1], (unsigned char)buffer[0], buffer[2]);
#endif
            acks[UPDATE]++;
            switch(buffer[2])
            {
                case 0:
                    isConnected = true;
                    isInitialised = true;
                    isRunning = true;
#if (DEBUG_SCM2 == 1)
                    printf("stimulation running\n");
#endif
                break;

                case -1:
#if (DEBUG_SCM2 == 1)
                    printf("transfer error\n");
#endif
                    errors[UPDATE]++;
                break;

                case -2:
#if (DEBUG_SCM2 == 1)
                    printf("parameter error\n");
#endif
                    errors[UPDATE]++;
                break;

                case -3:
#if (DEBUG_SCM2 == 1)
                    printf("wrong mode error\n");
#endif
                    errors[UPDATE]++;
                break;

                case -8:
#if (DEBUG_SCM2 == 1)
                    printf("busy error\n");
#endif
                    errors[UPDATE]++;
                break;

                default:
#if (DEBUG_SCM2 == 1)
                    printf("\n");
#endif
                    errors[UPDATE]++;
                break;
            }
        break;

        case 34:
#if (DEBUG_SCM2 == 1)
            printf("talk(): RehaStim2 said StopChannelListMode %i: package %u\n", buffer[1], (unsigned char)buffer[0]);
#endif
        break;

        case 35:
#if (DEBUG_SCM2 == 1)
            printf("talk(): RehaStim2 said StopChannelListModeAck %i: package %u, ErrorCode %i\n", buffer[1], (unsigned char)buffer[0], buffer[2]);
#endif
            acks[STOP]++;
            switch(buffer[2])
            {
                case 0:
#if (DEBUG_SCM2 == 1)
                    printf("stimulation stop in progress\n");
#endif
                    isConnected = true;
                    isInitialised = false;
                    isRunning = false;
                break;

                case -1:
#if (DEBUG_SCM2 == 1)
                    printf("transfer error\n");
#endif
                    errors[STOP]++;
                break;

                default:
#if (DEBUG_SCM2 == 1)
                    printf("\n");
#endif
                    errors[STOP]++;
                break;
            }
        break;

        case 36:
#if (DEBUG_SCM2 == 1)
            printf("talk(): RehaStim2 said SinglePulse %i: package %u\n", buffer[1], (unsigned char)buffer[0]);
#endif
        break;

        case 37:
#if (DEBUG_SCM2 == 1)
            printf("talk(): RehaStim2 said SinglePulseAck %i: package %u, ErrorCode %i\n", buffer[1], (unsigned char)buffer[0], buffer[2]);
#endif
            acks[SINGLE]++;
            switch(buffer[2])
            {
                case 0:
#if (DEBUG_SCM2 == 1)
                    printf("single pulse in progress\n");
#endif
                break;

                case -1:
#if (DEBUG_SCM2 == 1)
                    printf("transfer error\n");
#endif
                    errors[SINGLE]++;
                break;

                case -2:
#if (DEBUG_SCM2 == 1)
                    printf("parameter error\n");
#endif
                    errors[SINGLE]++;
                break;

                case -3:
#if (DEBUG_SCM2 == 1)
                    printf("wrong mode error\n");
#endif
                    errors[SINGLE]++;
                break;

                case -8:
#if (DEBUG_SCM2 == 1)
                    printf("busy error\n");
#endif
                    errors[SINGLE]++;
                break;

                default:
#if (DEBUG_SCM2 == 1)
                    printf("\n");
#endif
                    errors[SINGLE]++;
                break;
            }
        break;

        case 38:
#if (DEBUG_SCM2 == 1)
            printf("talk(): RehaStim2 said StimulationError %i: package %u, Error %i\n", buffer[1], (unsigned char)buffer[0], buffer[2]);
#endif
			/* This is set for our own programe control.
			 * In case of one of these errors the stimulator
			 * itself cancels stimulation.
			 */
            isConnected = true;
            isInitialised = false;
            isRunning = false;

            switch(buffer[2])
            {
                case -1:
#if (DEBUG_SCM2 == 1)
                    printf("emergency switch activated\n");
#endif
                    errstat = -1;
                break;

                case -2:
#if (DEBUG_SCM2 == 1)
                    printf("electrode failed\n");
#endif
                    errstat = -2;
                break;

                case -3:
#if (DEBUG_SCM2 == 1)
                    printf("stimulation module failed\n");
#endif
                    errstat = -3;
                break;

                default:
#if (DEBUG_SCM2 == 1)
                    printf("\n");
#endif
                break;
            }
        break;

        default:
        break;
    }
}

/*******************************************************************************
 \fn ScienceMode2::Send_Init_Param ()
 *
 * \brief wraps stimulator double parameters from scicos workspace to integers
 *
 * \param
 */
void ScienceMode2::Send_Init_Param ( unsigned int stim_channels_used, // Number of channels used
                                    double *Scicos_Channel_Stim, // List of channels
                                    unsigned int lf_channels_used, // Number of Channel_Lf
                                    double *Scicos_Channel_Lf, // List of Low Frequency Channels
                                    double Scicos_Main_Time, // Main Stimulation Interval in ms
                                    double Scicos_Group_Time, // Inter Pulse Interval in ms
                                    double Scicos_N_Factor ) // Low frequency factor
{
	unsigned int Channel_Stim;
    unsigned int Channel_Lf;
    unsigned int Group_Time_Coded;
    unsigned int Main_Time_Coded;
    unsigned int N_Factor;

    Make_Index( Scicos_Channel_Stim, stim_channels_used );
    Channel_Stim = Encode_Channel_Stim( Scicos_Channel_Stim, stim_channels_used );
    Channel_Lf = Encode_Channel_Lf( Scicos_Channel_Lf, lf_channels_used );

    if( Scicos_Main_Time == 0 )
        Main_Time_Coded = 0;
    else
        Main_Time_Coded = ( unsigned int )( ( Scicos_Main_Time - 1 ) * 2 );

    Group_Time_Coded = ( unsigned int )( ( Scicos_Group_Time - 1.5 ) * 2 );
    N_Factor = ( unsigned int )Scicos_N_Factor;

    InitChannelListMode( Channel_Stim, Channel_Lf, Main_Time_Coded, Group_Time_Coded, N_Factor );
}

/*******************************************************************************
 \fn ScienceMode2::getPaketnummer ()
 *
 * \brief generates current packagenumber for sendpackage
 *
 * \param
 */
unsigned char ScienceMode2::getPaketnummer(  )
{
    unsigned char retval = paketnummer;

    if(paketnummer < 255)
        paketnummer++;
    else
        paketnummer = 0;

    return retval;
}

/*******************************************************************************
 \fn ScienceMode2::InitChannelListMode ()
 *
 * \brief sends initcommand to stimulator
 *
 * \param
 */
void ScienceMode2::InitChannelListMode( unsigned int Channel_Stim,
                                       unsigned int Channel_Lf,
                                       unsigned int Main_Time_Coded,
                                       unsigned int Group_Time_Coded,
                                       unsigned int N_Factor )
{
    int chars_written;

    char Sendedaten[MAX_FRAME_LENGTH];

    char CRC = 0;

    unsigned int i, framelaenge_unstuffed = 15, framelaenge_stuffed = 0;
    unsigned int nutzdatenlaenge_unstuffed = 9, nutzdatenlaenge_stuffed = 0;

    unsigned int my_Channel_Lf;
    double Main_Time;
    double Group_Time;

    if( N_Factor == 0 )
        my_Channel_Lf = 0;
    else
    {
        // Mask out Channel_Lf to  only those channels in the Channel_Stim list
        my_Channel_Lf = Channel_Lf & Channel_Stim;
    }

    if( Channel_Stim == 0 )
    {
        //printf("Channel_Stim == 0\n");
    }

    Group_Time = Group_Time_Coded * 0.5 + 1.5; // in ms

    if ((Group_Time < 8) || (Group_Time_Coded < 13))
    {
        Group_Time_Coded = 13;
        Group_Time = 8; // in ms
    }
#if (DEBUG_SCM2 == 1)
    printf("InitChannelListMode: GroupTime = %0.1f ms, %i\n", Group_Time, Group_Time_Coded);
#endif

    Main_Time = Main_Time_Coded * 0.5 + 1.0; // in ms

    // Main_Time_Coded == 0 bedeutet externer Trigger !!!!
    if ( (Main_Time_Coded != 0) && ((Main_Time < 8) || (Main_Time_Coded < 14)))
    {
        Main_Time_Coded = 14;
        Main_Time = 8; // in ms
    }
#if (DEBUG_SCM2 == 1)
    printf("InitChannelListMode: MainTime = %0.1f ms, %i\n", Main_Time, Main_Time_Coded);
#endif
    if( Main_Time_Coded == 0 )
    {
        //Main_Time = 0;
        possible_Mode = TRIPLETS_ALLOWED;  // Triples allowed, frequency chosen by user
    }
    else
    {
        if( Main_Time >= Group_Time )
        {
            possible_Mode = SINGLES_ALLOWED;      // Only singles allowed

            if( Main_Time >= 2 * Group_Time )
                possible_Mode = DOUBLETS_ALLOWED;  // Doublets allowed

            if( Main_Time >= 3 * Group_Time )
                possible_Mode = TRIPLETS_ALLOWED;  // Triplets allowed
        }
        else
        {
#if (DEBUG_SCM2 == 1)
            printf("Main_Time_Coded < Group_Time_Coded\n");
#endif
        }
    }

    Sendedaten[0] = 0xF0; // Startbyte
    Sendedaten[1] = 0x81; // Stuffingbyte
    Sendedaten[3] = 0x81; // Stuffingbyte
    Sendedaten[5] = getPaketnummer();
    Sendedaten[6] = 0x1E; // InitChannelListMode

    Sendedaten[7] = (char)N_Factor; // Low Frequency Factor
    Sendedaten[8] = (char)Channel_Stim; // Active Channels
    Sendedaten[9] = (char)my_Channel_Lf; // Active Low Frequency Channels
    Sendedaten[10] = (char)((unsigned int)Group_Time_Coded & 0x0ff); // Inter Pulse Interval Coded
    Sendedaten[11] = (char)((((unsigned int)Main_Time_Coded) >> 8) & 0x0ff); // Main Stimulation Interval Coded MSB
    Sendedaten[12] = (char)((unsigned int)Main_Time_Coded & 0x0ff); // Main Stimulation Interval Coded LSB
    Sendedaten[13] = 0x00; // Channel Execution

    Sendedaten[14] = 0x0F; // Stopbyte

    nutzdatenlaenge_stuffed = stuffData(&Sendedaten[5], nutzdatenlaenge_unstuffed);

    Sendedaten[4] = nutzdatenlaenge_stuffed ^ 0x55; // stuffed datalength

    CRC = CalcCRC8(&Sendedaten[5], nutzdatenlaenge_stuffed);

    Sendedaten[2] = CRC ^ 0x55; // stuffed checksum

    framelaenge_stuffed = framelaenge_unstuffed + ( nutzdatenlaenge_stuffed - nutzdatenlaenge_unstuffed );

    //SENDING
    chars_written = sPort->serial_sendstring( Sendedaten, framelaenge_stuffed );

    for (i = 0; i < framelaenge_stuffed; i++)
    {
        last_req_message_data[i] = Sendedaten[i] & 0xFF;
        last_req_message_length = framelaenge_stuffed;
    }

    if (chars_written < 0)
        perror("ScienceMode2::InitChannelListMode()");
    else if ((unsigned int)chars_written != framelaenge_stuffed)
        printf("ScienceMode2::InitChannelListMode(): InitChannelListMode not sent correctly\n");
    else
    {
#if (DEBUG_SCM2 == 1)
        printf("ScienceMode2::InitChannelListMode(): sent\n");
    	debug_sent_frame( Sendedaten, framelaenge_stuffed );
#endif
        sent[INIT]++;
	}
}

/*******************************************************************************
 \fn ScienceMode2::stuffData()
 *
 * \brief stuffs databuffer if necessary
 *
 */
int ScienceMode2::stuffData( void *data,
                             unsigned int nutzdatenlaenge_unstuffed )
{
    unsigned int nutzdatenlaenge_stuffed = nutzdatenlaenge_unstuffed;

    int i = 0, j = 0;

    unsigned char *buf = (unsigned char *) data;

    while ( nutzdatenlaenge_unstuffed-- )
    {
#if (DEBUG_SCM2 == 1)
        printf("nutzdatenlaenge_unstuffed=%i, i=%i\n", nutzdatenlaenge_unstuffed, i);
#endif

        if ( ( buf[i] == 0xF0 ) || ( buf[i] == 0x81 ) || ( buf[i] == 0x0F ) )
        {
#if (DEBUG_SCM2 == 1)
            printf("Stuffing: i=%i, Inhalt=%02x\n", i, (unsigned char)buf[i]);
#endif
            for (j = nutzdatenlaenge_stuffed; j >= i; j--)
            {
#if (DEBUG_SCM2 == 1)
                printf("kopiere index j=%i nach j+1=%i\n", j, j + 1);
#endif
                buf[j + 1] = buf[j];
            }

            buf[i] = 0x81; // stuffingbyte einfuegen
            buf[i + 1] ^= 0x55; // mit stuffingkey schluesseln
#if (DEBUG_SCM2 == 1)
            printf("stuffed byte an index %i, schluessel an index %i\n", i + 1, i);
#endif

            nutzdatenlaenge_stuffed++;

#if (DEBUG_SCM2 == 1)
            printf("neue nutzdatenlaenge_stuffed %i\n", nutzdatenlaenge_stuffed);
#endif
            i += 2;
        }
        else
            i += 1;
    }

    return nutzdatenlaenge_stuffed;
}

/*******************************************************************************
 \fn ScienceMode2::Send_Update_Parameter ()
 *
 * \brief wraps stimulator double parameters from scicos workspace to integers
 *
 * \param
 */
void ScienceMode2::Send_Update_Parameter( double *Scicos_Pulse_Width,
                                         double *Scicos_Pulse_Current,
                                         double *Scicos_Mode,
                                         unsigned int stim_channels_used )
{
    unsigned int Pulse_Width[STIM_CHANNELS];
    unsigned int Pulse_Current[STIM_CHANNELS];
    unsigned int Mode[STIM_CHANNELS];
    unsigned int i;

    for( i = 0; i < stim_channels_used; i++ )
    {
        Pulse_Width[i] =
            Encode_Pulse_Width( Scicos_Pulse_Width[Kanalkonfiguration[i]] );
        Pulse_Current[i] =
            Encode_Pulse_Current( Scicos_Pulse_Current[Kanalkonfiguration[i]] );
        Mode[i] = Encode_Mode( Scicos_Mode[Kanalkonfiguration[i]] );
    }

    StartChannelListMode( Pulse_Width, Pulse_Current, Mode, stim_channels_used );
}

/*******************************************************************************
 \fn ScienceMode2::StartChannelListMode ()
 *
 * \brief update stimulator parameters only if data changed
 *
 * \param
 */
void ScienceMode2::StartChannelListMode( unsigned int *Pulse_Width,
                                        unsigned int *Pulse_Current,
                                        unsigned int *Mode,
                                        unsigned int stim_channels_used )
{
    int chars_written;
    char Sendedaten[MAX_FRAME_LENGTH];
    char CRC = 0;
    unsigned int i, framelaenge_unstuffed = 8 + (4 * stim_channels_used), framelaenge_stuffed = 0;
    unsigned int nutzdatenlaenge_unstuffed = 2 + (4 * stim_channels_used), nutzdatenlaenge_stuffed = 0;

    for( i = 0; i < stim_channels_used; i++ )
    {
        Pulse_Width[i] = ( Pulse_Width[i] & 0x01ff );
        Pulse_Current[i] = ( Pulse_Current[i] & 0x00ff);
        Mode[i] = Mode[Kanalkonfiguration[i]];
    }

	Sendedaten[0] = 0xF0; // Startbyte
	Sendedaten[1] = 0x81; // Stuffingbyte
	Sendedaten[3] = 0x81; // Stuffingbyte
	Sendedaten[5] = getPaketnummer();
	Sendedaten[6] = 0x20; // StartChannelListMode

	for (i = 0; i < stim_channels_used; i++)
	{
		Sendedaten[7 + (4 * i)] = (char)Mode[i]; // Mode
		Sendedaten[8 + (4 * i)] = (char)((Pulse_Width[i] >> 8) & 0x01); // Pulse Width MSB
		Sendedaten[9 + (4 * i)] = (char)(Pulse_Width[i] & 0x0ff); // Pulse Width LSB
		Sendedaten[10 + (4 * i)] = (char)(Pulse_Current[i] & 0x0ff); // Current
	}

	Sendedaten[11 + (4 * (i - 1))] = 0x0F; // Stopbyte

	nutzdatenlaenge_stuffed = stuffData(&Sendedaten[5], nutzdatenlaenge_unstuffed);

	Sendedaten[4] = (char)nutzdatenlaenge_stuffed ^ 0x55; // stuffed datalength

	CRC = CalcCRC8(&Sendedaten[5], nutzdatenlaenge_stuffed);

	framelaenge_stuffed = framelaenge_unstuffed + ( nutzdatenlaenge_stuffed - nutzdatenlaenge_unstuffed );

	Sendedaten[2] = CRC ^ 0x55; // stuffed checksum

	//SENDING
	chars_written = sPort->serial_sendstring( Sendedaten, framelaenge_stuffed );

	for (i = 0; i < framelaenge_stuffed; i++)
	{
		last_req_message_data[i] = Sendedaten[i] & 0xFF;
		last_req_message_length = framelaenge_stuffed;
	}

	if (chars_written < 0)
		perror("ScienceMode2::StartChannelListMode()");
    else if ((unsigned int)chars_written != framelaenge_stuffed)
		printf("ScienceMode2::StartChannelListMode(): StartChannelListMode not sent correctly\n");
	else
    {
#if (DEBUG_SCM2 == 1)
        printf("ScienceMode2::StartChannelListMode(): sent\n");
    	debug_sent_frame( Sendedaten, framelaenge_stuffed );
#endif
		sent[UPDATE]++;
	}
}

/*******************************************************************************
 \fn ScienceMode2::GetStimulationMode ()
 *
 * \brief get stimulator mode
 *
 * \param
 */
void ScienceMode2::GetStimulationMode(  )
{
    int chars_written;
    char Sendedaten[MAX_FRAME_LENGTH];
    char CRC = 0;
    unsigned int i, framelaenge_unstuffed = 8, framelaenge_stuffed = 0;
    unsigned int nutzdatenlaenge_unstuffed = 2, nutzdatenlaenge_stuffed = 0;

    Sendedaten[0] = 0xF0; // Startbyte
    Sendedaten[1] = 0x81; // Stuffingbyte
    Sendedaten[3] = 0x81; // Stuffingbyte
    Sendedaten[5] = getPaketnummer();
    Sendedaten[6] = 0x0a; // GetStimulationMode

    Sendedaten[7] = 0x0F; // Stopbyte

    nutzdatenlaenge_stuffed = stuffData(&Sendedaten[5], nutzdatenlaenge_unstuffed);

    Sendedaten[4] = (char)nutzdatenlaenge_stuffed ^ 0x55; // stuffed datalength

    CRC = CalcCRC8(&Sendedaten[5], nutzdatenlaenge_stuffed);

    framelaenge_stuffed = framelaenge_unstuffed + ( nutzdatenlaenge_stuffed - nutzdatenlaenge_unstuffed );

    Sendedaten[2] = CRC ^ 0x55; // stuffed checksum

    //SENDING
    chars_written = sPort->serial_sendstring( Sendedaten, framelaenge_stuffed );

    for (i = 0; i < framelaenge_stuffed; i++)
    {
        last_req_message_data[i] = Sendedaten[i] & 0xFF;
        last_req_message_length = framelaenge_stuffed;
    }

    if (chars_written < 0)
        perror("ScienceMode2::GetStimulationMode()");
    else if ((unsigned int)chars_written != framelaenge_stuffed)
        printf("ScienceMode2::GetStimulationMode(): GetStimulationMode not sent correctly\n");
    else
    {
#if (DEBUG_SCM2 == 1)
        printf("ScienceMode2::GetStimulationMode(): sent\n");
    	debug_sent_frame( Sendedaten, framelaenge_stuffed );
#endif
        sent[MODE]++;
	}
}

/*******************************************************************************
 \fn ScienceMode2::Send_Stop_Signal ()
 *
 * \brief stops stimulation
 *
 * \param
 */
void ScienceMode2::Send_Stop_Signal(  )
{
    int chars_written;
    char Sendedaten[MAX_FRAME_LENGTH], CRC = 0;
    unsigned int i, framelaenge_unstuffed = 8, framelaenge_stuffed = 0;
    unsigned int nutzdatenlaenge_unstuffed = 2, nutzdatenlaenge_stuffed = 0;

    Sendedaten[0] = 0xF0; // Startbyte
    Sendedaten[1] = 0x81; // Stuffingbyte
    Sendedaten[3] = 0x81; // Stuffingbyte
    Sendedaten[5] = getPaketnummer();
    Sendedaten[6] = 0x22; // StopChannelListMode
    Sendedaten[7] = 0x0F; // Stopbyte

    nutzdatenlaenge_stuffed = stuffData(&Sendedaten[5], nutzdatenlaenge_unstuffed);

    Sendedaten[4] = (char)nutzdatenlaenge_stuffed ^ 0x55; // stuffed datalength

    CRC = CalcCRC8(&Sendedaten[5], nutzdatenlaenge_stuffed);

    framelaenge_stuffed = framelaenge_unstuffed + ( nutzdatenlaenge_stuffed - nutzdatenlaenge_unstuffed );

    Sendedaten[2] = CRC ^ 0x55; // stuffed checksum

    //SENDING
    chars_written = sPort->serial_sendstring( Sendedaten, framelaenge_stuffed );

    for (i = 0; i < framelaenge_stuffed; i++)
    {
        last_req_message_data[i] = Sendedaten[i] & 0xFF;
        last_req_message_length = framelaenge_stuffed;
    }

    if (chars_written < 0)
        perror("ScienceMode2::Send_Stop_Signal()");
    else if ((unsigned int)chars_written == framelaenge_stuffed)
    {
        for( i = 0; i < STIM_CHANNELS; i++ )
            Kanalkonfiguration[i] = i; // Reset Indexkonfiguration (durch MakeIndex veraendert)

#if (DEBUG_SCM2 == 1)
        printf("ScienceMode2::Send_Stop_Signal(): sent\n");
    	debug_sent_frame( Sendedaten, framelaenge_stuffed );
#endif

        sent[STOP]++;
    }
    else
        printf("ScienceMode2::Send_Stop_Signal(): StopChannelListMode not sent correctly\n");
}

/*******************************************************************************
 \fn ScienceMode2::keepAlive ()
 *
 * \brief send keep alive command to stimulator
 *
 * \param
 */
void ScienceMode2::keepAlive(  )
{

    int chars_written;
    char Sendedaten[MAX_FRAME_LENGTH], CRC = 0;
    unsigned int i, framelaenge_unstuffed = 8, framelaenge_stuffed = 0;
    unsigned int nutzdatenlaenge_unstuffed = 2, nutzdatenlaenge_stuffed = 0;

    Sendedaten[0] = 0xF0; // Startbyte
    Sendedaten[1] = 0x81; // Stuffingbyte
    Sendedaten[3] = 0x81; // Stuffingbyte
    Sendedaten[5] = getPaketnummer();
    Sendedaten[6] = 0x04; // Watchdog
    Sendedaten[7] = 0x0F; // Stopbyte

    nutzdatenlaenge_stuffed = stuffData(&Sendedaten[5], nutzdatenlaenge_unstuffed);

    Sendedaten[4] = (char)nutzdatenlaenge_stuffed ^ 0x55; // stuffed datalength

    CRC = CalcCRC8(&Sendedaten[5], nutzdatenlaenge_stuffed);

    framelaenge_stuffed = framelaenge_unstuffed + ( nutzdatenlaenge_stuffed - nutzdatenlaenge_unstuffed );

    Sendedaten[2] = CRC ^ 0x55; // stuffed checksum

    //SENDING
    chars_written = sPort->serial_sendstring( Sendedaten, framelaenge_stuffed );

    for (i = 0; i < framelaenge_stuffed; i++)
    {
        last_req_message_data[i] = Sendedaten[i] & 0xFF;
        last_req_message_length = framelaenge_stuffed;
    }

    if (chars_written < 0)
        perror("ScienceMode2::keepAlive()");
    else if ((unsigned int)chars_written != framelaenge_stuffed)
        printf("ScienceMode2::keepAlive(): Watchdog not sent correctly\n");
    else
	{
#if (DEBUG_SCM2 == 1)
        printf("ScienceMode2::Watchdog(): sent\n");
    	debug_sent_frame( Sendedaten, framelaenge_stuffed );
#endif
        sent[ALIVE]++;
	}
}

/*******************************************************************************
 \fn ScienceMode2::Send_Init_Ack ()
 *
 * \brief send acknowledge for init request
 *
 * \param
 */
void ScienceMode2::Send_Init_Ack(  )
{

    int chars_written;
    char Sendedaten[MAX_FRAME_LENGTH], CRC = 0;
    unsigned int i, framelaenge_unstuffed = 9, framelaenge_stuffed = 0;
    unsigned int nutzdatenlaenge_unstuffed = 3, nutzdatenlaenge_stuffed = 0;


    Sendedaten[0] = 0xF0; // Startbyte
    Sendedaten[1] = 0x81; // Stuffingbyte
    Sendedaten[3] = 0x81; // Stuffingbyte
    Sendedaten[4] = 0x03 ^ 0x55; // stuffed datalength
    Sendedaten[5] = last_message_data[0]; // echo Paketnummer
    Sendedaten[6] = 0x02; // InitAck
    Sendedaten[7] = 0x00; // version accepted
    Sendedaten[8] = 0x0F; // Stopbyte

    nutzdatenlaenge_stuffed = stuffData(&Sendedaten[5], nutzdatenlaenge_unstuffed);

    Sendedaten[4] = (char)nutzdatenlaenge_stuffed ^ 0x55; // stuffed datalength

    CRC = CalcCRC8(&Sendedaten[5], nutzdatenlaenge_stuffed);

    framelaenge_stuffed = framelaenge_unstuffed + ( nutzdatenlaenge_stuffed - nutzdatenlaenge_unstuffed );

    Sendedaten[2] = CRC ^ 0x55; // stuffed checksum

    //SENDING
    chars_written = sPort->serial_sendstring( Sendedaten, framelaenge_stuffed );

    for (i = 0; i < framelaenge_stuffed; i++)
    {
        last_req_message_data[i] = Sendedaten[i] & 0xFF;
        last_req_message_length = framelaenge_stuffed;
    }

    if (chars_written < 0)
        perror("ScienceMode2::Send_Init_Ack()");
    else if ((unsigned int)chars_written != framelaenge_stuffed)
        printf("ScienceMode2::Send_Init_Ack(): InitAck not sent correctly\n");
    else
	{
#if (DEBUG_SCM2 == 1)
        printf("ScienceMode2::InitAck(): sent\n");
    	debug_sent_frame( Sendedaten, framelaenge_stuffed );
#endif
        sent[ACK]++;
	}
}

/*******************************************************************************
 \fn ScienceMode2::Send_Single_Pulse ()
 *
 * \brief send single pulse command to stimulator
 *
 * \param
 */
void ScienceMode2::Send_Single_Pulse( unsigned int Channel_Number,
                                      unsigned int Pulse_Width,
                                      unsigned int Pulse_Current )
{

    int chars_written;
    char Sendedaten[MAX_FRAME_LENGTH], CRC = 0;
    unsigned int i, framelaenge_unstuffed = 12, framelaenge_stuffed = 0;
    unsigned int nutzdatenlaenge_unstuffed = 6, nutzdatenlaenge_stuffed = 0;

    Sendedaten[0] = 0xF0; // Startbyte
    Sendedaten[1] = 0x81; // Stuffingbyte
    Sendedaten[3] = 0x81; // Stuffingbyte
    Sendedaten[5] = getPaketnummer();
    Sendedaten[6] = 0x24; // SinglePulse
    Sendedaten[7] = (char)Channel_Number; // Kanal
    Sendedaten[8] = (char)((Pulse_Width >> 8) & 0x01); // Pulsbreite MSB
    Sendedaten[9] = (char)(Pulse_Width & 0x0ff); // Pulsbreite LSB
    Sendedaten[10] = (char)Pulse_Current; // SinglePulse
    Sendedaten[11] = 0x0F; // Stopbyte

    nutzdatenlaenge_stuffed = stuffData(&Sendedaten[5], nutzdatenlaenge_unstuffed);

    Sendedaten[4] = (char)nutzdatenlaenge_stuffed ^ 0x55; // stuffed datalength

    CRC = CalcCRC8(&Sendedaten[5], nutzdatenlaenge_stuffed);

    framelaenge_stuffed = framelaenge_unstuffed + ( nutzdatenlaenge_stuffed - nutzdatenlaenge_unstuffed );

    Sendedaten[2] = CRC ^ 0x55; // stuffed checksum

    //SENDING
    chars_written = sPort->serial_sendstring( Sendedaten, framelaenge_stuffed );

    for (i = 0; i < framelaenge_stuffed; i++)
    {
        last_req_message_data[i] = Sendedaten[i] & 0xFF;
        last_req_message_length = framelaenge_stuffed;
    }

    if (chars_written < 0)
        perror("ScienceMode2::Send_Single_Pulse()");
    else if ((unsigned int)chars_written != framelaenge_stuffed)
        printf("ScienceMode2::Send_Single_Pulse(): SinglePulse not sent correctly\n");
    else
	{
#if (DEBUG_SCM2 == 1)
        printf("ScienceMode2::SinglePulse(): sent\n");
    	debug_sent_frame( Sendedaten, framelaenge_stuffed );
#endif
        sent[SINGLE]++;
	}
}

/*******************************************************************************
 \fn ScienceMode2::Make_Index ()
 *
 * \brief
 *
 * \param
 */
void ScienceMode2::Make_Index( double *S_Channel_Stim,
                               int stim_channels_used )
{
    int i,
        j,
        n;

    n = 0;

    // Beispiel:
    // S_Channel_Stim = {1; 3; 5; 2; 4; 7; 6; 8}
    // Schleifen : i     j      n
    //             1     0      0
    //             2     3      1
    //             3     1      2
    //             4     4      3
    //             5     2      4
    //             6     6      5
    //             7     5      6
    //             8     7      7
    // Kanalkonfiguration = {0; 3; 1; 4; 2; 6; 5; 7}
    // Kanal 1 an Index 0
    // Kanal 2 an Index 3
    // Kanal 3 an Index 1
    // Kanal 4 an Index 4
    // Kanal 5 an Index 2
    // Kanal 6 an Index 6
    // Kanal 7 an Index 5
    // Kanal 8 an Index 7

    for( i = 1; i <= STIM_CHANNELS; i++ )
        for( j = 0; j < stim_channels_used; j++ )
        {
            // S_Channel_Stim is an array containing the channels being used
            if( ( int )S_Channel_Stim[j] == i )
            {
                Kanalkonfiguration[n] = j;
                n++;
            }
        }
}

/*******************************************************************************
 \fn ScienceMode2::Encode_Mode ()
 *
 * \brief convert mode from double to integer
 *
 * \param
 */
unsigned int ScienceMode2::Encode_Mode( double Mode )
{

    unsigned int Int_Mode;

    if( Mode > 2 )
        Int_Mode = 2;

    else if( Mode < 0 )
        Int_Mode = 0;

    else
        Int_Mode = ( unsigned int )Mode;

    return Int_Mode & 0x03;

}

/*******************************************************************************
 \fn ScienceMode2::Encode_Pulse_Current ()
 *
 * \brief convert current from double to integer
 *
 * \param
 */
unsigned int ScienceMode2::Encode_Pulse_Current( double Current )
{

    unsigned int Int_Current = 0;

    if( Current > 127 )
        Int_Current = 127;

    else if( Current < 0 )
        Int_Current = 0;

    else
        Int_Current = ( unsigned int )Current;

    return Int_Current & 0x7F;

}

/*******************************************************************************
 \fn ScienceMode2::Encode_Pulse_Width ()
 *
 * \brief convert pulsewidth from double to integer
 *
 * \param
 */
unsigned int ScienceMode2::Encode_Pulse_Width( double Pulse_Width )
{

    unsigned int Int_Pulse_Width = 0;

    if( Pulse_Width > 500 )
        Int_Pulse_Width = 500;

    else if( Pulse_Width < 0 )
        Int_Pulse_Width = 0;

    else
        Int_Pulse_Width = ( unsigned int )Pulse_Width;

    return Int_Pulse_Width & 0x1FF;

}

/*******************************************************************************
 \fn ScienceMode2::Encode_Channel_Stim ()
 *
 * \brief
 *
 * \param
 */
unsigned int ScienceMode2::Encode_Channel_Stim( double *S_Channel_Stim,  /* array of nominal channels, 1..8 */
                                                int stim_channels_used   /* Number of channels used  */
     )
{
    int i;
    unsigned int Channel_Stim = 0;

    printf("stim_channels_used %u\n", stim_channels_used);

    for( i = 0; i < stim_channels_used; i++ )
    {
        printf("Channel_Stim[%i] %u\n", i, (unsigned int)S_Channel_Stim[i]);
        // S_Channel_Stim contains the electrode numbers of used channels
        // e.g 1 2 3 4 5 6 7 8 -> output: 00001111
        // or  1 2 5 6 0 0 0 0 -> output: 00110011
        // if stim_channels_used was set to 4
        if( powoftwo( (int)(S_Channel_Stim[i] - 1) ) >= 0 )
            Channel_Stim |= ( unsigned int )powoftwo( (int)(S_Channel_Stim[i] - 1) );
    }

    printf("Channel_Stim %u %02x\n", Channel_Stim & 0xFF, Channel_Stim & 0xFF);

    return Channel_Stim;
}

/*******************************************************************************
 \fn ScienceMode2::Encode_Channel_Lf ()
 *
 * \brief
 *
 * \param
 */
unsigned int ScienceMode2::Encode_Channel_Lf( double *S_Channel_Lf,  /* array of channel for low frequency  */
                                              int lf_channels_used   /* Number in the S_Channel_Lf vector */
     )
{
    int i;
    unsigned int Channel_Lf = 0;

    printf("lf_channels_used %u\n", lf_channels_used);

    for( i = 0; i < lf_channels_used; i++ )
    {
        printf("Channel_Lf[%i] %u\n", i, (unsigned int)S_Channel_Lf[i]);
        // S_Channel_Lf contains the electrode numbers of used channels
        // e.g 1 2 3 4 5 6 7 8 -> output: 00001111
        // or  1 2 5 6 0 0 0 0 -> output: 00110011
        // if lf_channels_used was set to 4
        if( powoftwo( (int)(S_Channel_Lf[i] - 1) ) >= 0 )
            Channel_Lf |= ( unsigned int )powoftwo( (int)(S_Channel_Lf[i] - 1) );
    }

    printf("Channel_Lf %u %02x\n", Channel_Lf & 0xFF, Channel_Lf & 0xFF);

    return Channel_Lf;
}

/*******************************************************************************
 \fn ScienceMode2::get_LastAnswerMessage ()
 *
 * \brief copy last reseived message to calling function through parameter
 *
 * \param
 */
void ScienceMode2::get_LastAnswerMessage( char *message_buffer,
                                          unsigned int *mbuf_len )
{
    unsigned int i = 0;

    for (i = 0; i < last_messagedata_length; i++)
    {
        *(message_buffer + i) = last_message_data[i];
    }

    *(mbuf_len) = last_messagedata_length;
}

/*******************************************************************************
 \fn ScienceMode2::get_LastRequestMessage ()
 *
 * \brief copy last sent message to calling function through parameter
 *
 * \param
 */
void ScienceMode2::get_LastRequestMessage( char *message_buffer,
                                           unsigned int *mbuf_len )
{
    unsigned int i = 0;

    for (i = 0; i < last_req_message_length; i++)
    {
        *(message_buffer + i) = last_req_message_data[i];
    }

    *(mbuf_len) = last_req_message_length;
}

/*******************************************************************************
 \fn ScienceMode2::getPossibleMode ()
 *
 * \brief delivers possible Pulse Mode calculated by given main- and group-times
 *
 * \param
 */
int ScienceMode2::getPossibleMode(  )
{
  return possible_Mode;
}

/*******************************************************************************
 \fn ScienceMode2::getInitialised ()
 *
 * \brief delivers true if stimulator has been initialised
 *
 * \param
 */
bool ScienceMode2::getInitialised(  )
{
  return isInitialised;
}

/*******************************************************************************
 \fn ScienceMode2::getConnected ()
 *
 * \brief delivers true if stimulator is connected
 *
 * \param
 */
bool ScienceMode2::getConnected(  )
{
  return isConnected;
}

/*******************************************************************************
 \fn ScienceMode2::getRunning ()
 *
 * \brief delivers true if stimulation is running
 *
 * \param
 */
bool ScienceMode2::getRunning(  )
{
  return isRunning;
}

/*******************************************************************************
 \fn ScienceMode2::getSent ()
 *
 * \brief delivers amount of sent commands
 *
 * \param
 */
int ScienceMode2::getSent( int type )
{
  return sent[type];
}

/*******************************************************************************
 \fn ScienceMode2::getAcks ()
 *
 * \brief delivers amount acknowledged commands
 *
 * \param
 */
int ScienceMode2::getAcks( int type )
{
  return acks[type];
}

/*******************************************************************************
 \fn ScienceMode2::getErrors ()
 *
 * \brief delivers amount of received errors
 *
 * \param
 */
int ScienceMode2::getErrors( int type )
{
  return errors[type];
}

/*******************************************************************************
 \fn ScienceMode2::getErrStat ()
 *
 * \brief delivers received StimulationError
 *
 * \param
 */
int ScienceMode2::getErrStat(  )
{
  return errstat;
}

/*******************************************************************************
 \fn bool ScienceMode2::getNewData ()
 *
 * \brief delivers if stim data has changed since last update command
 *
 * \param
 */
bool ScienceMode2::getNewData( double *Pulse_Width,
                               double *Pulse_Current,
                               double *Mode,
                               unsigned int stim_channels_used )

{
    unsigned int i = 0;

    isNewData = false;

    for( i = 0; i < stim_channels_used; i++ )
    {
        // Only send update, if stimulation data changed
        if ( ( Pulse_Width[i] != Pulse_Width_Alt[i] ) ||
             ( Pulse_Current[i] != Pulse_Current_Alt[i] ) ||
             ( Mode[i] != Mode_Alt[i] ) )
        {
            Pulse_Width_Alt[i] = Pulse_Width[i];
            Pulse_Current_Alt[i] = Pulse_Current[i];
            Mode_Alt[i] = Mode[i];
            isNewData = true;
        }
    }

    return isNewData;
}

#ifdef __unix || _TTY_POSIX_
/*******************************************************************************
 \fn ScienceMode2::delta_time()
 *
 * \brief delivers time delta to last call of function
 *
 */
unsigned long int ScienceMode2::delta_time( void )
{
    unsigned long int c_time = current_time();

    if (t == 0)
        t = c_time; // starttime

    delta_t = c_time - t;

    t = c_time;
    return (unsigned long int)(delta_t);
}

/*******************************************************************************
 \fn ScienceMode2::current_time()
 *
 * \brief deliver current time in microseconds
 *
 */
unsigned long int ScienceMode2::current_time( void )
{
    gettimeofday(&tv, nullptr);
    return (unsigned long int)(tv.tv_sec * 1000 + (tv.tv_usec / 1000));
}
#endif

/*******************************************************************************
 \fn ScienceMode2::powoftwo()
 *
 * \brief deliver given power of 2
 *
 */
int ScienceMode2::powoftwo( int power )
{
    int result = 1;

    if(power == 0)
    {
        return 1;
    }

    if(power == 1)
    {
        return 2;
    }
    
    // defined for 2¹.....2¹⁵
    if(power > 1 && power < 16)
    {
        for(int i = 1; i <= power; i++)
        {
            result = result * 2;
        }
    }
    else
    {
        printf("power out of range (0..15)\n");
        result = -1;
    }
    return result;
}

/*******************************************************************************
 \fn ScienceMode2::debug_sent_frame()
 *
 * \brief print sent frame
 *
 * \param
 */
void ScienceMode2::debug_sent_frame( char *frame,
                                     unsigned int frame_len )
{
    unsigned int i = 0;
#ifdef __unix || _TTY_POSIX
	printf("TIMER: %lu\n", current_time());
#endif
	printf("sent hex: ");

    for (i = 0; i < frame_len; i++)
    {
        printf( "%02x ", (unsigned char)*(frame + i) );
    }

	printf("\n");

	printf("sent uint: ");

    for (i = 0; i < frame_len; i++)
    {
        printf( "%u ", (unsigned char)*(frame + i) );
    }

	printf("\n");
}

/*******************************************************************************
 \fn ScienceMode2::debug_received_frame()
 *
 * \brief print received frame
 *
 * \param
 */
void ScienceMode2::debug_received_frame( char *frame,
                                         unsigned int frame_len )
{
    unsigned int i = 0;
#ifdef __unix || _TTY_POSIX
	printf("TIMER: %lu\n", current_time());
#endif
	printf("received hex: ");

    for (i = 0; i < frame_len; i++)
    {
        printf( "%02x ", (unsigned char)*(frame + i) );
    }

	printf("\n");

	printf("received uint: ");

    for (i = 0; i < frame_len; i++)
    {
        printf( "%u ", (unsigned char)*(frame + i) );
    }

	printf("\n");
}
