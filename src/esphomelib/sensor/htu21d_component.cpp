//
//  htu21d_component.cpp
//  esphomelib
//
//  Created by Otto Winter on 27.03.18.
//  Copyright © 2018 Otto Winter. All rights reserved.
//
// Based on:
//   - http://www.te.com/commerce/DocumentDelivery/DDEController?Action=showdoc&DocId=Data+Sheet%7FHPC199_6%7FA6%7Fpdf%7FEnglish%7FENG_DS_HPC199_6_A6.pdf%7FCAT-HSC0004
//   - https://github.com/jrowberg/i2cdevlib/tree/master/Arduino/HTU21D

#include "esphomelib/sensor/htu21d_component.h"

#include "esphomelib/log.h"

#ifdef USE_HTU21D_SENSOR

ESPHOMELIB_NAMESPACE_BEGIN

namespace sensor {

static const char *TAG = "sensor.htu21d";
static const uint8_t HTU21D_ADDRESS = 0x40;
static const uint8_t HTU21D_REGISTER_RESET = 0xFE;
static const uint8_t HTU21D_REGISTER_TEMPERATURE = 0xE3;
static const uint8_t HTU21D_REGISTER_HUMIDITY = 0xE5;

HTU21DComponent::HTU21DComponent(I2CComponent *parent,
                                 const std::string &temperature_name, const std::string &humidity_name,
                                 uint32_t update_interval)
    : PollingComponent(update_interval), I2CDevice(parent, HTU21D_ADDRESS),
      temperature_(new HTU21DTemperatureSensor(temperature_name, this)),
      humidity_(new HTU21DHumiditySensor(humidity_name, this)) {

}
void HTU21DComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up HTU21D...");

  if (!this->write_byte(HTU21D_REGISTER_RESET, 0x00)) {
    ESP_LOGE(TAG, "Connection to HTU21D failed.");
    this->mark_failed();
    return;
  }

  // Wait for software reset to complete
  delay(15);
}
void HTU21DComponent::update() {
  uint16_t raw_temperature;
  if (!this->read_byte_16(HTU21D_REGISTER_TEMPERATURE, &raw_temperature))
    return;

  float temperature = (float(raw_temperature & 0xFFFC)) * 175.72f / 65536.0f - 46.85f;

  uint16_t raw_humidity;
  if (!this->read_byte_16(HTU21D_REGISTER_HUMIDITY, &raw_humidity))
    return;

  float humidity = (float(raw_humidity & 0xFFFC)) * 125.0f / 65536.0f - 6.0f;
  ESP_LOGD(TAG, "Got Temperature=%.1f°C Humidity=%.1f%%", temperature, humidity);

  this->temperature_->push_new_value(temperature);
  this->humidity_->push_new_value(humidity);
}
HTU21DTemperatureSensor *HTU21DComponent::get_temperature_sensor() const {
  return this->temperature_;
}
HTU21DHumiditySensor *HTU21DComponent::get_humidity_sensor() const {
  return this->humidity_;
}
float HTU21DComponent::get_setup_priority() const {
  return setup_priority::HARDWARE_LATE;
}

} // namespace sensor

ESPHOMELIB_NAMESPACE_END

#endif //USE_HTU21D_SENSOR
