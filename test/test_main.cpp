#include <unity.h>
#include "./tests/test_pressure_profiler.cpp"
#include "./tests/test_pump.cpp"
#include "./tests/test_profile_serializer.cpp"
#include "./tests/test_momentary_switch.cpp"

int main(int argc, char **argv) {
    UNITY_BEGIN();
    runAllPressureProfilerTests();
    runAllPumpTests();
    runAllProfileSerializerTests();
    #ifdef ASCASO
    runAllSwitchTests();
    #endif
    return UNITY_END();
}
