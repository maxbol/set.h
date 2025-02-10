/* AUTOGENERATED FILE. DO NOT EDIT. */

/*=======Automagically Detected Files To Include=====*/
#include "unity.h"
#include "set.h"
#include <stdint.h>

/*=======External Functions This Runner Calls=====*/
extern void setUp(void);
extern void tearDown(void);
extern void test_addr_idx_conversions(void);
extern void test_initing_set(void);
extern void test_add_first_member(void);
extern void test_add_second_larger_member(void);
extern void test_add_second_smaller_member(void);
extern void test_add_third_member_line(void);
extern void test_add_third_member_triangle(void);
extern void test_insert_sizeidentity(void);


/*=======Mock Management=====*/
static void CMock_Init(void)
{
}
static void CMock_Verify(void)
{
}
static void CMock_Destroy(void)
{
}

/*=======Test Reset Options=====*/
void resetTest(void);
void resetTest(void)
{
  tearDown();
  CMock_Verify();
  CMock_Destroy();
  CMock_Init();
  setUp();
}
void verifyTest(void);
void verifyTest(void)
{
  CMock_Verify();
}

/*=======Test Runner Used To Run Each Test=====*/
static void run_test(UnityTestFunction func, const char* name, UNITY_LINE_TYPE line_num)
{
    Unity.CurrentTestName = name;
    Unity.CurrentTestLineNumber = (UNITY_UINT) line_num;
#ifdef UNITY_USE_COMMAND_LINE_ARGS
    if (!UnityTestMatches())
        return;
#endif
    Unity.NumberOfTests++;
    UNITY_CLR_DETAILS();
    UNITY_EXEC_TIME_START();
    CMock_Init();
    if (TEST_PROTECT())
    {
        setUp();
        func();
    }
    if (TEST_PROTECT())
    {
        tearDown();
        CMock_Verify();
    }
    CMock_Destroy();
    UNITY_EXEC_TIME_STOP();
    UnityConcludeTest();
}

/*=======MAIN=====*/
int main(void)
{
  UnityBegin("tests/inserts.c");
  run_test(test_addr_idx_conversions, "test_addr_idx_conversions", 12);
  run_test(test_initing_set, "test_initing_set", 31);
  run_test(test_add_first_member, "test_add_first_member", 52);
  run_test(test_add_second_larger_member, "test_add_second_larger_member", 79);
  run_test(test_add_second_smaller_member, "test_add_second_smaller_member", 118);
  run_test(test_add_third_member_line, "test_add_third_member_line", 157);
  run_test(test_add_third_member_triangle, "test_add_third_member_triangle", 191);
  run_test(test_insert_sizeidentity, "test_insert_sizeidentity", 232);

  return UNITY_END();
}
