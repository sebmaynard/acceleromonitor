#include <pebble.h>

#include "acceleromonitor.h"

#define HISTORY_MAX 144
#define MAX_ACCEL 4000

static Window *window;
static TextLayer *text_layer;
static TextLayer *speed_up_layer;
static TextLayer *speed_down_layer;

static GRect window_frame;
static Layer *graph_layer;
static AppTimer *timer;

int timer_frequency = 100;
bool running = false;

static int last_x = 0;
static AccelData history[HISTORY_MAX];

static void set_timer() {
  if (running) timer = app_timer_register(timer_frequency, timer_callback, NULL);
}

static void draw_pts(GContext *ctx, int x, int y_start, int y_height, int16_t accel0, int16_t accel1) {
  int magnitude = y_height / 2;
  int midpoint = y_start + magnitude;
  int y0 = (accel0 * magnitude / MAX_ACCEL) + midpoint;
  int y1 = (accel1 * magnitude / MAX_ACCEL) + midpoint;

  GPoint p0 = { x-1, y0 };
  GPoint p1 = { x, y1 };
  graphics_draw_line(ctx, p0, p1);
}

static void graph_layer_update_callback(Layer *me, GContext *ctx) {
  const GRect frame = window_frame;

  graphics_context_set_fill_color(ctx, GColorBlack);
  
  int segment = frame.size.h / 3;
  for (int i=1; i<HISTORY_MAX; ++i) {
    draw_pts(ctx, i, 0, segment, history[i-1].x, history[i].x);
    draw_pts(ctx, i, segment, segment, history[i-1].y, history[i].y);
    draw_pts(ctx, i, segment*2, segment, history[i-1].z, history[i].z);
  }
  GPoint status_line1 = {last_x, 0};
  GPoint status_line2 = {last_x, frame.size.h};
  graphics_draw_line(ctx, status_line1, status_line2);

}

static void timer_callback() {
  if (!running) return;

  AccelData accel = {false, 0, 0, 0, 0};

  accel_service_peek(&accel);
  
  history[last_x].x = accel.x;
  history[last_x].y = accel.y;
  history[last_x].z = accel.z;
  last_x++;
  if (last_x >= HISTORY_MAX) last_x = 0;

  layer_mark_dirty(graph_layer);

  set_timer();
}

static void click_handler_select(ClickRecognizerRef recognizer, void *context) {
  running = !running;
  if (running) {
  // should process events - 5 times a second
    accel_service_set_sampling_rate(ACCEL_SAMPLING_10HZ);
    accel_data_service_subscribe(0, handle_accel);
    layer_set_hidden((Layer *)text_layer, true);
    layer_set_hidden((Layer *)speed_up_layer, true);
    layer_set_hidden((Layer *)speed_down_layer, true);
  }
  else {
    layer_set_hidden((Layer *)text_layer, false);
    layer_set_hidden((Layer *)speed_up_layer, false);
    layer_set_hidden((Layer *)speed_down_layer, false);
    accel_data_service_unsubscribe();
  }

  set_timer();

}

static void click_handler_up(ClickRecognizerRef recognizer, void *context) {
  timer_frequency -= 10;
  if (timer_frequency < 20) timer_frequency = 20;
}

static void click_handler_down(ClickRecognizerRef recognizer, void *context) {
  timer_frequency += 10;
  if (timer_frequency > 1000) timer_frequency = 1000;
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, click_handler_up);
  window_single_click_subscribe(BUTTON_ID_SELECT, click_handler_select);
  window_single_click_subscribe(BUTTON_ID_DOWN, click_handler_down);
}

static void handle_accel(AccelData *accel_data, uint32_t num_samples) {
  // nothing
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);

  GRect frame = window_frame = layer_get_frame(window_layer);
  graph_layer = layer_create(frame);
  layer_set_update_proc(graph_layer, graph_layer_update_callback);
  layer_add_child(window_layer, graph_layer);

  GRect bounds = layer_get_bounds(window_layer);
  text_layer = text_layer_create((GRect) { .origin = { 0, bounds.size.h / 2 - 10 }, .size = { bounds.size.w, 20 } });
  text_layer_set_text(text_layer, "press to toggle --->");
  text_layer_set_text_alignment(text_layer, GTextAlignmentRight);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));

  speed_up_layer = text_layer_create((GRect) { .origin = { 0, 0 }, .size = { bounds.size.w, 20 } });
  text_layer_set_text(speed_up_layer, "speed up --->");
  text_layer_set_text_alignment(speed_up_layer, GTextAlignmentRight);
  layer_add_child(window_layer, text_layer_get_layer(speed_up_layer));

  speed_down_layer = text_layer_create((GRect) { .origin = { 0, bounds.size.h - 20 }, .size = { bounds.size.w, 20 } });
  text_layer_set_text(speed_down_layer, "speed down --->");
  text_layer_set_text_alignment(speed_down_layer, GTextAlignmentRight);
  layer_add_child(window_layer, text_layer_get_layer(speed_down_layer));

}

static void window_unload(Window *window) {
  text_layer_destroy(speed_down_layer);
  text_layer_destroy(speed_up_layer);
  text_layer_destroy(text_layer);
  layer_destroy(graph_layer);
}

static void init(void) {
  window = window_create();
  window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);

  for (int i=0; i<HISTORY_MAX; ++i) {
    AccelData pt = {false, 0, 0, 0, 0};
    history[i] = pt;
  }

  set_timer();
}

static void deinit(void) {
  accel_data_service_unsubscribe();
  window_destroy(window);
}

int main(void) {
  init();

  /* APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window); */
  app_event_loop();

  deinit();
}
