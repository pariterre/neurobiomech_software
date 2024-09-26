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
 * ScienceMode2.h
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

#ifndef SCIENCEMODE2_H
#define SCIENCEMODE2_H

#define DEBUG_SCM2 0

#ifndef STIM_CHANNELS
#  define STIM_CHANNELS 8
#endif

#define SINGLES_ALLOWED 0
#define DOUBLETS_ALLOWED 1
#define TRIPLETS_ALLOWED 2

#include "serialport.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#ifdef __unix || _TTY_POSIX_
#include <sys/time.h>
#endif

class ScienceMode2
{
  public:
    /*******************************************************************************
     \fn ScienceMode2::ScienceMode2 ()
     *
     * \brief standard constructor
     *
     * \param
     */
    ScienceMode2(  );

    /*******************************************************************************
     \fn ScienceMode2::~ScienceMode2 ()
     *
     * \brief standard destructor
     *
     * \param
     */
    virtual ~ ScienceMode2(  );

    //ChargeControl *CControl;

    static const int BUF_SIZE = 65536;
    static const int MIN_FRAME_LENGTH = 8; // min framelength (no databyte and nothing stuffed)
    static const int MAX_FRAME_LENGTH = 130; // max framelength (with all databytes stuffed)

    /*******************************************************************************
     \fn void ScienceMode2::setDeviceName ( char * port)
     *
     * \brief sets the devicename for serial communication as a class attribute
     *
     * \param
     */
    void setDeviceName( char * devicename );

    /*******************************************************************************
     \fn int ScienceMode2::Open_serial ()
     *
     * \brief open serial communication port named by class attribute devicename
     *
     * \param
     */
    int Open_serial(  );

    /*******************************************************************************
     \fn ScienceMode2::Close_serial ()
     *
     * \brief
     *
     * \param
     */
    int Close_serial(  );

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
    bool getFrame(  );
    bool getInitialised(  );
    bool getConnected(  );
    bool getRunning(  );

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
    int getData(  );

    /*******************************************************************************
     \fn ScienceMode2::getFrameBufLen()
     *
     * \brief delivers the current framebuflen
     *
     * \param
     */
    int getFrameBufLen(  );

    /*******************************************************************************
     \fn ScienceMode2::talk ()
     *
     * \brief answers received commands
     *
     * \param
     */
    void talk(  );

    int getSent( int type );
    int getAcks( int type );
    int getErrors( int type );
    int getErrStat();

    /*******************************************************************************
     \fn bool ScienceMode2::getNewData ()
     *
     * \brief delivers if stim data has changed since last update command
     *
     * \param
     */
    bool getNewData( double *Pulse_Width, double *Pulse_Current,
                     double *Mode, unsigned int stim_channels_used );

    void keepAlive(  );
    int getPossibleMode(  );
    void get_LastAnswerMessage(char *message_buffer, unsigned int *mbuf_len);
    void get_LastRequestMessage(char *message_buffer, unsigned int *mbuf_len);

    /*******************************************************************************
    \fn int ScienceMode2::Send_Init_Param(unsigned int channels_used, // Number of channels used
                                    double *S_Channel_Stim, // List of channels
                                    unsigned int n_lf,// Number in Channel_Lf
                                    double *S_Channel_Lf, //
                                    double  S_Main_Time,
                                    double  S_Group_Time,
                                    double  S_N_Factor)

    \param channels_used Numbers of channels in the S_Channel_Stim
    \param S_Channel_Stim The channel list as a vector e.g. 1 2 5 6 - - - -
    \param n_lf Numbers of channels in the S_Channel_Lf
    \param S_Channel_Lf The Channel_Lf list as a vector
    \param S_Main_Time The Main Time in the real values
    \param S_Group_Time The Group Time in real value
    \param S_N_Factor The N_Factor setting the frequency of the channels in the
    S_Channel_Lf

    this function differs from the int ScienceMode2::Send_Init_Param(unsigned int channels_used,
                                    unsigned int Channel_Stim,
                                    unsigned int Channel_Lf,
                                    unsigned int Main_Time,
                                    unsigned int Group_Time,
                                    unsigned int N_Factor)
    in that the parameters are all in real values, and the vectors S_Channel_Stim and S_Channel_Lf are real vectors.
    Make sure when the parameters are updated the pulse width and current have to appear in the same order as the channels in the
    S_Channel_Stim vector.


    Error code:
    \verbatim
    Error                  Explanation
    ------------------    ----------------------------------------------------
    0                      No error has occurred
    1                      Channel_Stim == 0, no channels in the channel list
    2                      Group Time > Main_Time Initialisation unsuccessfully
    3                      did not receive any byte from ScienceMode2
    4                      did not receive confirmation from ScienceMode2
    5                      Group Time < channels_used * 1.5

    \endverbatim
    */
    void Send_Init_Param( unsigned int stim_channels_used,
                         double *S_Channel_Stim,
                         unsigned int lf_channels_used,
                         double *S_Channel_Lf,
                         double S_Main_Time,
                         double S_Group_Time,
                         double S_N_Factor );

    /*******************************************************************************
    \fn int ScienceMode2::InitChannelListMode(unsigned int channels_used,
                                    unsigned int Channel_Stim,
                                    unsigned int Channel_Lf,
                                    unsigned int Main_Time,
                                    unsigned int Group_Time,
                                    unsigned int N_Factor)

    \param channels_used Number of channels in the channel list
    \param Channel_Stim The channel list coded in bits. LSB is channel 1, bit 7 is channel 8.
    \param Channel_Lf The low frequency channel list. Then channels in this list will be
    stimulated with a lower frequency.
    \param Main_Time This time is the time between the main pulses. The coding is as
    following 1.0 + 0.5* Main_Time.
    \param Group_Time This time is the time in between the doublets and triplets. The coding is as following:1.5 +0.5*Group_Time.
    \param N_Factor The N_Factor is deciding the frequency of the channels in the Channel_Lf list.

    This function codes the information and sends this information to the ScienceMode2 to initialise the
    "Channel List Mode". Before opening the channel list mode, the serial port must be opened
    by using Open_serial(char *Portname). Channel_Stim is the list of channels to be set up in the
    channel list. The channels are coded in the bits starting with LSB is channel 1 to MSB is channel 8. The corresponding channels
    in Channel_Lf will be stimulated with a lower frequency defined by N_Factor.

    Returns code:
    \verbatim
    Error                  Explanation
    ------------------    ----------------------------------------------------
    0                      No error has occurred. ScienceMode2 successfully initialised
    1                      Channel_Stim==0, no channels in the channel list
    2                      Group Time > Main_Time.
    3                      ScienceMode2 is not responding. Check your cable
    4                      The ScienceMode2 returns that initialisation is unsuccessfully.
    5                      Group time is less than channels_used * 1.5.

    \endverbatim
    */
    void InitChannelListMode( unsigned int Channel_Stim,
                         unsigned int Channel_Lf,
                         unsigned int Main_Time,
                         unsigned int Group_Time,
                         unsigned int N_Factor );

    /*******************************************************************************
    \fn int ScienceMode2::Send_Update_Parameter(double *S_Pulse_Width,
                                          double *S_Pulse_Current,
                                          double *S_Mode,
                                          unsigned int channesl_used)

    \param S_Pulse_Width Vector of Pulse Width
    \param S_Pulse_Current Vector of Current
    \param S_Mode 0 single pulse, 1 double pulse ,2 triple pulse.
    \param channels_used  Number of channels used

    This function sends information to the ScienceMode2 to update the
    stimulation parameters set up in "Channel List Mode". Pulse_Width and
    Pulse_Current and Mode are vector with the length of number of
    channels set up in the initialisation. The vectors S_Pulse_With, S_Pulse_Current and
    S_Mode must appear in the same order as in the S_Channel_Stim in the initialisation done by
    int ScienceMode2::Send_Init_Param(unsigned int channels_used, // Number of channels used
                                    double *S_Channel_Stim, // List of channels
                                    unsigned int n_lf,// Number in Channel_Lf
                                    double *S_Channel_Lf, //
                                    double  S_Main_Time,
                                    double  S_Group_Time,
                                    double  S_N_Factor)

    Returns code:
    \verbatim
    Error                  Explanation
    ------------------    ----------------------------------------------------
    0                      No error has occurred
    1                      Motionstim not initialised
    \endverbatim
    */
    void Send_Update_Parameter( double *S_Pulse_Width,
                               double *S_Pulse_Current,
                               double *S_Mode,
                               unsigned int stim_channels_used );

    /*******************************************************************************
    \fn int ScienceMode2::StartChannelListMode(unsigned int *Pulse_Width,
                                          unsigned int *Pulse_Current,
                                          unsigned int *Mode,
                                          unsigned int channels_used)

    \param Pulse_Width Vector of Pulse Width
    \param Pulse_Current Vector of Current
    \param Mode 0 single pulse, 1 double pulse ,2 triple pulse.
    \param channels_used Number of channels used

    This function sends information to the ScienceMode2 to update the
    stimulation parameters set up in "Channel List Mode". Pulse_Width and
    Pulse_Current and Mode are vector with the length of number of
    channels set up in the initialisation. The vectors are so arranged, so that
    the first entry in the vectors correspond with the channel with the lowest channel
    number.

    Returns code:
    \verbatim
    Error                  Explanation
    ------------------    ----------------------------------------------------
    0                      No error has occurred
    1                      ScienceMode2 not initialised
    2                      Number of channel is not the same as initialised.
    \endverbatim
    */
    void StartChannelListMode( unsigned int *Pulse_Width,
                               unsigned int *Pulse_Current,
                               unsigned int *Mode,
                               unsigned int stim_channels_used );

    /*******************************************************************************
    \fn int ScienceMode2::Send_Single_Pulse(unsigned int Channel_Number,
                                          unsigned int Pulse_Width,
                                          unsigned int Pulse_Current)

    \brief This function sends a single pulse.

    \param Channel_Number The number of the channel to be activated
    \param Pulse_Width Pulse Width
    \param Pulse_Current Current

    Error code:
    \verbatim
    Error                  Explanation
    ------------------    ----------------------------------------------------
    0                      No error has occurred
    1                      ScienceMode2 not initialised

    \endverbatim
    */
    void Send_Single_Pulse( unsigned int Channel_Number,
                           unsigned int Pulse_Width,
                           unsigned int Pulse_Current );

    /*******************************************************************************
    \fn int ScienceMode2::Send_Stop_Signal()

    \brief Send stop signal to a stimulator using sciencemode2-protocol
    */
    void Send_Stop_Signal(  );

    /*******************************************************************************
    \fn int ScienceMode2::getPaketnummer(  )

    \brief get stimulator mode
    */
    void GetStimulationMode( );

  protected:

  private:

    unsigned char crctable[256];

    SerialPort *sPort;

    /*******************************************************************************
    \fn void ScienceMode2::Make_Index(double *S_Channel_Stim, int channels_used)

    \brief

    \param
    \param

    */
    void Make_Index( double *S_Channel_Stim,
                     int stim_channels_used );

    /*******************************************************************************
    \fn unsigned int ScienceMode2::Encode_Mode(double Mode)

    \brief saturates mode to its max or min and casts from double to uint

    \param mode

    */
    unsigned int Encode_Mode( double S_Mode );

    /*******************************************************************************
    \fn unsigned int ScienceMode2::Encode_Pulse_Current(double Current)

    \brief saturates pulsewidth to its max or min and casts from double to uint

    \param current

    */
    unsigned int Encode_Pulse_Current( double Current );

    /*******************************************************************************
    \fn unsigned int ScienceMode2::Encode_Pulse_Width(double Pulse_Width)

    \brief saturates pulsewidth to its max or min and casts from double to uint

    \param pulsewidth

    */
    unsigned int Encode_Pulse_Width( double Pulse_Width );

    /*******************************************************************************
    \fn unsigned int ScienceMode2::Encode_Channel_Stim(double *S_Channel_Stim,
                                                       int stim_channels_used)

    \brief converts used stim channels to a bitpattern

    \param Pointer to array of stimulation channels activated, electrode number
    \param Number of channels used


    */
    unsigned int Encode_Channel_Stim( double *S_Channel_Stim,
                                      int stim_channels_used );

    /*******************************************************************************
    \fn unsigned int ScienceMode2::Encode_Channel_Lf(double *S_Channel_Lf,
                                                     int lf_channels_used)

    \brief converts used low frequency channels to a bitpattern

    \param Pointer to array of low frequency channels activated, electrode number
    \param Number of channels used
    */
    unsigned int Encode_Channel_Lf( double *S_Channel_Lf,
                                    int lf_channels_used );

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
    unsigned char CalcCRC8( const void *data, unsigned int length );

    /*******************************************************************************
    \fn int ScienceMode2::stuffData( const void *data, int length )

    \brief stuffes data to new sendbuffer

    \param Pointer to array of data
    \param length of data array
    */
    int stuffData( void *data, unsigned int length );

    /*******************************************************************************
    \fn unsigned char ScienceMode2::getPaketnummer(  )

    \brief manages Paketnummer


    */
    unsigned char getPaketnummer(  );

    void Send_Init_Ack(  );

    int powoftwo( int power );

	void debug_sent_frame( char * frame, unsigned int frame_len );

	void debug_received_frame( char * frame, unsigned int frame_len );

    void talk(void * datenpuffer);

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
    void adjustBuffer(int);

    int Kanalkonfiguration[STIM_CHANNELS];  //for use with simulink

    char   *devicename;

    char    framebuf[BUF_SIZE];
    int 	framebuflen;
    char	tmpbuf[BUF_SIZE];
    int     tmpbuflen;
    char          last_message_data[MAX_FRAME_LENGTH];
    unsigned int  last_messagedata_length;
    char          last_req_message_data[MAX_FRAME_LENGTH];
    unsigned int  last_req_message_length;

    int     paketnummer, last_sent_pkg, last_received_pkg;

    enum CMD_STATES { INIT = 0, UPDATE = 1, ALIVE = 2, ACK = 3, MODE = 4, STOP = 5, SINGLE = 6 };

    int     errors[7], errstat, sent[7], acks[7];

    int	    possible_Mode;

    // Modi der Stimulator-Zustandsmaschine (GetStimulationMode)
    bool    isInitialised, isConnected, isRunning, isNewData;

    double  Pulse_Width_Alt[STIM_CHANNELS], Pulse_Current_Alt[STIM_CHANNELS], Mode_Alt[STIM_CHANNELS];

#ifdef __unix || _TTY_POSIX_        
    timeval 	tv;
    unsigned long int current_time(void);
    unsigned long int delta_time(void);
    unsigned long int t, delta_t;
#endif    
};

#endif
