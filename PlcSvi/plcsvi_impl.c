/**
********************************************************************************
* This file was generated automatically and must be implemented.
********************************************************************************
* @file     plcsvi_impl.c
* @author   PIDL C Generator
* @date     $LastChangeDate: ${DateAndTime} $
* @brief
* @note     Functions and Functionblocks that can be called from M-PLC must 
*           be capitalized.
*
********************************************************************************
* COPYRIGHT BY BACHMANN ELECTRONIC GmbH ${Year}
*******************************************************************************/

#define IMPLFILE
#include "src-gen/plcsvi_impl.h"
#include "src-gen/plcsvi.h"
#undef IMPLFILE

/*
 **********************************************************************
 system includes
 **********************************************************************
 */
#include <math.h>
#include <svi_e.h>
#include <prof_e.h>
#include <lst_e.h>

/*
 **********************************************************************
 project includes
 **********************************************************************
 */
#include <plc_TestDebug.h>
#include <plc_constants.h>

/*--- Variables ---*/
UINT32 plcsvi_LibHandle;				/* library-handle */
SINT32 s32TaskHandle;
UINT32 u32TaskDelay;

CHAR* pPlcName;
LST_ID *LST_SVI_READ;
LST_ID *LST_SVI_WRITE;

/***********************************
 * Test Registry
 * Structure with List of Testsuites
 */
typedef struct plc_Variable
{
  SVI_FUNC *pLib;
  SVI_ADDR sAddr;
  void* pData;
  UINT32 u32Data;
  UINT32 u32SizeType;
  UINT32 u32Size;
  UINT32 u32Format;
  BOOL bValid;
  BOOL bLogged;

} plc_Variable;


plc_Variable sTest;
/*
 **********************************************************************
 function definitions
 **********************************************************************
 */
void PLCSVI_Main(void);
void updateRead(void);
void updateWrite(void);
VOID InitVariable(CHAR *strModule, CHAR *strVariable, SINT32 pData, plc_Variable *sTemp);

/**
 ********************************************************************************
 * @brief Function is called when a plc-project is installed.

 * @param[in]  pProject      plc-project
 * @param[in]  pInfo        information about library
 *
 * @param[out] pConfig      library configuration
 *
 * @retval     != Ok     : error, loading the project is interrupted
 * @retval     Ok        : initialization is ok
 *
*******************************************************************************/
MLOCAL SINT32 PLCSVI_PlcDllPrepareEx_Impl(PLCPROJ *pProject, PLC_LIBINFO *pInfo, PLC_EXTLIBCONFIG *pConfig)// @suppress("Unused static function")
{
    int s32Handle = 0;
    pPlcName      = libplc_GetProjectName(pProject);

    s32Handle = taskNameToId("SviUpdate");

    if (s32Handle != ERROR)
    {
        log_Err("Can not install module, Testing-Task SviUpdate alread running");
        return s32Handle;
    }

    return OK;
}

/**
 ********************************************************************************
 * @brief Function is called when a plc-project is initialized.

 * @param[in]  pProject      plc-project
 * @param[in]  pInfo        information about library
 *
 * @retval     N/A
*******************************************************************************/
MLOCAL VOID PLCSVI_PlcDllInit_Impl(PLCPROJ *pProject, PLC_LIBINFO *pInfo)// @suppress("Unused static function")
{
    plcsvi_LibHandle   = 0;
    u32TaskDelay       = 0;
    s32TaskHandle      = 0;
    LST_SVI_READ       = 0;
    LST_SVI_WRITE      = 0;
    SINT32 s32Prio     = 0;
    pPlcName           = libplc_GetProjectName(pProject);

    pf_GetInt(pPlcName, "BaseParms", "Priority", 255, &s32Prio, 0, 0);
    s32Prio--;

    // set task cycletime to 100 ms
    u32TaskDelay = (sysClkRateGet() * TASK_CYCLETIME) / 1000;

    s32TaskHandle =
    sys_TaskSpawn(libplc_GetProjectName(pProject),
                  "SviUpdate",
                  s32Prio,
                  VX_FP_TASK,
                  10000,
                  (FUNCPTR)PLCSVI_Main);

    if (s32TaskHandle != 0)
    {
        test_Info("Task for SviUpdate spawned!");

        LST_SVI_READ = lst_New(sizeof(plc_Variable));

        if (LST_SVI_READ == NULL)
        {
            test_Err("Failed to get Memory for SVI READ List");
        }
        LST_SVI_WRITE = lst_New(sizeof(plc_Variable));

        if (LST_SVI_WRITE == NULL)
        {
            test_Err("Failed to get Memory for SVI WRITE List");
        }
    }

	return;
}

/**
 ********************************************************************************
 * @brief Function is called when a plc-project is deinitialized.

 * @param[in]  pProject      plc-project
 * @param[in]  pInfo        information about library
 *
 * @retval     N/A
*******************************************************************************/
MLOCAL VOID PLCSVI_PlcDllDeinit_Impl(PLCPROJ *pProject, PLC_LIBINFO *pInfo) // @suppress("Unused static function")
{
    test_Info("Clean up PlcSviLib!");

    if (s32TaskHandle != 0)
    {
        test_Info("Delete sviupdate Task!");
        taskDelete(s32TaskHandle);
    }

    if (LST_SVI_READ != 0)
    {
        lst_Del(LST_SVI_READ);
    }

    if (LST_SVI_WRITE != 0)
    {
        lst_Del(LST_SVI_WRITE);
    }

	return;
}

/* ----------------------------------------------------------------- */
void PLCSVI_Main(void)
{

    test_Info("PLCSVI_Main: Started");

    //--- Wait for cycle delay.
    do
    {
        updateWrite();

        sys_CycleEnd();
        taskDelay(u32TaskDelay);
        sys_CycleStart();

        updateRead();

    } while(1);

    test_Info("PLCSVI_Main: Removed");

}

// update list with variables to read
void updateRead()
{
    struct plc_Variable *sTemp = 0;
    UINT32 u32Size = 0;

    sTemp = lst_GetHead(LST_SVI_READ);

    while(sTemp != NULL)
    {
        if (sTemp->bValid == TRUE)
        {
            if ((sTemp->u32Format & SVI_F_BLK) == SVI_F_BLK)
            {
                u32Size = sTemp->u32Size;

                svi_GetBlk(sTemp->pLib, sTemp->sAddr, sTemp->pData, &u32Size);
            }
            else
            {
                svi_GetVal(sTemp->pLib, sTemp->sAddr, &sTemp->u32Data);

                switch (sTemp->u32SizeType)
                {
                case SVI_F_BOOL8:
                case SVI_F_UINT1:
                case SVI_F_UINT8:
                case SVI_F_SINT8:  memcpy(sTemp->pData, &sTemp->u32Data, 1); break;

                case SVI_F_UINT16:
                case SVI_F_SINT16: memcpy(sTemp->pData, &sTemp->u32Data, 2); break;

                case SVI_F_UINT32:
                case SVI_F_SINT32:
                case SVI_F_REAL32: memcpy(sTemp->pData, &sTemp->u32Data, 4); break;

                case SVI_F_UINT64:
                case SVI_F_SINT64:
                case SVI_F_REAL64: memcpy(sTemp->pData, &sTemp->u32Data, 4); break;

                default:

                    if (sTemp->bLogged == FALSE)
                    {
                        test_Err("PLCSVI_Main : can not copy variable data! size not defined");
                        sTemp->bLogged = TRUE;
                    }

                }
            }
        }

        sTemp = lst_GetNext(sTemp);
    }
}

// update list with variables to write
void updateWrite()
{
    UINT32 u32Value = 0;
    struct plc_Variable *sTemp = 0;

    sTemp = lst_GetHead(LST_SVI_WRITE);

    while(sTemp != NULL)
    {
        if (sTemp->bValid == TRUE)
        {
            u32Value = 0;

            switch (sTemp->u32SizeType)
            {
            case SVI_F_BOOL8:
            case SVI_F_UINT1:
            case SVI_F_UINT8:
            case SVI_F_SINT8:  memcpy(&u32Value, (sTemp->pData), 1); break;

            case SVI_F_UINT16:
            case SVI_F_SINT16: memcpy(&u32Value, (sTemp->pData), 2); break;

            case SVI_F_UINT32:
            case SVI_F_SINT32:
            case SVI_F_REAL32: memcpy(&u32Value, (sTemp->pData), 4); break;

            case SVI_F_UINT64:
            case SVI_F_SINT64:
            case SVI_F_REAL64: memcpy(&u32Value, (sTemp->pData), 4); break;

            default:
                test_Err("PLCSVI_Main : can not copy variable data! size not defined");
            }

            if (u32Value != sTemp->u32Data)
            {
                sTemp->u32Data = u32Value;

                svi_SetVal(sTemp->pLib, sTemp->sAddr, sTemp->u32Data);
            }

        }

        sTemp = lst_GetNext(sTemp);
    }

}

/**
 ********************************************************************************
 * @brief Function that demonstrates how to create a function, that can be called
 * from M-PLC. Input is returned as output.
 *
 * @param[in]  strModule     name of module
 * @param[in]  strVariable   name of variable
 * @param[in]  pData         pointer to variable
 *
 * @retval     0
 * 
 * @note       Functions that can be called from M-PLC must be capitalized.
*******************************************************************************/
SINT32 ADDVARIABLE(CHAR *strModule, CHAR *strVariable, SINT32 pData)
{
    struct plc_Variable *sTemp = 0;

    sTemp = lst_AddTail(LST_SVI_READ);

    InitVariable(strModule, strVariable, pData, sTemp);

	return 0;
}

/**
 ********************************************************************************
 * @brief Function that demonstrates how to create a function, that can be called
 * from M-PLC. Input is returned as output.
 *
 * @param[in]  strModule     name of module
 * @param[in]  strVariable   name of variable
 * @param[in]  pData         pointer to variable
 *
 * @retval     0
 *
 * @note       Functions that can be called from M-PLC must be capitalized.
*******************************************************************************/
SINT32 ADDVARIABLEWRITE(CHAR *strModule, CHAR *strVariable, SINT32 pData)
{
    struct plc_Variable *sTemp = 0;

    sTemp = lst_AddTail(LST_SVI_WRITE);

    InitVariable(strModule, strVariable, pData, sTemp);

    return 0;
}

/**
 ********************************************************************************
 * @brief Function that demonstrates how to create a function, that can be called
 * from M-PLC. Input is returned as output.
 *
 * @param[in]  strModule     name of module
 * @param[in]  strVariable   name of variable
 * @param[in]  pData         pointer to variable
 *
 * @retval     0
 *
 * @note       Functions that can be called from M-PLC must be capitalized.
*******************************************************************************/
VOID InitVariable(CHAR *strModule, CHAR *strVariable, SINT32 pData, plc_Variable *pTemp)
{

    if (pTemp == NULL)
    {
        return;
    }

    memset(pTemp, 0, sizeof(plc_Variable));

    if (strModule == NULL)
    {
        return;
    }

    pTemp->pLib = svi_GetLib(strModule);

    if (strVariable == NULL)
    {
        return;
    }

    pTemp->pData     = (void*)pData;
    pTemp->u32Format = SVI_F_EXTLEN;

    pTemp->bValid = (svi_GetAddr(pTemp->pLib, strVariable, &pTemp->sAddr, &pTemp->u32Format) == SVI_E_OK) &&
                    (pTemp->pData  != NULL);

    pTemp->u32SizeType = pTemp->u32Format & 0xF;
    pTemp->u32Size     = pTemp->u32Format >> 16;

    if (pTemp->bValid == TRUE)
    {
        svi_GetVal(pTemp->pLib, pTemp->sAddr, &pTemp->u32Data);
    }
}


/**
 ********************************************************************************
 * @brief Function that demonstrates how to create a function, that can be called
 * from M-PLC. Input is returned as output.
 *
 * @param[in]  u32Time      parameter
 *
 * @retval     0 = ok, -1 = falied
 *
 * @note       Functions that can be called from M-PLC must be capitalized.
*******************************************************************************/
SINT32 SETTASKTIME(UINT32 u32Time)
{

    if (u32Time > 1000)
    {
        return -1;
    }

    u32TaskDelay = (sysClkRateGet() * u32Time) / 1000;

    return 0;
}

