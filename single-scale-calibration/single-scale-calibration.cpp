#include <Arduino.h>
#ifdef ADC_HX711
#include <HX711_ADC.h>
#else 
#include <HX711.h>
#endif

#define LOADCELL_1_DOUT_PIN  PB8
#define LOADCELL_2_DOUT_PIN  PB9
#define LOADCELL_1_SCK_PIN  PB0
#define LOADCELL_2_SCK_PIN  PB1
#define USART_DEBUG SerialUSB

HX711 LoadCell;

unsigned long t = 0;

void calibrate();
void changeSavedCalFactor();

void setup() {
  USART_DEBUG.begin(115200); delay(10 * 1000);
  USART_DEBUG.println();
  USART_DEBUG.println("Starting...");
  pinMode(LOADCELL_1_SCK_PIN, OUTPUT_OPEN_DRAIN);

  LoadCell.begin(LOADCELL_1_DOUT_PIN, LOADCELL_1_SCK_PIN, 128);
  //LoadCell.setReverseOutput(); //uncomment to turn a negative output value to positive

  unsigned long stabilizingtime = 2000; // preciscion right after power-up can be improved by adding a few seconds of stabilizing time

  if (!LoadCell.wait_ready_timeout(stabilizingtime)) {
    USART_DEBUG.println("Timeout, check MCU>HX711 wiring and pin designations");
    while (1);
  }
  else {
    LoadCell.set_scale(); // user set calibration value (float), initial value 1.0 may be used for this sketch
    LoadCell.power_up();
    USART_DEBUG.println("Startup is complete");
  }
  while (!LoadCell.wait_ready_timeout()) {
    USART_DEBUG.println("Waiting for cell ready");
  }
  calibrate(); //start calibration procedure
}

void loop() {
  const int serialPrintInterval = 1000; //increase value to slow down serial print activity

  // check for new data/start next conversion:
  // LoadCell.power_up();
  if (!LoadCell.wait_ready_timeout()) {
    USART_DEBUG.print("Load_cell not ready");
    return;
  }

    if (millis() > t + serialPrintInterval) {
      float i = LoadCell.get_units(5U);
      USART_DEBUG.print("Load_cell output val: ");
      USART_DEBUG.println(i);
      t = millis();
    }

  // receive command from serial terminal
  if (USART_DEBUG.available() > 0) {
    char inByte = USART_DEBUG.read();
    if (inByte == 't') LoadCell.tare(); //tare
    else if (inByte == 'r') calibrate(); //calibrate
    else if (inByte == 'c') changeSavedCalFactor(); //edit calibration value manually
  }

  // check if last tare operation is complete
  // if (LoadCell.getTareStatus() == true) {
  //   USART_DEBUG.println("Tare complete");
  // }

}

//  -1021.80
void calibrate() {
  USART_DEBUG.println("***");
  USART_DEBUG.println("Start calibration:");
  LoadCell.set_scale();
  
  USART_DEBUG.println("Place the load cell an a level stable surface.");
  USART_DEBUG.println("Remove any load applied to the load cell.");
  USART_DEBUG.println("Send 't' from serial monitor to set the tare offset.");

  bool _resume = false;
  while (_resume == false) {
    if (USART_DEBUG.available() > 0) {
      char inByte = USART_DEBUG.read();
      if (inByte == 't') {
        LoadCell.tare();
        USART_DEBUG.println("Tare complete");
        _resume = true;
      } 
    }
  }

  USART_DEBUG.println("Now, place your known mass on the loadcell.");
  USART_DEBUG.println("Then send the weight of this mass (i.e. 100.0) from serial monitor.");

  float known_mass = 0;
  _resume = false;
  while (_resume == false) {
    if (USART_DEBUG.available() > 0) {
      known_mass = USART_DEBUG.parseFloat();
      if (known_mass != 0) {
        USART_DEBUG.print("Known mass is: ");
        USART_DEBUG.println(known_mass);
        _resume = true;
      }
    }
  }
  float readValue = LoadCell.get_value(10);
  USART_DEBUG.printf("calibration reading: %f\n", readValue);
  // LoadCell.refreshDataSet(); //refresh the dataset to be sure that the known mass is measured correct
  // float newCalibrationValue = LoadCell.getNewCalibration(known_mass); //get the new calibration value
  float newCalibrationValue = readValue / known_mass ;

  USART_DEBUG.print("New calibration value has been set to: ");
  USART_DEBUG.print(newCalibrationValue);
  USART_DEBUG.println(", use this as calibration value (calFactor) in your project sketch.");
  LoadCell.set_scale(newCalibrationValue);
  USART_DEBUG.println("End calibration");
  USART_DEBUG.println("***");
  USART_DEBUG.println("To re-calibrate, send 'r' from serial monitor.");
  USART_DEBUG.println("For manual edit of the calibration value, send 'c' from serial monitor.");
  USART_DEBUG.println("***");
}

void changeSavedCalFactor() {
  float oldCalibrationValue = LoadCell.get_scale();
  bool _resume = false;
  USART_DEBUG.println("***");
  USART_DEBUG.print("Current value is: ");
  USART_DEBUG.println(oldCalibrationValue);
  USART_DEBUG.println("Now, send the new value from serial monitor, i.e. 696.0");
  float newCalibrationValue;
  while (_resume == false) {
    if (USART_DEBUG.available() > 0) {
      newCalibrationValue = USART_DEBUG.parseFloat();
      if (newCalibrationValue != 0) {
        USART_DEBUG.print("New calibration value is: ");
        USART_DEBUG.println(newCalibrationValue);
        LoadCell.set_scale(newCalibrationValue);
        _resume = true;
      }
    }
  }
  _resume = false;
  
  USART_DEBUG.println("End change calibration value");
  USART_DEBUG.println("***");
}