#include <ArduinoFake.h>
#include <unity.h>
#include <peripherals/peripherals.h>
// #include <mock.h>

using namespace fakeit;

void setUp(void) {
  ArduinoFakeReset();
}

void test_momentary_switch_toggle(void) {
  When(Method(ArduinoFake(), digitalRead)).Return(HIGH, LOW, HIGH, LOW);
  When(Method(ArduinoFake(), millis)).Return(2000, 4001, 6002, 8003, 10004);

  TEST_ASSERT_FALSE(brewState());
  TEST_ASSERT_TRUE(brewState());
  TEST_ASSERT_TRUE(brewState());
  TEST_ASSERT_FALSE(brewState());

  Verify(Method(ArduinoFake(), digitalRead)).Exactly(4_Times);
  Verify(Method(ArduinoFake(), millis)).Exactly(5_Times);
}

void test_momentary_switch_debounce(void) {
  When(Method(ArduinoFake(), digitalRead)).Return(HIGH, LOW, HIGH, LOW);
  When(Method(ArduinoFake(), millis)).Return(10005, 16001, 16002, 16003, 16004, 16005);

  TEST_ASSERT_FALSE(brewState());
  TEST_ASSERT_TRUE(brewState());
  TEST_ASSERT_TRUE(brewState());
  TEST_ASSERT_TRUE(brewState());

  Verify(Method(ArduinoFake(), digitalRead)).Exactly(4_Times);
}

void runAllSwitchTests(void) {
  RUN_TEST(test_momentary_switch_toggle);
  RUN_TEST(test_momentary_switch_debounce);
}
