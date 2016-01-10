#include <pebble.h>
#define KEY_TEMPERATURE 0
#define KEY_CONDITIONS 1
  

static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_weather_layer;


static GFont s_time_font;
static GFont s_weather_font;

static void update_time() {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  static char buffer[] = "00:00";
  
if(clock_is_24h_style() == true){
  strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
  } else {
  strftime(buffer,sizeof("00:00"), "%I:%M", tick_time);
  }
  
  text_layer_set_text(s_time_layer, buffer);
}

static void main_window_load(Window *window){
  window_set_background_color(s_main_window,GColorBlack);
  s_time_layer = text_layer_create(GRect(0, 45, 144, 255));
  text_layer_set_background_color(s_time_layer,GColorClear);
  text_layer_set_text_color(s_time_layer,GColorWhite);
  //text_layer_set_text(s_time_layer,"00:00");
 // text_layer_set_font(s_time_layer,fonts_get_system_font(FONT_KEY_BITHAM_42_MEDIUM_NUMBERS));
  text_layer_set_text_alignment(s_time_layer,GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_HANDWRITING_72));
  text_layer_set_font(s_time_layer, s_time_font);


  
  //weather
   s_weather_layer = text_layer_create(GRect(0, 130, 144, 25));
   text_layer_set_background_color(s_weather_layer,GColorClear);
  text_layer_set_text_color(s_weather_layer,GColorWhite);
  text_layer_set_text_alignment(s_weather_layer,GTextAlignmentCenter);
  text_layer_set_text(s_weather_layer,"Loading");
  
  //fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);

  //s_weather_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_HANDWRITING_72));
  s_weather_font = fonts_get_system_font(FONT_KEY_GOTHIC_18);
  
  text_layer_set_font(s_weather_layer, s_weather_font);
   layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_weather_layer));

  //text_layer_set_font(s_weather_layer,s_weather_font);
layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_weather_layer));
  update_time();
}
static void main_window_unload(Window *window){
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_weather_layer);
 // fonts_unload_custom_font(s_weather_font);
  
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  
    
  // Get weather update every 30 minutes
  if(tick_time->tm_min % 30 == 0) {
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    // Add a key-value pair
    dict_write_uint8(iter, 0, 0);

    // Send the message!
    app_message_outbox_send();
  }

}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  
  static char temperature_buffer[8];
  static char conditions_buffer[32];
  static char weather_layer_buffer[32];
  
  // Read first item
  Tuple *t = dict_read_first(iterator);

  // For all items
  while(t != NULL) {
    // Which key was received?
    switch(t->key) {
    case KEY_TEMPERATURE:
      snprintf(temperature_buffer, sizeof(temperature_buffer), "%d F", (int)t->value->int32);
      break;
    case KEY_CONDITIONS:
      snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", t->value->cstring);
      break;
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
      break;
    }

    // Look for next item
    t = dict_read_next(iterator);
  }
    snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s - %s", temperature_buffer, conditions_buffer);
  text_layer_set_text(s_weather_layer, weather_layer_buffer);

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





static void init() {
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
});
  window_stack_push(s_main_window,true);
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);


app_message_register_inbox_received(inbox_received_callback);
app_message_register_inbox_dropped(inbox_dropped_callback);
app_message_register_outbox_failed(outbox_failed_callback);
app_message_register_outbox_sent(outbox_sent_callback);

app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}
static void deinit(){
  window_destroy(s_main_window);
  

}
int main(void) {
  init ();
    app_event_loop ();
  deinit ();
}
