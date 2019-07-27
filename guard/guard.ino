#include <TMC2130Stepper.h>

#define EN_PIN                  7       // Nano v3:  16 Mega:  38  //enable (CFG6)
#define DIR_PIN                 8       //      19      55  //direction
#define STEP_PIN                9       //      18      54  //step
#define CS_PIN                  10      //      17      64  //chip select
#define INPUT1_PIN              2       //      17      64  //chip select
#define INPUT2_PIN              3       //      17      64  //chip select
#define ARDUINO_OUT_RASP_IN     5
#define ARDUINO_IN_RASP_OUT     6


int feedSignal = 0;
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
  driver.begin();             // Initiate pins and registeries
  driver.rms_current(400);    // Set stepper current to 600mA. The command is the same as command TMC2130.setCurrent(600, 0.11, 0.5);
  //driver.stealthChop(2);    // Enable extremely quiet stepping
  driver.en_pwm_mode(1);      // Enable extremely quiet stepping
  driver.pwm_autoscale(1);

  driver.microsteps(16);
  pinMode(INPUT1_PIN, INPUT_PULLUP);
  pinMode(INPUT2_PIN, INPUT_PULLUP);
  digitalWrite(EN_PIN, LOW);
}

void turnOnMotor() {
  digitalWrite(EN_PIN, LOW);
}

void turnOffMotor() {
  digitalWrite(EN_PIN, HIGH);
}

void generatePulse() {
  digitalWrite(STEP_PIN, HIGH);
  delayMicroseconds(100);
  digitalWrite(STEP_PIN, LOW);
  delayMicroseconds(100);
}

void closeTheGate() {
  turnOnMotor();
  generatePulse();
  driver.shaft_dir(0);
}

void openTheGate() {
  turnOnMotor();
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
    turnOffMotor();
  }
}

void loop() {
  // Read Raspberry out
  feedSignal = digitalRead(ARDUINO_IN_RASP_OUT);
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
      turnOffMotor();
    }
    else if ((switchState) limitSwitchFullyClosed == ACTIVE) {
      gateCurrentState = CLOSED;
      digitalWrite(ARDUINO_OUT_RASP_IN, LOW);
      turnOffMotor();
    }
    if (feedSignal == 1) {
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

}
