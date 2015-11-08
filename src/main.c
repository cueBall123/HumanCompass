/*
 * main.c
 * Constructs a Window and TextLayer, then subscribes to the CompassService to
 * output the data. Also shows state of calibration.
 */

#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_output_layer;

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
        " %ld°\n%ld.%02ldpi",    TRIGANGLE_TO_DEG(data.magnetic_heading),
    // display radians, units digit
    (TRIGANGLE_TO_DEG(data.magnetic_heading) * 2) / 360,
    // radians, digits after decimal
    ((TRIGANGLE_TO_DEG(data.magnetic_heading) * 200) / 360) % 100
    );
      text_layer_set_text(s_output_layer, s_heading_buf);
      break;
    // Compass data is ready for use, write the heading in to the buffer
    
    // CompassStatus is unknown
    default:
      snprintf(s_buffer, sizeof(s_buffer), "Unknown CompassStatus: %d", data.compass_status);
      text_layer_set_text(s_output_layer, s_buffer);
      break;
  }
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);

  // Create output TextLayer
  s_output_layer = text_layer_create(GRect(0, 0, window_bounds.size.w, window_bounds.size.h));
  text_layer_set_text(s_output_layer, "Calibrating...");
  text_layer_set_text_alignment(s_output_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_output_layer));
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
