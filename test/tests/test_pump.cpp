#include <ArduinoFake.h>
#include "mock.h"
#include "../src/peripherals/pump.cpp"
#include "utils/test-utils.h"

using namespace fakeit;

void test_pump_clicks_for_flow_correct_binary_search(void) {
  //corrected from 31 old value due to correcting the formula for flow/click calculations
  TEST_ASSERT_EQUAL_FLOAT_ACCURACY(47, getClicksPerSecondForFlow(5, 2), 0);
  TEST_ASSERT_EQUAL_FLOAT_ACCURACY(50, getClicksPerSecondForFlow(10, 9), 0);
  TEST_ASSERT_EQUAL_FLOAT_ACCURACY(0, getClicksPerSecondForFlow(0, 0), 0);
}

void test_pump_get_flow(void) {
  TEST_ASSERT_EQUAL_FLOAT_ACCURACY(5.f, getPumpFlow(getClicksPerSecondForFlow(5, 2), 2), 1);
  TEST_ASSERT_EQUAL_FLOAT_ACCURACY(4.5f, getPumpFlow(getClicksPerSecondForFlow(4.5, 5), 5), 1);
}

void test_pump_pct_for_pressure(void) {
  float startPressure = 0.f;
  SensorState testState;

  // get to 6 bar asap
  When(Method(ArduinoFake(), millis)).Return(2000);
  testState.smoothedPressure = 0.f;
  testState.smoothedPumpFlow = 0.f;
  TEST_ASSERT_EQUAL_FLOAT_ACCURACY(1, getPumpPct(6.f, 0.f, testState), -1);

  // maintaint 2ml/s flow
  When(Method(ArduinoFake(), millis)).Return(2000);
  testState.smoothedPressure = 0.f;
  testState.smoothedPumpFlow = 0.f;
  float pctFor2Mls = getClicksPerSecondForFlow(2.f, 0) / (float)maxPumpClicksPerSecond;
  TEST_ASSERT_EQUAL_FLOAT_ACCURACY(pctFor2Mls, getPumpPct(6.f, 2.f, testState), -1);

  // Pressure target maintain #1
  When(Method(ArduinoFake(), millis)).Return(2000);
  printf("pressure target maintain #1\n");
  testState.smoothedPressure = 6.f;
  testState.smoothedPumpFlow = getPumpFlow(maxPumpClicksPerSecond, 6);
  TEST_ASSERT_EQUAL_FLOAT_ACCURACY(1, getPumpPct(6.f, 0.f, testState), -1);

  // Pressure target maintain #2
  When(Method(ArduinoFake(), millis)).Return(2000);
  printf("pressure target maintain #2\n");
  testState.smoothedPressure = 2.f;
  testState.smoothedPumpFlow = getPumpFlow(getClicksPerSecondForFlow(2.f, 2.f), 2.f);
  // TEST_ASSERT_EQUAL_FLOAT_ACCURACY(1, getPumpPct(6.f, 0.f, testState), 1);
  TEST_ASSERT_EQUAL_FLOAT_ACCURACY(1, getPumpPct(6.f, 0.f, testState, false), -1);

  // test pressure target is lower than current
  When(Method(ArduinoFake(), millis)).Return(2000);
  printf("pressure target is lower than current: 9 -> 6bar\n");
  testState.smoothedPressure = 9.f;
  testState.smoothedPumpFlow = getPumpFlow(maxPumpClicksPerSecond, 9);
  // TODO: figure out the slope
  printf("ref pct: %f\n", getPumpPct(6.f, 0.f, testState, false));
  TEST_ASSERT_EQUAL_FLOAT_ACCURACY(0, getPumpPct(6.f, 0.f, testState), -1);

  When(Method(ArduinoFake(), millis)).Return(2000);
  printf("pressure target is higher than current\n");
  // test pressure target is higher than current
  testState.smoothedPressure = 2.f;
  testState.smoothedPumpFlow = getPumpFlow(getClicksPerSecondForFlow(2.f, 2.f), 2.f);
  // TODO: figure out the slope
  printf("ref pct: %f\n", getPumpPct(3.f, 0.f, testState, false));
  TEST_ASSERT_EQUAL_FLOAT_ACCURACY(1, getPumpPct(3.f, 0.f, testState), -1);

  When(Method(ArduinoFake(), millis)).Return(2000);
  printf("pressure target is much higher than current\n");
  // test pressure target is higher than current
  testState.smoothedPressure = 2.f;
  testState.smoothedPumpFlow = getPumpFlow(getClicksPerSecondForFlow(2.f, 2.f), 2.f);
  // TODO: figure out the slope
  printf("ref pct: %f\n", getPumpPct(6.f, 0.f, testState, false));
  TEST_ASSERT_EQUAL_FLOAT_ACCURACY(1, getPumpPct(6.f, 0.f, testState), 0);

  // printf("decent simulator\n");
  // testState.smoothedPressure = 8.9f;
  // testState.smoothedPumpFlow = getPumpFlow(getClicksPerSecondForFlow(1.7f, 8.9f), 9.f);
  // // TODO: figure out the slope
  // printf("ref pct: %f\n", getPumpPct(9.f, 0.f, testState, false));
  // printf("current pct: %f\n", getClicksPerSecondForFlow(1.7f, 8.9f)/maxPumpClicksPerSecond);
  // TEST_ASSERT_EQUAL_FLOAT_ACCURACY(0, getPumpPct(9.f, 0.f, testState), 0);

  // printf("fast ceiling approach\n");
  // startPressure = 6.2;
  // testState.smoothedPressure = startPressure;
  // testState.pressureChangeSpeed = 2.0;
  // testState.smoothedPumpFlow = getPumpFlow(getClicksPerSecondForFlow(3.7f, startPressure), startPressure);
  // printf("with fast rising\n");
  // printf("ref pct: %f\n", getPumpPct(9.f, 0.f, testState, false));
  // printf("current pct: %f\n", getClicksPerSecondForFlow(3.7f, startPressure) / maxPumpClicksPerSecond);
  // TEST_ASSERT_EQUAL_FLOAT_ACCURACY(1, getPumpPct(9.f, 0.f, testState), 0);
  // testState.pressureChangeSpeed = 0;
  // testState.smoothedPumpFlow = getPumpFlow(getClicksPerSecondForFlow(3.7f, startPressure), startPressure);
  // printf("without fast rising\n");
  // printf("ref pct: %f\n", getPumpPct(9.f, 0.f, testState, false));
  // printf("current pct: %f\n", getClicksPerSecondForFlow(3.7f, startPressure) / maxPumpClicksPerSecond);
  // TEST_ASSERT_EQUAL_FLOAT_ACCURACY(0, getPumpPct(9.f, 0.f, testState), 0);
}

void runAllPumpTests() {
  RUN_TEST(test_pump_clicks_for_flow_correct_binary_search);
  RUN_TEST(test_pump_get_flow);
  RUN_TEST(test_pump_pct_for_pressure);
}
