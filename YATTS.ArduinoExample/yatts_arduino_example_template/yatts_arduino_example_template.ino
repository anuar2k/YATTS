/**
 * This file contains a receiver code template matching the variables defined in:
 * YATTS.Plugin/sample_YATTS.Config.json
 * 
 * The user is free to add any code, which will make use of the received data.
 */

const uint8_t VARS_ID_NONE = 255;

#pragma pack(push, 1)
union {
  struct {
    uint32_t game_time;
    float truck_speed;
    float truck_engine_rpm;
    float truck_navigation_speed_limit;
    int32_t truck_displayed_gear;
    float truck_fuel_amount;
    uint8_t truck_light_lblinker;
    uint8_t truck_light_rblinker;
  } vars;
  uint8_t data[sizeof(vars)];
} channel_vars;

union {
  struct {
    float fuel_capacity;  
  } vars;
  uint8_t data[sizeof(vars)];
} config_vars_truck;

union {
  struct {
    float cargo_mass;
  } vars;
  uint8_t data[sizeof(vars)];
} config_vars_job;
#pragma pop

const size_t vars_sizes[] = {
  sizeof(channel_vars),
  sizeof(config_vars_truck),
  sizeof(config_vars_job)
};

uint8_t* const vars_data_bufs[] = {
  channel_vars.data,
  config_vars_truck.data,
  config_vars_job.data
};

void channel_vars_handler();
void config_vars_game_handler();
void config_vars_job_handler();

void (*const vars_handlers[])() = {
  channel_vars_handler,
  config_vars_game_handler,
  config_vars_job_handler
};

//User section START

//User section END

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;
  //User section START

  //User section END
}

void loop() {
  //User section START
  
  //User section END
  
  static size_t progress = 0;
  static uint8_t selected_vars_id = VARS_ID_NONE;

  while (Serial.available() > 0) {
    if (selected_vars_id == VARS_ID_NONE) {
      selected_vars_id = Serial.read();
      progress = 0;
    }
    else {
      vars_data_bufs[selected_vars_id][progress++] = Serial.read();
      if (progress == vars_sizes[selected_vars_id]) {
        vars_handlers[selected_vars_id]();
        selected_vars_id = VARS_ID_NONE;
      }
    }
  }
}

//User section START
void channel_vars_handler() {
  
}

void config_vars_game_handler() {
  
}

void config_vars_job_handler() {
  
}
//User section END
