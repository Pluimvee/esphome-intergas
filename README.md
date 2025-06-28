Intergase HRE24/18 Parser for ESPHome
=========================

Custom component for ESPHome to communicate with an INtergas HRE28/18 boiler

Usage
-----

Requires ESPHome v2022.3.0 or newer.

```yaml
logger:
  level: DEBUG  
  baud_rate: 0  # Disable the use of the serial as we need it !!!!!

uart:
  id: intergas_serial
  baud_rate: 9600
  tx_pin: GPIO01
  rx_pin: GPIO03
  # debug:    # use this to enable logging by UART

# Add the external component to supprt communicating with the Intergas Boiler
external_components:
  - source: github://Pluimvee/esphome-intergas

# Now attach the Serial to this intergas communication component
intergas_boiler:
  uart_id: intergas_serial
  update_interval: 5s       # add a communication interval

#the text sensors supported
text_sensor:
  - platform: intergas_boiler
    mode:
      name: "Operating Mode"    # at least specify a name

# some supported binary sensors
binary_sensor:
  - platform: intergas_boiler
    alarm:
      name: "Fault"
    pump_on:
      name: "Pump On"
    tap_switch:
      name: "Water Flow"

# and the supported sensors
sensor:
  # Add sensors to monitor the boiler's temperatures
  - platform: intergas_boiler
    heater:
      name: "Heater"
    inlet:
      name: "Heater Inlet"
    outlet:
      name: "Heater Outlet"
    set_point:
      name: "Heater Setpoint"
    pump:
      name: "Pump"
    flow:
      name: "DHW Flow"
    dhw:
      name: "DHW Outlet"
    pressure:
      name: "CH Pressure"
    power:
      name: "Power"
    energy_ch:
      name: "CH Energy"
    energy_dhw:
      name: "DHW Energy"
    room:
      name: "Room Current"
    room_set:
      name: "Room Setpoint"
    fault_code:
      name: "Fault Code"
    last_fault:
      name: "Fault Last"
    fan:
      name: "Fan"
    fan_set:
      name: "Fan Setpoint"
    fan_rpm:
      name: "Fan Speed"   
```


