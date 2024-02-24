#include <ArduinoFake.h>
#include <unity.h>
#include <peripherals/peripherals.h>
#include <functional/descale.h>
#include <functional/descale.cpp>
#include <mock.h>

using namespace fakeit;

void justDoCoffee(eepromValues_t const&, SensorState const&, bool) {}
void lcdShowPopup(char const*) {}
void lcdBrewTimerStop() {}
void lcdSetDescaleCycle(int) {}

void test_backflush_phases(void) {
  When(Method(ArduinoFake(), millis)).Return(
    0,
    7001,
    7002,
    14003,
    14004,
    21005,
    21006,
    28007,
    28008,
    35009,
    35010,
    42011,
    42012,
    49013,
    49014,
    56015,
    56016,
    63017,
    63018,
    70019,
    70020,
    77021,
    77022,
    84023,
    84024,
    91025,
    91026,
    98027,
    98028,
    105029,
    105030,
    112031
    );
  When(Method(ArduinoFake(), digitalWrite)).AlwaysReturn();
  
  flushCounter = 0;
  
  flushPhases();
  TEST_ASSERT_EQUAL(1, flushCounter);
  flushPhases();
  TEST_ASSERT_EQUAL(2, flushCounter);
  flushPhases();
  TEST_ASSERT_EQUAL(3, flushCounter);
  flushPhases();
  TEST_ASSERT_EQUAL(4, flushCounter);
  flushPhases();
  TEST_ASSERT_EQUAL(5, flushCounter);
  flushPhases();
  TEST_ASSERT_EQUAL(6, flushCounter);
  flushPhases();
  TEST_ASSERT_EQUAL(7, flushCounter);
  flushPhases();
  TEST_ASSERT_EQUAL(8, flushCounter);
  flushPhases();
  TEST_ASSERT_EQUAL(9, flushCounter);
  flushPhases();
  TEST_ASSERT_EQUAL(10, flushCounter);
  flushPhases();

  Verify(Method(ArduinoFake(), digitalWrite).Using(valvePin, HIGH)).Exactly(5_Times);
  Verify(Method(ArduinoFake(), digitalWrite).Using(valvePin, LOW)).Exactly(6_Times);

}

void test_backflush_phases_noskip(void) {
  ArduinoFakeReset();
  When(Method(ArduinoFake(), millis)).Return(
    78022,
    79023,
    80024,
    80025
  );
  When(Method(ArduinoFake(), digitalWrite)).AlwaysReturn();

  flushCounter = 0;
  flushPhases();
  TEST_ASSERT_EQUAL(0, flushCounter);
  flushPhases();
  TEST_ASSERT_EQUAL(0, flushCounter);
  flushPhases();
  TEST_ASSERT_EQUAL(0, flushCounter);

  Verify(Method(ArduinoFake(), digitalWrite).Using(valvePin, LOW)).Exactly(3_Times);

}

void runAllFlushTests(void) {
  RUN_TEST(test_backflush_phases);
  RUN_TEST(test_backflush_phases_noskip);
}