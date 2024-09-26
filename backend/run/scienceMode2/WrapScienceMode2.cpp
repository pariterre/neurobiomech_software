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
 * WrapScienceMode2.cpp
 *
 *  Created on: 17.08.2012
 *       eMail: mendicus@gmx.de
 *      Author: Ralph Stephan
 *
 */
 
#include "ScienceMode2.h"
#include "WrapScienceMode2.h"

extern void SCM2_Create_Object( CLASS_HANDLE * pCPP_CLASS )
{
    ScienceMode2 *pSCM2 = new ScienceMode2();

    printf("Create_Object reference-address: %lu\n", (long unsigned int)pSCM2);

    *pCPP_CLASS = (CLASS_HANDLE) pSCM2;
}

extern void SCM2_Destroy_Object( CLASS_HANDLE CPP_CLASS )
{
    ScienceMode2 *pSCM2 = (ScienceMode2 *) CPP_CLASS;

    printf("Destroy_Object reference-address: %lu\n", (long unsigned int)pSCM2);
    
    free( pSCM2 );
}

extern void SCM2_SetDeviceName( CLASS_HANDLE CPP_CLASS, char * pInput )
{
    ScienceMode2 *pSCM2 = (ScienceMode2 *) CPP_CLASS;

    pSCM2->setDeviceName( pInput );
}

extern void SCM2_OpenSerial( CLASS_HANDLE CPP_CLASS, int * pOutput )
{
    int retval = 0;

    ScienceMode2 *pSCM2 = (ScienceMode2 *) CPP_CLASS;

    retval = pSCM2->Open_serial();

    *pOutput = retval;	
}

extern void SCM2_CloseSerial( CLASS_HANDLE CPP_CLASS, int * pOutput )
{
    int retval = 0;

    ScienceMode2 *pSCM2 = (ScienceMode2 *) CPP_CLASS;

    retval = pSCM2->Close_serial();

    *pOutput = retval;	
}

extern void SCM2_GetInitialised( CLASS_HANDLE CPP_CLASS, bool * pOutput )
{
    bool retval = 0;

    ScienceMode2 *pSCM2 = (ScienceMode2 *) CPP_CLASS;

    retval = pSCM2->getInitialised();

    *pOutput = retval;	
}

extern void SCM2_GetConnected( CLASS_HANDLE CPP_CLASS, bool * pOutput )
{
    bool retval = 0;

    ScienceMode2 *pSCM2 = (ScienceMode2 *) CPP_CLASS;

    retval = pSCM2->getConnected();

    *pOutput = retval;	
}

extern void SCM2_GetRunning( CLASS_HANDLE CPP_CLASS, bool * pOutput )
{
    bool retval = 0;

    ScienceMode2 *pSCM2 = (ScienceMode2 *) CPP_CLASS;

    retval = pSCM2->getRunning();

    *pOutput = retval;	
}

extern void SCM2_GetData( CLASS_HANDLE CPP_CLASS, int * pOutput )
{
    int retval = 0;

    ScienceMode2 *pSCM2 = (ScienceMode2 *) CPP_CLASS;
    
    retval = pSCM2->getData();
    
    *pOutput = retval;
}

extern void SCM2_GetFrame( CLASS_HANDLE CPP_CLASS, bool * pOutput )
{
    bool retval = false;

    ScienceMode2 *pSCM2 = (ScienceMode2 *) CPP_CLASS;

    retval = pSCM2->getFrame();

    *pOutput = retval;	
}

extern void SCM2_GetFrameBufLen( CLASS_HANDLE CPP_CLASS, int * pOutput )
{
    int retval = 0;

    ScienceMode2 *pSCM2 = (ScienceMode2 *) CPP_CLASS;

    retval = pSCM2->getFrameBufLen();

    *pOutput = retval;	
}

extern void SCM2_Talk( CLASS_HANDLE CPP_CLASS )
{
    ScienceMode2 *pSCM2 = (ScienceMode2 *) CPP_CLASS;

    pSCM2->talk();
}

extern void SCM2_KeepAlive( CLASS_HANDLE CPP_CLASS )
{
    ScienceMode2 *pSCM2 = (ScienceMode2 *) CPP_CLASS;	

    pSCM2->keepAlive();
}

extern void SCM2_GetPossibleMode( CLASS_HANDLE CPP_CLASS, int * pOutput )
{
    int retval = 0;

    ScienceMode2 *pSCM2 = (ScienceMode2 *) CPP_CLASS;

    retval = pSCM2->getPossibleMode();

    *pOutput = retval;	
}

extern void SCM2_Send_Init_Param( CLASS_HANDLE CPP_CLASS,
		unsigned int stim_channels_used,
                double *S_Channel_Stim,
                unsigned int lf_channels_used,
                double *S_Channel_Lf,
                double S_Main_Time,
                double S_Group_Time,
                double S_N_Factor )
{
    ScienceMode2 *pSCM2 = (ScienceMode2 *) CPP_CLASS;

    pSCM2->Send_Init_Param( stim_channels_used, S_Channel_Stim, lf_channels_used, S_Channel_Lf, S_Main_Time, S_Group_Time, S_N_Factor );
}

extern void SCM2_Send_Update_Parameter( CLASS_HANDLE CPP_CLASS,
                double *S_Pulse_Width,
                double *S_Pulse_Current,
                double *S_Mode,
                unsigned int stim_channels_used )
{
    ScienceMode2 *pSCM2 = (ScienceMode2 *) CPP_CLASS;
    
    pSCM2->Send_Update_Parameter( S_Pulse_Width, S_Pulse_Current, S_Mode, stim_channels_used );
}

extern void SCM2_InitChannelListMode( CLASS_HANDLE CPP_CLASS,
                unsigned int Channel_Stim,
                unsigned int Channel_Lf,
                unsigned int Main_Time,
                unsigned int Group_Time,
                unsigned int N_Factor )
{
    ScienceMode2 *pSCM2 = (ScienceMode2 *) CPP_CLASS;	

    pSCM2->InitChannelListMode( Channel_Stim, Channel_Lf, Main_Time, Group_Time, N_Factor );
}

extern void SCM2_GetNewData( CLASS_HANDLE CPP_CLASS,
		double *Pulse_Width,
                double *Pulse_Current,
                double *Mode,
                unsigned int stim_channels_used,
		bool *pOutput )
{
    bool retval = false;	
	
    ScienceMode2 *pSCM2 = (ScienceMode2 *) CPP_CLASS;
  
    retval = pSCM2->getNewData( Pulse_Width, Pulse_Current, Mode, stim_channels_used );
    
    *pOutput = retval;
}

extern void SCM2_StartChannelListMode( CLASS_HANDLE CPP_CLASS,
                unsigned int *Pulse_Width,
                unsigned int *Pulse_Current,
                unsigned int *Mode,
                unsigned int stim_channels_used )
{
    ScienceMode2 *pSCM2 = (ScienceMode2 *) CPP_CLASS;

    pSCM2->StartChannelListMode( Pulse_Width, Pulse_Current, Mode, stim_channels_used );
}

extern void SCM2_Send_Single_Pulse( CLASS_HANDLE CPP_CLASS,
                unsigned int Channel_Number,
                unsigned int Pulse_Width,
                unsigned int Pulse_Current )
{
    ScienceMode2 *pSCM2 = (ScienceMode2 *) CPP_CLASS;	

    pSCM2->Send_Single_Pulse( Channel_Number, Pulse_Width, Pulse_Current );
}

extern void SCM2_Send_Stop_Signal( CLASS_HANDLE CPP_CLASS )
{
    ScienceMode2 *pSCM2 = (ScienceMode2 *) CPP_CLASS;

    pSCM2->Send_Stop_Signal();
}

extern void SCM2_GetStimulationMode( CLASS_HANDLE CPP_CLASS )
{
    ScienceMode2 *pSCM2 = (ScienceMode2 *) CPP_CLASS;

    pSCM2->GetStimulationMode();
}

extern void SCM2_GetErrors( CLASS_HANDLE CPP_CLASS, int Input, int * pOutput )
{
    int retval = 0;

    ScienceMode2 *pSCM2 = (ScienceMode2 *) CPP_CLASS;

    retval = pSCM2->getErrors(Input);

    *pOutput = retval;
}

extern void SCM2_GetSent( CLASS_HANDLE CPP_CLASS, int Input, int * pOutput )
{
    int retval = 0;

    ScienceMode2 *pSCM2 = (ScienceMode2 *) CPP_CLASS;

    retval = pSCM2->getSent(Input);

    *pOutput = retval;
}

extern void SCM2_GetAcks( CLASS_HANDLE CPP_CLASS, int Input, int * pOutput )
{
    int retval = 0;

    ScienceMode2 *pSCM2 = (ScienceMode2 *) CPP_CLASS;

    retval = pSCM2->getAcks(Input);

    *pOutput = retval;
}

extern void SCM2_GetErrStat( CLASS_HANDLE CPP_CLASS, int * pOutput )
{
    int retval = 0;

    ScienceMode2 *pSCM2 = (ScienceMode2 *) CPP_CLASS;

    retval = pSCM2->getErrStat();

    *pOutput = retval;
}
