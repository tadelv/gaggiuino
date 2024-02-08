#include <Arduino.h>
#include <unity.h>
#include <peripherals/peripherals.h>
// #include <mock.h>

using namespace fakeit;

void setUp(void)
{
  ArduinoFakeReset();
}

void test_momentary_switch_toggle(void)
{

  When(Method(ArduinoFake(), digitalRead)).Return(HIGH, LOW, HIGH, LOW);

  TEST_ASSERT_FALSE(brewState());
  TEST_ASSERT_TRUE(brewState());
  TEST_ASSERT_TRUE(brewState());
  TEST_ASSERT_FALSE(brewState());

  Verify(Method(ArduinoFake(), digitalRead)).Exactly(4_Times);
}

void runAllSwitchTests(void)
{
  RUN_TEST(test_momentary_switch_toggle);
}