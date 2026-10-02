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
SEM_ID m_Semaphore;

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
void enterCriticalSection();
void leaveCriticalSection();
void updateRead(void);
void updateWrite(void);
BOOL InitVariable(CHAR *strModule, CHAR *strVariable, SINT32 pData, plc_Variable *sTemp);

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
    m_Semaphore        = 0;
    pPlcName           = libplc_GetProjectName(pProject);

    LST_SVI_READ = lst_New(sizeof(plc_Variable));

    if (LST_SVI_READ == NULL)
    {
        test_Err("Failed to get Memory for SVI READ List");
        return;
    }

    LST_SVI_WRITE = lst_New(sizeof(plc_Variable));

    if (LST_SVI_WRITE == NULL)
    {
        test_Err("Failed to get Memory for SVI WRITE List");
        return;
    }

    m_Semaphore = semMCreate(SEM_Q_PRIORITY | SEM_INVERSION_SAFE);

    if (m_Semaphore == NULL)
    {
        test_Err("Failed to get semaphoe for update task!");
        return;
    }

    pf_GetInt(pPlcName, "BaseParms", "Priority", 255, &s32Prio, 0, 0);
    s32Prio--;

    // set task cycletime to 100 ms
    u32TaskDelay = (sysClkRateGet() * TASK_CYCLETIME) / 1000;

    u32TaskDelay = u32TaskDelay == 0 ? u32TaskDelay : 1;

    s32TaskHandle =
    sys_TaskSpawn(libplc_GetProjectName(pProject),
                  "SviUpdate",
                  s32Prio,
                  VX_FP_TASK,
                  10000,
                  (FUNCPTR)PLCSVI_Main);

    if (s32TaskHandle == ERROR)
    {
        test_Err("SviUpdate Task not spawned!");
    }
    else
    {
        test_Info("Task for SviUpdate runnning!");
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
        s32TaskHandle = 0;
    }

    if (m_Semaphore != NULL)
    {
        semDelete(m_Semaphore);

        m_Semaphore = NULL;
    }

    if (LST_SVI_READ != 0)
    {
        lst_Del(LST_SVI_READ);
        LST_SVI_READ = 0;
    }

    if (LST_SVI_WRITE != 0)
    {
        lst_Del(LST_SVI_WRITE);
        LST_SVI_WRITE = 0;
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

        sys_CycleEnd();
        taskDelay(u32TaskDelay);
        sys_CycleStart();

        enterCriticalSection();

        updateRead();

        updateWrite();

        leaveCriticalSection();


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

                if (svi_GetBlk(sTemp->pLib, sTemp->sAddr, sTemp->pData, &u32Size) != SVI_E_OK)
                {
                    test_Err("svi_GetBlk returned error!");
                    sTemp->bValid = FALSE;
                }
            }
            else
            {
                if (svi_GetVal(sTemp->pLib, sTemp->sAddr, &sTemp->u32Data) != SVI_E_OK)
                {
                    test_Err("svi_GetVal returned error!");
                    sTemp->bValid = FALSE;
                }
                else
                {
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

                    default:

                        if (sTemp->bLogged == FALSE)
                        {
                            test_Err("updateRead : can not copy variable data! size not defined");
                            sTemp->bLogged = TRUE;
                        }

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
    UINT32 u32Size  = 0;
    struct plc_Variable *sTemp = 0;

    sTemp = lst_GetHead(LST_SVI_WRITE);

    while(sTemp != NULL)
    {
        if (sTemp->bValid == TRUE)
        {

            if ((sTemp->u32Format & SVI_F_BLK) == SVI_F_BLK)
            {
                u32Size = sTemp->u32Size;

                if (svi_SetBlk(sTemp->pLib, sTemp->sAddr, sTemp->pData, u32Size) != SVI_E_OK)
                {
                    test_Err("svi_SetBlk returned error!");
                    sTemp->bValid =FALSE;
                }
            }
            else
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

                default:

                    if (sTemp->bLogged == FALSE)
                    {
                        test_Err("updateWrite : can not copy variable data! size not defined");
                        sTemp->bLogged = TRUE;
                    }
                }

                if (u32Value != sTemp->u32Data)
                {
                    sTemp->u32Data = u32Value;

                    if (svi_SetVal(sTemp->pLib, sTemp->sAddr, sTemp->u32Data) != SVI_E_OK)
                    {
                        test_Err("svi_SetVal returned error!");
                        sTemp->bValid = FALSE;
                    }
                }
            }

        }

        sTemp = lst_GetNext(sTemp);
    }

}

/**
 ********************************************************************************
 * @brief This function adds a variable into READ-list
 *
 * @param[in]  strModule     name of module
 * @param[in]  strVariable   name of variable
 * @param[in]  pData         pointer to variable
 * @param[in]  u32Size       size of variable
 *
 * @retval     handle to variable
 * 
 * @note       Functions that can be called from M-PLC must be capitalized.
*******************************************************************************/
SINT32 ADDVARIABLE(CHAR *strModule, CHAR *strVariable, SINT32 pData, UINT32 u32Size)
{
    struct plc_Variable *sTemp = 0;

    if (LST_SVI_READ == NULL)
    {
        return 0;
    }

    enterCriticalSection();

    sTemp = lst_AddTail(LST_SVI_READ);

    if (sTemp != NULL)
    {
        memset(sTemp, 0, sizeof(plc_Variable));
        sTemp->u32Size = u32Size;

        if (InitVariable(strModule, strVariable, pData, sTemp) == FALSE)
        {
            lst_RemTail(LST_SVI_READ);

            sTemp = 0;
        }
    }

    leaveCriticalSection();

    return (SINT32)sTemp;
}

/**
 ********************************************************************************
 * @brief This function adds a variable into WRITE-list
 *
 * @param[in]  strModule     name of module
 * @param[in]  strVariable   name of variable
 * @param[in]  pData         pointer to variable
 * @param[in]  u32Size       size of variable
 *
 * @retval     handle to variable
 *
 * @note       Functions that can be called from M-PLC must be capitalized.
*******************************************************************************/
SINT32 ADDVARIABLEWRITE(CHAR *strModule, CHAR *strVariable, SINT32 pData, UINT32 u32Size)
{
    struct plc_Variable *sTemp = 0;

    if (LST_SVI_WRITE == NULL)
    {
        return 0;
    }

    enterCriticalSection();

    sTemp = lst_AddTail(LST_SVI_WRITE);

    if (sTemp != NULL)
    {
        memset(sTemp, 0, sizeof(plc_Variable));
        sTemp->u32Size = u32Size;

        if (InitVariable(strModule, strVariable, pData, sTemp) == FALSE)
        {
            lst_RemTail(LST_SVI_WRITE);
            sTemp = 0;
        }
    }

    leaveCriticalSection();

    return (SINT32)sTemp;
}

/**
 ********************************************************************************
 * @brief This function initializes a the svi-data
 *
 * @param[in]  strModule     name of module
 * @param[in]  strVariable   name of variable
 * @param[in]  pData         pointer to variable
 * @param[in]  pTemp         node of list
 *
 * @retval     TRUE = init sucessfull, FALSE = init failed
 *
 * @note       Functions that can be called from M-PLC must be capitalized.
*******************************************************************************/
BOOL InitVariable(CHAR *strModule, CHAR *strVariable, SINT32 pData, plc_Variable *pTemp)
{

    if (pTemp == NULL)
    {
        test_Err("InitVariable failed, list entry not available");
        return FALSE;
    }

    if (strModule == NULL)
    {
        test_Err("InitVariable failed, module name not available");
        return FALSE;
    }

    if (strVariable == NULL)
    {
        test_Err("InitVariable failed, variable name not available");
        return FALSE;
    }

    if (pData == 0)
    {
        test_Err("InitVariable failed, external memory not available");
        return FALSE;
    }

    pTemp->pLib = svi_GetLib(strModule);

    if (pTemp->pLib == NULL)
    {
        test_Err("InitVariable failed, Module %s not found", strModule);
        return FALSE;
    }

    pTemp->pData     = (void*)pData;
    pTemp->u32Format = SVI_F_EXTLEN;

    pTemp->bValid = (svi_GetAddr(pTemp->pLib, strVariable, &pTemp->sAddr, &pTemp->u32Format) == SVI_E_OK);

    if (pTemp->bValid == FALSE)
    {
        test_Err("InitVariable failed, check if %s exist", strVariable);
        return FALSE;
    }

    pTemp->u32SizeType = pTemp->u32Format & 0xF;
    //check size of variable or use target size
    if (pTemp->u32Size == 0)
    {
        pTemp->u32Size = pTemp->u32Format >> 16;
    }
    else
    {
        pTemp->bValid = (pTemp->u32Size == (pTemp->u32Format >> 16));
    }

    if (pTemp->bValid == FALSE)
    {
        test_Err("InitVariable failed, check size of %s", strVariable);
        return FALSE;
    }

    if (pTemp->u32Size <= 4)
    {
        svi_GetVal(pTemp->pLib, pTemp->sAddr, &pTemp->u32Data);
    }

    return TRUE;
}

/**
 ********************************************************************************
 * @brief This Function removes a variable from the list
 *
 * @param[in]  s32Variable      parameter
 *
 * @retval     TRUE = ok, FALSE = failed
 *
 * @note       Functions that can be called from M-PLC must be capitalized.
*******************************************************************************/
BOOL8 REMOVEVARIABLE(SINT32 s32Variable)
{

    BOOL8 bReturn = FALSE;

    enterCriticalSection();

    if (s32Variable != 0)
    {
        lst_RemNode((void*)s32Variable);

        bReturn = TRUE;
    }

    leaveCriticalSection();

    return bReturn;
}

/**
 ********************************************************************************
 * @brief This Function checks if handle contains valid data
 *
 * @param[in]  s32Variable      parameter
 *
 * @retval     TRUE = ok, FALSE = failed
 *
 * @note       Functions that can be called from M-PLC must be capitalized.
*******************************************************************************/
BOOL8 ISVALID(SINT32 s32Variable)
{
    plc_Variable *sTemp = (plc_Variable*)s32Variable;

    if (sTemp != NULL)
    {
        return sTemp->bValid;
    }

    return FALSE;
}

/**
 ********************************************************************************
 * @brief Set tasktime of update task
 *
 * @param[in]  u32Time      parameter
 *
 * @retval     0 = ok, -1 = falied
 *
 * @note       Functions that can be called from M-PLC must be capitalized.
*******************************************************************************/
SINT32 SETTASKTIME(UINT32 u32Time)
{

    if (u32Time > 1000 || u32Time == 0)
    {
        return -1;
    }

    u32TaskDelay = (sysClkRateGet() * u32Time) / 1000;

    u32TaskDelay = u32TaskDelay != 0 ? u32TaskDelay : 1;

    return 0;
}

/* ----------------------------------------------------------------- */
void enterCriticalSection()
{
    if (m_Semaphore == NULL)
    {
        return;
    }
    // test immediate availability of semaphore (like it should be most of the time)
    if (semTake(m_Semaphore, NO_WAIT) == ERROR)
    {
        semTake(m_Semaphore, WAIT_FOREVER);
    }
}

/* ----------------------------------------------------------------- */
void leaveCriticalSection()
{
    if (m_Semaphore == NULL)
    {
        return;
    }

    if (semGive(m_Semaphore) == ERROR)
    {
        test_Err("leaveCriticalSection: semaphore not valid or not owned by me");
    }

}
