/**
 **********************************************************************
 * \file plc_constants.h
 *
 * Short description of the file.
 * Long description of the file.
 *
 * Copyright (c) 2024, hoox.software, Helmbrechts
 * All Rights Reserved.
 *
 * Created on: 28.04.2026
 *     Author: patrickdressel
 *
 * Template: cpp_class_definition.hpp (Version 1)
 **********************************************************************
 */

#ifndef SRC_PLC_CONSTANTS_H_
#define SRC_PLC_CONSTANTS_H_


/*
 **********************************************************************
 constant definitions
 **********************************************************************
 */
const UINT32 TASK_CYCLETIME = 100;
const UINT32 POS_INFINITE   = 0x7F800000;
const UINT32 NEG_INFINITE   = 0xFF800000;
const UINT32 NAN_HIGH       = 0x7FC00000;
const UINT32 NAN_LOW        = 0x000FFFFF;

#endif /* SRC_PLC_CONSTANTS_H_ */
