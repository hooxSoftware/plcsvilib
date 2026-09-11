/**
 **********************************************************************
 * \file plc_TestDebug.c
 *
 * Short description of the file.
 * Long description of the file.
 *
 * Copyright (c) 2024, Hoox.Software, Helmbrechts \n
 * All Rights Reserved.
 *
 * Created on: 15.03.2024
 *     Author: patrickdressel
 *
 * Template: cpp_class_definition.hpp (Version 1)
 **********************************************************************
 */

/*
 **********************************************************************
 system includes
 **********************************************************************
 */


/*
 **********************************************************************
 project includes
 **********************************************************************
 */
#include <plc_TestDebug.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/*
 **********************************************************************
 static member variable definitions
 **********************************************************************
 */


/*
 **********************************************************************
 variable definitions
 **********************************************************************
 */
SINT32 plc_debugmode;

/*
 **********************************************************************
 function definitions
 **********************************************************************
 */

void plc_SetDebugMode(SINT32 s32Mode)
{
    plc_debugmode = s32Mode;
}

/* ----------------------------------------------------------------- */
void test_Info(const CHAR* pMessage, ...)
{
    CHAR    l_LogMessage[256];
    va_list l_Args = NULL;

    if (plc_debugmode == 0)
    {
        return;
    }

    bzero(l_LogMessage, sizeof(l_LogMessage));

    va_start (l_Args, pMessage);
    vsprintf (l_LogMessage , pMessage, l_Args);
    va_end (l_Args);


    log_Info2(l_LogMessage);
}

/* ----------------------------------------------------------------- */
void test_Err(const CHAR* pMessage, ...)
{
    CHAR    l_LogMessage[256];
    va_list l_Args = NULL;

    bzero(l_LogMessage, sizeof(l_LogMessage));

    va_start (l_Args, pMessage);
    vsprintf (l_LogMessage , pMessage, l_Args);
    va_end (l_Args);

    log_Err(l_LogMessage);
}
