#include <pebble.h>

#define COLOR_THEME 1 //0 debug mode text layer visible, 1 black with white text, 2 white with black text, 3 backgroung image
#define TIME_SIZE 3 //1 small, 2 medium, 3 large, 4 huge
#define DATE_SIZE 2 //0 no date, 1 small, 2 medium, 3 large
#define BLUETOOTH_ALARM 2 //0 off, 1 icon, 2 icon+alarm
#define BATTERY_ICON 1 //0 off, 1 10%, 2 20% & 10%

//settings match
typedef enum {
  AppKeyColorTheme = 0,
  AppKeyTimeSize = 1,
  AppKeyDateSize = 2,
  AppKeyBluetoothAlarm = 3,
  AppKeyBatteryIcon = 4
} AppKey;

static int settings[5]={COLOR_THEME, TIME_SIZE, DATE_SIZE, BLUETOOTH_ALARM, BATTERY_ICON};
/*
static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  Tuple *colorTheme = dict_find(iter, AppKeyColorTheme);
  if(colorTheme) {
    int *colorTheme = colorTheme->value->int8;
    COLOR_THEME=colorTheme;
  }
}*/

static int settings_get_default(AppKey key) {
    switch (key) {
    case AppKeyColorTheme:
        return 1;
    case AppKeyTimeSize:
        return 3;
    case AppKeyDateSize:
        return 2;
    case AppKeyBluetoothAlarm:
        return 2;
    case AppKeyBatteryIcon:
        return 2;
    default:
        return 0;
    }
}

static int settings_get(AppKey key) {
        return settings_get_default(key);
}