#include "pebble.h"

static Window *s_main_window;

static TextLayer *s_USD_layer;
static TextLayer *s_EURO_layer;
static BitmapLayer *s_icon_layer;
static GBitmap *s_icon_bitmap = NULL;

static AppSync s_sync;
static uint8_t s_sync_buffer[64];

enum RateKey {
  RATE_ICON_KEY = 0x0,         // TUPLE_INT
  RATE_USD_KEY = 0x1,          // TUPLE_CSTRING
  RATE_EURO_KEY = 0x2,         // TUPLE_CSTRING
};

static const uint32_t RATE_ICONS[] = {
  RESOURCE_ID_RATE_ICONS // 0
};

static void sync_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Sync Error: %d", app_message_error);
}

static void sync_tuple_changed_callback(const uint32_t key, const Tuple* new_tuple, const Tuple* old_tuple, void* context) {
  switch (key) {
    case RATE_ICON_KEY:
      if (s_icon_bitmap) {
        gbitmap_destroy(s_icon_bitmap);
      }

      s_icon_bitmap = gbitmap_create_with_resource(RATE_ICONS[new_tuple->value->uint8]);
      bitmap_layer_set_compositing_mode(s_icon_layer, GCompOpSet);
      bitmap_layer_set_bitmap(s_icon_layer, s_icon_bitmap);
      break;

    case RATE_USD_KEY:
      // App Sync keeps new_tuple in s_sync_buffer, so we may use it directly
      text_layer_set_text(s_USD_layer, new_tuple->value->cstring);
      break;

    case RATE_EURO_KEY:
      text_layer_set_text(s_EURO_layer, new_tuple->value->cstring);
      break;
  }
}

static void request_rate(void) {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  if (!iter) {
    // Error creating outbound message
    return;
  }

  int value = 1;
  dict_write_int(iter, 1, &value, sizeof(int), true);
  dict_write_end(iter);

  app_message_outbox_send();
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_icon_layer = bitmap_layer_create(GRect(0, 10, bounds.size.w, 80));
  layer_add_child(window_layer, bitmap_layer_get_layer(s_icon_layer));

  s_USD_layer = text_layer_create(GRect(0, 90, bounds.size.w, 32));
  text_layer_set_text_color(s_USD_layer, GColorWhite);
  text_layer_set_background_color(s_USD_layer, GColorClear);
  text_layer_set_font(s_USD_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(s_USD_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_USD_layer));

  s_EURO_layer = text_layer_create(GRect(0, 122, bounds.size.w, 32));
  text_layer_set_text_color(s_EURO_layer, GColorWhite);
  text_layer_set_background_color(s_EURO_layer, GColorClear);
  text_layer_set_font(s_EURO_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(s_EURO_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_EURO_layer));

  Tuplet initial_values[] = {
    TupletInteger(RATE_ICON_KEY, (uint8_t) 0),
    TupletCString(RATE_USD_KEY, "Loading.."),
    TupletCString(RATE_EURO_KEY, "Loading.."),
  };

  app_sync_init(&s_sync, s_sync_buffer, sizeof(s_sync_buffer),
      initial_values, ARRAY_LENGTH(initial_values),
      sync_tuple_changed_callback, sync_error_callback, NULL);

  request_rate();
}

static void window_unload(Window *window) {
  if (s_icon_bitmap) {
    gbitmap_destroy(s_icon_bitmap);
  }

  text_layer_destroy(s_EURO_layer);
  text_layer_destroy(s_USD_layer);
  bitmap_layer_destroy(s_icon_layer);
}

static void init(void) {
  s_main_window = window_create();
  window_set_background_color(s_main_window, PBL_IF_COLOR_ELSE(GColorOrange, GColorGray));
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload
  });
  window_stack_push(s_main_window, true);

  app_message_open(64, 64);
}

static void deinit(void) {
  window_destroy(s_main_window);

  app_sync_deinit(&s_sync);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
