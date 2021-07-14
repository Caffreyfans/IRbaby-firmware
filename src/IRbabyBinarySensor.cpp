/*
 * @Author: Caffreyfans
 * @Date: 2021-07-12 23:18:11
 * @LastEditTime: 2021-07-12 23:45:29
 * @Description:
 */
#include "IRbabyBinarySensor.h"

#include "IRbabyMQTT.h"
#include "defines.h"

void binary_sensor_init() { pinMode(SENSOR_PIN, INPUT_PULLUP); }

void binary_sensor_loop() {
  if (mqttConnected()) {
    String chip_id = String(ESP.getChipId(), HEX);
    chip_id.toUpperCase();
    int value = digitalRead(SENSOR_PIN);
    String message = value == HIGH ? "on" : "off";
    String topic = "/IRbaby/" + chip_id + "/sensor";
    mqttPublish(topic, message);
  }
}