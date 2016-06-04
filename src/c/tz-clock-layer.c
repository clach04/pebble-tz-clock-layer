#include <pebble.h>
#include "tz-clock-layer.h"

typedef struct TzClockSettings {
  char api_key[30];
  char timezone[30];
  int32_t offset_seconds;
  TextLayer *text_layer;
  char time_buffer[8];
  bool app_ready;
} TzClockSettings;


static void tz_clock_force_tick(TzClockLayer *tz_clock_layer);

static void tz_clock_msg_failure() {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to send request!");
}

static void tz_clock_fetch(TzClockLayer *tz_clock_layer) {
  TzClockSettings *data = layer_get_data(tz_clock_layer);

  if(!data->app_ready) {
    return;
  }

  DictionaryIterator *out;
  AppMessageResult result = app_message_outbox_begin(&out);
  if(result != APP_MSG_OK) {
    tz_clock_msg_failure();
  }

  dict_write_cstring(out, MESSAGE_KEY_API_KEY, data->api_key);
  dict_write_cstring(out, MESSAGE_KEY_TIMEZONE, data->timezone);

  result = app_message_outbox_send();
  if(result != APP_MSG_OK) {
    tz_clock_msg_failure();
  }
}

static void tz_clock_inbox_received_handler(DictionaryIterator *iter, void *context) {
  TzClockLayer *tz_clock_layer = (TzClockLayer*) context;
  TzClockSettings *data = layer_get_data(tz_clock_layer);

  Tuple *ready_tuple = dict_find(iter, MESSAGE_KEY_APP_READY);
  if(ready_tuple) {
    data->app_ready = true;
    tz_clock_force_tick(tz_clock_layer);
  }

  Tuple *offset_tuple = dict_find(iter, MESSAGE_KEY_OFFSET);
  if(offset_tuple) {
    data->offset_seconds = offset_tuple->value->int32;
    tz_clock_force_tick(tz_clock_layer);
  }
}

void tz_clock_tick_event(TzClockLayer *tz_clock_layer, struct tm *local_tick_time, TimeUnits units_changed) {
  TzClockSettings *data = layer_get_data(tz_clock_layer);
  if (units_changed & DAY_UNIT) {
    tz_clock_fetch(tz_clock_layer);
  }
  if(data->offset_seconds == -1) {
    return;
  }
  if (units_changed & MINUTE_UNIT) {
    time_t now_seconds = mktime(local_tick_time);
    now_seconds += data->offset_seconds;
    struct tm *remote_tick_time = gmtime(&now_seconds);
    strftime(data->time_buffer, sizeof(data->time_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", remote_tick_time);
    text_layer_set_text(data->text_layer, data->time_buffer);
  }
}

static void tz_clock_force_tick(TzClockLayer *tz_clock_layer) {
	time_t now = time(NULL);
	struct tm *tick_time = localtime(&now);
	tz_clock_tick_event(tz_clock_layer, tick_time, DAY_UNIT|MINUTE_UNIT);
}

// Create a set of default settings.
static void tz_clock_set_defaults(TzClockSettings *data) {
  data->offset_seconds = -1;
  data->app_ready = false;
}

TzClockLayer* tz_clock_layer_create(GRect frame, char api_key[30], char timezone[30]) {
  static Layer *tz_clock_layer;
  tz_clock_layer = layer_create_with_data(frame, sizeof(TzClockSettings));

  TzClockSettings *data = layer_get_data(tz_clock_layer);
  tz_clock_set_defaults(data);
  memcpy(data->api_key, api_key, sizeof(data->api_key));
  memcpy(data->timezone, timezone, sizeof(data->timezone));

  data->text_layer = text_layer_create(GRect(0, 0, frame.size.w, frame.size.h));
  layer_add_child(tz_clock_layer, text_layer_get_layer(data->text_layer));

  app_message_register_inbox_received(tz_clock_inbox_received_handler);
  app_message_set_context(tz_clock_layer);
  app_message_open(256, 256);

  return tz_clock_layer;
}

void tz_clock_layer_destroy(TzClockLayer *tz_clock_layer) {
  TzClockSettings *data = layer_get_data(tz_clock_layer);

  layer_remove_from_parent(text_layer_get_layer(data->text_layer));
  layer_remove_from_parent(tz_clock_layer);
  layer_destroy(text_layer_get_layer(data->text_layer));
  layer_destroy(tz_clock_layer);
}

void tz_clock_layer_set_font(TzClockLayer *tz_clock_layer, GFont font) {
  TzClockSettings *data = layer_get_data(tz_clock_layer);
  text_layer_set_font(data->text_layer, font);
}

void tz_clock_layer_set_text_alignment(TzClockLayer *tz_clock_layer, GTextAlignment alignment) {
  TzClockSettings *data = layer_get_data(tz_clock_layer);
  text_layer_set_text_alignment(data->text_layer, alignment);
}

void tz_clock_layer_set_background_color(TzClockLayer *tz_clock_layer, GColor color) {
  TzClockSettings *data = layer_get_data(tz_clock_layer);
  text_layer_set_background_color(data->text_layer, color);
}

void tz_clock_layer_set_text_color(TzClockLayer *tz_clock_layer, GColor color) {
  TzClockSettings *data = layer_get_data(tz_clock_layer);
  text_layer_set_text_color(data->text_layer, color);
}

GSize tz_clock_layer_get_content_size(TzClockLayer *tz_clock_layer) {
  TzClockSettings *data = layer_get_data(tz_clock_layer);
  return text_layer_get_content_size(data->text_layer);
}
