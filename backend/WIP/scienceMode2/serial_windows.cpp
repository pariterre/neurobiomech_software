/***************************************************************************
                          serial_windows.cpp  -  description
                             -------------------
    begin                :
    copyright            : (C) 2003 Max Planck Institute for Dynamics
                                    of Complex Dynamical Systems
    Author               :  Nils-Otto Negård
                            Thomas Schauer
                            Ralph Stephan

    email                : negaard@mpi-magdburg.mpg.de
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "serial_windows.h"


SerialPort::SerialPort()
{
}

SerialPort::~SerialPort()
{
}
/*-------------------------------------------------------------------------
  openserial()
  Opens COM and returns a handle
  ------------------------------------------------------------------------- */

int SerialPort::serial_openport(char *Portname)
{

	unsigned int dwError;

  this->hCom = CreateFile(Portname,
                    GENERIC_READ | GENERIC_WRITE,
                    0,    /* comm devices must be opened w/exclusive-access */
                    NULL, /* no security attrs */
                    OPEN_EXISTING, /* comm devices must use OPEN_EXISTING */
                    0,    /* if 1 overlapped I/O, ie asynchronous */
                    NULL  /* hTemplate must be NULL for comm devices */
                    );

  if (this->hCom == INVALID_HANDLE_VALUE)
  {
    dwError = GetLastError();
    printf("Error! Invalid Handle while opening port %s: ", Portname, dwError);
	return -1;
  }
  else
    return 0;

}


/*-------------------------------------------------------------------------
  serial_setupport()
  Set the input and output buffer size.
  Set the COM port for use with the stimulator.
  Fill in a DCB structure with the current configuration.
  The DCB structure is then modified and used to reconfigure the device.
  Set the time-out parameters.
  ------------------------------------------------------------------------- */
int SerialPort::serial_setupport(long baudrate, unsigned char handshake, unsigned char stopbits, unsigned char parity)
{
  DCB dcb;
  unsigned int dwError;
  BOOL fSuccess;
  COMMTIMEOUTS ct;
  unsigned int InQueue = 100;  // size of the input buffer (in bytes)
  unsigned int OutQueue = 100;  // size of the output buffer (in bytes)


  /* set up the input / output buffers */
  fSuccess = SetupComm(this->hCom, InQueue, OutQueue);

  if (!fSuccess)
  {
    dwError = GetLastError();
	printf("Error setup COM: ", dwError);
	return -1;
  }
  else
  {
	  /* Get the current configuration. */
	  fSuccess = GetCommState(this->hCom, &dcb);

	  if (!fSuccess)
	  {
		dwError = GetLastError();
		printf("Error getting COM state: ", dwError);
		return -1;
	  }
	  else
	  {

		  /* Fill the DCB */
		  switch ( baudrate )
		  {
			case 460800:
    			dcb.BaudRate = 460800;
			break;

			case 115200:
				dcb.BaudRate = 115200;
			break;
			
			case 57600:
				dcb.BaudRate = 57600;
			break;			

			case 38400:
				dcb.BaudRate = 38400;
			break;

			case 19200:
				dcb.BaudRate = 19200;
			break;

			case 9600:
				dcb.BaudRate = 9600;
			break;

			case 4800:
				dcb.BaudRate = 4800;
			break;

			default:
    			dcb.BaudRate = 57600;
			break;
		  }  //end of switch baud_rate

		  dcb.fBinary = FALSE;           // binary mode, no EOF check

		  if ( parity > 0 )
  			dcb.fParity = TRUE;          // enable parity checking
		  else
  			dcb.fParity = FALSE;         // disable parity checking

		  dcb.fOutxCtsFlow = FALSE;      // CTS output flow control off
		  dcb.fOutxDsrFlow = FALSE;      // DSR output flow control
		  dcb.fDtrControl = DTR_CONTROL_DISABLE; // DTR flow control type
		  dcb.fDsrSensitivity = FALSE;   // DSR sensitivity
		  dcb.fTXContinueOnXoff = FALSE; // XOFF continues Tx
		  dcb.fOutX = FALSE;             // XON/XOFF out flow control
		  dcb.fInX = FALSE;              // XON/XOFF in flow control

		  if ( handshake == 1 )
			dcb.fRtsControl = RTS_CONTROL_HANDSHAKE; // RTS flow control
		  else
  			dcb.fRtsControl = RTS_CONTROL_DISABLE; // RTS flow control

		  dcb.ByteSize = 8;              // number of bits/byte, 4-8

		  dcb.Parity = parity;         // 0-4=no,odd,even,mark,space
		  dcb.StopBits = stopbits;     // 0,1,2 = 1, 1.5, 2

		  /* Set the new DCB configuration */
		  fSuccess = SetCommState(this->hCom, &dcb);

		  if (!fSuccess)
		  {
			dwError = GetLastError();
		    printf("Error setting COM state: ", dwError);
			return -1;
		  }
		  else
		  {

			  /* Get the current time-outs. */
			  fSuccess = GetCommTimeouts(this->hCom, &ct);

			  if (!fSuccess)
			  {
				dwError = GetLastError();
				printf("Error setting COM state: ", dwError);
				return -1;
			  }
			  else
			  {

				  /* Fill in the COMMTIMEOUTS: Disable all of them by setting them to 0 */
				  /* setting all to 0 means nonblocking mode */
				  ct.ReadIntervalTimeout = MAXDWORD;
				  ct.ReadTotalTimeoutMultiplier = 0; /*old value was 10*/
				  ct.ReadTotalTimeoutConstant = 0; /*old value was 10*/
				  ct.WriteTotalTimeoutMultiplier = 0;
				  ct.WriteTotalTimeoutConstant = 0;
				  fSuccess = SetCommTimeouts(this->hCom, &ct);

				  if (!fSuccess)
				  {
					dwError = GetLastError();
					printf("Error setting COM time-outs: ", dwError);
					return -1;
				  }
				  else
					  return 0;
			  }
		  }
    }
  }
}


/*-------------------------------------------------------------------------
  closeserial()
  close the serial port
  ------------------------------------------------------------------------- */
int SerialPort::serial_closeport()
{
  unsigned int dwError;
  BOOL fSuccess;

  fSuccess = CloseHandle(this->hCom);

  if (!fSuccess)
  {
    dwError = GetLastError();
    printf("Error closing COM: ", dwError);
    return -1;
  }
  else
	return 0;
}

/*-------------------------------------------------------------------------
  sendstring()
  send a string to the serial port
  ------------------------------------------------------------------------- */
int SerialPort::serial_sendstring(	char *buffer,   //pointer to the string
                					unsigned int nb        //number of bytes in the string
                				 )
{
  DWORD dwBytesWritten, dwError;
  BOOL fSuccess;

  fSuccess =  WriteFile(this->hCom, (LPSTR) buffer, nb, &dwBytesWritten, NULL);

  if (!fSuccess)
  {
	dwError = GetLastError();
	printf("Error writing to COM: ", dwError);
	return -1;
  }
  else
  	return (int)dwBytesWritten;
}

/*-------------------------------------------------------------------------
  readstring()
  reads a string from the serial port
  ------------------------------------------------------------------------- */
int SerialPort::serial_readstring(
                char *buffer,   //pointer to the string
                unsigned int nb        //number of bytes in the string
                )
{
  DWORD dwBytesRead, dwError;
  BOOL fSuccess;

  fSuccess = ReadFile(this->hCom, (LPVOID ) buffer, nb, &dwBytesRead, NULL);

  if (!fSuccess)
  {
    dwError = GetLastError();
    printf("Error reading from COM: ", dwError);
    return -1;
  }
  else
	return (int)dwBytesRead;
}
