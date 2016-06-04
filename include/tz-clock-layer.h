#pragma once

typedef Layer TzClockLayer;

TzClockLayer* tz_clock_layer_create(GRect frame, char api_key[30], char timezone[30]);
void tz_clock_layer_destroy(TzClockLayer *tz_clock_layer);
void tz_clock_tick_event(TzClockLayer *tz_clock_layer, struct tm *local_tick_time, TimeUnits units_changed);
void tz_clock_layer_set_font(TzClockLayer *tz_clock_layer, GFont font);
void tz_clock_layer_set_text_alignment(TzClockLayer *tz_clock_layer, GTextAlignment alignment);
void tz_clock_layer_set_background_color(TzClockLayer *tz_clock_layer, GColor color);
void tz_clock_layer_set_text_color(TzClockLayer *tz_clock_layer, GColor color);
GSize tz_clock_layer_get_content_size(TzClockLayer *tz_clock_layer);
