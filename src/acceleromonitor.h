#pragma once

static void timer_callback();
static void draw_pts(GContext *ctx, int x, int y_start, int y_height, int16_t accel0, int16_t accel1);
static void graph_layer_update_callback(Layer *me, GContext *ctx);
static void timer_callback();

static void click_handler_select(ClickRecognizerRef recognizer, void *context);
static void click_handler_up(ClickRecognizerRef recognizer, void *context);
static void click_handler_down(ClickRecognizerRef recognizer, void *context);
static void click_config_provider(void *context);
static void handle_accel(AccelData *accel_data, uint32_t num_samples);
static void window_load(Window *window);
static void window_unload(Window *window);
static void init(void);
static void deinit(void);

int main(void);
