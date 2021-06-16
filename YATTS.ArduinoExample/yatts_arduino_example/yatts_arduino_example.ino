#include <TFT_HX8357.h>

#include "blinker_l.h"
#include "blinker_r.h"
#include "weight.h"
#include "clock_icon.h"
#include "gear.h"
#include "fuel.h"

#define WEIGHT_COLOR 0x051D
#define FUEL_COLOR 0xFE41
const uint8_t VARS_ID_NONE = 255;

#pragma pack(push, 1)
union {
  struct {
    uint32_t game_time;
    float truck_speed;
    float truck_engine_rpm;
    int32_t truck_navigation_speed_limit;
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
void config_vars_truck_handler();
void config_vars_job_handler();

void (*const vars_handlers[])() = {
  channel_vars_handler,
  config_vars_truck_handler,
  config_vars_job_handler
};

//User section START
void draw_icon(const unsigned short* icon, int16_t x, int16_t y, int8_t width, int8_t height);

const uint16_t DISP_W = 480;
const uint16_t DISP_H = 320;

const uint16_t outer_lim_rad = 46;
const uint16_t inner_lim_rad = 38;

const uint16_t outer_clock_rad = 80;
const uint16_t inner_clock_rad = 78;
const uint16_t tick_rad = 74;
const uint16_t needle_rad = 70;
const uint16_t needle_dot_rad = 3;
const uint16_t txt_rad = 82;
const uint16_t clock_off = 46;
const float eight_angle = TWO_PI / 8;
const float fortyth_angle = TWO_PI / 40;
const float first_mark = eight_angle * 3;
uint8_t datums[] = {TR_DATUM, MR_DATUM, R_BASELINE, C_BASELINE, L_BASELINE, ML_DATUM, TL_DATUM, TC_DATUM};

const uint16_t rev_meter_x = clock_off + outer_clock_rad;
const uint16_t rev_meter_y = DISP_H - clock_off - outer_clock_rad;

const uint16_t spd_meter_x = DISP_W - clock_off - outer_clock_rad;
const uint16_t spd_meter_y = DISP_H - clock_off - outer_clock_rad;

TFT_HX8357 tft;

void drawBackground() {
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.fillScreen(TFT_BLACK);

  draw_icon(weight, 10, 10, 32, 32);
  draw_icon(clock_icon, 10, 10 + 32 + 10, 32, 32);
  draw_icon(gear, DISP_W - 10 - 32, 10, 32, 32);
  draw_icon(fuel, DISP_W - 10 - 32, 10 + 32 + 10, 32, 32);

  //speed limit
  tft.fillCircle(DISP_W / 2, 10 + outer_lim_rad, outer_lim_rad, TFT_RED);
  tft.fillCircle(DISP_W / 2, 10 + outer_lim_rad, inner_lim_rad, TFT_WHITE);

  //rev meter
  for (size_t i = 0; i < 7; i++) {
    const float angle = first_mark + i * eight_angle;
    tft.setTextDatum(datums[i]);
    tft.drawNumber(i * 5, rev_meter_x + (cos(angle) * txt_rad), rev_meter_y + (sin(angle) * txt_rad), 4);
  }
  tft.setTextDatum(datums[7]);
  tft.drawString("rpm x100", rev_meter_x, rev_meter_y + txt_rad, 2);

  tft.fillCircle(rev_meter_x, rev_meter_y, outer_clock_rad, TFT_WHITE);
  tft.fillCircle(rev_meter_x, rev_meter_y, inner_clock_rad, TFT_DARKGREY);
  
  for (size_t i = 0; i < 31; i++) {
    const float angle = first_mark + i * fortyth_angle;
    const float ang_cos = cos(angle);
    const float ang_sin = sin(angle);
    
    tft.drawLine(
      rev_meter_x + (ang_cos * tick_rad), 
      rev_meter_y + (ang_sin * tick_rad), 
      rev_meter_x + (ang_cos * inner_clock_rad), 
      rev_meter_y + (ang_sin * inner_clock_rad), TFT_WHITE
    );
  }
  
  //speedo
  for (size_t i = 0; i < 7; i++) {
    const float angle = first_mark + i * eight_angle;
    tft.setTextDatum(datums[i]);
    tft.drawNumber(i * 25, spd_meter_x + (cos(angle) * txt_rad), spd_meter_y + (sin(angle) * txt_rad), 4);
  }
  tft.setTextDatum(datums[7]);
  tft.drawString("km/h", spd_meter_x, spd_meter_y + txt_rad, 2);
  
  tft.fillCircle(spd_meter_x, spd_meter_y, outer_clock_rad, TFT_WHITE);
  tft.fillCircle(spd_meter_x, spd_meter_y, inner_clock_rad, TFT_DARKGREY);

  for (size_t i = 0; i < 31; i++) {
    const float angle = first_mark + i * fortyth_angle;
    const float ang_cos = cos(angle);
    const float ang_sin = sin(angle);
    
    tft.drawLine(
      spd_meter_x + (ang_cos * tick_rad), 
      spd_meter_y + (ang_sin * tick_rad), 
      spd_meter_x + (ang_cos * inner_clock_rad), 
      spd_meter_y + (ang_sin * inner_clock_rad), TFT_WHITE
    );
  }
}

//User section END

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;
  //User section START
  tft.init();
  tft.setRotation(1);
  
  drawBackground();
   //User section END
}

void loop() {
  //User section START
  
  //User section END
  
  static size_t progress;
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
struct line {
  int16_t x_to;
  int16_t y_to;
};

void drawFatLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
  tft.drawLine(x0, y0, x1, y1, color);
  tft.drawLine(x0 + 1, y0, x1 + 1, y1, color);
  tft.drawLine(x0, y0 + 1, x1, y1 + 1, color);
  tft.drawLine(x0 + 1, y0 + 1, x1 + 1, y1 + 1, color);
}

void channel_vars_handler() {
  static line prev_rpm = {
    .x_to = rev_meter_x, .y_to = rev_meter_y
  };
  static line prev_spd = {
    .x_to = spd_meter_x, .y_to = spd_meter_y
  };
  static uint8_t prev_lblinker = 0;
  static uint8_t prev_rblinker = 0;
  static uint32_t prev_time = 0;
  static int32_t prev_limit = 0;
  static int32_t prev_gear = -999;
  static float prev_fuel = 0;

  drawFatLine(rev_meter_x, rev_meter_y, prev_rpm.x_to, prev_rpm.y_to, TFT_DARKGREY);
  prev_rpm.x_to = rev_meter_x + (cos(first_mark + channel_vars.vars.truck_engine_rpm) * needle_rad);
  prev_rpm.y_to = rev_meter_y + (sin(first_mark + channel_vars.vars.truck_engine_rpm) * needle_rad);
  drawFatLine(rev_meter_x, rev_meter_y, prev_rpm.x_to, prev_rpm.y_to, TFT_CYAN);
  tft.fillCircle(rev_meter_x, rev_meter_y, needle_dot_rad, TFT_CYAN);

  drawFatLine(spd_meter_x, spd_meter_y, prev_spd.x_to, prev_spd.y_to, TFT_DARKGREY);
  prev_spd.x_to = spd_meter_x + (cos(first_mark + channel_vars.vars.truck_speed) * needle_rad);
  prev_spd.y_to = spd_meter_y + (sin(first_mark + channel_vars.vars.truck_speed) * needle_rad);
  drawFatLine(spd_meter_x, spd_meter_y, prev_spd.x_to, prev_spd.y_to, TFT_CYAN);
  tft.fillCircle(spd_meter_x, spd_meter_y, needle_dot_rad, TFT_CYAN);

  if (channel_vars.vars.truck_navigation_speed_limit != prev_limit) {
    prev_limit = channel_vars.vars.truck_navigation_speed_limit;
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_BLACK, TFT_WHITE);
    tft.fillCircle(DISP_W / 2, 10 + outer_lim_rad, inner_lim_rad, TFT_WHITE);
    tft.drawNumber(prev_limit, DISP_W / 2, 10 + outer_lim_rad + 4, 6);
  }

  if (channel_vars.vars.game_time != prev_time) {
    prev_time = channel_vars.vars.game_time;
    uint32_t hour = prev_time / 60 % 24;
    uint32_t minute = prev_time % 60;
    String t;
    if (hour < 10) {
      t += '0';
    }
    t += hour;
    t += ':';
    if (minute < 10) {
      t += '0';
    }
    t += minute;
    tft.fillRect(45, 10 + 32 + 10, 100, 32, TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextDatum(TL_DATUM);
    tft.drawString(t.c_str(), 45, 10 + 32 + 10 + 4, 4);
  }

  if (channel_vars.vars.truck_displayed_gear != prev_gear) {
    prev_gear = channel_vars.vars.truck_displayed_gear;
    tft.fillRect(DISP_W - 45 - 100, 10, 100, 32, TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextDatum(TR_DATUM);
    if (prev_gear == 0) {
      tft.drawString("N", DISP_W - 45, 10 + 4, 4);
    }
    else {
      tft.drawNumber(prev_gear, DISP_W - 45, 10 + 4, 4);
    }
  }

  if (channel_vars.vars.truck_light_lblinker != prev_lblinker) {
    prev_lblinker = channel_vars.vars.truck_light_lblinker;
    const int16_t lblinker_x = 150;
    const int16_t lblinker_y = 10;
    if (prev_lblinker) {
      draw_icon(blinker_l, lblinker_x, lblinker_y, 32, 32); 
    }
    else {
      tft.fillRect(lblinker_x, lblinker_y, 32, 32, TFT_BLACK);
    }
  }
   
  if (channel_vars.vars.truck_light_rblinker != prev_rblinker) {
    prev_rblinker = channel_vars.vars.truck_light_rblinker;
    const int16_t rblinker_x = 480 - 150 - 32;
    const int16_t rblinker_y = 10;
    if (prev_rblinker) {
      draw_icon(blinker_r, rblinker_x, rblinker_y, 32, 32); 
    }
    else {
      tft.fillRect(rblinker_x, rblinker_y, 32, 32, TFT_BLACK);
    }
  }

  float diff = prev_fuel - channel_vars.vars.truck_fuel_amount;
  if (abs(diff) > 1.0f) {
    prev_fuel = channel_vars.vars.truck_fuel_amount;
    tft.fillRect(DISP_W - 45 - 100, 10 + 32 + 10, 100, 32, TFT_BLACK);

    if (config_vars_truck.vars.fuel_capacity > 0.0f) {
      float percentage = prev_fuel / config_vars_truck.vars.fuel_capacity * 100;
      String p = String(percentage, 0) + '%';
      tft.setTextColor(FUEL_COLOR, TFT_BLACK);
      tft.setTextDatum(TR_DATUM);
      tft.drawString(p.c_str(), DISP_W - 45, 10 + 32 + 10 + 4, 4);
    }
  }
}

void config_vars_truck_handler() {
  
}

void config_vars_job_handler() {
  tft.fillRect(45, 10, 100, 32, TFT_BLACK);
  tft.setTextColor(WEIGHT_COLOR, TFT_BLACK);
  tft.setTextDatum(TL_DATUM);
  tft.drawNumber(config_vars_job.vars.cargo_mass, 45, 10 + 8, 4);
}

#define BUFF_SIZE 64

void draw_icon(const unsigned short* icon, int16_t x, int16_t y, int8_t width, int8_t height) {

  uint16_t  pix_buffer[BUFF_SIZE];   // Pixel buffer (16 bits per pixel)

  // Set up a window the right size to stream pixels into
  tft.setWindow(x, y, x + width - 1, y + height - 1);

  // Work out the number whole buffers to send
  uint16_t nb = ((uint16_t)height * width) / BUFF_SIZE;

  // Fill and send "nb" buffers to TFT
  for (int i = 0; i < nb; i++) {
    for (int j = 0; j < BUFF_SIZE; j++) {
      pix_buffer[j] = pgm_read_word(&icon[i * BUFF_SIZE + j]);
    }
    tft.pushColors(pix_buffer, BUFF_SIZE);
  }

  // Work out number of pixels not yet sent
  uint16_t np = ((uint16_t)height * width) % BUFF_SIZE;

  // Send any partial buffer left over
  if (np) {
    for (int i = 0; i < np; i++) pix_buffer[i] = pgm_read_word(&icon[nb * BUFF_SIZE + i]);
    tft.pushColors(pix_buffer, np);
  }
}
//User section END
