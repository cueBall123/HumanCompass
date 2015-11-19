/*
 * main.c
 * Constructs a Window and TextLayer, then subscribes to the CompassService to
 * output the data. Also shows state of calibration.
 */

#include <pebble.h>

static uint16_t bearing = 0;
static uint8_t threshold = 10;
static Window *s_main_window;
static TextLayer *s_output_layer;
static TextLayer *s_price_layer;
static char s_symbol[5];
static void compass_handler(CompassHeadingData data) {
  // Allocate a static output buffer
  static char s_buffer[32];
// display heading in degrees and radians
  static char s_heading_buf[64];
  // Determine status of the compass
  switch (data.compass_status) {
    // Compass data is not yet valid
    case CompassStatusDataInvalid:
      text_layer_set_text(s_output_layer, "Compass data invalid");
      break;

    // Compass is currently calibrating, but a heading is available
    case CompassStatusCalibrated:
    case CompassStatusCalibrating:
     
      snprintf(s_heading_buf, sizeof(s_heading_buf),
        " %ld°\n%ld.%02ldpi %d",    TRIGANGLE_TO_DEG(data.magnetic_heading),
    // display radians, units digit
    (TRIGANGLE_TO_DEG(data.magnetic_heading) * 2) / 360,
    // radians, digits after decimal
    ((TRIGANGLE_TO_DEG(data.magnetic_heading) * 200) / 360) % 100, bearing
    );
      text_layer_set_text(s_output_layer, s_heading_buf);
      
      if (((TRIGANGLE_TO_DEG(data.magnetic_heading) + threshold)%360 > bearing) &&((TRIGANGLE_TO_DEG(data.magnetic_heading) - threshold)%360 < bearing)){
           vibes_short_pulse();
      }
              
      break;
    // Compass data is ready for use, write the heading in to the buffer
    
    // CompassStatus is unknown
    default:
      snprintf(s_buffer, sizeof(s_buffer), "Unknown CompassStatus: %d", data.compass_status);
      text_layer_set_text(s_output_layer, s_buffer);
      break;
  }
}
static void received_handler(DictionaryIterator *iter, void *context) {
  Tuple *result_tuple = dict_find(iter, 1);
    static char s_buffer[32];
    
    if(result_tuple) {
        
         bearing = result_tuple->value->uint16;
        snprintf(s_buffer, sizeof(s_buffer)," %d°",  bearing);
        
    }
    text_layer_set_text(s_price_layer,s_buffer);
    
}
static void in_dropped_handler(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Dropped!");
     text_layer_set_text(s_output_layer,"dropped");
}



static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);

  // Create output TextLayer
  s_output_layer = text_layer_create(GRect(0, 20, window_bounds.size.w, 60));
  text_layer_set_text(s_output_layer, "Calibrating...");
  text_layer_set_text_alignment(s_output_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_output_layer));
    
  s_price_layer = text_layer_create(GRect(0, 75, window_bounds.size.w, 50));
  text_layer_set_text(s_price_layer, "0.00");
  text_layer_set_text_alignment(s_price_layer, GTextAlignmentCenter);
  text_layer_set_font(s_price_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));
  layer_add_child(window_layer, text_layer_get_layer(s_price_layer));
}

static void main_window_unload(Window *window) {
  // Destroy output TextLayer
  text_layer_destroy(s_output_layer);
}

static void init(void) {
  // Create main Window
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);

  // Subscribe to compass updates when angle changes by 10 degrees
  compass_service_subscribe(compass_handler);
  compass_service_set_heading_filter(TRIG_MAX_ANGLE / 36);
  app_message_register_inbox_received(received_handler);
  app_message_register_inbox_dropped(in_dropped_handler);
;
  app_message_open(64, 64);
}

static void deinit(void) {
  // Destroy main Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
