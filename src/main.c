#include <pebble.h>

#define COLOR_THEME 1 //0 debug mode text layer visible, 1 black with white text, 2 white with black text, 3 backgroung image

#define SHOW_DATE 1

static Window *s_main_window;
static TextLayer *s_time_layer, *s_date_layer;
static GFont s_time_font;
static GColor color_background;
static GColor color_time_text;
static GColor color_time_textlayer;
static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;
static int time_text_layer_height = 72;
static int time_position_offset_withdate = 0;

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_buffer+(('0' == s_buffer[0])?1:0));
  
#if SHOW_DATE == 1
  // Copy date into buffer from tm structure
  static char date_buffer[16];
  strftime(date_buffer, sizeof(date_buffer), "%d %b, %a", tick_time);
  // Show the date
  text_layer_set_text(s_date_layer, date_buffer);
#endif
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

#if COLOR_THEME == 3
  // Create GBitmap
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND_PILLOW3);
  //create bitmap layer
  s_background_layer = bitmap_layer_create(bounds);
  // Set the bitmap onto the layer and add to the window
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_background_layer));
#endif

  
  // Create time TextLayer with specific bounds
  s_time_layer = text_layer_create(
      GRect(0, (bounds.size.h-time_text_layer_height)/2-time_position_offset_withdate, bounds.size.w, time_text_layer_height));
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_TALLPIX_56));
  //s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_NEWSFLASH_56));
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_background_color(s_time_layer, color_time_textlayer);
  text_layer_set_text_color(s_time_layer, color_time_text);
  text_layer_set_text(s_time_layer, "0:00");
  //text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  
#if SHOW_DATE == 1
  // Create date TextLayer
  s_date_layer = text_layer_create(GRect(0, bounds.size.h-24+12-8, 144, 16));
  text_layer_set_text_color(s_date_layer, GColorWhite);
  text_layer_set_background_color(s_date_layer, GColorBlack);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));
  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
#endif
}

static void main_window_unload(Window *window) {
  // Destroy TextLayers
  text_layer_destroy(s_time_layer);
  
#if SHOW_DATE == 1
  text_layer_destroy(s_date_layer);
#endif
  
  // Unload GFont
  fonts_unload_custom_font(s_time_font);
#if COLOR_THEME == 3
  // Destroy GBitmap
  gbitmap_destroy(s_background_bitmap);
  //Destroy BitmapLayer
  bitmap_layer_destroy(s_background_layer);
#endif

}

//---------------INIT-----------------------------
static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();
  
#if COLOR_THEME == 1
  color_background=GColorBlack;
  color_time_text=GColorWhite;
  color_time_textlayer=GColorClear;
#elif COLOR_THEME == 2
  color_background=GColorWhite;
  color_time_text=GColorBlack;
  color_time_textlayer=GColorClear;
#elif COLOR_THEME == 3
  color_background=GColorBlack;
  color_time_text=GColorBlack;
  color_time_textlayer=GColorClear;  
#elif COLOR_THEME == 0
  color_background=GColorBlack;
  color_time_text=GColorBlack;
  color_time_textlayer=GColorWhite;  
#endif
  
#if SHOW_DATE == 1
  time_position_offset_withdate=9;
#else
  time_position_offset_withdate=0;
#endif
  
  // Set window background color
  window_set_background_color(s_main_window, color_background);


  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);

  // Make sure the time is displayed from the start
  update_time();

  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}