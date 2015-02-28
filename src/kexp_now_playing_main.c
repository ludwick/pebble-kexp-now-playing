/*
 * Copyright (C) 2015, Rachael Ludwick
 * Licensed under the terms of the MIT License.
 */

#include <pebble.h>

static Window *window;
static TextLayer *title_text_layer;
static ScrollLayer *current_info_scroll_layer;
static TextLayer *current_info_text_layer;
static GRect bounds;

static char current_info[255] = "--\n--\n--";

static void log_rect(char* label, GRect r) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "%s bounds: origin: [%d, %d], size: [%d, %d]\n", label, r.origin.x,r.origin.y,r.size.w, r.size.h);
}

static void display_current() {
  text_layer_set_text(current_info_text_layer, current_info);
  GSize max_size = text_layer_get_content_size(current_info_text_layer);
  //text_layer_set_size(current_info_text_layer, max_size);
  scroll_layer_set_content_size(current_info_scroll_layer, GSize(bounds.size.w, max_size.h + 4));
}

#define KEY_SONG_INFO 0
static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Read first item
  Tuple *t = dict_read_first(iterator);

  // For all items
  while(t != NULL) {
    // Which key was received?
    switch(t->key) {
    case KEY_SONG_INFO:
      snprintf(current_info, sizeof(current_info), "%s", t->value->cstring);
      break;
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
      break;
    }

    // Look for next item
    t = dict_read_next(iterator);
  }
  
  display_current();
}
                      
static void trigger_update() {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  // Add a key-value pair
  dict_write_uint8(iter, 0, 0);

  // Send the message!
  app_message_outbox_send();
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  trigger_update();
  display_current();
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  bounds = layer_get_bounds(window_layer);
  
  int16_t box_w = bounds.size.w;
  int16_t box_h = bounds.size.h;
  log_rect("window", bounds);
  
  const int16_t title_o_x = 5;
  const int16_t title_o_y = 5;
  const int16_t title_h = 30;

  GRect title_box = (GRect) { .origin = {title_o_x, title_o_y}, .size = { box_w, title_h }};
  log_rect("title", title_box);
  title_text_layer = text_layer_create(title_box);
  text_layer_set_text(title_text_layer, "KEXP Now Playing");
  text_layer_set_text_alignment(title_text_layer, GTextAlignmentCenter);
  text_layer_set_font(title_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(title_text_layer));
  
  int16_t remaining_h = box_h - title_o_y - title_h;
  const int16_t padding = 2;

  GRect scroll_layer_box = (GRect) {
    .origin = { 0, title_o_y + title_h + padding },
    .size = { box_w, remaining_h}
  };
  log_rect("scroll layer box", scroll_layer_box );

  current_info_scroll_layer = scroll_layer_create(scroll_layer_box);
  scroll_layer_set_click_config_onto_window(current_info_scroll_layer, window);
  layer_add_child(window_layer, scroll_layer_get_layer(current_info_scroll_layer));
  scroll_layer_set_callbacks(current_info_scroll_layer, (ScrollLayerCallbacks) {
    .click_config_provider = click_config_provider
    }
  );

  GRect text_layer_box = (GRect) {
    .origin = { 0, 0 },
    .size = { box_w, scroll_layer_box.size.h*3 }
  };
  log_rect("text layer box", text_layer_box );

  current_info_text_layer = text_layer_create(text_layer_box);
  text_layer_set_text_alignment(current_info_text_layer, GTextAlignmentCenter);
  text_layer_set_overflow_mode(current_info_text_layer, GTextOverflowModeTrailingEllipsis);
  text_layer_set_font(current_info_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));

  scroll_layer_add_child(current_info_scroll_layer, text_layer_get_layer(current_info_text_layer));
  scroll_layer_set_content_size(current_info_scroll_layer, text_layer_get_content_size(current_info_text_layer));

  // JS on ready loads data, so this just displays
  display_current();
}

static void window_unload(Window *window) {
  text_layer_destroy(title_text_layer);
  text_layer_destroy(current_info_text_layer);
  scroll_layer_destroy(current_info_scroll_layer);
}

static void init(void) {
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
