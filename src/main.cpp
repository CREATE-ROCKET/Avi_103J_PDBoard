#include <Arduino.h>
#include <CANCREATE.h>

#define CAM_CNT 13
#define CAN_RX 15
#define CAN_TX 16

hw_timer_t *timer = NULL;

CAN_CREATE CAN(true);

can_setting_t Settings = {
  .baudRate = (long)100E3,
  .multiData_send = true,
  .filter_config = CAN_FILTER_DEFAULT,
};

IRAM_ATTR void counter() {
  digitalWrite(CAM_CNT, LOW);
  Serial.println("Camera 3.3V Power off");
}

void setup() {
  pinMode(CAM_CNT, OUTPUT);
  digitalWrite(CAM_CNT, HIGH);
  timer = timerBegin(0, getApbFrequency() / 1000000, true);
  timerAttachInterrupt(timer, &counter, false);
  timerAlarmWrite(timer, 20 * 1000000, false);
  Serial.begin(115200);
  while (!Serial)
    ;
  delay(1000);
  Serial.println("CAN Sender");
  if (CAN.begin(Settings, CAN_RX, CAN_TX)) {
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
}

void loop() {
  if (CAN.available()) {
    can_return_t Data;
    if (CAN.readWithDetail(&Data)) {
      Serial.println("failed to get CAN data");
    }
    if (Data.id == 0x030) {
      digitalWrite(CAM_CNT, LOW);
      Serial.println("Camera 3.3V Power off");
    }
    if (Data.id == 0x02f) {
      digitalWrite(CAM_CNT, HIGH);
      Serial.println("Camera 3.3V Power on");
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
      digitalWrite(CAM_CNT, LOW);
      Serial.println("Camera 3.3V Power off");
    }
    if (cmd == 'i') {
      digitalWrite(CAM_CNT, HIGH);
      Serial.println("Camera 3.3V Power on");
    }
    if (cmd == 't') {
      timerRestart(timer);
      timerAlarmEnable(timer);
    }
  }
}