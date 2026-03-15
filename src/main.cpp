#include <Arduino.h>
#include <CANCREATE.h>

#define CAM_CNT 13
#define CAN_RX 15
#define CAN_TX 16

bool CAM_PWR = true;
bool CAM_PWR_Prev = true;

hw_timer_t *timer = NULL;

CAN_CREATE CAN(true);

can_setting_t can_setting = {
  .baudRate = (long)100E3,
  .multiData_send = true,
  .filter_config = CAN_FILTER_DEFAULT,
};

IRAM_ATTR void counter() {
  CAM_PWR = false;
}

void setup() {
  pinMode(CAM_CNT, OUTPUT);
  digitalWrite(CAM_CNT, LOW);
  Serial.begin(115200);
  while (!Serial)
    ;
  delay(1000);
  Serial.println("CAN Sender");
  if (CAN.begin(can_setting, CAN_RX, CAN_TX)) {
    Serial.println("Starting CAN failed!");
  }
  delay(100);
  switch (CAN.test())
  {
  case CAN_SUCCESS:
    Serial.println("Success!!!");
    break;
  case CAN_UNKNOWN_ERROR:
    Serial.println("Unknown error occurred");
    break;
  case CAN_NO_RESPONSE_ERROR:
    Serial.println("No response error");
    break;
  case CAN_CONTROLLER_ERROR:
    Serial.println("CAN CONTROLLER ERROR");
    break;
  default:
    break;
  }
  timer = timerBegin(0, getApbFrequency() / 1000000, true);
  timerAttachInterrupt(timer, &counter, false);
  timerAlarmWrite(timer, 20 * 1000000, false);
}

void loop() {
  if (CAN.available()) {
    can_return_t Data;
    if (CAN.readWithDetail(&Data)) {
      Serial.println("failed to get CAN data");
    }
    if (Data.id == 0x030) {
      CAM_PWR = false;
    }
    if (Data.id == 0x02f) {
      CAM_PWR = true;
    }
    if (Data.id == 0x12a) {
      timerRestart(timer);
      timerAlarmEnable(timer);
    }
  }
  if (Serial.available()) {
    char cmd = Serial.read();
    Serial.println(cmd);
    if (cmd == 'o') {
      CAM_PWR = false;
    }
    if (cmd == 'i') {
      CAM_PWR = true;
    }
    if (cmd == 't') {
      timerRestart(timer);
      timerAlarmEnable(timer);
    }
  }
  if (CAM_PWR) {
    digitalWrite(CAM_CNT, HIGH);
    if (!CAM_PWR_Prev) {
      Serial.println("Camera 3.3V Power on");
    }
  }
  else {
    digitalWrite(CAM_CNT, LOW);
    if (CAM_PWR_Prev) {
      Serial.println("Camera 3.3V Power off");
    }
  }
  CAM_PWR_Prev = CAM_PWR;
}