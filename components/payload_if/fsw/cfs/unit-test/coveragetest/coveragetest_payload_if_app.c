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
** File: coveragetest_payload_if_app.c
**
** Purpose:
** Coverage Unit Test cases for the PAYLOAD_IF Application
**
** Notes:
** This implements various test cases to exercise all code
** paths through all functions defined in the PAYLOAD_IF application.
**
** It is primarily focused at providing examples of the various
** stub configurations, hook functions, and wrapper calls that
** are often needed when coercing certain code paths through
** complex functions.
*/

/*
 * Includes
 */

#include "payload_if_app_coveragetest_common.h"
#include "ut_payload_if_app.h"

/* to get the PAYLOAD_IF_LIB_Function() declaration */

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

void Test_PAYLOAD_IF_AppMain(void)
{
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;

    /*
     * Test Case For:
     * void PAYLOAD_IF_AppMain( void )
     */

    UT_CheckEvent_t EventTest;

    /*
     * PAYLOAD_IF_AppMain does not return a value,
     * but it has several internal decision points
     * that need to be exercised here.
     *
     * First call it in "nominal" mode where all
     * dependent calls should be successful by default.
     */
    PAYLOAD_IF_AppMain();

    /*
     * Confirm that CFE_ES_ExitApp() was called at the end of execution
     */
    UtAssert_True(UT_GetStubCount(UT_KEY(CFE_ES_ExitApp)) == 1, "CFE_ES_ExitApp() called");

    /*
     * Now set up individual cases for each of the error paths.
     * The first is for PAYLOAD_IF_AppInit().  As this is in the same
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
     * the PAYLOAD_IF_AppInit() failure path was taken.
     */
    PAYLOAD_IF_AppMain();

    /*
     * This can validate that the internal "RunStatus" was
     * set to CFE_ES_RunStatus_APP_ERROR, by querying the struct directly.
     *
     * It is always advisable to include the _actual_ values
     * when asserting on conditions, so if/when it fails, the
     * log will show what the incorrect value was.
     */
    UtAssert_True(PAYLOAD_IF_AppData.RunStatus == CFE_ES_RunStatus_APP_ERROR,
                  "PAYLOAD_IF_AppData.RunStatus (%lu) == CFE_ES_RunStatus_APP_ERROR",
                  (unsigned long)PAYLOAD_IF_AppData.RunStatus);

    UT_SetDeferredRetcode(UT_KEY(CFE_EVS_SendEvent), 5, CFE_EVS_INVALID_PARAMETER);
    PAYLOAD_IF_AppMain();

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
    PAYLOAD_IF_AppMain();

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
    UT_CheckEvent_Setup(&EventTest, PAYLOAD_IF_PIPE_ERR_EID, "PAYLOAD_IF: SB Pipe Read Error = %d");

    /*
     * Invoke again
     */
    PAYLOAD_IF_AppMain();

    /*
     * Confirm that the event was generated
     */
    UtAssert_True(EventTest.MatchCount == 1, "PAYLOAD_IF_PIPE_ERR_EID generated (%u)", (unsigned int)EventTest.MatchCount);
}

void Test_PAYLOAD_IF_AppInit(void)
{
    /*
     * Test Case For:
     * int32 PAYLOAD_IF_AppInit( void )
     */

    /* nominal case should return CFE_SUCCESS */
    UT_TEST_FUNCTION_RC(PAYLOAD_IF_AppInit(), CFE_SUCCESS);

    /* trigger a failure for each of the sub-calls,
     * and confirm a write to syslog for each.
     * Note that this count accumulates, because the status
     * is _not_ reset between these test cases. */
    UT_SetDeferredRetcode(UT_KEY(CFE_EVS_Register), 1, CFE_EVS_INVALID_PARAMETER);
    UT_TEST_FUNCTION_RC(PAYLOAD_IF_AppInit(), CFE_EVS_INVALID_PARAMETER);
    UtAssert_True(UT_GetStubCount(UT_KEY(CFE_ES_WriteToSysLog)) == 1, "CFE_ES_WriteToSysLog() called");

    UT_SetDeferredRetcode(UT_KEY(CFE_SB_CreatePipe), 1, CFE_SB_BAD_ARGUMENT);
    UT_TEST_FUNCTION_RC(PAYLOAD_IF_AppInit(), CFE_SB_BAD_ARGUMENT);

    UT_SetDeferredRetcode(UT_KEY(CFE_SB_Subscribe), 1, CFE_SB_BAD_ARGUMENT);
    UT_TEST_FUNCTION_RC(PAYLOAD_IF_AppInit(), CFE_SB_BAD_ARGUMENT);

    UT_SetDeferredRetcode(UT_KEY(CFE_SB_Subscribe), 2, CFE_SB_BAD_ARGUMENT);
    UT_TEST_FUNCTION_RC(PAYLOAD_IF_AppInit(), CFE_SB_BAD_ARGUMENT);

    // UT_SetDeferredRetcode(UT_KEY(CFE_EVS_SendEvent), 1, CFE_SB_BAD_ARGUMENT);
    // UT_TEST_FUNCTION_RC(PAYLOAD_IF_AppInit(), CFE_SB_BAD_ARGUMENT);
}

void Test_PAYLOAD_IF_ProcessTelemetryRequest(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    UT_CheckEvent_t   EventTest;
    CFE_MSG_FcnCode_t FcnCode;
    FcnCode = PAYLOAD_IF_REQ_DATA_TLM;

    TestMsgId = CFE_SB_ValueToMsgId(PAYLOAD_IF_CMD_MID);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDeferredRetcode(UT_KEY(PAYLOAD_IF_RequestData), 1, OS_SUCCESS);

    UT_CheckEvent_Setup(&EventTest, PAYLOAD_IF_REQ_DATA_ERR_EID, NULL);
    PAYLOAD_IF_ProcessTelemetryRequest();
    UtAssert_True(EventTest.MatchCount == 0, "PAYLOAD_IF_REQ_DATA_ERR_EID generated (%u)",
                  (unsigned int)EventTest.MatchCount);

    FcnCode = 99;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    PAYLOAD_IF_ProcessTelemetryRequest();
    UtAssert_True(EventTest.MatchCount == 0, "PAYLOAD_IF_REQ_DATA_ERR_EID generated (%u)",
                  (unsigned int)EventTest.MatchCount);
}

void Test_PAYLOAD_IF_ProcessCommandPacket(void)
{
    /*
     * Test Case For:
     * void PAYLOAD_IF_ProcessCommandPacket
     */
    /* a buffer large enough for any command message */
    union
    {
        CFE_SB_Buffer_t     SBBuf;
        PAYLOAD_IF_NoArgs_cmd_t Noop;
    } TestMsg;
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    size_t            MsgSize;
    UT_CheckEvent_t   EventTest;

    memset(&TestMsg, 0, sizeof(TestMsg));
    UT_CheckEvent_Setup(&EventTest, PAYLOAD_IF_PROCESS_CMD_ERR_EID, NULL);

    /*
     * The CFE_MSG_GetMsgId() stub uses a data buffer to hold the
     * message ID values to return.
     */
    TestMsgId = CFE_SB_ValueToMsgId(PAYLOAD_IF_CMD_MID);
    FcnCode   = PAYLOAD_IF_NOOP_CC;
    MsgSize   = sizeof(TestMsg.Noop);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);
    PAYLOAD_IF_ProcessCommandPacket();
    UtAssert_True(EventTest.MatchCount == 0, "PAYLOAD_IF_CMD_ERR_EID not generated (%u)",
                  (unsigned int)EventTest.MatchCount);

    TestMsgId = CFE_SB_ValueToMsgId(PAYLOAD_IF_REQ_HK_MID);
    FcnCode   = PAYLOAD_IF_REQ_HK_TLM;
    MsgSize   = sizeof(TestMsg.Noop);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);
    PAYLOAD_IF_ProcessCommandPacket();
    UtAssert_True(EventTest.MatchCount == 0, "PAYLOAD_IF_CMD_ERR_EID not generated (%u)",
                  (unsigned int)EventTest.MatchCount);

    /* invalid message id */
    TestMsgId = CFE_SB_INVALID_MSG_ID;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    PAYLOAD_IF_ProcessCommandPacket();
    UtAssert_True(EventTest.MatchCount == 1, "PAYLOAD_IF_CMD_ERR_EID generated (%u)", (unsigned int)EventTest.MatchCount);
}

void Test_PAYLOAD_IF_ProcessGroundCommand(void)
{
    /*
     * Test Case For:
     * void PAYLOAD_IF_ProcessGroundCommand
     */
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(PAYLOAD_IF_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode;
    size_t            Size;

    /* a buffer large enough for any command message */
    union
    {
        CFE_SB_Buffer_t     SBBuf;
        PAYLOAD_IF_NoArgs_cmd_t Noop;
        PAYLOAD_IF_NoArgs_cmd_t Reset;
        PAYLOAD_IF_NoArgs_cmd_t Enable;
        PAYLOAD_IF_NoArgs_cmd_t Disable;
        PAYLOAD_IF_Config_cmd_t Config;
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
    FcnCode = PAYLOAD_IF_NOOP_CC;
    Size    = sizeof(TestMsg.Noop);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &Size, sizeof(Size), false);
    UT_CheckEvent_Setup(&EventTest, PAYLOAD_IF_CMD_NOOP_INF_EID, NULL);
    PAYLOAD_IF_ProcessGroundCommand();
    UtAssert_True(EventTest.MatchCount == 1, "PAYLOAD_IF_CMD_NOOP_INF_EID generated (%u)",
                  (unsigned int)EventTest.MatchCount);
    /* test failure of command length */
    FcnCode = PAYLOAD_IF_NOOP_CC;
    Size    = sizeof(TestMsg.Config);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &Size, sizeof(Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_CheckEvent_Setup(&EventTest, PAYLOAD_IF_LEN_ERR_EID, NULL);
    PAYLOAD_IF_ProcessGroundCommand();
    UtAssert_True(EventTest.MatchCount == 1, "PAYLOAD_IF_LEN_ERR_EID generated (%u)", (unsigned int)EventTest.MatchCount);

    /* test dispatch of RESET */
    FcnCode = PAYLOAD_IF_RESET_COUNTERS_CC;
    Size    = sizeof(TestMsg.Reset);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &Size, sizeof(Size), false);
    UT_CheckEvent_Setup(&EventTest, PAYLOAD_IF_CMD_RESET_INF_EID, NULL);
    PAYLOAD_IF_ProcessGroundCommand();
    UtAssert_True(EventTest.MatchCount == 1, "PAYLOAD_IF_CMD_RESET_INF_EID generated (%u)",
                  (unsigned int)EventTest.MatchCount);
    /* test failure of command length */
    FcnCode = PAYLOAD_IF_RESET_COUNTERS_CC;
    Size    = sizeof(TestMsg.Config);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &Size, sizeof(Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_CheckEvent_Setup(&EventTest, PAYLOAD_IF_LEN_ERR_EID, NULL);
    PAYLOAD_IF_ProcessGroundCommand();
    UtAssert_True(EventTest.MatchCount == 1, "PAYLOAD_IF_LEN_ERR_EID generated (%u)", (unsigned int)EventTest.MatchCount);

    /* test dispatch of ENABLE */
    FcnCode = PAYLOAD_IF_ENABLE_CC;
    Size    = sizeof(TestMsg.Enable);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &Size, sizeof(Size), false);
    UT_CheckEvent_Setup(&EventTest, PAYLOAD_IF_CMD_ENABLE_INF_EID, NULL);
    PAYLOAD_IF_ProcessGroundCommand();
    // UtAssert_True(EventTest.MatchCount == 1, "PAYLOAD_IF_CMD_ENABLE_INF_EID generated (%u)",
    //               (unsigned int)EventTest.MatchCount);
    /* test failure of command length */
    FcnCode = PAYLOAD_IF_ENABLE_CC;
    Size    = sizeof(TestMsg.Config);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &Size, sizeof(Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_CheckEvent_Setup(&EventTest, PAYLOAD_IF_LEN_ERR_EID, NULL);
    PAYLOAD_IF_ProcessGroundCommand();
    UtAssert_True(EventTest.MatchCount == 1, "PAYLOAD_IF_LEN_ERR_EID generated (%u)", (unsigned int)EventTest.MatchCount);

    /* test dispatch of DISABLE */
    FcnCode = PAYLOAD_IF_DISABLE_CC;
    Size    = sizeof(TestMsg.Disable);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &Size, sizeof(Size), false);
    UT_CheckEvent_Setup(&EventTest, PAYLOAD_IF_CMD_DISABLE_INF_EID, NULL);
    PAYLOAD_IF_ProcessGroundCommand();
    // UtAssert_True(EventTest.MatchCount == 1, "PAYLOAD_IF_CMD_DISABLE_INF_EID generated (%u)",
    //               (unsigned int)EventTest.MatchCount);
    /* test failure of command length */
    FcnCode = PAYLOAD_IF_DISABLE_CC;
    Size    = sizeof(TestMsg.Config);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &Size, sizeof(Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_CheckEvent_Setup(&EventTest, PAYLOAD_IF_LEN_ERR_EID, NULL);
    PAYLOAD_IF_ProcessGroundCommand();
    UtAssert_True(EventTest.MatchCount == 1, "PAYLOAD_IF_LEN_ERR_EID generated (%u)", (unsigned int)EventTest.MatchCount);

    /* test dispatch of CONFIG */
    FcnCode = PAYLOAD_IF_CONFIG_CC;
    Size    = sizeof(TestMsg.Config);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &Size, sizeof(Size), false);
    UT_CheckEvent_Setup(&EventTest, PAYLOAD_IF_CMD_CONFIG_INF_EID, NULL);
    UT_SetDeferredRetcode(UT_KEY(PAYLOAD_IF_CommandDevice), 1, OS_ERROR);
    CFE_MSG_Message_t msgPtr;
    PAYLOAD_IF_AppData.MsgPtr = &msgPtr;
    PAYLOAD_IF_ProcessGroundCommand();
    // UtAssert_True(EventTest.MatchCount == 1, "PAYLOAD_IF_CMD_CONFIG_INF_EID generated (%u)",
    //               (unsigned int)EventTest.MatchCount);
    /* test failure of command length */
    FcnCode = PAYLOAD_IF_CONFIG_CC;
    Size    = sizeof(TestMsg.Reset);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &Size, sizeof(Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_CheckEvent_Setup(&EventTest, PAYLOAD_IF_LEN_ERR_EID, NULL);
    PAYLOAD_IF_ProcessGroundCommand();
    UtAssert_True(EventTest.MatchCount == 1, "PAYLOAD_IF_LEN_ERR_EID generated (%u)", (unsigned int)EventTest.MatchCount);

    FcnCode = PAYLOAD_IF_CONFIG_CC;
    Size    = sizeof(TestMsg.Config);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &Size, sizeof(Size), false);
    UT_CheckEvent_Setup(&EventTest, PAYLOAD_IF_CMD_CONFIG_INF_EID, NULL);
    UT_SetDeferredRetcode(UT_KEY(PAYLOAD_IF_CommandDevice), 1, OS_SUCCESS);
    PAYLOAD_IF_AppData.MsgPtr = &msgPtr;
    PAYLOAD_IF_ProcessGroundCommand();
    // UtAssert_True(EventTest.MatchCount == 1, "PAYLOAD_IF_CMD_CONFIG_INF_EID generated (%u)",
    //               (unsigned int)EventTest.MatchCount);

    /* test an invalid CC */
    FcnCode = 99;
    Size    = sizeof(TestMsg.Noop);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_CheckEvent_Setup(&EventTest, PAYLOAD_IF_CMD_ERR_EID, NULL);
    PAYLOAD_IF_ProcessGroundCommand();
    UtAssert_True(EventTest.MatchCount == 1, "PAYLOAD_IF_CMD_ERR_EID generated (%u)", (unsigned int)EventTest.MatchCount);
}

void Test_PAYLOAD_IF_ReportHousekeeping(void)
{
    /*
     * Test Case For:
     * void PAYLOAD_IF_ReportHousekeeping()
     */
    CFE_MSG_Message_t *MsgSend;
    CFE_MSG_Message_t *MsgTimestamp;
    CFE_SB_MsgId_t     MsgId = CFE_SB_ValueToMsgId(PAYLOAD_IF_REQ_HK_TLM);

    /* Set message id to return so PAYLOAD_IF_Housekeeping will be called */
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &MsgId, sizeof(MsgId), false);

    /* Set up to capture send message address */
    UT_SetDataBuffer(UT_KEY(CFE_SB_TransmitMsg), &MsgSend, sizeof(MsgSend), false);

    /* Set up to capture timestamp message address */
    UT_SetDataBuffer(UT_KEY(CFE_SB_TimeStampMsg), &MsgTimestamp, sizeof(MsgTimestamp), false);

    PAYLOAD_IF_AppData.HkTelemetryPkt.DeviceEnabled = PAYLOAD_IF_DEVICE_ENABLED;

    /* Call unit under test, NULL pointer confirms command access is through APIs */
    PAYLOAD_IF_ReportHousekeeping();

    /* Confirm message sent*/
    UtAssert_True(UT_GetStubCount(UT_KEY(CFE_SB_TransmitMsg)) == 1, "CFE_SB_TransmitMsg() called once");
    UtAssert_True(MsgSend == &PAYLOAD_IF_AppData.HkTelemetryPkt.TlmHeader.Msg,
                  "CFE_SB_TransmitMsg() address matches expected");

    /* Confirm timestamp msg address */
    UtAssert_True(UT_GetStubCount(UT_KEY(CFE_SB_TimeStampMsg)) == 1, "CFE_SB_TimeStampMsg() called once");
    UtAssert_True(MsgTimestamp == &PAYLOAD_IF_AppData.HkTelemetryPkt.TlmHeader.Msg,
                  "CFE_SB_TimeStampMsg() address matches expected");

    UT_CheckEvent_t EventTest;
    UT_SetDeferredRetcode(UT_KEY(PAYLOAD_IF_RequestHK), 1, OS_ERROR);
    PAYLOAD_IF_ReportHousekeeping();
    UT_CheckEvent_Setup(&EventTest, PAYLOAD_IF_REQ_HK_ERR_EID, "PAYLOAD_IF: Request device HK reported error -1");
}

void Test_PAYLOAD_IF_VerifyCmdLength(void)
{
    /*
     * Test Case For:
     * bool PAYLOAD_IF_VerifyCmdLength
     */
    UT_CheckEvent_t   EventTest;
    size_t            size    = 1;
    CFE_MSG_FcnCode_t fcncode = 2;
    CFE_SB_MsgId_t    msgid   = CFE_SB_ValueToMsgId(PAYLOAD_IF_CMD_MID);

    /*
     * test a match case
     */
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &size, sizeof(size), false);
    UT_CheckEvent_Setup(&EventTest, PAYLOAD_IF_LEN_ERR_EID, NULL);

    PAYLOAD_IF_VerifyCmdLength(NULL, size);

    /*
     * Confirm that the event was NOT generated
     */
    UtAssert_True(EventTest.MatchCount == 0, "PAYLOAD_IF_LEN_ERR_EID NOT generated (%u)",
                  (unsigned int)EventTest.MatchCount);

    /*
     * test a mismatch case
     */
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &size, sizeof(size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &msgid, sizeof(msgid), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &fcncode, sizeof(fcncode), false);
    UT_CheckEvent_Setup(&EventTest, PAYLOAD_IF_LEN_ERR_EID, NULL);
    PAYLOAD_IF_VerifyCmdLength(NULL, size + 1);

    /*
     * Confirm that the event WAS generated
     */
    UtAssert_True(EventTest.MatchCount == 1, "PAYLOAD_IF_LEN_ERR_EID generated (%u)", (unsigned int)EventTest.MatchCount);
}

void Test_PAYLOAD_IF_ReportDeviceTelemetry(void)
{
    PAYLOAD_IF_ReportDeviceTelemetry();

    UT_SetDeferredRetcode(UT_KEY(PAYLOAD_IF_RequestData), 1, OS_SUCCESS);
    PAYLOAD_IF_ReportDeviceTelemetry();

    UT_SetDeferredRetcode(UT_KEY(PAYLOAD_IF_RequestData), 1, OS_ERROR);
    PAYLOAD_IF_ReportDeviceTelemetry();

    PAYLOAD_IF_AppData.HkTelemetryPkt.DeviceEnabled = PAYLOAD_IF_DEVICE_DISABLED;
    PAYLOAD_IF_ReportDeviceTelemetry();

    PAYLOAD_IF_AppData.HkTelemetryPkt.DeviceHK.DeviceStatus = 1;
    PAYLOAD_IF_AppData.HkTelemetryPkt.DeviceEnabled         = PAYLOAD_IF_DEVICE_ENABLED;
    PAYLOAD_IF_ReportDeviceTelemetry();
}

void Test_PAYLOAD_IF_Configure(void)
{
    PAYLOAD_IF_Configure();

    PAYLOAD_IF_Config_cmd_t command;
    PAYLOAD_IF_AppData.MsgPtr                                     = (CFE_MSG_Message_t *)&command;
    ((PAYLOAD_IF_Config_cmd_t *)PAYLOAD_IF_AppData.MsgPtr)->DeviceCfg = 0xFFFFFFFF;
    PAYLOAD_IF_Configure();

    ((PAYLOAD_IF_Config_cmd_t *)PAYLOAD_IF_AppData.MsgPtr)->DeviceCfg = 0x0;
    PAYLOAD_IF_AppData.HkTelemetryPkt.DeviceEnabled               = PAYLOAD_IF_DEVICE_ENABLED;
    PAYLOAD_IF_Configure();

    UT_SetDeferredRetcode(UT_KEY(PAYLOAD_IF_CommandDevice), 1, OS_ERROR);
    PAYLOAD_IF_AppData.HkTelemetryPkt.DeviceEnabled = PAYLOAD_IF_DEVICE_ENABLED;
    PAYLOAD_IF_Configure();
}

void Test_PAYLOAD_IF_Enable(void)
{
    UT_CheckEvent_t EventTest;

    UT_CheckEvent_Setup(&EventTest, PAYLOAD_IF_ENABLE_INF_EID, NULL);
    PAYLOAD_IF_AppData.HkTelemetryPkt.DeviceEnabled = PAYLOAD_IF_DEVICE_DISABLED;
    UT_SetDeferredRetcode(UT_KEY(uart_init_port), 1, OS_SUCCESS);
    PAYLOAD_IF_Enable();
    UtAssert_True(EventTest.MatchCount == 1, "PAYLOAD_IF: Device enabled (%u)", (unsigned int)EventTest.MatchCount);

    UT_CheckEvent_Setup(&EventTest, PAYLOAD_IF_UART_INIT_ERR_EID, NULL);
    PAYLOAD_IF_AppData.HkTelemetryPkt.DeviceEnabled = PAYLOAD_IF_DEVICE_DISABLED;
    UT_SetDeferredRetcode(UT_KEY(uart_init_port), 1, OS_ERROR);
    PAYLOAD_IF_Enable();
    UtAssert_True(EventTest.MatchCount == 1, "PAYLOAD_IF: UART port initialization error (%u)",
                  (unsigned int)EventTest.MatchCount);

    UT_CheckEvent_Setup(&EventTest, PAYLOAD_IF_ENABLE_ERR_EID, NULL);
    PAYLOAD_IF_AppData.HkTelemetryPkt.DeviceEnabled = PAYLOAD_IF_DEVICE_ENABLED;
    UT_SetDeferredRetcode(UT_KEY(uart_init_port), 1, OS_ERROR);
    PAYLOAD_IF_Enable();
    UtAssert_True(EventTest.MatchCount == 1, "PAYLOAD_IF: Device enable failed, already enabled (%u)",
                  (unsigned int)EventTest.MatchCount);
}

void Test_PAYLOAD_IF_Disable(void)
{
    UT_CheckEvent_t EventTest;

    UT_CheckEvent_Setup(&EventTest, PAYLOAD_IF_DISABLE_INF_EID, NULL);
    PAYLOAD_IF_AppData.HkTelemetryPkt.DeviceEnabled = PAYLOAD_IF_DEVICE_ENABLED;
    UT_SetDeferredRetcode(UT_KEY(uart_close_port), 1, OS_SUCCESS);
    PAYLOAD_IF_Disable();
    UtAssert_True(EventTest.MatchCount == 1, "PAYLOAD_IF: Device disabled (%u)", (unsigned int)EventTest.MatchCount);

    UT_CheckEvent_Setup(&EventTest, PAYLOAD_IF_UART_CLOSE_ERR_EID, NULL);
    PAYLOAD_IF_AppData.HkTelemetryPkt.DeviceEnabled = PAYLOAD_IF_DEVICE_ENABLED;
    UT_SetDeferredRetcode(UT_KEY(uart_close_port), 1, OS_ERROR);
    PAYLOAD_IF_Disable();
    UtAssert_True(EventTest.MatchCount == 1, "PAYLOAD_IF: UART port close error (%u)", (unsigned int)EventTest.MatchCount);

    UT_CheckEvent_Setup(&EventTest, PAYLOAD_IF_DISABLE_ERR_EID, NULL);
    PAYLOAD_IF_AppData.HkTelemetryPkt.DeviceEnabled = PAYLOAD_IF_DEVICE_DISABLED;
    UT_SetDeferredRetcode(UT_KEY(uart_close_port), 1, OS_ERROR);
    PAYLOAD_IF_Disable();
    UtAssert_True(EventTest.MatchCount == 1, "PAYLOAD_IF: Device disable failed, already disabled (%u)",
                  (unsigned int)EventTest.MatchCount);
}

void Test_PAYLOAD_IF_HandleDecodedFrame_LengthMismatch(void)
{
    /*
     * Test Case For:
     * int32 PAYLOAD_IF_HandleDecodedFrame(void)
     * A CCSDS length field that disagrees with the actual body length
     * must be rejected before anything is published.
     */
    int32 result;

    memset(&PAYLOAD_IF_AppData.DecodeCtx, 0, sizeof(PAYLOAD_IF_AppData.DecodeCtx));
    PAYLOAD_IF_AppData.DecodeCtx.body[0] = 0x00;
    PAYLOAD_IF_AppData.DecodeCtx.body[1] = 0x10; /* APID 0x010 */
    PAYLOAD_IF_AppData.DecodeCtx.body[4] = 0x00;
    PAYLOAD_IF_AppData.DecodeCtx.body[5] = 0x63; /* claims 99 extra bytes, way more than we have */
    PAYLOAD_IF_AppData.DecodeCtx.body_len = 7;   /* actual body is only 7 bytes */

    result = PAYLOAD_IF_HandleDecodedFrame();

    UtAssert_INT32_EQ(result, OS_ERROR);
}

void Test_PAYLOAD_IF_HandleDecodedFrame_DisallowedApid(void)
{
    /*
     * Test Case For:
     * int32 PAYLOAD_IF_HandleDecodedFrame(void)
     * An APID that is not on the PayOBC->BusOBC allowlist (e.g. one that
     * only exists in the other direction) must be rejected.
     */
    int32 result;

    memset(&PAYLOAD_IF_AppData.DecodeCtx, 0, sizeof(PAYLOAD_IF_AppData.DecodeCtx));
    /* 0x010 is BusOBC->PayOBC only (PAYLOAD_COMMAND); not allowed inbound */
    PAYLOAD_IF_AppData.DecodeCtx.body[0] = 0x00;
    PAYLOAD_IF_AppData.DecodeCtx.body[1] = 0x10;
    PAYLOAD_IF_AppData.DecodeCtx.body[4] = 0x00;
    PAYLOAD_IF_AppData.DecodeCtx.body[5] = 0x00; /* length agrees: 7 - 7 = 0 */
    PAYLOAD_IF_AppData.DecodeCtx.body_len = 7;

    result = PAYLOAD_IF_HandleDecodedFrame();

    UtAssert_INT32_EQ(result, OS_ERROR);
}

void Test_PAYLOAD_IF_HandleDecodedFrame_AllowedApid(void)
{
    /*
     * Test Case For:
     * int32 PAYLOAD_IF_HandleDecodedFrame(void)
     * A properly formed, allowed inbound APID (0x011, PAYLOAD_TELEMETRY)
     * should be accepted and published.
     */
    int32 result;
    CFE_SB_Buffer_t StubBuf;

    memset(&PAYLOAD_IF_AppData.DecodeCtx, 0, sizeof(PAYLOAD_IF_AppData.DecodeCtx));
    PAYLOAD_IF_AppData.DecodeCtx.body[0] = 0x00;
    PAYLOAD_IF_AppData.DecodeCtx.body[1] = 0x11; /* APID 0x011, PayOBC->BusOBC telemetry */
    PAYLOAD_IF_AppData.DecodeCtx.body[4] = 0x00;
    PAYLOAD_IF_AppData.DecodeCtx.body[5] = 0x00; /* length agrees: 7 - 7 = 0 */
    PAYLOAD_IF_AppData.DecodeCtx.body_len = 7;

    CFE_SB_Buffer_t *StubBufPtr = &StubBuf;
    UT_SetDataBuffer(UT_KEY(CFE_SB_AllocateMessageBuffer), &StubBufPtr, sizeof(StubBufPtr), false);
    UT_SetDefaultReturnValue(UT_KEY(CFE_SB_TransmitBuffer), CFE_SUCCESS);

    result = PAYLOAD_IF_HandleDecodedFrame();

    UtAssert_INT32_EQ(result, OS_SUCCESS);
}

/* Captures the bytes actually passed to uart_write_port for comparison */
static uint8_t Captured_UartWriteData[PL_MAX_FRAME_LEN];
static size_t  Captured_UartWriteLen;

static void UartWriteCapture_Hook(void *UserObj, UT_EntryKey_t FuncKey, const UT_StubContext_t *Context)
{
    uint8_t *data           = UT_Hook_GetArgValueByName(Context, "data", uint8_t *);
    uint32_t       numBytes = UT_Hook_GetArgValueByName(Context, "numBytes", uint32_t);

    Captured_UartWriteLen = numBytes;
    if (numBytes <= sizeof(Captured_UartWriteData))
    {
        memcpy(Captured_UartWriteData, data, numBytes);
    }
}

void Test_PAYLOAD_IF_SendToPayload_ValidPacket(void)
{
    /*
     * Test Case For:
     * void PAYLOAD_IF_SendToPayload(void)
     * A valid outbound message must be encoded and the exact frame bytes
     * must reach uart_write_port unmodified.
     */
    uint8_t test_msg[7] = {0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0xAB};
    size_t  msg_size    = sizeof(test_msg);
    uint8_t expected_frame[PL_MAX_FRAME_LEN];
    size_t  expected_frame_len;

    PAYLOAD_IF_AppData.MsgPtr = (CFE_MSG_Message_t *)test_msg;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &msg_size, sizeof(msg_size), false);
    UT_SetHandlerFunction(UT_KEY(uart_write_port), UartWriteCapture_Hook, NULL);
    UT_SetDeferredRetcode(UT_KEY(uart_write_port), 1, (int32_t)msg_size + 4 + 2 + 2);

    expected_frame_len = plframe_encode(test_msg, msg_size, expected_frame);

    PAYLOAD_IF_SendToPayload();

    UtAssert_INT32_EQ((int32)Captured_UartWriteLen, (int32)expected_frame_len);
    UtAssert_True(memcmp(Captured_UartWriteData, expected_frame, expected_frame_len) == 0,
                  "UART write bytes match expected encoded frame exactly");
}

void Test_PAYLOAD_IF_SendToPayload_OversizedRejected(void)
{
    /*
     * Test Case For:
     * void PAYLOAD_IF_SendToPayload(void)
     * A message larger than PL_MAX_BODY_LEN must be rejected before any
     * encode or UART write is attempted.
     */
    size_t oversized = PL_MAX_BODY_LEN + 1;
    int    call_count_before;
    int    call_count_after;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &oversized, sizeof(oversized), false);

    call_count_before = UT_GetStubCount(UT_KEY(uart_write_port));

    PAYLOAD_IF_SendToPayload();

    call_count_after = UT_GetStubCount(UT_KEY(uart_write_port));

    UtAssert_INT32_EQ(call_count_after, call_count_before);
}

void Test_PAYLOAD_IF_SendToPayload_MinSizeAccepted(void)
{
    /*
     * Test Case For:
     * void PAYLOAD_IF_SendToPayload(void)
     * The smallest allowed body size (PL_MIN_BODY_LEN) must be accepted
     * and actually written to UART, not rejected as too small.
     */
    size_t   msg_size = PL_MIN_BODY_LEN;
    int32_t  write_retcode = (int32_t)msg_size + 4 + 2 + 2;
    int      call_count_before;
    int      call_count_after;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &msg_size, sizeof(msg_size), false);
    UT_SetDeferredRetcode(UT_KEY(uart_write_port), 1, write_retcode);

    call_count_before = UT_GetStubCount(UT_KEY(uart_write_port));
    PAYLOAD_IF_SendToPayload();
    call_count_after = UT_GetStubCount(UT_KEY(uart_write_port));

    UtAssert_INT32_EQ(call_count_after, call_count_before + 1);
}

void Test_PAYLOAD_IF_SendToPayload_MaxSizeAccepted(void)
{
    /*
     * Test Case For:
     * void PAYLOAD_IF_SendToPayload(void)
     * The largest allowed body size (PL_MAX_BODY_LEN) must be accepted
     * and actually written to UART, not rejected as too large.
     */
    size_t   msg_size = PL_MAX_BODY_LEN;
    int32_t  write_retcode = (int32_t)msg_size + 4 + 2 + 2;
    int      call_count_before;
    int      call_count_after;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &msg_size, sizeof(msg_size), false);
    UT_SetDeferredRetcode(UT_KEY(uart_write_port), 1, write_retcode);

    call_count_before = UT_GetStubCount(UT_KEY(uart_write_port));
    PAYLOAD_IF_SendToPayload();
    call_count_after = UT_GetStubCount(UT_KEY(uart_write_port));

    UtAssert_INT32_EQ(call_count_after, call_count_before + 1);
}

void Test_PAYLOAD_IF_HandleDecodedFrame_TransmitFailure(void)
{
    /*
     * Test Case For:
     * int32 PAYLOAD_IF_HandleDecodedFrame(void)
     * If CFE_SB_TransmitBuffer fails after a successful allocation, the
     * buffer must be released via CFE_SB_ReleaseMessageBuffer and the
     * function must report failure rather than crashing or leaking.
     */
    int32 result;
    CFE_SB_Buffer_t  StubBuf;
    CFE_SB_Buffer_t *StubBufPtr = &StubBuf;
    int              release_count_before;
    int              release_count_after;

    memset(&PAYLOAD_IF_AppData.DecodeCtx, 0, sizeof(PAYLOAD_IF_AppData.DecodeCtx));
    PAYLOAD_IF_AppData.DecodeCtx.body[0] = 0x00;
    PAYLOAD_IF_AppData.DecodeCtx.body[1] = 0x11; /* allowed APID */
    PAYLOAD_IF_AppData.DecodeCtx.body[4] = 0x00;
    PAYLOAD_IF_AppData.DecodeCtx.body[5] = 0x00;
    PAYLOAD_IF_AppData.DecodeCtx.body_len = 7;

    UT_SetDataBuffer(UT_KEY(CFE_SB_AllocateMessageBuffer), &StubBufPtr, sizeof(StubBufPtr), false);
    UT_SetDefaultReturnValue(UT_KEY(CFE_SB_TransmitBuffer), CFE_SB_BAD_ARGUMENT);

    release_count_before = UT_GetStubCount(UT_KEY(CFE_SB_ReleaseMessageBuffer));

    result = PAYLOAD_IF_HandleDecodedFrame();

    release_count_after = UT_GetStubCount(UT_KEY(CFE_SB_ReleaseMessageBuffer));

    UtAssert_INT32_EQ(result, OS_ERROR);
    UtAssert_INT32_EQ(release_count_after, release_count_before + 1);
}

void Test_PAYLOAD_IF_SendToPayload_PartialWriteFails(void)
{
    /*
     * Test Case For:
     * void PAYLOAD_IF_SendToPayload(void)
     * A UART write that returns fewer bytes than the full frame size must
     * be counted as a failure, not treated as a successful send.
     */
    uint8_t test_msg[7] = {0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0xAB};
    size_t  msg_size    = sizeof(test_msg);
    uint32  err_count_before;
    uint32  err_count_after;

    PAYLOAD_IF_AppData.MsgPtr = (CFE_MSG_Message_t *)test_msg;
    PAYLOAD_IF_AppData.HkTelemetryPkt.CommandErrorCount = 0;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &msg_size, sizeof(msg_size), false);
    /* Full frame would be 7+4+2+2=15 bytes; simulate only 5 bytes written */
    UT_SetDeferredRetcode(UT_KEY(uart_write_port), 1, 5);

    err_count_before = PAYLOAD_IF_AppData.HkTelemetryPkt.CommandErrorCount;
    PAYLOAD_IF_SendToPayload();
    err_count_after = PAYLOAD_IF_AppData.HkTelemetryPkt.CommandErrorCount;

    UtAssert_INT32_EQ((int32)err_count_after, (int32)(err_count_before + 1));
}

void Test_PAYLOAD_IF_SendToPayload_FullWriteSucceeds(void)
{
    /*
     * Test Case For:
     * void PAYLOAD_IF_SendToPayload(void)
     * A UART write that returns exactly the full frame size must be
     * counted as a success.
     */
    uint8_t test_msg[7] = {0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0xAB};
    size_t  msg_size    = sizeof(test_msg);
    uint32  ok_count_before;
    uint32  ok_count_after;

    PAYLOAD_IF_AppData.MsgPtr = (CFE_MSG_Message_t *)test_msg;
    PAYLOAD_IF_AppData.HkTelemetryPkt.CommandCount = 0;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &msg_size, sizeof(msg_size), false);
    UT_SetDeferredRetcode(UT_KEY(uart_write_port), 1, (int32_t)msg_size + 4 + 2 + 2);

    ok_count_before = PAYLOAD_IF_AppData.HkTelemetryPkt.CommandCount;
    PAYLOAD_IF_SendToPayload();
    ok_count_after = PAYLOAD_IF_AppData.HkTelemetryPkt.CommandCount;

    UtAssert_INT32_EQ((int32)ok_count_after, (int32)(ok_count_before + 1));
}

void Test_PAYLOAD_IF_ResetCounters_ClearsAll(void)
{
    /*
     * Test Case For:
     * void PAYLOAD_IF_ResetCounters(void)
     * All link-layer and command counters must return to zero after a
     * reset, even if they were previously nonzero.
     */
    PAYLOAD_IF_AppData.HkTelemetryPkt.CommandCount      = 5;
    PAYLOAD_IF_AppData.HkTelemetryPkt.CommandErrorCount = 3;
    PAYLOAD_IF_AppData.HkTelemetryPkt.DeviceCount       = 7;
    PAYLOAD_IF_AppData.HkTelemetryPkt.DeviceErrorCount  = 2;

    PAYLOAD_IF_ResetCounters();

    UtAssert_INT32_EQ((int32)PAYLOAD_IF_AppData.HkTelemetryPkt.CommandCount, 0);
    UtAssert_INT32_EQ((int32)PAYLOAD_IF_AppData.HkTelemetryPkt.CommandErrorCount, 0);
    UtAssert_INT32_EQ((int32)PAYLOAD_IF_AppData.HkTelemetryPkt.DeviceCount, 0);
    UtAssert_INT32_EQ((int32)PAYLOAD_IF_AppData.HkTelemetryPkt.DeviceErrorCount, 0);
}

void Test_PAYLOAD_IF_ReportHousekeeping_ReflectsCounters(void)
{
    /*
     * Test Case For:
     * void PAYLOAD_IF_ReportHousekeeping(void)
     * The published housekeeping packet must reflect whatever counter
     * values are currently set, since reporting no longer polls the
     * device -- it just publishes existing state.
     */
    PAYLOAD_IF_AppData.HkTelemetryPkt.CommandCount      = 11;
    PAYLOAD_IF_AppData.HkTelemetryPkt.DeviceErrorCount  = 4;

    PAYLOAD_IF_ReportHousekeeping();

    UtAssert_INT32_EQ((int32)PAYLOAD_IF_AppData.HkTelemetryPkt.CommandCount, 11);
    UtAssert_INT32_EQ((int32)PAYLOAD_IF_AppData.HkTelemetryPkt.DeviceErrorCount, 4);
    UtAssert_STUB_COUNT(CFE_SB_TransmitMsg, 1);
}

/*
 * Setup function prior to every test
 */
void Payload_if_UT_Setup(void)
{
    UT_ResetState(0);
}

/*
 * Teardown function after every test
 */
void Payload_if_UT_TearDown(void) {}

/*
 * Register the test cases to execute with the unit test tool
 */
void UtTest_Setup(void)
{
    ADD_TEST(PAYLOAD_IF_AppMain);
    ADD_TEST(PAYLOAD_IF_AppInit);
    ADD_TEST(PAYLOAD_IF_ProcessCommandPacket);
    ADD_TEST(PAYLOAD_IF_ProcessGroundCommand);
    ADD_TEST(PAYLOAD_IF_ReportHousekeeping);
    ADD_TEST(PAYLOAD_IF_VerifyCmdLength);
    ADD_TEST(PAYLOAD_IF_ReportDeviceTelemetry);
    ADD_TEST(PAYLOAD_IF_ProcessTelemetryRequest);
    ADD_TEST(PAYLOAD_IF_Configure);
    ADD_TEST(PAYLOAD_IF_Enable);
    ADD_TEST(PAYLOAD_IF_Disable);
    ADD_TEST(PAYLOAD_IF_HandleDecodedFrame_LengthMismatch);
    ADD_TEST(PAYLOAD_IF_HandleDecodedFrame_DisallowedApid);
    ADD_TEST(PAYLOAD_IF_HandleDecodedFrame_AllowedApid);
    ADD_TEST(PAYLOAD_IF_SendToPayload_ValidPacket);
    ADD_TEST(PAYLOAD_IF_SendToPayload_OversizedRejected);
    ADD_TEST(PAYLOAD_IF_SendToPayload_MinSizeAccepted);
    ADD_TEST(PAYLOAD_IF_SendToPayload_MaxSizeAccepted);
    ADD_TEST(PAYLOAD_IF_HandleDecodedFrame_TransmitFailure);
    ADD_TEST(PAYLOAD_IF_SendToPayload_PartialWriteFails);
    ADD_TEST(PAYLOAD_IF_SendToPayload_FullWriteSucceeds);
    ADD_TEST(PAYLOAD_IF_ResetCounters_ClearsAll);
    ADD_TEST(PAYLOAD_IF_ReportHousekeeping_ReflectsCounters);
}