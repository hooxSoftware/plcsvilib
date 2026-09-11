/**
 **********************************************************************
 * \file plc_TestDebug.h
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


#ifndef SRC_PLC_TESTDEBUG_H_
#define SRC_PLC_TESTDEBUG_H_

/*
 **********************************************************************
 system includes
 **********************************************************************
 */
#include <mtypes.h>
#include <log_e.h>
/*
 **********************************************************************
 project includes
 **********************************************************************
 */


/*
 **********************************************************************
 Typedefintions
 **********************************************************************
 */


/*
 **********************************************************************
 variable defintions
 **********************************************************************
 */


/*
 **********************************************************************
 functiondefintions
 **********************************************************************
 */
void test_Info(const CHAR* pMessage, ...);

void test_Err(const CHAR* pMessage, ...);

void plc_SetDebugMode(SINT32 s32Mode);

#endif /* SRC_PLC_TESTDEBUG_H_ */
