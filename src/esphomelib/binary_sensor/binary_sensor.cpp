//
// Created by Otto Winter on 25.11.17.
//

#include "esphomelib/binary_sensor/binary_sensor.h"
#include "esphomelib/log.h"

#ifdef USE_BINARY_SENSOR

ESPHOMELIB_NAMESPACE_BEGIN

namespace binary_sensor {

static const char *TAG = "binary_sensor.binary_sensor";

void BinarySensor::add_on_state_callback(binary_callback_t &&callback) {
  this->state_callback_.add(std::move(callback));
}

void BinarySensor::publish_state(bool state) {
  bool actual = state != this->inverted_;
  if (!this->first_value_ && actual == this->value)
    return;
  this->first_value_ = false;
  this->value = actual;
  this->state_callback_.call(actual);
}
bool BinarySensor::is_inverted() const {
  return this->inverted_;
}
void BinarySensor::set_inverted(bool inverted) {
  this->inverted_ = inverted;
}
std::string BinarySensor::device_class() {
  return "";
}
BinarySensor::BinarySensor(const std::string &name) : Nameable(name) {}
bool BinarySensor::get_value() const {
  return this->value;
}
void BinarySensor::set_device_class(const std::string &device_class) {
  this->device_class_ = device_class;
}
std::string BinarySensor::get_device_class() {
  if (this->device_class_.has_value())
    return *this->device_class_;
  return this->device_class();
}
PressTrigger *BinarySensor::make_press_trigger() {
  return new PressTrigger(this);
}
ReleaseTrigger *BinarySensor::make_release_trigger() {
  return new ReleaseTrigger(this);
}
ClickTrigger *BinarySensor::make_click_trigger(uint32_t min_length, uint32_t max_length) {
  return new ClickTrigger(this, min_length, max_length);
}
DoubleClickTrigger *BinarySensor::make_double_click_trigger(uint32_t min_length, uint32_t max_length) {
  return new DoubleClickTrigger(this, min_length, max_length);
}

PressTrigger::PressTrigger(BinarySensor *parent) {
  parent->add_on_state_callback([this](bool state) {
    if (state)
      this->trigger();
  });
}

ReleaseTrigger::ReleaseTrigger(BinarySensor *parent) {
  parent->add_on_state_callback([this](bool state) {
    if (!state)
      this->trigger();
  });
}

bool match_interval(uint32_t min_length, uint32_t max_length, uint32_t length) {
  if (max_length == 0) {
    return length >= min_length;
  } else {
    return length >= min_length && length <= max_length;
  }
}

ClickTrigger::ClickTrigger(BinarySensor *parent, uint32_t min_length, uint32_t max_length)
    : min_length_(min_length), max_length_(max_length) {
  parent->add_on_state_callback([this](bool state) {
    if (state) {
      this->start_time_ = millis();
    } else {
      const uint32_t length = millis() - this->start_time_;
      if (match_interval(this->min_length_, this->max_length_, length))
        this->trigger();
    }
  });
}

DoubleClickTrigger::DoubleClickTrigger(BinarySensor *parent, uint32_t min_length, uint32_t max_length)
    : min_length_(min_length), max_length_(max_length) {
  parent->add_on_state_callback([this](bool state) {
    const uint32_t now = millis();

    if (state && this->start_time_ != 0 && this->end_time_ != 0) {
      if (match_interval(this->min_length_, this->max_length_, this->end_time_ - this->start_time_) &&
          match_interval(this->min_length_, this->max_length_, now - this->end_time_)) {
        this->trigger();
        this->start_time_ = 0;
        this->end_time_ = 0;
        return;
      }
    }

    this->start_time_ = this->end_time_;
    this->end_time_ = now;
  });
}

} // namespace binary_sensor

ESPHOMELIB_NAMESPACE_END

#endif //USE_BINARY_SENSOR
