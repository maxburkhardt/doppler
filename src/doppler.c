#include <pebble.h>

static Window *window;
static TextLayer *time_text_layer;
static BitmapLayer *radar_image_layer;
static AppTimer *radar_timer;

//static GBitmap radar_image_bitmap;
/*
static uint32_t radar_image_buffer[IMAGE_ROWS][IMAGE_COLS];

void radar_image_mark_dirty() {
    layer_mark_dirty(bitmap_layer_get_layer(radar_image_layer));
}

void radar_image_set_dword(uint16_t index, uint32_t dword) {
    uint8_t row = index / IMAGE_COLS;
    uint8_t col = index % IMAGE_COLS;
    radar_image_buffer[row][col] = dword;
}

static void radar_image_init() {
    radar_image_bitmap = (GBitmap) {
        .addr = radar_image_buffer,
        .row_size_bytes = 20,
        .info_flags = 0x01,
        .bounds = GRect(0, 0, IMAGE_WIDTH, IMAGE_HEIGHT)
    };
    bitmap_layer_set_bitmap(radar_image_layer, &radar_image_bitmap);

    const ResHandle image_handle = resource_get_handle(RESOURCE_ID_IMAGE_DEFAULT);
    const size_t image_res_size = resource_size(image_handle);
    const size_t image_header_size = sizeof(GBitmap) - sizeof(void *);
    resource_load_byte_range(image_handle, image_header_size, (uint8_t *) radar_image_buffer, image_res_size - image_header_size);

    radar_image_mark_dirty();
}
*/

void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
  static char time_text[] = "00:00";
  static char date_text[] = "Xxxxxxxxx 00";

  char *time_format;

  if (!tick_time) {
    time_t now = time(NULL);
    tick_time = localtime(&now);
  }

  // TODO: Only update the date when it's changed.
  strftime(date_text, sizeof(date_text), "%B %e", tick_time);
  // text_layer_set_text(text_date_layer, date_text);


  if (clock_is_24h_style()) {
    time_format = "%R";
  } else {
    time_format = "%I:%M";
  }

  strftime(time_text, sizeof(time_text), time_format, tick_time);

  // Kludge to handle lack of non-padded hour format string
  // for twelve hour clock.
  if (!clock_is_24h_style() && (time_text[0] == '0')) {
    memmove(time_text, &time_text[1], sizeof(time_text) - 1);
  }

  text_layer_set_text(time_text_layer, time_text);
}

void request_radar() {
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
    if (iter == NULL) 
        APP_LOG(APP_LOG_LEVEL_DEBUG, "null iterator");
    Tuplet tuple = TupletInteger(1, 1);
    dict_write_tuplet(iter, &tuple);
    dict_write_end(iter);
    app_message_outbox_send();
    radar_timer = app_timer_register(30000, request_radar, NULL);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  time_text_layer = text_layer_create((GRect) { .origin = { 0, 10 }, .size = { bounds.size.w, 20 } });
  text_layer_set_text(time_text_layer, "Press a button");
  text_layer_set_text_color(time_text_layer, GColorWhite);
  text_layer_set_background_color(time_text_layer, GColorClear);
  text_layer_set_text_alignment(time_text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(time_text_layer));

  radar_image_layer = bitmap_layer_create(GRect(0, 24, 144, 144));
  //radar_image_init();
  layer_add_child(window_layer, bitmap_layer_get_layer(radar_image_layer));
}

static void window_unload(Window *window) {
  text_layer_destroy(time_text_layer);
}

static void in_received_handler(DictionaryIterator *iter, void *context) {
}

static void in_dropped_handler(AppMessageResult reason, void *context) {
}

static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
}

static void app_message_init(void) {
    app_message_register_inbox_received(in_received_handler);
    app_message_register_inbox_dropped(in_dropped_handler);
    app_message_register_outbox_failed(out_failed_handler);
    app_message_open(124, 124);
}

static void init(void) {
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);
  window_set_background_color(window, GColorBlack);
  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
  app_message_init();
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}
