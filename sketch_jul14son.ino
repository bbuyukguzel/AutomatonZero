
/**
   Author Teemu Mäntykallio
   Initializes the library and turns the motor in alternating directions.
*/
#include <TMC2130Stepper.h>

#define EN_PIN    7          // Nano v3:  16 Mega:  38  //enable (CFG6)
#define DIR_PIN  8          //      19      55  //direction
#define STEP_PIN  9         //      18      54  //step
#define CS_PIN    10          //      17      64  //chip select
#define INPUT1_PIN    2       //      17      64  //chip select
#define INPUT2_PIN    3       //      17      64  //chip select
#define ARDUINO_OUT_RASP_IN    5
#define ARDUINO_IN_RASP_OUT    6

bool dir = true;
int feedMe = 0;
typedef enum gateState { INITIAL, OPENED, OPENING, CLOSED, CLOSING };
typedef enum switchState { ACTIVE, NOT_ACTIVE };

TMC2130Stepper driver = TMC2130Stepper(EN_PIN, DIR_PIN, STEP_PIN, CS_PIN);
gateState gateCurrentState = INITIAL;
switchState limitSwitchFullyOpen = NOT_ACTIVE;
switchState limitSwitchFullyClosed = NOT_ACTIVE;

void setup() {
  Serial.begin(9600);
  while (!Serial);
  Serial.println("Start...");
  SPI.begin();
  pinMode(MISO, INPUT_PULLUP);
  driver.begin();       // Initiate pins and registeries
  driver.rms_current(400);  // Set stepper current to 600mA. The command is the same as command TMC2130.setCurrent(600, 0.11, 0.5);
  //driver.stealthChop(2);  // Enable extremely quiet stepping
  driver.en_pwm_mode(1);      // Enable extremely quiet stepping
  driver.pwm_autoscale(1);

  driver.microsteps(16);
  pinMode(INPUT1_PIN, INPUT_PULLUP);
  pinMode(INPUT2_PIN, INPUT_PULLUP);
  digitalWrite(EN_PIN, LOW);
}

void generatePulse() {
  digitalWrite(STEP_PIN, HIGH);
  delayMicroseconds(100);
  digitalWrite(STEP_PIN, LOW);
  delayMicroseconds(100);
}

void closeTheGate() {
  digitalWrite(EN_PIN, LOW);
  generatePulse();
  driver.shaft_dir(0);
}

void openTheGate() {
  digitalWrite(EN_PIN, LOW);
  generatePulse();
  driver.shaft_dir(1);
}

void handleInitialState(switchState limitSwitchFullyClosed) {
  // close if it is not already closed
  if ((switchState) limitSwitchFullyClosed == NOT_ACTIVE) {
    closeTheGate();
  }
  else {
    gateCurrentState = CLOSED;
    digitalWrite(EN_PIN, HIGH); // cut the power
  }
}

void loop() {
  // Read Raspberry out
  feedMe = digitalRead(ARDUINO_IN_RASP_OUT);
  // Read switch states
  limitSwitchFullyOpen = (switchState) digitalRead(INPUT1_PIN);
  limitSwitchFullyClosed = (switchState) digitalRead(INPUT2_PIN);

  // init state
  if (gateCurrentState == INITIAL) {
    handleInitialState(limitSwitchFullyClosed);
  }
  else {
    if ((switchState) limitSwitchFullyOpen == ACTIVE) {
      gateCurrentState = OPENED;
      digitalWrite(ARDUINO_OUT_RASP_IN, HIGH);
    }
    else if ((switchState) limitSwitchFullyClosed == ACTIVE) {
      gateCurrentState = CLOSED;
      digitalWrite(ARDUINO_OUT_RASP_IN, LOW);
    }
    if (feedMe == 1) {
      if ((gateCurrentState == CLOSED) || (gateCurrentState == CLOSING)) {
        gateCurrentState = OPENING;
      }
      if (gateCurrentState == OPENING) {
        openTheGate();
      }
    }
    else {
      if ((gateCurrentState == OPENED) || (gateCurrentState == OPENING)) {
        gateCurrentState = CLOSING;
      }
      if (gateCurrentState == CLOSING) {
        closeTheGate();
      }
    }
  }


  //  else {
  //    if (feedMe == 1) {
  //      if ((gateCurrentState == CLOSE) || ((switchState) limitSwitchFullyOpen == NOT_ACTIVE)) {
  //        openTheGate();
  //        gateCurrentState = OPEN;
  //      }
  //    }
  //    else if (feedMe == 0) {
  //      if ((gateCurrentState == OPEN) || ((switchState) limitSwitchFullyClosed == ACTIVE)) {
  //        //Serial.println("Closing");
  //        closeTheGate();
  //      }
  //    }
  //  }



  //if (feedMe == 1) {
  //  Serial.println("Rasp High");
  //  digitalWrite(EN_PIN, HIGH);
  //  driver.shaft_dir(1);
  //}
  //
  //// fully open
  //if (limitSwitch_1 == 0) {
  //  //Serial.println("Dir -> 0");
  //  driver.shaft_dir(0);
  //  digitalWrite(EN_PIN, HIGH);
  //}
  //
  //// fully close
  //if (limitSwitch_2 == 0) {
  //  //Serial.println("Dir -> 1");
  //  driver.shaft_dir(1);
  //  digitalWrite(EN_PIN, HIGH);
  //}

}
