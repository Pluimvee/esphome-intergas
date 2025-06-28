////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2025 Pluimvee
// Intergas HRE Boiler protocol

////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"

enum class IntergasSensorId {
  SENSOR_BOILER_TEMP,
  SENSOR_POWER,
  SENSOR_MODE,
  SENSOR_ALARM,
  // uitbreidbaar
};

////////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////////
class IntergasComponent : public esphome::PollingComponent, public esphome::uart::UARTDevice
{
public:
  ////////////////////////////////////////////////////////////////////////////////////////////
  IntergasComponent(esphome::uart::UARTComponent *parent) : PollingComponent(1000), UARTDevice(parent) {};

  void setup() override;
  void update() override;
  void dump_config() override 
  {
    ESP_LOGCONFIG("intergas", "Intergas Boiler Communication (C)Pluimvee");
    ESP_LOGCONFIG("intergas", "  Communicating every %d ms", this->get_update_interval());
    ESP_LOGCONFIG("intergas", "  Number of sensors registered %d", this->sensors_.size() + this->text_sensors_.size() + this->bin_sensors_.size());
  };
  // register the sensors
  void register_value_sensor(const std::string &key, esphome::sensor::Sensor *sensor) {
    ESP_LOGI("intergas", "Registering sensor with key: %s", key.c_str());
    sensors_[key] = sensor;
  };
  void register_text_sensor(const std::string &key, esphome::text_sensor::TextSensor *sensor) {
    ESP_LOGI("intergas", "Registering sensor with key: %s", key.c_str());
    text_sensors_[key] = sensor;
  };
  void register_binary_sensor(const std::string &key, esphome::binary_sensor::BinarySensor *sensor) {
    ESP_LOGI("intergas", "Registering sensor with key: %s", key.c_str());
    bin_sensors_[key] = sensor;
  };
protected:
  std::map<std::string, esphome::sensor::Sensor *> sensors_;
  std::map<std::string, esphome::text_sensor::TextSensor *> text_sensors_;
  std::map<std::string, esphome::binary_sensor::BinarySensor *> bin_sensors_;

  void parse_status_1_(const uint8_t *buf, size_t size);
  void parse_status_2_(const uint8_t *buf, size_t size);
  void parse_statistics_(const uint8_t *buf, size_t size);
};
