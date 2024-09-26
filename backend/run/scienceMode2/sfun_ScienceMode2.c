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
 * sfun_ScienceMode2.c
 *
 *  Created on: 17.08.2012
 *       eMail: mendicus@gmx.de
 *      Author: Ralph Stephan
 *
 */

#define S_FUNCTION_NAME sfun_ScienceMode2
#define S_FUNCTION_LEVEL 2
#include "simstruc.h"
#ifdef __unix || _TTY_POSIX_
	#include <unistd.h>
#else
    #include <windows.h>
#endif
#include "WrapScienceMode2.h"
	/*#include "C:/MinGW/include/unistd.h"*/
	/*#include "unistd.h"*/
	/*#include "C:/MinGW/include/windows.h"*/
	/*#include <windows.h>*/
	/*#include "C:/MinGW/include/mmsystem.h"*/
	/*#include <mmsystem.h>*/
	/*#define usleep(x) Sleep((x)/1000)*/

/*
 * mdlInitializeSizes - initialize the sizes array
 */
static void mdlInitializeSizes(SimStruct *S)
{

  /* See sfuntmpl_doc.c for more details on the macros below */

  ssSetNumSFcnParams(S, 8);  /* Number of expected parameters */
  if (ssGetNumSFcnParams(S) != ssGetSFcnParamsCount(S)) {
    /* Return if number of expected != number of actual parameters */
    return;
  }

  ssSetNumContStates(S, 0);
  ssSetNumDiscStates(S, 0);

  if (!ssSetNumInputPorts(S, 3)) return; /* three inputs */
  ssSetInputPortDimensionInfo(S, 0, DYNAMIC_DIMENSION);  /* current */
  ssSetInputPortRequiredContiguous(S, 0, true); /*direct input signal access*/
  ssSetInputPortDimensionInfo(S, 1, DYNAMIC_DIMENSION);  /* pulsewidth */
  ssSetInputPortRequiredContiguous(S, 1, true); /*direct input signal access*/
  ssSetInputPortDimensionInfo(S, 2, DYNAMIC_DIMENSION);  /* pulsewidth */
  ssSetInputPortRequiredContiguous(S, 2, true); /*direct input signal access*/

  /*
   * Set direct feedthrough flag (1=yes, 0=no).
   * A port has direct feedthrough if the input is used in either
   * the mdlOutputs or mdlGetTimeOfNextVarHit functions.
   * See matlabroot/simulink/src/sfuntmpl_directfeed.txt.
   */
  ssSetInputPortDirectFeedThrough(S, 0, 1);
  ssSetInputPortDirectFeedThrough(S, 1, 1);
  ssSetInputPortDirectFeedThrough(S, 2, 1);
  
  if (!ssSetNumOutputPorts(S, 1)) return;
  /*ssSetOutputPortDimensionInfo(S, 0, 1);*/
  ssSetOutputPortWidth(S, 0, 1);

  ssSetNumSampleTimes(S, 1);
  ssSetNumRWork(S, 0);
  ssSetNumIWork(S, 0);
  ssSetNumPWork(S, 1);
  ssSetNumModes(S, 0);
  ssSetNumNonsampledZCs(S, 0);
  ssSetOptions(S, 0);

}

/*
 * mdlInitializeSampleTimes - indicate that this S-function runs
 * at the rate of the source (driving block)
 */
static void mdlInitializeSampleTimes(SimStruct *S)
{
  ssSetSampleTime(S, 0, mxGetScalar(ssGetSFcnParam(S, 0))); /* USED FOR TESTING  */
  /*ssSetSampleTime(S, 0, INHERITED_SAMPLE_TIME);  */
  ssSetOffsetTime(S, 0, 0.0);
} 

enum CMD_STATES { INIT = 0, UPDATE = 1, ALIVE = 2, ACK = 3, MODE = 4, STOP = 5, SINGLE = 6 };

#define MDL_START  /* Change to #undef to remove function */
#if defined(MDL_START) 
/* Function: mdlStart =======================================================
 * Abstract:
 *    This function is called once at start of model execution. If you
 *    have states that should be initialized once, this is the place
 *    to do it.
 */
/*
 * mdlInitializeSampleTimes - indicate that this S-function runs
 * at the rate of the source (driving block)
 */
static void mdlStart(SimStruct *S)
{   
    void **PWork = ssGetPWork(S);
    void * pWrapperScienceMode2;
    
    int result = 0,
	i = 0,
	timer = 0,
	iparCount = 0,
	framebuflen = 0,
	ErrStat = 0,
	errors = 0,
	sent = 0,
	acks = 0,
	numberofchannels = 0,
	numberoflfchannels = 0,
	comportindex = 0;
    
    char * devicename;
    
    double S_Channel_Stim[8],
	   S_Channel_Lf[8],
	   S_N_Factor = 0,
	   S_Main_Time = 0,
	   S_Group_Time = 0;
    
    bool isConnected = false,
         isInitialised = false,
         isRunning = false,
	 isValid = false;    
    
    SCM2_Create_Object( &pWrapperScienceMode2 );
    /*ssPrintf( "mdlStart: reference-address %lu\n", pWrapperScienceMode2 );*/
    
    if (ssGetSimMode(S) == SS_SIMMODE_NORMAL)
    {
	    
        comportindex = mxGetScalar( ssGetSFcnParam(S, 1) );

        switch( comportindex )
        {
            case 1:
                devicename = "/dev/sciencemode2";
            break;

            case 2:
                devicename = "/dev/ttyUSB0";
            break;

            case 3:
                devicename = "/dev/ttyUSB1";
            break;

            case 4:
                devicename = "/dev/ttyUSB2";
            break;

            case 5:
                devicename = "/dev/ttyUSB3";
            break;

            case 6:
                devicename = "COM1";
            break;
	    
            case 7:
                devicename = "COM2";
            break;
	    
            case 8:
                devicename = "COM3";
            break;
	    
            case 9:
                devicename = "COM4";
            break;	    

            case 10:
                devicename = "COM5";
            break;
	    
            case 11:
                devicename = "COM6";
            break;
	    
            case 12:
                devicename = "COM7";
            break;
	    
            case 13:
                devicename = "COM8";
            break;            
            
            default:
                ssPrintf("The port you've chosen is not allowed in this version\n");
            break;
        }
	
        ssPrintf("Schnittstelle %s\n", devicename);
        
        SCM2_SetDeviceName( pWrapperScienceMode2, devicename );
        SCM2_OpenSerial( pWrapperScienceMode2, &result );	

        if (result >= 0)
        {
            /* Parameter aus der Modellblockdefinition holen */

            /* Parametervektor (integer) */
            /* Laenge des Channel Vektors */
            numberofchannels = mxGetNumberOfElements( ssGetSFcnParam(S, 2) );

            /* Channelvektor (integer) */
            for (i = 0; i < numberofchannels; i++)
                S_Channel_Stim[i] = (double) *(mxGetPr( ssGetSFcnParam(S, 2) ) + i);

            /* Parametervektor (integer) */
            numberoflfchannels = mxGetNumberOfElements( ssGetSFcnParam(S, 6) );

            /* Lf_Channelvektor (integer) */
            for (i = 0; i < numberoflfchannels; i++)		
                S_Channel_Lf[i] = (double) *(mxGetPr( ssGetSFcnParam(S, 6) ) + i);

            /* Lf Faktor (integer) */
            S_N_Factor = (double)mxGetScalar( ssGetSFcnParam(S, 7) );

            /* Maintime (real) */
            S_Main_Time = (double)mxGetScalar( ssGetSFcnParam(S, 3) );

            /* Grouptime (real) */
            S_Group_Time = (double)mxGetScalar( ssGetSFcnParam(S, 4) );

            /* Solange die Stimulation noch nicht bereits initialisiert worden ist, */
            /* der Stimulator aber verbunden ist */
            while ( !isInitialised )
            {

                SCM2_GetStimulationMode( pWrapperScienceMode2 );

                /* Daten von der seriellen Schnittstelle (USB) holen */
                SCM2_GetData( pWrapperScienceMode2, &framebuflen );

                while (framebuflen > 0)
                {
                    /* Den aktuellen Frame im Puffer auswaehlen */
                    /* Die soeben von der Schnittstelle geholten Daten auf gueltige Frames */
                    /* ueberpruefen */				
                    SCM2_GetFrame( pWrapperScienceMode2, &isValid );
                    /* Jeder gueltige Frame wird aus dem Puffer entfernt */
                    /* Somit verringert sich die Pufferlaenge um die jeweilige Framelaenge */
                    SCM2_GetFrameBufLen( pWrapperScienceMode2, &framebuflen );

                    if ( isValid )
                        /* Zustandsmaschine aufrufen um auf Stimulatorantwort zu reagieren. */
                        /* Der aktuelle Frame wird beantwortet bzw. dessen Daten ausgewertet */	
                        SCM2_Talk( pWrapperScienceMode2 );
                }



                /* Solange keine Stimulation laeuft und die Stimulation noch nicht bereits initialisiert worden ist, */
                /* der Stimulator aber verbunden ist */
                SCM2_GetConnected( pWrapperScienceMode2, &isConnected );
                SCM2_GetInitialised( pWrapperScienceMode2, &isInitialised );
                SCM2_GetRunning( pWrapperScienceMode2, &isRunning );

                if ( isConnected && !isInitialised && !isRunning )
                {

                    SCM2_Send_Init_Param( pWrapperScienceMode2,
                                numberofchannels,
                                S_Channel_Stim,
                                numberoflfchannels,
                                S_Channel_Lf,
                                S_Main_Time,
                                S_Group_Time,
                                S_N_Factor );
                }

                SCM2_GetErrStat( pWrapperScienceMode2, &ErrStat );

                SCM2_GetErrors( pWrapperScienceMode2, (int)INIT, &errors );
                SCM2_GetSent( pWrapperScienceMode2, (int)INIT, &sent );
                SCM2_GetAcks( pWrapperScienceMode2, (int)INIT, &acks );    

                ssPrintf("mdlStart: InitsReceived %i, SentInitAcks %i, ErrorsReceived %i, ErrorStatus %i\n", acks, sent, errors, ErrStat);    

                SCM2_GetErrors( pWrapperScienceMode2, (int)MODE, &errors );
                SCM2_GetSent( pWrapperScienceMode2, (int)MODE, &sent );
                SCM2_GetAcks( pWrapperScienceMode2, (int)MODE, &acks );    

                ssPrintf("mdlStart: SentGetModeCommands %i, ACKsReceived %i, ErrorsReceived %i, ErrorStatus %i\n", sent, acks, errors, ErrStat);
#ifdef __unix || _TTY_POSIX_
                usleep(50000);
#else
                Sleep(50);
#endif
            }
        }
		else
		{
			ssPrintf("Schnittstelle %s konnte nicht geoeffnet werden!!!\n", devicename);
		}
    }

    PWork[0] = pWrapperScienceMode2;
} 
#endif /*  MDL_START */

/*
 * mdlOutputs - compute the outputs by calling my_alg, which
 * resides in another module, my_alg.c
 */
static void mdlOutputs(SimStruct *S, int_T tid)
{
    void * pWrapperScienceMode2;

    double *S_Pulse_Width,
 	   *S_Pulse_Current,
	   *S_Mode,
	   S_Channel_Stim[8],
	   S_Channel_Lf[8],
	   S_N_Factor = 0,
	   S_Main_Time = 0,
	   S_Group_Time = 0;
	   
    int numberofchannels = 0,
        numberoflfchannels = 0,
	i = 0,
	iparCount = 0,
	framebuflen = 0,
	ErrStat = 0,
	errors = 0,
	sent = 0,
	acks = 0,
	allowed_max_mode = 0;

    bool isConnected = false,
         isInitialised = false,
         isRunning = false,
	 isValid = false,
	 isNewData = false;

    real_T *mute_period = ssGetOutputPortRealSignal(S, 0); 
   
    pWrapperScienceMode2 = (void *) ssGetPWorkValue(S, 0);

	/* Parameter aus der Modellblockdefinition holen */

	/* Parametervektor (integer) */
	/* Laenge des Channel Vektors */
	numberofchannels = mxGetNumberOfElements( ssGetSFcnParam(S, 2) );

	/* Channelvektor (integer) */
	for (i = 0; i < numberofchannels; i++)
		S_Channel_Stim[i] = (double) *(mxGetPr( ssGetSFcnParam(S, 2) ) + i);

	/* Parametervektor (integer) */
	numberoflfchannels = mxGetNumberOfElements( ssGetSFcnParam(S, 6) );

	/* Lf_Channelvektor (integer) */
	for (i = 0; i < numberoflfchannels; i++)		
		S_Channel_Lf[i] = (double) *(mxGetPr( ssGetSFcnParam(S, 6) ) + i);

	/* Lf Faktor (integer) */
	S_N_Factor = (double)mxGetScalar( ssGetSFcnParam(S, 7) );

	/* Maintime (real) */
	S_Main_Time = (double)mxGetScalar( ssGetSFcnParam(S, 3) );

	/* Grouptime (real) */
	S_Group_Time = (double)mxGetScalar( ssGetSFcnParam(S, 4) );

    /*SCM2_GetStimulationMode( pWrapperScienceMode2 );*/

    /* Daten von der seriellen Schnittstelle (USB) holen */
    SCM2_GetData( pWrapperScienceMode2, &framebuflen );

    while ( framebuflen > 0 )    
    {
        /* Den aktuellen Frame im Puffer auswaehlen */
        /* Die soeben von der Schnittstelle geholten Daten auf gueltige Frames */
        /* ueberpruefen */
        SCM2_GetFrame( pWrapperScienceMode2, &isValid );

        /* Jeder gueltige Frame wird aus dem Puffer entfernt */
        /* Somit verringert sich die Pufferlaenge um die jeweilige Framelaenge */	
        SCM2_GetFrameBufLen( pWrapperScienceMode2, &framebuflen );

        if ( isValid )
            /* Zustandsmaschine aufrufen um auf Stimulatorantwort zu reagieren. */
            /* Der aktuelle Frame wird beantwortet bzw. dessen Daten ausgewertet */	
            SCM2_Talk( pWrapperScienceMode2 );
    }

    SCM2_GetErrors( pWrapperScienceMode2, (int)UPDATE, &errors );
    SCM2_GetErrStat( pWrapperScienceMode2, &ErrStat );
    SCM2_GetSent( pWrapperScienceMode2, (int)UPDATE, &sent );
    SCM2_GetAcks( pWrapperScienceMode2, (int)UPDATE, &acks );    
    
    /* Wenn Stimulator initialisiert worden ist */
    /* dann erst regelmaessig aktualisieren */
    SCM2_GetConnected( pWrapperScienceMode2, &isConnected );
    SCM2_GetInitialised( pWrapperScienceMode2, &isInitialised );
    SCM2_GetRunning( pWrapperScienceMode2, &isRunning );          

    if ( isConnected && isInitialised )
    {

      S_Pulse_Current = (real_T *)ssGetInputPortRealSignalPtrs(S, 0);
      S_Pulse_Width = (real_T *)ssGetInputPortRealSignalPtrs(S, 1);
      S_Mode = (real_T *)ssGetInputPortRealSignalPtrs(S, 2);
    
      SCM2_GetPossibleMode( pWrapperScienceMode2, &allowed_max_mode );

      for (i = 0; i < numberofchannels; i++)
      {
        if ( S_Mode[i] > allowed_max_mode )
        {
          ssPrintf("mdlOutputs: Max allowed mode for channel %i is %i\n", i, allowed_max_mode);
        }
      }
      
      *mute_period = (real_T) (double) (S_Group_Time * allowed_max_mode);

      /* S_Main_Time gleich 0 bedeutet PC triggert Stimulation */
      if ( S_Main_Time == 0 )
      {
        /* Stimulation wird vom PC getriggert */
        SCM2_Send_Update_Parameter( pWrapperScienceMode2,
                                    S_Pulse_Width,
                                    S_Pulse_Current,
                                    S_Mode,
                                    numberofchannels );

      }
      else if ( S_Main_Time != 0 )
      {
        SCM2_GetNewData( pWrapperScienceMode2,
                         S_Pulse_Width,
                         S_Pulse_Current,
                         S_Mode,
                         numberofchannels,
                         &isNewData);
      
        if ( !isRunning || isNewData )
        {
          /* Stimulation wird Stimulatorintern getriggert */
          /* Einmal initialisieren und dann bei Aenderung eines Wertes */
          SCM2_Send_Update_Parameter( pWrapperScienceMode2,
                                      S_Pulse_Width,
                                      S_Pulse_Current,
                                      S_Mode,
                                      numberofchannels );
        }
        else
        {
          /* Wenn keine Aenderung dann keepAlive senden */
          SCM2_KeepAlive( pWrapperScienceMode2 );
        }
      }
    }
    else if ( !isInitialised && !isRunning )
    {
        ssPrintf("mdlOutputs: stimulation stopped/failed, please restart simulation!\n");
    }
}

/*
 * mdlTerminate - called when the simulation is terminated.
 */
static void mdlTerminate(SimStruct *S)
{
	void * pWrapperScienceMode2;
	
	int    result = 0;
	int    i = 0;
	int    framebuflen = 0;
	int    ErrStat = 0;
	int    errors = 0;
	int    sent = 0;
	int    acks = 0;
    
	bool isValid = false;

    pWrapperScienceMode2 = (void *) ssGetPWorkValue(S, 0);
	
	/* 1 Sekunde warten */
#ifdef __unix || _TTY_POSIX_
	usleep(1000000);
#else
    Sleep(1000);
#endif
    
    if (ssGetSimMode(S) == SS_SIMMODE_NORMAL)
    {
        /* Daten von der seriellen Schnittstelle (USB) holen */
        SCM2_GetData( pWrapperScienceMode2, &framebuflen );

        while (framebuflen > 0)
        {
            /* Den aktuellen Frame im Puffer auswaehlen */
            /* Die soeben von der Schnittstelle geholten Daten auf gueltige Frames */
            /* ueberpruefen */
            SCM2_GetFrame( pWrapperScienceMode2, &isValid );

            /* Jeder gueltige Frame wird aus dem Puffer entfernt */
            /* Somit verringert sich die Pufferlaenge um die jeweilige Framelaenge */	
            SCM2_GetFrameBufLen( pWrapperScienceMode2, &framebuflen );

            if ( isValid )
                /* Zustandsmaschine aufrufen um auf Stimulatorantwort zu reagieren. */
                /* Der aktuelle Frame wird beantwortet bzw. dessen Daten ausgewertet */	
                SCM2_Talk( pWrapperScienceMode2 );
        }
	
        SCM2_GetErrStat( pWrapperScienceMode2, &ErrStat );
        ssPrintf("mdlTerminate: error-status %i\n\n", ErrStat);

        ssPrintf("mdlTerminate: statistics: INIT_STIM = 0, START_STIM = 1, KEEP_ALIVE = 2, INIT_ACK = 3, GET_MODE = 4, STOP = 5, SINGLE_PULSE = 6\n\n");

        for ( i = 0; i <= 6; i++)
        {
            SCM2_GetErrors( pWrapperScienceMode2, i, &errors );
            SCM2_GetSent( pWrapperScienceMode2, i, &sent );
            SCM2_GetAcks( pWrapperScienceMode2, i, &acks );	

            ssPrintf("mdlTerminate %u: SentCommands %i, ACKsReceived %i, ErrorsReceived %i\n", i, sent, acks, errors);   
        }    

        SCM2_Send_Stop_Signal( pWrapperScienceMode2 );
        SCM2_CloseSerial( pWrapperScienceMode2, &result );
    }
    
    SCM2_Destroy_Object( pWrapperScienceMode2 );
    ssPrintf("simulation ended!\n");
    ssSetPWorkValue(S, 0, NULL);
}

#ifdef MATLAB_MEX_FILE /* Is this file being compiled as a MEX-file? */
#include "simulink.c" /* MEX-file interface mechanism */
#else
#include "cg_sfun.h" /* Code generation registration function */
#endif
