/**
  @AUTHOR (c) 2025 Pluimvee (Erik Veer)

 */
#include "intergas_boiler.h"

using namespace esphome;

////////////////////////////////////////////////////////////////////////////////////////////
#define GAS_FLOW    38.7            // gasflow in ml/sec (cm3/sec)
#define GAS_WATT    1361            // gasflow watt (1cm3 = 35.17 Joule)
#define GAS_USAGE_CALIBRATED  11619.27f // calibrated gas usage in m3 
#define PROD_CODE   "B?\r"          // not supported yet in below code
#define STATUS_1    "S?\r"
#define STATUS_2    "S2\r"
#define PARAMS      "V?\r"          // not supported yet in below code
#define SETTINGS    "V1\r"          // not supported yet in below code
#define STATISTICS  "HN\r"

////////////////////////////////////////////////////////////////////////////////////////////
// we send one of the above intergas instructions per update() call, 
//  so we need to keep track of the current task
////////////////////////////////////////////////////////////////////////////////////////////
enum Task { 
    IDLE, 
    JOB1,
    JOB2,
    JOB3
};

int current_task = IDLE;
  
////////////////////////////////////////////////////////////////////////////////////////////
// some helpers
float getfloat(uint8_t lsb, uint8_t msb) 
{
    if (msb > 127) 
        return -(float((msb ^ 255) + 1) * 256 - lsb) / 100.0f;
    // else
    return float(msb * 256 + lsb) / 100.0f;
}
std::string to_hex(uint8_t b) 
{
    char hex[5];
    snprintf(hex, sizeof(hex), "%02X", b);
    return std::string(hex);
}

////////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////////
void IntergasComponent::setup() {

}

////////////////////////////////////////////////////////////////////////////////////////////
// Send command to boiler on each update()
////////////////////////////////////////////////////////////////////////////////////////////
void IntergasComponent::update() 
{
    // read the UART buffer
    std::vector<uint8_t> buffer;
    while (available())
      buffer.push_back(read());  // read byte from UART

    // based on the current task parse the response
    switch (current_task)
    {
    default:
      break;
    case JOB1:
      parse_status_1_(buffer.data(), buffer.size());  
      break;
    case JOB2:
      parse_status_2_(buffer.data(), buffer.size());
      break;
    case JOB3:
      parse_statistics_(buffer.data(), buffer.size());
      break;
    }
    // go to next task
    if (current_task++ == JOB3) {
      current_task = JOB1;  // reset first task 
    }
    switch (current_task)
    {
    default:
      break;
    case JOB1:
      ESP_LOGI("intergas", "Requesting Status 1");
      write_str(STATUS_1);
      break;
    case JOB2:      
      ESP_LOGI("intergas", "Requesting Status 2");
      write_str(STATUS_2);       
      break;
    case JOB3:      
      ESP_LOGI("intergas", "Requesting Statistics");
      write_str(STATISTICS);     
      break;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////
// publish the value to the sensor, making sure the sensor is registered
#define PUBLISH_VALUE(key, value)   if (sensors_.count(key) > 0)        sensors_[key]->publish_state(value)
#define PUBLISH_TEXT(key, value)    if (text_sensors_.count(key) > 0)   text_sensors_[key]->publish_state(value)
#define PUBLISH_BOOL(key, value)    if (bin_sensors_.count(key) > 0)    bin_sensors_[key]->publish_state(value)

////////////////////////////////////////////////////////////////////////////////////////////
// parse status_1 
////////////////////////////////////////////////////////////////////////////////////////////
void IntergasComponent::parse_status_1_(const uint8_t *buf, size_t size) 
{
    if (size < 32) {        // we actually go for 30 bytes, but the last 2 bytes are not used
        ESP_LOGE("intergas", "Status 1 buffer too short: %d bytes", size);
        return;
    }
    PUBLISH_VALUE("heater",     getfloat(buf[ 0], buf[ 1])); 
    PUBLISH_VALUE("inlet",      getfloat(buf[ 2], buf[ 3])); 
    PUBLISH_VALUE("outlet",     getfloat(buf[ 4], buf[ 5]));
    PUBLISH_VALUE("dhw",        getfloat(buf[ 6], buf[ 7]));
    PUBLISH_VALUE("dhw_in",     getfloat(buf[ 8], buf[ 9]));    // NC, always -50.81
    PUBLISH_VALUE("outside",    getfloat(buf[10], buf[11]));    // NC, always -50.81
    PUBLISH_VALUE("pressure",   getfloat(buf[12], buf[13]));
    PUBLISH_VALUE("set_point",  getfloat(buf[14], buf[15]));

    uint16_t fan_set = getfloat(buf[16], buf[17]) * 100;            // target fanspeed, remember the fanspeed for boiler modus
    PUBLISH_VALUE("fan_set",    fan_set);
    PUBLISH_VALUE("fan_rpm",    getfloat(buf[18], buf[19]) * 100); // current fanspeed in rpm
    PUBLISH_VALUE("fan",        getfloat(buf[20], buf[21]) * 10); // current fan speed in percent
    PUBLISH_VALUE("power",      getfloat(buf[22], buf[23]) * GAS_WATT/1000.0f);

    std::string new_mode = "unknown";
    switch (buf[24]) 
    {
    case 126: 
        new_mode = "idle"; break;
    case 231: 
        new_mode = "spindown"; break;
    case 0:   
        if (fan_set > 0)     // heat is requested (Tmax>0), but are we also heating?
            new_mode = "heating"; 
        else
            new_mode = "standby";
        break;
  //  case 102:   // CV burns -> false, this is 0. 
  //  case 51:    // Hot water-> false, maybe its spondown from HW
    case 204: 
        new_mode = "hot_water"; break;
    default:  
        new_mode = "code_0x" + to_hex(buf[24]); break;
    }
    // 25 ??    
    PUBLISH_BOOL("gp_switch",   bool(buf[26] & (1 << 7)));
    PUBLISH_BOOL("tap_switch",  bool(buf[26] & (1 << 6)));
    PUBLISH_BOOL("roomtherm",   bool(buf[26] & (1 << 5)));
    PUBLISH_BOOL("pump_on",        bool(buf[26] & (1 << 4)));
    PUBLISH_BOOL("dwk",         bool(buf[26] & (1 << 3)));
    PUBLISH_BOOL("alarm",       bool(buf[26] & (1 << 2)));
    PUBLISH_BOOL("ch_cascade",  bool(buf[26] & (1 << 1)));
    PUBLISH_BOOL("opentherm",   bool(buf[26] & (1 << 0)));

    if (buf[27] == 128) {        // what about the other bits? 
        PUBLISH_VALUE("fault_code", buf[29]);      // current listed fault code is the active fault code
    } else {
        PUBLISH_VALUE("fault_code", 0);
        PUBLISH_VALUE("last_fault", buf[29]);
    }
    PUBLISH_BOOL("gasvalve",        bool(buf[26] & (1 << 7)));
    PUBLISH_BOOL("spark",           bool(buf[26] & (1 << 6)));
    PUBLISH_BOOL("io_signal",       bool(buf[26] & (1 << 5)));
    PUBLISH_BOOL("ch_ot_disabled",  bool(buf[26] & (1 << 4)));
    PUBLISH_BOOL("low_pressure",    bool(buf[26] & (1 << 3)));
    PUBLISH_BOOL("pressure_sensor", bool(buf[26] & (1 << 2)));
    bool burner_block =             bool(buf[28] & (1 << 1));
    PUBLISH_BOOL("burner_block",    burner_block);
    PUBLISH_BOOL("grad_flag",       bool(buf[26] & (1 << 0)));
    
    if (burner_block) 
        new_mode = "lock";

    // 30 ??
    // 31 ??	
    PUBLISH_TEXT("mode", new_mode);
}

////////////////////////////////////////////////////////////////////////////////////////////
// parse status_2
////////////////////////////////////////////////////////////////////////////////////////////
void IntergasComponent::parse_status_2_(const uint8_t *buf, size_t size) 
{
    if (size < 32) {    // we actually go for 10 bytes, but the last 22 bytes are not used
        ESP_LOGE("intergas", "Status 2 buffer too short: %d bytes", size);
        return;
    }
    PUBLISH_VALUE("flow",       getfloat(buf[ 0], buf[ 1])); // tap flow in l/min
    PUBLISH_VALUE("pump",       (200 - buf[2]) / 2.0f); // pump speed in percent
    // data['OT_master_member_id']   = ig_raw[3]
    //  T_z1_override = getFloat(sbuf[4], sbuf[5]);
    PUBLISH_VALUE("room_set",   getfloat(buf[ 6], buf[ 7]));
    PUBLISH_VALUE("room",       getfloat(buf[ 8], buf[ 9]));

    //  T_z2_override = getFloat(sbuf[10], sbuf[11]);   // ?
    //  T_z2_set      = getFloat(sbuf[12], sbuf[13]);   // requested room temperature zone 2
    //  T_z2_cur      = getFloat(sbuf[14], sbuf[15]);   // current room temperature
    //  outside   = getFloat(sbuf[16], sbuf[17]);   // override_outside_temp?     always 327.67 (0xFFFF = signed -1)
    // data['OT_therm_prod_version'] = ig_raw[18]
    // data['OT_therm_prod_type']    = ig_raw[19]  

    // 20 - 31?
}

////////////////////////////////////////////////////////////////////////////////////////////
// parse statistics
////////////////////////////////////////////////////////////////////////////////////////////
void IntergasComponent::parse_statistics_(const uint8_t *buf, size_t size) 
{
    if (size < 32) {    // we actually go for 24 bytes, but the last 8 bytes are not used
        ESP_LOGE("intergas", "Statistics buffer too short: %d bytes", size);
        return;
    }
    // 0 - 15?
    float raw_cv = (float)(buf[16] + (buf[17] << 8) + (buf[18] << 16) + (buf[19] << 24));
    float raw_hw = (float)(buf[20] + (buf[21] << 8) + (buf[22] << 16) + (buf[23] << 24));
    
    float cv_m3 = raw_cv / GAS_USAGE_CALIBRATED;
    float hw_m3 = raw_hw / GAS_USAGE_CALIBRATED;

    PUBLISH_VALUE("energy_ch",  cv_m3); // cv gas usage in m3
    PUBLISH_VALUE("energy_dhw", hw_m3); // hw gas usage in m3
}
////////////////////////////////////////////////////////////////////////////////////////////};
