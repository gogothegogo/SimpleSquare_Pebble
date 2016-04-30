Pebble.addEventListener('ready', function() {
  console.log('PebbleKit JS ready!');
});

Pebble.addEventListener('showConfiguration', function() {
  var url = 'http://gogothegogo.github.io/SimpleSquare_Pebble/';

  console.log('Showing configuration page: ' + url);

  Pebble.openURL(url);
});

Pebble.addEventListener('webviewclosed', function(e) {
  var configData = JSON.parse(decodeURIComponent(e.response));

  console.log('Configuration page returned: ' + JSON.stringify(configData));

  if (configData.colorTheme) {
    Pebble.sendAppMessage({
      //colorTheme: parseInt(configData.colorTheme),
      timeSize: parseInt(configData.timeSize),
      dateSize: parseInt(configData.dateSize),
      bluetoothAlarm: parseInt(configData.bluetoothAlarm),
      batteryIcon: parseInt(configData.batteryIcon),
      dateFormat: parseInt(configData.dateFormat),
      croatianDate: parseInt(configData.croatianDate)
    }, function() {
      console.log('Send successful!');
    }, function() {
      console.log('Send failed!');
    });
  }
});