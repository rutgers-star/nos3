/*
**  GSC-18128-1, "Core Flight Executive Version 6.7"
**
**  Copyright (c) 2006-2019 United States Government as represented by
**  the Administrator of the National Aeronautics and Space Administration.
**  All Rights Reserved.
**
**  Licensed under the Apache License, Version 2.0 (the "License");
**  you may not use this file except in compliance with the License.
**  You may obtain a copy of the License at
**
**    http://www.apache.org/licenses/LICENSE-2.0
**
**  Unless required by applicable law or agreed to in writing, software
**  distributed under the License is distributed on an "AS IS" BASIS,
**  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
**  See the License for the specific language governing permissions and
**  limitations under the License.
*/

/*
** File: coveragetest_raspberry_pi_app.c
**
** Purpose:
** Coverage Unit Test cases for the RASPBERRY_PI Application
**
** Notes:
** This implements various test cases to exercise all code
** paths through all functions defined in the RASPBERRY_PI application.
**
** It is primarily focused at providing examples of the various
** stub configurations, hook functions, and wrapper calls that
** are often needed when coercing certain code paths through
** complex functions.
*/

/*
 * Includes
 */

#include "raspberry_pi_app_coveragetest_common.h"
#include "ut_raspberry_pi_app.h"

/* to get the RASPBERRY_PI_LIB_Function() declaration */

typedef struct
{
    uint16      ExpectedEvent;
    uint32      MatchCount;
    const char *ExpectedFormat;
} UT_CheckEvent_t;

/*
 * An example hook function to check for a specific event.
 */
static int32 UT_CheckEvent_Hook(void *UserObj, int32 StubRetcode, uint32 CallCount, const UT_StubContext_t *Context,
                                va_list va)
{
    UT_CheckEvent_t *State = UserObj;
    uint16           EventId;
    const char      *Spec;

    /*
     * The CFE_EVS_SendEvent stub passes the EventID as the
     * first context argument.
     */
    if (Context->ArgCount > 0)
    {
        EventId = UT_Hook_GetArgValueByName(Context, "EventID", uint16);
        if (EventId == State->ExpectedEvent)
        {
            if (State->ExpectedFormat != NULL)
            {
                Spec = UT_Hook_GetArgValueByName(Context, "Spec", const char *);
                if (Spec != NULL)
                {
                    /*
                     * Example of how to validate the full argument set.
                     * ------------------------------------------------
                     *
                     * If really desired one can call something like:
                     *
                     * char TestText[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
                     * vsnprintf(TestText, sizeof(TestText), Spec, va);
                     *
                     * And then compare the output (TestText) to the expected fully-rendered string.
                     *
                     * NOTE: While this can be done, use with discretion - This isn't really
                     * verifying that the FSW code unit generated the correct event text,
                     * rather it is validating what the system snprintf() library function
                     * produces when passed the format string and args.
                     *
                     * This type of check has been demonstrated to make tests very fragile,
                     * because it is influenced by many factors outside the control of the
                     * test case.
                     *
                     * __This derived string is not an actual output of the unit under test__
                     */
                    if (strcmp(Spec, State->ExpectedFormat) == 0)
                    {
                        ++State->MatchCount;
                    }
                }
            }
            else
            {
                ++State->MatchCount;
            }
        }
    }

    return 0;
}

/*
 * Helper function to set up for event checking
 * This attaches the hook function to CFE_EVS_SendEvent
 */
static void UT_CheckEvent_Setup(UT_CheckEvent_t *Evt, uint16 ExpectedEvent, const char *ExpectedFormat)
{
    memset(Evt, 0, sizeof(*Evt));
    Evt->ExpectedEvent  = ExpectedEvent;
    Evt->ExpectedFormat = ExpectedFormat;
    UT_SetVaHookFunction(UT_KEY(CFE_EVS_SendEvent), UT_CheckEvent_Hook, Evt);
}

/*
**********************************************************************************
**          TEST CASE FUNCTIONS
**********************************************************************************
*/

void Test_RASPBERRY_PI_AppMain(void)
{
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;

    /*
     * Test Case For:
     * void RASPBERRY_PI_AppMain( void )
     */

    UT_CheckEvent_t EventTest;

    /*
     * RASPBERRY_PI_AppMain does not return a value,
     * but it has several internal decision points
     * that need to be exercised here.
     *
     * First call it in "nominal" mode where all
     * dependent calls should be successful by default.
     */
    RASPBERRY_PI_AppMain();

    /*
     * Confirm that CFE_ES_ExitApp() was called at the end of execution
     */
    UtAssert_True(UT_GetStubCount(UT_KEY(CFE_ES_ExitApp)) == 1, "CFE_ES_ExitApp() called");

    /*
     * Now set up individual cases for each of the error paths.
     * The first is for RASPBERRY_PI_AppInit().  As this is in the same
     * code unit, it is not a stub where the return code can be
     * easily set.  In order to get this to fail, an underlying
     * call needs to fail, and the error gets propagated through.
     * The call to CFE_EVS_Register is the first opportunity.
     * Any identifiable (non-success) return code should work.
     */
    UT_SetDeferredRetcode(UT_KEY(CFE_EVS_Register), 1, CFE_EVS_INVALID_PARAMETER);

    /*
     * Just call the function again.  It does not return
     * the value, so there is nothing to test for here directly.
     * However, it should show up in the coverage report that
     * the RASPBERRY_PI_AppInit() failure path was taken.
     */
    RASPBERRY_PI_AppMain();

    /*
     * This can validate that the internal "RunStatus" was
     * set to CFE_ES_RunStatus_APP_ERROR, by querying the struct directly.
     *
     * It is always advisable to include the _actual_ values
     * when asserting on conditions, so if/when it fails, the
     * log will show what the incorrect value was.
     */
    UtAssert_True(RASPBERRY_PI_AppData.RunStatus == CFE_ES_RunStatus_APP_ERROR,
                  "RASPBERRY_PI_AppData.RunStatus (%lu) == CFE_ES_RunStatus_APP_ERROR",
                  (unsigned long)RASPBERRY_PI_AppData.RunStatus);

    UT_SetDeferredRetcode(UT_KEY(CFE_EVS_SendEvent), 5, CFE_EVS_INVALID_PARAMETER);
    RASPBERRY_PI_AppMain();

    /*
     * Note that CFE_ES_RunLoop returns a boolean value,
     * so in order to exercise the internal "while" loop,
     * it needs to return TRUE.  But this also needs to return
     * FALSE in order to get out of the loop, otherwise
     * it will stay there infinitely.
     *
     * The deferred retcode will accomplish this.
     */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_RunLoop), 1, true);

    /* Set up buffer for command processing */
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &MsgId, sizeof(MsgId), false);

    /*
     * Invoke again
     */
    RASPBERRY_PI_AppMain();

    /*
     * Confirm that CFE_SB_ReceiveBuffer() (inside the loop) was called
     */
    UtAssert_True(UT_GetStubCount(UT_KEY(CFE_SB_ReceiveBuffer)) == 1, "CFE_SB_ReceiveBuffer() called");

    /*
     * Now also make the CFE_SB_ReceiveBuffer call fail,
     * to exercise that error path.  This sends an
     * event which can be checked with a hook function.
     */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_RunLoop), 1, true);
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_ReceiveBuffer), 1, CFE_SB_PIPE_RD_ERR);
    UT_CheckEvent_Setup(&EventTest, RASPBERRY_PI_PIPE_ERR_EID, "RASPBERRY_PI: SB Pipe Read Error = %d");

    /*
     * Invoke again
     */
    RASPBERRY_PI_AppMain();

    /*
     * Confirm that the event was generated
     */
    UtAssert_True(EventTest.MatchCount == 1, "RASPBERRY_PI_PIPE_ERR_EID generated (%u)", (unsigned int)EventTest.MatchCount);
}

void Test_RASPBERRY_PI_AppInit(void)
{
    /*
     * Test Case For:
     * int32 RASPBERRY_PI_AppInit( void )
     */

    /* nominal case should return CFE_SUCCESS */
    UT_TEST_FUNCTION_RC(RASPBERRY_PI_AppInit(), CFE_SUCCESS);

    /* trigger a failure for each of the sub-calls,
     * and confirm a write to syslog for each.
     * Note that this count accumulates, because the status
     * is _not_ reset between these test cases. */
    UT_SetDeferredRetcode(UT_KEY(CFE_EVS_Register), 1, CFE_EVS_INVALID_PARAMETER);
    UT_TEST_FUNCTION_RC(RASPBERRY_PI_AppInit(), CFE_EVS_INVALID_PARAMETER);
    UtAssert_True(UT_GetStubCount(UT_KEY(CFE_ES_WriteToSysLog)) == 1, "CFE_ES_WriteToSysLog() called");

    UT_SetDeferredRetcode(UT_KEY(CFE_SB_CreatePipe), 1, CFE_SB_BAD_ARGUMENT);
    UT_TEST_FUNCTION_RC(RASPBERRY_PI_AppInit(), CFE_SB_BAD_ARGUMENT);

    UT_SetDeferredRetcode(UT_KEY(CFE_SB_Subscribe), 1, CFE_SB_BAD_ARGUMENT);
    UT_TEST_FUNCTION_RC(RASPBERRY_PI_AppInit(), CFE_SB_BAD_ARGUMENT);

    UT_SetDeferredRetcode(UT_KEY(CFE_SB_Subscribe), 2, CFE_SB_BAD_ARGUMENT);
    UT_TEST_FUNCTION_RC(RASPBERRY_PI_AppInit(), CFE_SB_BAD_ARGUMENT);

    // UT_SetDeferredRetcode(UT_KEY(CFE_EVS_SendEvent), 1, CFE_SB_BAD_ARGUMENT);
    // UT_TEST_FUNCTION_RC(RASPBERRY_PI_AppInit(), CFE_SB_BAD_ARGUMENT);
}

void Test_RASPBERRY_PI_ProcessTelemetryRequest(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    UT_CheckEvent_t   EventTest;
    CFE_MSG_FcnCode_t FcnCode;
    FcnCode = RASPBERRY_PI_REQ_DATA_TLM;

    TestMsgId = CFE_SB_ValueToMsgId(RASPBERRY_PI_CMD_MID);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDeferredRetcode(UT_KEY(RASPBERRY_PI_RequestData), 1, OS_SUCCESS);

    UT_CheckEvent_Setup(&EventTest, RASPBERRY_PI_REQ_DATA_ERR_EID, NULL);
    RASPBERRY_PI_ProcessTelemetryRequest();
    UtAssert_True(EventTest.MatchCount == 0, "RASPBERRY_PI_REQ_DATA_ERR_EID generated (%u)",
                  (unsigned int)EventTest.MatchCount);

    FcnCode = 99;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    RASPBERRY_PI_ProcessTelemetryRequest();
    UtAssert_True(EventTest.MatchCount == 0, "RASPBERRY_PI_REQ_DATA_ERR_EID generated (%u)",
                  (unsigned int)EventTest.MatchCount);
}

void Test_RASPBERRY_PI_ProcessCommandPacket(void)
{
    /*
     * Test Case For:
     * void RASPBERRY_PI_ProcessCommandPacket
     */
    /* a buffer large enough for any command message */
    union
    {
        CFE_SB_Buffer_t     SBBuf;
        RASPBERRY_PI_NoArgs_cmd_t Noop;
    } TestMsg;
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    size_t            MsgSize;
    UT_CheckEvent_t   EventTest;

    memset(&TestMsg, 0, sizeof(TestMsg));
    UT_CheckEvent_Setup(&EventTest, RASPBERRY_PI_PROCESS_CMD_ERR_EID, NULL);

    /*
     * The CFE_MSG_GetMsgId() stub uses a data buffer to hold the
     * message ID values to return.
     */
    TestMsgId = CFE_SB_ValueToMsgId(RASPBERRY_PI_CMD_MID);
    FcnCode   = RASPBERRY_PI_NOOP_CC;
    MsgSize   = sizeof(TestMsg.Noop);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);
    RASPBERRY_PI_ProcessCommandPacket();
    UtAssert_True(EventTest.MatchCount == 0, "RASPBERRY_PI_CMD_ERR_EID not generated (%u)",
                  (unsigned int)EventTest.MatchCount);

    TestMsgId = CFE_SB_ValueToMsgId(RASPBERRY_PI_REQ_HK_MID);
    FcnCode   = RASPBERRY_PI_REQ_HK_TLM;
    MsgSize   = sizeof(TestMsg.Noop);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);
    RASPBERRY_PI_ProcessCommandPacket();
    UtAssert_True(EventTest.MatchCount == 0, "RASPBERRY_PI_CMD_ERR_EID not generated (%u)",
                  (unsigned int)EventTest.MatchCount);

    /* invalid message id */
    TestMsgId = CFE_SB_INVALID_MSG_ID;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    RASPBERRY_PI_ProcessCommandPacket();
    UtAssert_True(EventTest.MatchCount == 1, "RASPBERRY_PI_CMD_ERR_EID generated (%u)", (unsigned int)EventTest.MatchCount);
}

void Test_RASPBERRY_PI_ProcessGroundCommand(void)
{
    /*
     * Test Case For:
     * void RASPBERRY_PI_ProcessGroundCommand
     */
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(RASPBERRY_PI_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode;
    size_t            Size;

    /* a buffer large enough for any command message */
    union
    {
        CFE_SB_Buffer_t     SBBuf;
        RASPBERRY_PI_NoArgs_cmd_t Noop;
        RASPBERRY_PI_NoArgs_cmd_t Reset;
        RASPBERRY_PI_NoArgs_cmd_t Enable;
        RASPBERRY_PI_NoArgs_cmd_t Disable;
        RASPBERRY_PI_Config_cmd_t Config;
    } TestMsg;
    UT_CheckEvent_t EventTest;

    memset(&TestMsg, 0, sizeof(TestMsg));

    /*
     * call with each of the supported command codes
     * The CFE_MSG_GetFcnCode stub allows the code to be
     * set to whatever is needed.  There is no return
     * value here and the actual implementation of these
     * commands have separate test cases, so this just
     * needs to exercise the "switch" statement.
     */

    /* test dispatch of NOOP */
    FcnCode = RASPBERRY_PI_NOOP_CC;
    Size    = sizeof(TestMsg.Noop);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &Size, sizeof(Size), false);
    UT_CheckEvent_Setup(&EventTest, RASPBERRY_PI_CMD_NOOP_INF_EID, NULL);
    RASPBERRY_PI_ProcessGroundCommand();
    UtAssert_True(EventTest.MatchCount == 1, "RASPBERRY_PI_CMD_NOOP_INF_EID generated (%u)",
                  (unsigned int)EventTest.MatchCount);
    /* test failure of command length */
    FcnCode = RASPBERRY_PI_NOOP_CC;
    Size    = sizeof(TestMsg.Config);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &Size, sizeof(Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_CheckEvent_Setup(&EventTest, RASPBERRY_PI_LEN_ERR_EID, NULL);
    RASPBERRY_PI_ProcessGroundCommand();
    UtAssert_True(EventTest.MatchCount == 1, "RASPBERRY_PI_LEN_ERR_EID generated (%u)", (unsigned int)EventTest.MatchCount);

    /* test dispatch of RESET */
    FcnCode = RASPBERRY_PI_RESET_COUNTERS_CC;
    Size    = sizeof(TestMsg.Reset);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &Size, sizeof(Size), false);
    UT_CheckEvent_Setup(&EventTest, RASPBERRY_PI_CMD_RESET_INF_EID, NULL);
    RASPBERRY_PI_ProcessGroundCommand();
    UtAssert_True(EventTest.MatchCount == 1, "RASPBERRY_PI_CMD_RESET_INF_EID generated (%u)",
                  (unsigned int)EventTest.MatchCount);
    /* test failure of command length */
    FcnCode = RASPBERRY_PI_RESET_COUNTERS_CC;
    Size    = sizeof(TestMsg.Config);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &Size, sizeof(Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_CheckEvent_Setup(&EventTest, RASPBERRY_PI_LEN_ERR_EID, NULL);
    RASPBERRY_PI_ProcessGroundCommand();
    UtAssert_True(EventTest.MatchCount == 1, "RASPBERRY_PI_LEN_ERR_EID generated (%u)", (unsigned int)EventTest.MatchCount);

    /* test dispatch of ENABLE */
    FcnCode = RASPBERRY_PI_ENABLE_CC;
    Size    = sizeof(TestMsg.Enable);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &Size, sizeof(Size), false);
    UT_CheckEvent_Setup(&EventTest, RASPBERRY_PI_CMD_ENABLE_INF_EID, NULL);
    RASPBERRY_PI_ProcessGroundCommand();
    // UtAssert_True(EventTest.MatchCount == 1, "RASPBERRY_PI_CMD_ENABLE_INF_EID generated (%u)",
    //               (unsigned int)EventTest.MatchCount);
    /* test failure of command length */
    FcnCode = RASPBERRY_PI_ENABLE_CC;
    Size    = sizeof(TestMsg.Config);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &Size, sizeof(Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_CheckEvent_Setup(&EventTest, RASPBERRY_PI_LEN_ERR_EID, NULL);
    RASPBERRY_PI_ProcessGroundCommand();
    UtAssert_True(EventTest.MatchCount == 1, "RASPBERRY_PI_LEN_ERR_EID generated (%u)", (unsigned int)EventTest.MatchCount);

    /* test dispatch of DISABLE */
    FcnCode = RASPBERRY_PI_DISABLE_CC;
    Size    = sizeof(TestMsg.Disable);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &Size, sizeof(Size), false);
    UT_CheckEvent_Setup(&EventTest, RASPBERRY_PI_CMD_DISABLE_INF_EID, NULL);
    RASPBERRY_PI_ProcessGroundCommand();
    // UtAssert_True(EventTest.MatchCount == 1, "RASPBERRY_PI_CMD_DISABLE_INF_EID generated (%u)",
    //               (unsigned int)EventTest.MatchCount);
    /* test failure of command length */
    FcnCode = RASPBERRY_PI_DISABLE_CC;
    Size    = sizeof(TestMsg.Config);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &Size, sizeof(Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_CheckEvent_Setup(&EventTest, RASPBERRY_PI_LEN_ERR_EID, NULL);
    RASPBERRY_PI_ProcessGroundCommand();
    UtAssert_True(EventTest.MatchCount == 1, "RASPBERRY_PI_LEN_ERR_EID generated (%u)", (unsigned int)EventTest.MatchCount);

    /* test dispatch of CONFIG */
    FcnCode = RASPBERRY_PI_CONFIG_CC;
    Size    = sizeof(TestMsg.Config);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &Size, sizeof(Size), false);
    UT_CheckEvent_Setup(&EventTest, RASPBERRY_PI_CMD_CONFIG_INF_EID, NULL);
    UT_SetDeferredRetcode(UT_KEY(RASPBERRY_PI_CommandDevice), 1, OS_ERROR);
    CFE_MSG_Message_t msgPtr;
    RASPBERRY_PI_AppData.MsgPtr = &msgPtr;
    RASPBERRY_PI_ProcessGroundCommand();
    // UtAssert_True(EventTest.MatchCount == 1, "RASPBERRY_PI_CMD_CONFIG_INF_EID generated (%u)",
    //               (unsigned int)EventTest.MatchCount);
    /* test failure of command length */
    FcnCode = RASPBERRY_PI_CONFIG_CC;
    Size    = sizeof(TestMsg.Reset);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &Size, sizeof(Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_CheckEvent_Setup(&EventTest, RASPBERRY_PI_LEN_ERR_EID, NULL);
    RASPBERRY_PI_ProcessGroundCommand();
    UtAssert_True(EventTest.MatchCount == 1, "RASPBERRY_PI_LEN_ERR_EID generated (%u)", (unsigned int)EventTest.MatchCount);

    FcnCode = RASPBERRY_PI_CONFIG_CC;
    Size    = sizeof(TestMsg.Config);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &Size, sizeof(Size), false);
    UT_CheckEvent_Setup(&EventTest, RASPBERRY_PI_CMD_CONFIG_INF_EID, NULL);
    UT_SetDeferredRetcode(UT_KEY(RASPBERRY_PI_CommandDevice), 1, OS_SUCCESS);
    RASPBERRY_PI_AppData.MsgPtr = &msgPtr;
    RASPBERRY_PI_ProcessGroundCommand();
    // UtAssert_True(EventTest.MatchCount == 1, "RASPBERRY_PI_CMD_CONFIG_INF_EID generated (%u)",
    //               (unsigned int)EventTest.MatchCount);

    /* test an invalid CC */
    FcnCode = 99;
    Size    = sizeof(TestMsg.Noop);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_CheckEvent_Setup(&EventTest, RASPBERRY_PI_CMD_ERR_EID, NULL);
    RASPBERRY_PI_ProcessGroundCommand();
    UtAssert_True(EventTest.MatchCount == 1, "RASPBERRY_PI_CMD_ERR_EID generated (%u)", (unsigned int)EventTest.MatchCount);
}

void Test_RASPBERRY_PI_ReportHousekeeping(void)
{
    /*
     * Test Case For:
     * void RASPBERRY_PI_ReportHousekeeping()
     */
    CFE_MSG_Message_t *MsgSend;
    CFE_MSG_Message_t *MsgTimestamp;
    CFE_SB_MsgId_t     MsgId = CFE_SB_ValueToMsgId(RASPBERRY_PI_REQ_HK_TLM);

    /* Set message id to return so RASPBERRY_PI_Housekeeping will be called */
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &MsgId, sizeof(MsgId), false);

    /* Set up to capture send message address */
    UT_SetDataBuffer(UT_KEY(CFE_SB_TransmitMsg), &MsgSend, sizeof(MsgSend), false);

    /* Set up to capture timestamp message address */
    UT_SetDataBuffer(UT_KEY(CFE_SB_TimeStampMsg), &MsgTimestamp, sizeof(MsgTimestamp), false);

    RASPBERRY_PI_AppData.HkTelemetryPkt.DeviceEnabled = RASPBERRY_PI_DEVICE_ENABLED;

    /* Call unit under test, NULL pointer confirms command access is through APIs */
    RASPBERRY_PI_ReportHousekeeping();

    /* Confirm message sent*/
    UtAssert_True(UT_GetStubCount(UT_KEY(CFE_SB_TransmitMsg)) == 1, "CFE_SB_TransmitMsg() called once");
    UtAssert_True(MsgSend == &RASPBERRY_PI_AppData.HkTelemetryPkt.TlmHeader.Msg,
                  "CFE_SB_TransmitMsg() address matches expected");

    /* Confirm timestamp msg address */
    UtAssert_True(UT_GetStubCount(UT_KEY(CFE_SB_TimeStampMsg)) == 1, "CFE_SB_TimeStampMsg() called once");
    UtAssert_True(MsgTimestamp == &RASPBERRY_PI_AppData.HkTelemetryPkt.TlmHeader.Msg,
                  "CFE_SB_TimeStampMsg() address matches expected");

    UT_CheckEvent_t EventTest;
    UT_SetDeferredRetcode(UT_KEY(RASPBERRY_PI_RequestHK), 1, OS_ERROR);
    RASPBERRY_PI_ReportHousekeeping();
    UT_CheckEvent_Setup(&EventTest, RASPBERRY_PI_REQ_HK_ERR_EID, "RASPBERRY_PI: Request device HK reported error -1");
}

void Test_RASPBERRY_PI_VerifyCmdLength(void)
{
    /*
     * Test Case For:
     * bool RASPBERRY_PI_VerifyCmdLength
     */
    UT_CheckEvent_t   EventTest;
    size_t            size    = 1;
    CFE_MSG_FcnCode_t fcncode = 2;
    CFE_SB_MsgId_t    msgid   = CFE_SB_ValueToMsgId(RASPBERRY_PI_CMD_MID);

    /*
     * test a match case
     */
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &size, sizeof(size), false);
    UT_CheckEvent_Setup(&EventTest, RASPBERRY_PI_LEN_ERR_EID, NULL);

    RASPBERRY_PI_VerifyCmdLength(NULL, size);

    /*
     * Confirm that the event was NOT generated
     */
    UtAssert_True(EventTest.MatchCount == 0, "RASPBERRY_PI_LEN_ERR_EID NOT generated (%u)",
                  (unsigned int)EventTest.MatchCount);

    /*
     * test a mismatch case
     */
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &size, sizeof(size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &msgid, sizeof(msgid), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &fcncode, sizeof(fcncode), false);
    UT_CheckEvent_Setup(&EventTest, RASPBERRY_PI_LEN_ERR_EID, NULL);
    RASPBERRY_PI_VerifyCmdLength(NULL, size + 1);

    /*
     * Confirm that the event WAS generated
     */
    UtAssert_True(EventTest.MatchCount == 1, "RASPBERRY_PI_LEN_ERR_EID generated (%u)", (unsigned int)EventTest.MatchCount);
}

void Test_RASPBERRY_PI_ReportDeviceTelemetry(void)
{
    RASPBERRY_PI_ReportDeviceTelemetry();

    UT_SetDeferredRetcode(UT_KEY(RASPBERRY_PI_RequestData), 1, OS_SUCCESS);
    RASPBERRY_PI_ReportDeviceTelemetry();

    UT_SetDeferredRetcode(UT_KEY(RASPBERRY_PI_RequestData), 1, OS_ERROR);
    RASPBERRY_PI_ReportDeviceTelemetry();

    RASPBERRY_PI_AppData.HkTelemetryPkt.DeviceEnabled = RASPBERRY_PI_DEVICE_DISABLED;
    RASPBERRY_PI_ReportDeviceTelemetry();

    RASPBERRY_PI_AppData.HkTelemetryPkt.DeviceHK.DeviceStatus = 1;
    RASPBERRY_PI_AppData.HkTelemetryPkt.DeviceEnabled         = RASPBERRY_PI_DEVICE_ENABLED;
    RASPBERRY_PI_ReportDeviceTelemetry();
}

void Test_RASPBERRY_PI_Configure(void)
{
    RASPBERRY_PI_Configure();

    RASPBERRY_PI_Config_cmd_t command;
    RASPBERRY_PI_AppData.MsgPtr                                     = (CFE_MSG_Message_t *)&command;
    ((RASPBERRY_PI_Config_cmd_t *)RASPBERRY_PI_AppData.MsgPtr)->DeviceCfg = 0xFFFFFFFF;
    RASPBERRY_PI_Configure();

    ((RASPBERRY_PI_Config_cmd_t *)RASPBERRY_PI_AppData.MsgPtr)->DeviceCfg = 0x0;
    RASPBERRY_PI_AppData.HkTelemetryPkt.DeviceEnabled               = RASPBERRY_PI_DEVICE_ENABLED;
    RASPBERRY_PI_Configure();

    UT_SetDeferredRetcode(UT_KEY(RASPBERRY_PI_CommandDevice), 1, OS_ERROR);
    RASPBERRY_PI_AppData.HkTelemetryPkt.DeviceEnabled = RASPBERRY_PI_DEVICE_ENABLED;
    RASPBERRY_PI_Configure();
}

void Test_RASPBERRY_PI_Enable(void)
{
    UT_CheckEvent_t EventTest;

    UT_CheckEvent_Setup(&EventTest, RASPBERRY_PI_ENABLE_INF_EID, NULL);
    RASPBERRY_PI_AppData.HkTelemetryPkt.DeviceEnabled = RASPBERRY_PI_DEVICE_DISABLED;
    UT_SetDeferredRetcode(UT_KEY(uart_init_port), 1, OS_SUCCESS);
    RASPBERRY_PI_Enable();
    UtAssert_True(EventTest.MatchCount == 1, "RASPBERRY_PI: Device enabled (%u)", (unsigned int)EventTest.MatchCount);

    UT_CheckEvent_Setup(&EventTest, RASPBERRY_PI_UART_INIT_ERR_EID, NULL);
    RASPBERRY_PI_AppData.HkTelemetryPkt.DeviceEnabled = RASPBERRY_PI_DEVICE_DISABLED;
    UT_SetDeferredRetcode(UT_KEY(uart_init_port), 1, OS_ERROR);
    RASPBERRY_PI_Enable();
    UtAssert_True(EventTest.MatchCount == 1, "RASPBERRY_PI: UART port initialization error (%u)",
                  (unsigned int)EventTest.MatchCount);

    UT_CheckEvent_Setup(&EventTest, RASPBERRY_PI_ENABLE_ERR_EID, NULL);
    RASPBERRY_PI_AppData.HkTelemetryPkt.DeviceEnabled = RASPBERRY_PI_DEVICE_ENABLED;
    UT_SetDeferredRetcode(UT_KEY(uart_init_port), 1, OS_ERROR);
    RASPBERRY_PI_Enable();
    UtAssert_True(EventTest.MatchCount == 1, "RASPBERRY_PI: Device enable failed, already enabled (%u)",
                  (unsigned int)EventTest.MatchCount);
}

void Test_RASPBERRY_PI_Disable(void)
{
    UT_CheckEvent_t EventTest;

    UT_CheckEvent_Setup(&EventTest, RASPBERRY_PI_DISABLE_INF_EID, NULL);
    RASPBERRY_PI_AppData.HkTelemetryPkt.DeviceEnabled = RASPBERRY_PI_DEVICE_ENABLED;
    UT_SetDeferredRetcode(UT_KEY(uart_close_port), 1, OS_SUCCESS);
    RASPBERRY_PI_Disable();
    UtAssert_True(EventTest.MatchCount == 1, "RASPBERRY_PI: Device disabled (%u)", (unsigned int)EventTest.MatchCount);

    UT_CheckEvent_Setup(&EventTest, RASPBERRY_PI_UART_CLOSE_ERR_EID, NULL);
    RASPBERRY_PI_AppData.HkTelemetryPkt.DeviceEnabled = RASPBERRY_PI_DEVICE_ENABLED;
    UT_SetDeferredRetcode(UT_KEY(uart_close_port), 1, OS_ERROR);
    RASPBERRY_PI_Disable();
    UtAssert_True(EventTest.MatchCount == 1, "RASPBERRY_PI: UART port close error (%u)", (unsigned int)EventTest.MatchCount);

    UT_CheckEvent_Setup(&EventTest, RASPBERRY_PI_DISABLE_ERR_EID, NULL);
    RASPBERRY_PI_AppData.HkTelemetryPkt.DeviceEnabled = RASPBERRY_PI_DEVICE_DISABLED;
    UT_SetDeferredRetcode(UT_KEY(uart_close_port), 1, OS_ERROR);
    RASPBERRY_PI_Disable();
    UtAssert_True(EventTest.MatchCount == 1, "RASPBERRY_PI: Device disable failed, already disabled (%u)",
                  (unsigned int)EventTest.MatchCount);
}

/*
 * Setup function prior to every test
 */
void Raspberry_pi_UT_Setup(void)
{
    UT_ResetState(0);
}

/*
 * Teardown function after every test
 */
void Raspberry_pi_UT_TearDown(void) {}

/*
 * Register the test cases to execute with the unit test tool
 */
void UtTest_Setup(void)
{
    ADD_TEST(RASPBERRY_PI_AppMain);
    ADD_TEST(RASPBERRY_PI_AppInit);
    ADD_TEST(RASPBERRY_PI_ProcessCommandPacket);
    ADD_TEST(RASPBERRY_PI_ProcessGroundCommand);
    ADD_TEST(RASPBERRY_PI_ReportHousekeeping);
    ADD_TEST(RASPBERRY_PI_VerifyCmdLength);
    ADD_TEST(RASPBERRY_PI_ReportDeviceTelemetry);
    ADD_TEST(RASPBERRY_PI_ProcessTelemetryRequest);
    ADD_TEST(RASPBERRY_PI_Configure);
    ADD_TEST(RASPBERRY_PI_Enable);
    ADD_TEST(RASPBERRY_PI_Disable);
}