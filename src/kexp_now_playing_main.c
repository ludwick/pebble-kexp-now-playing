#include <pebble.h>

static Window *window;
static TextLayer *title_text_layer;
static TextLayer *current_artist_text_layer;
static TextLayer *current_song_text_layer;
static TextLayer *current_album_text_layer;

// Functions for fetching the song:

static char current_artist[50] = "The Artist Name is Looong!";
static char current_song[50] = "Song!";
static char current_album[50] = "Album Name!";

static void display_current() {
  text_layer_set_text(current_artist_text_layer, current_artist);
  text_layer_set_text(current_song_text_layer, current_song);
  text_layer_set_text(current_album_text_layer, current_album);
}

#define KEY_ARTIST 0
#define KEY_SONG 1
#define KEY_ALBUM 2
  
static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Read first item
  Tuple *t = dict_read_first(iterator);

  // For all items
  while(t != NULL) {
    // Which key was received?
    switch(t->key) {
    case KEY_ARTIST:
      snprintf(current_artist, sizeof(current_artist), "%s", t->value->cstring);
      break;
    case KEY_SONG:
      snprintf(current_song, sizeof(current_song), "%s", t->value->cstring);
      break;
    case KEY_ALBUM:
      snprintf(current_album, sizeof(current_album), "%s", t->value->cstring);
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

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  //text_layer_set_text(text_layer, "Up");
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  //text_layer_set_text(text_layer, "Down");
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  //window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  //window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Things to fix about this layout:
  // - Title should get some decoration to make it clear what it is.
  // - Artist name should be larger
  // - Album should be italicized
  // - Positions of the artist/album/positions should be calculated from box.
  //
  title_text_layer = text_layer_create(GRect(5, 5, bounds.size.w, 30));
  text_layer_set_text(title_text_layer, "KEXP Now Playing");
  text_layer_set_text_alignment(title_text_layer, GTextAlignmentCenter);
  text_layer_set_font(title_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(title_text_layer));
  
  current_artist_text_layer = text_layer_create((GRect) { .origin = { 0, 35 }, .size = { bounds.size.w, 30 } });
  text_layer_set_text_alignment(current_artist_text_layer, GTextAlignmentCenter);
  text_layer_set_overflow_mode(current_artist_text_layer, GTextOverflowModeTrailingEllipsis);
  text_layer_set_font(current_artist_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  layer_add_child(window_layer, text_layer_get_layer(current_artist_text_layer));

  current_song_text_layer = text_layer_create((GRect) { .origin = { 0, 70 }, .size = { bounds.size.w, 30 } });
  text_layer_set_text_alignment(current_song_text_layer, GTextAlignmentCenter);
  text_layer_set_overflow_mode(current_artist_text_layer, GTextOverflowModeTrailingEllipsis);
  text_layer_set_font(current_song_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  layer_add_child(window_layer, text_layer_get_layer(current_song_text_layer));

  current_album_text_layer = text_layer_create((GRect) { .origin = { 0, 105 }, .size = { bounds.size.w, 30 } });
  text_layer_set_text_alignment(current_album_text_layer, GTextAlignmentCenter);
  text_layer_set_overflow_mode(current_album_text_layer, GTextOverflowModeTrailingEllipsis);
  text_layer_set_font(current_album_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  layer_add_child(window_layer, text_layer_get_layer(current_album_text_layer));

  // JS on ready loads data, so this just displays
  display_current();
}

static void window_unload(Window *window) {
  text_layer_destroy(title_text_layer);
  text_layer_destroy(current_song_text_layer);
}

static void init(void) {
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  
  window = window_create();
  window_set_click_config_provider(window, click_config_provider);
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