(function() {
  loadOptions();
  submitHandler();
})();

function submitHandler() {
  var $submitButton = $('#submitButton');

  $submitButton.on('click', function() {
    console.log('Submit');

    var return_to = getQueryParam('return_to', 'pebblejs://close#');
    document.location = return_to + encodeURIComponent(JSON.stringify(getAndStoreConfigData()));
  });
}

function loadOptions() {
  var $colorTheme = $('#colorTheme');
  var $timeSize = $('#timeSize');
  var $dateSize = $('#dateSize');
  var $bluetoothAlarm = $('#bluetoothAlarm');
  var $batteryIcon = $('#batteryIcon');
  var $dateFormat = $('#dateFormat');
  var $croatianDate = $('#croatianDate');

  if (localStorage.colorTheme) {
    $colorTheme[0].value = localStorage.colorTheme;
    $timeSize[0].value = localStorage.timeSize;
    $dateSize[0].value = localStorage.dateSize;
    $bluetoothAlarm[0].value = localStorage.bluetoothAlarm;
    $batteryIcon[0].value = localStorage.batteryIcon;
    $dateFormat[0].value = localStorage.dateFormat;
    $croatianDate[0].value = localStorage.croatianDate;
  }
}

function getAndStoreConfigData() {
  var $colorTheme = $('#colorTheme');
  var $timeSize = $('#timeSize');
  var $dateSize = $('#dateSize');
  var $bluetoothAlarm = $('#bluetoothAlarm');
  var $batteryIcon = $('#batteryIcon');
  var $dateFormat = $('#dateFormat');
  var $croatianDate = $('#croatianDate');
  
  var options = {
    colorTheme: $colorTheme.val(),
    timeSize: $timeSize.val(),
    dateSize: $dateSize.val(),
    bluetoothAlarm: $bluetoothAlarm.val(),
    batteryIcon: $batteryIcon.val(),
    dateFormat: $dateFormat.val(),
    croatianDate: $croatianDate[0].checked ? 1:0
  };

  localStorage.colorTheme = options.colorTheme;
  localStorage.timeSize = options.timeSize;
  localStorage.dateSize = options.dateSize;
  localStorage.bluetoothAlarm = options.bluetoothAlarm;
  localStorage.batteryIcon = options.batteryIcon;
  localStorage.dateFormat = options.dateFormat;
  localStorage.croatianDate = options.croatianDate;

  console.log('Got options: ' + JSON.stringify(options));
  return options;
}

function getQueryParam(variable, defaultValue) {
  var query = location.search.substring(1);
  var vars = query.split('&');
  for (var i = 0; i < vars.length; i++) {
    var pair = vars[i].split('=');
    if (pair[0] === variable) {
      return decodeURIComponent(pair[1]);
    }
  }
  return defaultValue || false;
}
