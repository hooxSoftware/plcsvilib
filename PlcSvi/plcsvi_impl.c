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

/***********************************
 * Test Registry
 * Structure with List of Testsuites
 */
typedef struct plc_Variable
{
  SVI_FUNC *pLib;
  SVI_ADDR sAddr;
  void* pData;
  UINT32 u32Format;
  BOOL bValid;

} plc_Variable;


plc_Variable sTest;
/*
 **********************************************************************
 function definitions
 **********************************************************************
 */
void PLCTEST_Main(void);

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
MLOCAL SINT32 PLCSVI_PlcDllPrepareEx_Impl(PLCPROJ *pProject, PLC_LIBINFO *pInfo, PLC_EXTLIBCONFIG *pConfig)
{
    int s32Handle = 0;
    pPlcName      = libplc_GetProjectName(pProject);

    s32Handle = taskNameToId("SviUpdate");

    if (s32Handle != ERROR)
    {
        log_Err("Can not install module, Testing-Task HooxTest alread running");
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
MLOCAL VOID PLCSVI_PlcDllInit_Impl(PLCPROJ *pProject, PLC_LIBINFO *pInfo)
{
    plcsvi_LibHandle   = 0;
    u32TaskDelay       = 0;
    s32TaskHandle      = 0;
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
                  (FUNCPTR)PLCTEST_Main);

    if (s32TaskHandle != 0)
    {
        test_Info("Task for SviUpdate spawned!");
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
MLOCAL VOID PLCSVI_PlcDllDeinit_Impl(PLCPROJ *pProject, PLC_LIBINFO *pInfo)
{
    test_Info("Clean up Testregistry!");

    if (s32TaskHandle != ERROR)
    {
        test_Info("Delete Test Task!");
        taskDelete(s32TaskHandle);
    }


	return;
}

/* ----------------------------------------------------------------- */
void PLCTEST_Main(void)
{
    test_Info("PLCSVI_Main: Started");

    //--- Wait for cycle delay.
    do
    {
        sys_CycleEnd();
        taskDelay(u32TaskDelay);
        sys_CycleStart();

        if (sTest.bValid == TRUE)
        {
            svi_GetVal(sTest.pLib, sTest.sAddr, sTest.pData);
        }

    } while(1);

    test_Info("PLCSVI_Main: Removed");

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

    if (strModule != NULL)
    {
        sTest.pLib = svi_GetLib(strModule);
    }
    if (strVariable != NULL)
    {
        sTest.pData = (void*)pData;

        sTest.bValid = (svi_GetAddr(sTest.pLib, strVariable, &sTest.sAddr, &sTest.u32Format) == SVI_E_OK) &&
                       (sTest.pData  != NULL);

    }

	return 0;
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

