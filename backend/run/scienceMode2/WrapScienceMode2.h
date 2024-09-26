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
 * WrapScienceMode2.h
 *
 *  Created on: 17.08.2012
 *       eMail: mendicus@gmx.de
 *      Author: Ralph Stephan
 *
 */
 
#ifdef __unix || _TTY_POSIX_
	#include <stdbool.h>
#endif
/*typedef int bool;*/ 
/*#define false 0*/ 
/*#define true 1*/ 

typedef void * CLASS_HANDLE;

#ifdef __cplusplus
extern "C"
{
#endif

extern void SCM2_Create_Object( CLASS_HANDLE * pCPP_CLASS );
extern void SCM2_Destroy_Object( CLASS_HANDLE CPP_CLASS );

extern void SCM2_SetDeviceName( CLASS_HANDLE CPP_CLASS, char * pInput );
extern void SCM2_OpenSerial( CLASS_HANDLE CPP_CLASS, int * Output );
extern void SCM2_CloseSerial( CLASS_HANDLE CPP_CLASS, int * Output );

extern void SCM2_GetInitialised( CLASS_HANDLE CPP_CLASS, bool * Output );
extern void SCM2_GetConnected( CLASS_HANDLE CPP_CLASS, bool * Output );
extern void SCM2_GetRunning( CLASS_HANDLE CPP_CLASS, bool * Output );

extern void SCM2_GetNewData( CLASS_HANDLE CPP_CLASS,
            double * S_Pulse_Width,
            double * S_Pulse_Current,
            double * S_Mode,
            unsigned int stim_channels_used,
            bool * Output );

extern void SCM2_GetErrors( CLASS_HANDLE CPP_CLASS, int Input, int * Output );
extern void SCM2_GetSent( CLASS_HANDLE CPP_CLASS, int Input, int * Output );
extern void SCM2_GetAcks( CLASS_HANDLE CPP_CLASS, int Input, int * Output );
extern void SCM2_GetErrStat( CLASS_HANDLE CPP_CLASS, int * Output );

extern void SCM2_GetData( CLASS_HANDLE CPP_CLASS, int * Output );
extern void SCM2_GetFrame( CLASS_HANDLE CPP_CLASS, bool * Output );
extern void SCM2_GetFrameBufLen( CLASS_HANDLE CPP_CLASS, int * Output );

extern void SCM2_Talk( CLASS_HANDLE CPP_CLASS );
extern void SCM2_KeepAlive( CLASS_HANDLE CPP_CLASS );

extern void SCM2_GetPossibleMode( CLASS_HANDLE CPP_CLASS, int * Output );

extern void SCM2_Send_Init_Param( CLASS_HANDLE CPP_CLASS,
			unsigned int stim_channels_used,
            double * S_Channel_Stim,
			unsigned int lf_channels_used,
            double * S_Channel_Lf,
			double S_Main_Time,
			double S_Group_Time,
			double S_N_Factor );

extern void SCM2_Send_Update_Parameter( CLASS_HANDLE CPP_CLASS,
            double * S_Pulse_Width,
            double * S_Pulse_Current,
            double * S_Mode,
			unsigned int stim_channels_used );

extern void SCM2_InitChannelListMode( CLASS_HANDLE CPP_CLASS,
			unsigned int Channel_Stim,
			unsigned int Channel_Lf,
			unsigned int Main_Time,
			unsigned int Group_Time,
			unsigned int N_Factor );

extern void SCM2_StartChannelListMode( CLASS_HANDLE CPP_CLASS,
            unsigned int * Pulse_Width,
            unsigned int * Pulse_Current,
            unsigned int * Mode,
			unsigned int stim_channels_used );

extern void SCM2_Send_Single_Pulse( CLASS_HANDLE CPP_CLASS,
			unsigned int Channel_Number,
			unsigned int Pulse_Width,
			unsigned int Pulse_Current );

extern void SCM2_Send_Stop_Signal( CLASS_HANDLE CPP_CLASS );

extern void SCM2_GetStimulationMode( CLASS_HANDLE CPP_CLASS );

#ifdef __cplusplus
}
#endif
