#include <pebble.h>

#define COLOR_THEME 1 //0 debug mode text layer visible, 1 black with white text, 2 white with black text, 3 backgroung image
#define TIME_SIZE 3 //1 small, 2 medium, 3 large, 4 huge
#define DATE_SIZE 2 //0 no date, 1 small, 2 medium, 3 large
#define BLUETOOTH_ALARM 2 //0 off, 1 icon, 2 icon+alarm
#define BATTERY_ICON 1 //0 off, 1 on

static Window *s_main_window;
static TextLayer *s_time_layer, *s_date_layer;
static GFont s_time_font;
static GColor color_background;
static GColor color_time_text;
static GColor color_time_textlayer;
static GColor color_date_text;
static GColor color_date_textlayer;
static BitmapLayer *s_background_layer, *s_bt_icon_layer, *s_battery_layer;
static GBitmap *s_background_bitmap, *s_bt_icon_bitmap, *s_battery_empty_bitmap, *s_battery_halfempty_bitmap, *s_battery_charging_bitmap;
static int time_position_offset_withdate = 0;
static GAlign background_bitmap_alignment;

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_buffer+(('0' == s_buffer[0])?1:0));
  
#if DATE_SIZE > 0
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

#if BLUETOOTH_ALARM > 0
static void bluetooth_callback(bool connected) {
  // Show icon if disconnected
  layer_set_hidden(bitmap_layer_get_layer(s_bt_icon_layer), connected);
#if BLUETOOTH_ALARM == 2
  if(connected) {
    // Issue a vibrating alert
    vibes_double_pulse();
  } else {
    // Issue a vibrating alert
    vibes_double_pulse();
  }
#endif
}
#endif

#if BATTERY_ICON == 1
static void battery_callback(BatteryChargeState state) {
  if (state.is_charging) {
    bitmap_layer_set_bitmap(s_battery_layer, s_battery_charging_bitmap);
    } else if (state.charge_percent <= 10) {
    bitmap_layer_set_bitmap(s_battery_layer, s_battery_empty_bitmap);     
    } else if (state.charge_percent <= 20) {
    bitmap_layer_set_bitmap(s_battery_layer, s_battery_halfempty_bitmap);            
    } else {
    layer_set_hidden(bitmap_layer_get_layer(s_battery_layer), true);
    return;
  }
  layer_set_hidden(bitmap_layer_get_layer(s_battery_layer), false);
}
#endif



static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

#if COLOR_THEME == 3
  // Create GBitmap
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND_CIRCLE);
  //create bitmap layer
  s_background_layer = bitmap_layer_create(bounds);
  // Set the bitmap onto the layer and add to the window
  bitmap_layer_set_alignment(s_background_layer, background_bitmap_alignment);
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_background_layer));
#endif

  
  // Create time TextLayer with specific bounds
#if TIME_SIZE == 1
  s_time_layer = text_layer_create(GRect(0, 52-time_position_offset_withdate, bounds.size.w, 50));
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_TALLPIX_48));
#elif TIME_SIZE == 2
  s_time_layer = text_layer_create(GRect(0, 42-time_position_offset_withdate, bounds.size.w, 66));
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_TALLPIX_64));
#elif TIME_SIZE == 3
  s_time_layer = text_layer_create(GRect(0, 36-time_position_offset_withdate, bounds.size.w, 76));
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_TALLPIX_74));
#elif TIME_SIZE == 4
  s_time_layer = text_layer_create(GRect(0, 28-time_position_offset_withdate, bounds.size.w, 92));
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_TALLPIX_90));
#endif
  
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_background_color(s_time_layer, color_time_textlayer);
  text_layer_set_text_color(s_time_layer, color_time_text);
  text_layer_set_text(s_time_layer, "0:00");
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  
#if DATE_SIZE > 0
  // Create date TextLayer
#if DATE_SIZE == 1
  s_date_layer = text_layer_create(GRect(0, bounds.size.h-24+2, 144, 16));
  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
#elif DATE_SIZE == 2
  s_date_layer = text_layer_create(GRect(0, bounds.size.h-24, 144, 20));
  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
#elif DATE_SIZE == 3
  s_date_layer = text_layer_create(GRect(0, bounds.size.h-24-6, 144, 26));
  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
#endif
  text_layer_set_text_color(s_date_layer, color_date_text);
  text_layer_set_background_color(s_date_layer, color_date_textlayer);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));
#endif
  
#if BLUETOOTH_ALARM > 0
  // Create the Bluetooth icon GBitmap
  s_bt_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BLUETOOTH);
  // Create the BitmapLayer to display the GBitmap
  s_bt_icon_layer = bitmap_layer_create(GRect(3, 3, 8, 13));
   //bitmap_layer_set_compositing_mode(s_bt_icon_layer, GCompOpAssignInverted);
  bitmap_layer_set_bitmap(s_bt_icon_layer, s_bt_icon_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_bt_icon_layer));
  // Show the correct state of the BT connection from the start
  layer_set_hidden(bitmap_layer_get_layer(s_bt_icon_layer), connection_service_peek_pebble_app_connection() ? true : false);
#endif
  
#if BATTERY_ICON == 1
  // Create the Battery icon GBitmap
  s_battery_charging_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY_CHARGING);
  s_battery_empty_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY_EMPTY);
  s_battery_halfempty_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY_HALFEMPTY);
  // Create the BitmapLayer to display the GBitmap
  s_battery_layer = bitmap_layer_create(GRect(144-3-15, 6, 15, 8));
  //bitmap_layer_set_bitmap(s_battery_layer, s_battery_empty_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_battery_layer));
  layer_set_hidden(bitmap_layer_get_layer(s_battery_layer), true);
#endif
}

static void main_window_unload(Window *window) {
  // Destroy TextLayers
  text_layer_destroy(s_time_layer);
  
#if DATE_SIZE > 0
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

#if BLUETOOTH_ALARM > 0
  gbitmap_destroy(s_bt_icon_bitmap);
  bitmap_layer_destroy(s_bt_icon_layer);
#endif
  
#if BATTERY_ICON == 1
  gbitmap_destroy(s_battery_halfempty_bitmap);
  gbitmap_destroy(s_battery_empty_bitmap);
  bitmap_layer_destroy(s_battery_layer);
#endif

}

//---------------INIT-----------------------------
static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

//0 debug mode text layer visible, 1 black with white text, 2 white with black text, 3 backgroung image
#if COLOR_THEME == 1
  color_background=GColorBlack;
  color_time_text=GColorWhite;
  color_time_textlayer=GColorClear;
  color_date_text=GColorWhite;
  color_date_textlayer=GColorClear;
#elif COLOR_THEME == 2
  color_background=GColorWhite;
  color_time_text=GColorBlack;
  color_time_textlayer=GColorClear;
  color_date_text=GColorBlack;
  color_date_textlayer=GColorClear;
#elif COLOR_THEME == 3
  color_background=GColorBlack;
  color_time_text=GColorBlack;
  color_time_textlayer=GColorClear;
  color_date_text=GColorWhite;
  color_date_textlayer=GColorClear;
#elif COLOR_THEME == 0
  color_background=GColorBlack;
  color_time_text=GColorBlack;
  color_time_textlayer=GColorWhite;
  color_date_text=GColorBlack;
  color_date_textlayer=GColorWhite;
#endif
  
#if DATE_SIZE > 0
  time_position_offset_withdate=9;
  background_bitmap_alignment=GAlignTop;
#else
  time_position_offset_withdate=0;
  background_bitmap_alignment=GAlignCenter;
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
  
#if BLUETOOTH_ALARM > 0
  // Register for Bluetooth connection updates
  bluetooth_connection_service_subscribe(bluetooth_callback);
  //connection_service_subscribe( (ConnectionHandlers) {.pebble_app_connection_handler = bluetooth_callback} );
#endif
  
#if BATTERY_ICON == 1
  // Register for battery level updates
  battery_state_service_subscribe(battery_callback);
#endif
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