#include <pebble.h>
#include "settings.h"

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
static char *croMonths[12];
static char *croDays[7];

static void init();
//static void deinit();

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  Tuple *colorTheme = dict_find(iter, AppKeyColorTheme);
  Tuple *timeSize = dict_find(iter, AppKeyTimeSize);
  Tuple *dateSize = dict_find(iter, AppKeyDateSize);
  Tuple *bluetoothAlarm = dict_find(iter, AppKeyBluetoothAlarm);
  Tuple *batteryIcon = dict_find(iter, AppKeyBatteryIcon);
  Tuple *dateFormat = dict_find(iter, AppKeyDateFormat);
  Tuple *croatianDate = dict_find(iter, AppKeyCroatianDate);

  if (colorTheme) {
    persist_write_int(AppKeyColorTheme, colorTheme->value->int32);
  }
  if (timeSize) {
    persist_write_int(AppKeyTimeSize, timeSize->value->int32);
  }
  if (dateSize) {
    persist_write_int(AppKeyDateSize, dateSize->value->int32);
  }
  if (bluetoothAlarm) {
    persist_write_int(AppKeyBluetoothAlarm, bluetoothAlarm->value->int32);
  }
  if (batteryIcon) {
    persist_write_int(AppKeyBatteryIcon, batteryIcon->value->int32);
  }
  if (dateFormat) {
    persist_write_int(AppKeyDateFormat, dateFormat->value->int32);
  }
  if (croatianDate) {
    persist_write_int(AppKeyCroatianDate, croatianDate->value->int32);
  }
  //deinit();
  init();
}

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_buffer+(('0' == s_buffer[0])?1:0));
  
 if (settings[AppKeyDateSize]>0) {
  // Copy date into buffer from tm structure
  static char date_buffer[16];
  if (settings[AppKeyCroatianDate] == 0) {
    switch (settings[AppKeyDateFormat]) { //1 - 27 Apr, Wed, 2 - Apr 27, Wed, 3 - Wed, 27 Apr, 4 - Wed, Apr 27
      case 2 : strftime(date_buffer, sizeof(date_buffer), "%b %d, %a", tick_time);break;
      case 3 : strftime(date_buffer, sizeof(date_buffer), "%a, %d %b", tick_time);break;
      case 4 : strftime(date_buffer, sizeof(date_buffer), "%a, %b %d", tick_time);break;
      default : strftime(date_buffer, sizeof(date_buffer), "%d %b, %a", tick_time);
    }
  } else {
    
    switch (settings[AppKeyDateFormat]) { //1 - 27 Apr, Wed, 2 - Apr 27, Wed, 3 - Wed, 27 Apr, 4 - Wed, Apr 27
      case 2 : snprintf(date_buffer, sizeof(date_buffer), "%s %d, %s", croMonths[tick_time->tm_mon], tick_time->tm_mday, croDays[tick_time->tm_wday]);break;
      case 3 : snprintf(date_buffer, sizeof(date_buffer), "%s, %d %s", croDays[tick_time->tm_wday], tick_time->tm_mday, croMonths[tick_time->tm_mon]);break;
      case 4 : snprintf(date_buffer, sizeof(date_buffer), "%s, %s %d", croDays[tick_time->tm_wday], croMonths[tick_time->tm_mon], tick_time->tm_mday);break;
      default : snprintf(date_buffer, sizeof(date_buffer), "%d %s, %s", tick_time->tm_mday, croMonths[tick_time->tm_mon], croDays[tick_time->tm_wday]);
    }
  }
  // Show the date
  text_layer_set_text(s_date_layer, date_buffer);
 }
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

//  if (settings[AppKeyBluetoothAlarm]>0) {
  static void bluetooth_callback(bool connected) {
    // Show icon if disconnected
    layer_set_hidden(bitmap_layer_get_layer(s_bt_icon_layer), connected);
    if (settings[AppKeyBluetoothAlarm]==2) {
      if(connected) {
        // Issue a vibrating alert
        vibes_double_pulse();
      } else {
        // Issue a vibrating alert
        vibes_double_pulse();
      }
    }
  }
//}

static void battery_callback(BatteryChargeState state) {
  if (state.is_charging) {
    bitmap_layer_set_bitmap(s_battery_layer, s_battery_charging_bitmap);
    APP_LOG(APP_LOG_LEVEL_INFO, "Battery charging");
    }
  else if (state.charge_percent <= 10 && settings[AppKeyBatteryIcon] > 0) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Battery 10");
    bitmap_layer_set_bitmap(s_battery_layer, s_battery_empty_bitmap);     
    }
  else if (state.charge_percent <= 20 && settings[AppKeyBatteryIcon] == 2) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Battery 20");
    bitmap_layer_set_bitmap(s_battery_layer, s_battery_halfempty_bitmap);            
    }
  else {
    layer_set_hidden(bitmap_layer_get_layer(s_battery_layer), true);
    return;
  }
  layer_set_hidden(bitmap_layer_get_layer(s_battery_layer), false);
}

static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  if (settings[AppKeyColorTheme]==3) {
    // Create GBitmap
    s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND_CIRCLE);
    //create bitmap layer
    s_background_layer = bitmap_layer_create(bounds);
    // Set the bitmap onto the layer and add to the window
    bitmap_layer_set_alignment(s_background_layer, background_bitmap_alignment);
    bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
    layer_add_child(window_layer, bitmap_layer_get_layer(s_background_layer));
  }
  
  // Create time TextLayer with specific bounds
  if (settings[AppKeyTimeSize]==1) {
    s_time_layer = text_layer_create(GRect(0, 52-time_position_offset_withdate, bounds.size.w, 50));
    s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_TALLPIX_48));
  } else if (settings[AppKeyTimeSize]==2) {
    s_time_layer = text_layer_create(GRect(0, 42-time_position_offset_withdate, bounds.size.w, 66));
    s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_TALLPIX_64));
  } else if (settings[AppKeyTimeSize]==3) {
    s_time_layer = text_layer_create(GRect(0, 36-time_position_offset_withdate, bounds.size.w, 76));
    s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_TALLPIX_74));
  } else if (settings[AppKeyTimeSize]==4) {
    s_time_layer = text_layer_create(GRect(0, 28-time_position_offset_withdate, bounds.size.w, 92));
    s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_TALLPIX_90));
  } else { //timesize == 5
    s_time_layer = text_layer_create(GRect(0, 10-time_position_offset_withdate, bounds.size.w, 118));
    s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_TALLPIX_116));
  }
  
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_background_color(s_time_layer, color_time_textlayer);
  text_layer_set_text_color(s_time_layer, color_time_text);
  text_layer_set_text(s_time_layer, "0:00");
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  
  if (settings[AppKeyDateSize]>0) {
  // Create date TextLayer
    if (settings[AppKeyDateSize]==1) {
      s_date_layer = text_layer_create(GRect(0, bounds.size.h-24+2, 144, 16));
      text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
    } else if (settings[AppKeyDateSize]==2) {
      s_date_layer = text_layer_create(GRect(0, bounds.size.h-24, 144, 20));
      text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
    } else { //datesize==3
      s_date_layer = text_layer_create(GRect(0, bounds.size.h-24-6, 144, 26));
      text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
    }
      text_layer_set_text_color(s_date_layer, color_date_text);
      text_layer_set_background_color(s_date_layer, color_date_textlayer);
      text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
      layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));
  }
  
  if (settings[AppKeyBluetoothAlarm]>0) {
    // Create the Bluetooth icon GBitmap
    s_bt_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BLUETOOTH);
    // Create the BitmapLayer to display the GBitmap
    s_bt_icon_layer = bitmap_layer_create(GRect(3, 3, 8, 13));
     //bitmap_layer_set_compositing_mode(s_bt_icon_layer, GCompOpAssignInverted);
    bitmap_layer_set_bitmap(s_bt_icon_layer, s_bt_icon_bitmap);
    layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_bt_icon_layer));
    // Show the correct state of the BT connection from the start
    layer_set_hidden(bitmap_layer_get_layer(s_bt_icon_layer), connection_service_peek_pebble_app_connection() ? true : false);
  }
  
  if (settings[AppKeyBatteryIcon] > 0) {
    // Create the Battery icon GBitmap
    s_battery_charging_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY_CHARGING);
    s_battery_empty_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY_EMPTY);
    s_battery_halfempty_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY_HALFEMPTY);
    // Create the BitmapLayer to display the GBitmap
    s_battery_layer = bitmap_layer_create(GRect(144-3-15, 6, 15, 8));
    //bitmap_layer_set_bitmap(s_battery_layer, s_battery_empty_bitmap);
    layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_battery_layer));
    layer_set_hidden(bitmap_layer_get_layer(s_battery_layer), true);
  }
  
  if (settings[AppKeyCroatianDate] == 1) {
    //static char *croMonths[12];
    croMonths[0] = "Sij"; croMonths[1] = "Vlj"; croMonths[2] = "Ožu"; croMonths[3] = "Tra"; croMonths[4] = "Svi"; croMonths[5] = "Lip";
    croMonths[6] = "Srp"; croMonths[7] = "Kol"; croMonths[8] = "Ruj"; croMonths[9] = "Lis"; croMonths[10] = "Stu"; croMonths[11] = "Pro";
    //static char *croDays[7];
    croDays[1] = "Pon"; croDays[2] = "Uto"; croDays[3] = "Sri"; croDays[4] = "Čet"; croDays[5] = "Pet"; croDays[6] = "Sub"; croDays[0] = "Ned";
  
  }
   
}

static void main_window_unload(Window *window) {
  APP_LOG(APP_LOG_LEVEL_INFO, "entered main_window_unload");
  // Destroy TextLayers
  text_layer_destroy(s_time_layer);
  
  if (settings[AppKeyDateSize]>0) {
    text_layer_destroy(s_date_layer);
  }
  
  // Unload GFont
  fonts_unload_custom_font(s_time_font);
  if (settings[AppKeyColorTheme]==3) {
    // Destroy GBitmap
    gbitmap_destroy(s_background_bitmap);
    //Destroy BitmapLayer
    bitmap_layer_destroy(s_background_layer);
  }

  if (settings[AppKeyBluetoothAlarm]>0) {
    gbitmap_destroy(s_bt_icon_bitmap);
    bitmap_layer_destroy(s_bt_icon_layer);
  }
  
  if (settings[AppKeyBatteryIcon] > 0) {
    gbitmap_destroy(s_battery_halfempty_bitmap);
    gbitmap_destroy(s_battery_empty_bitmap);
    bitmap_layer_destroy(s_battery_layer);
  }

}

//---------------INIT-----------------------------
static void init() {
  
  APP_LOG(APP_LOG_LEVEL_INFO, "entered init");
  settings_read();
  // Create main Window element and assign to pointer
  s_main_window = window_create();

//0 debug mode text layer visible, 1 black with white text, 2 white with black text, 3 backgroung image
  if (settings[AppKeyColorTheme]==1) {
    color_background=GColorBlack;
    color_time_text=GColorWhite;
    color_time_textlayer=GColorClear;
    color_date_text=GColorWhite;
    color_date_textlayer=GColorClear;
  } else if (settings[AppKeyColorTheme]==2) {
    color_background=GColorWhite;
    color_time_text=GColorBlack;
    color_time_textlayer=GColorClear;
    color_date_text=GColorBlack;
    color_date_textlayer=GColorClear;
  } else if (settings[AppKeyColorTheme]==3) {
    color_background=GColorBlack;
    color_time_text=GColorBlack;
    color_time_textlayer=GColorClear;
    color_date_text=GColorWhite;
    color_date_textlayer=GColorClear;
  } else { //colortheme==0
  color_background=GColorBlack;
  color_time_text=GColorBlack;
  color_time_textlayer=GColorWhite;
  color_date_text=GColorBlack;
  color_date_textlayer=GColorWhite;
  }
  
  if (settings[AppKeyDateSize]>0) {
    time_position_offset_withdate=9;
    background_bitmap_alignment=GAlignTop;
  } else {
    time_position_offset_withdate=0;
    background_bitmap_alignment=GAlignCenter;
  }
  
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
  
  if (settings[AppKeyBluetoothAlarm]>0) {
    // Register for Bluetooth connection updates
    bluetooth_connection_service_subscribe(bluetooth_callback);
    //connection_service_subscribe( (ConnectionHandlers) {.pebble_app_connection_handler = bluetooth_callback} );
  }
  
  if (settings[AppKeyBatteryIcon] > 0) {
    // Register for battery level updates
    battery_state_service_subscribe(battery_callback);
  }
  
  app_message_register_inbox_received(inbox_received_handler);
  //app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  app_message_open(256, 256);
}

static void deinit() {
  APP_LOG(APP_LOG_LEVEL_INFO, "entered deinit");
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}