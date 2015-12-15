#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_output_layer;
static TextLayer *s_label_layer;
static BitmapLayer *s_icon_layer;
static ActionBarLayer *s_action_bar_layer;
static GBitmap *s_icon_bitmap, *s_tick_bitmap, *s_cross_bitmap;

static DictationSession *s_dictation_session;
static char s_last_text[256];
static char s_buffer[256];

#define UBER 1
#define CABBIE 2
#define TARGET 3
/******************************* Dictation API ********************************/

static void dictation_session_callback(DictationSession *session, DictationSessionStatus status, 
                                       char *transcription, void *context) {
  if(status == DictationSessionStatusSuccess) {
    // Display the dictated text
    DictionaryIterator *iterator;
    app_message_outbox_begin(&iterator);
    Tuplet tuple = TupletCString(TARGET,transcription);
    dict_write_tuplet(iterator, &tuple);
    dict_write_end(iterator);
    Tuple *data = dict_find(iterator, TARGET);
    app_message_outbox_send();
    snprintf(s_last_text, sizeof(s_buffer), "Received '%s'", data->value->cstring);
     //snprintf(s_last_text, sizeof(s_buffer), "Received '%s'", "my name sucks");
    //snprintf(s_last_text, sizeof(s_last_text), "Transcription:\n\n%s", transcription);
    text_layer_set_text(s_output_layer, s_last_text);
  } else {
    // Display the reason for any error
    static char s_failed_buff[128];
    snprintf(s_failed_buff, sizeof(s_failed_buff), "Transcription failed.\n\nError ID:\n%d", (int)status);
    text_layer_set_text(s_output_layer, s_failed_buff);
  }
}

/************************************ App *************************************/

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  // Start voice dictation UI
  dictation_session_start(s_dictation_session);
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Up Button Pressed");
}

static void uberwindow_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_RESOURCE_ID_CONFIRM);
  GRect bitmap_bounds = gbitmap_get_bounds(s_icon_bitmap);

  s_icon_layer = bitmap_layer_create(GRect((bounds.size.w / 2) - (bitmap_bounds.size.w / 2) - (ACTION_BAR_WIDTH / 2), 10, bitmap_bounds.size.w, bitmap_bounds.size.h));
  bitmap_layer_set_bitmap(s_icon_layer, s_icon_bitmap);
  bitmap_layer_set_compositing_mode(s_icon_layer, GCompOpSet);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_icon_layer));

  s_label_layer = text_layer_create(GRect(10, 10 + bitmap_bounds.size.h + 5, 124 - ACTION_BAR_WIDTH, bounds.size.h - (10 + bitmap_bounds.size.h + 15)));
  text_layer_set_text(s_label_layer, "Call Uber?");
  text_layer_set_background_color(s_label_layer, GColorClear);
  text_layer_set_text_alignment(s_label_layer, GTextAlignmentCenter);
  text_layer_set_font(s_label_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(s_label_layer));

  s_tick_bitmap = gbitmap_create_with_resource(RESOURCE_ID_RESOURCE_ID_TICK);
  s_cross_bitmap = gbitmap_create_with_resource(RESOURCE_ID_RESOURCE_ID_CROSS);

  s_action_bar_layer = action_bar_layer_create();
  action_bar_layer_set_icon(s_action_bar_layer, BUTTON_ID_UP, s_tick_bitmap);
  action_bar_layer_set_icon(s_action_bar_layer, BUTTON_ID_DOWN, s_cross_bitmap);
  action_bar_layer_add_to_window(s_action_bar_layer, window);
}

static void uberwindow_unload(Window *window) {
  text_layer_destroy(s_label_layer);
  action_bar_layer_destroy(s_action_bar_layer);
  bitmap_layer_destroy(s_icon_layer);

  gbitmap_destroy(s_icon_bitmap); 
  gbitmap_destroy(s_tick_bitmap);
  gbitmap_destroy(s_cross_bitmap);

  window_destroy(window);
  s_main_window = NULL;
}

void dialog_choice_window_push() {
  if(true) {
    s_main_window = window_create();
    window_set_background_color(s_main_window, GColorJaegerGreen);
    window_set_window_handlers(s_main_window, (WindowHandlers) {
        .load = uberwindow_load,
        .unload = uberwindow_unload,
    });
  }
  window_stack_push(s_main_window, true);
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Down Button Pressed");
  dialog_choice_window_push(); 
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
 // (void) context;
  // Get the first pair
  Tuple *data = dict_read_first(iterator);
  APP_LOG(APP_LOG_LEVEL_ERROR, "message get");
  char uber_buffer[32], cabbie_buffer[32];
  while(data != NULL)
    {
        APP_LOG(APP_LOG_LEVEL_ERROR, "message get");
         int key = data->key;
         char string_value[32];
         strcpy(string_value, data->value->cstring);
         switch(key){
           case UBER:
             strcpy(uber_buffer,string_value);
           case CABBIE:
             strcpy(cabbie_buffer,string_value);
         }
         data = dict_read_next(iterator);
    }
    snprintf(s_last_text, sizeof(s_last_text), "Uber Price:%s \n\n Yellow Cab Price:\n$%s", uber_buffer,cabbie_buffer);
    text_layer_set_text(s_output_layer, s_last_text);
    psleep(250);
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

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);

  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_output_layer = text_layer_create(GRect(bounds.origin.x, (bounds.size.h - 24) / 2, bounds.size.w, bounds.size.h));
  text_layer_set_text(s_output_layer, "Press Select to dictate a destination");
  text_layer_set_text_alignment(s_output_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_output_layer));
}

static void window_unload(Window *window) {
  text_layer_destroy(s_output_layer);
}

static void init() {
  s_main_window = window_create();
  window_set_click_config_provider(s_main_window, click_config_provider);
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  app_message_open(64, 64);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_main_window, true);

  // Create new dictation session
  s_dictation_session = dictation_session_create(sizeof(s_last_text), dictation_session_callback, NULL);
}

static void deinit() {
  // Free the last session data
  dictation_session_destroy(s_dictation_session);

  window_destroy(s_main_window);
}

int main() {
  init();
  app_event_loop();
  deinit();
}
