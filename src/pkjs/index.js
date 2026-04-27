var CONFIG_URL = 'https://cache4gold.github.io/PebbleTextWatchWithDate/config/config.html';

Pebble.addEventListener('ready', function() {});

Pebble.addEventListener('showConfiguration', function() {
  var currentConfig = {};
  var keys = [
    'bg_color', 'hour_color', 'min_color', 'date_color', 'day_color',
    'time_align', 'date_align', 'prefix', 'date_format', 'day_format',
    'font_choice', 'time_case', 'date_case', 'day_case'
  ];
  keys.forEach(function(key) {
    var val = localStorage.getItem(key);
    if (val !== null) currentConfig[key] = parseInt(val, 10);
  });
  var url = CONFIG_URL + '?config=' + encodeURIComponent(JSON.stringify(currentConfig));
  Pebble.openURL(url);
});

Pebble.addEventListener('webviewclosed', function(e) {
  if (!e || !e.response || e.response === 'CANCELLED') return;
  var payload;
  try {
    payload = JSON.parse(decodeURIComponent(e.response));
  } catch(err) {
    console.log('Error parsing config: ' + err);
    return;
  }

  Object.keys(payload).forEach(function(key) {
    localStorage.setItem(key, payload[key]);
  });

  var keyMap = {
    bg_color:    1,
    time_color:  2,
    date_color:  3,
    day_color:   4,
    time_align:  5,
    date_align:  6,
    prefix:      7,
    date_format: 8,
    font_choice: 9,
    hour_color:  10,
    min_color:   11,
    day_format:  12,
    time_case:   13,
    date_case:   14,
    day_case:    15,
  };

  var message = {};
  Object.keys(keyMap).forEach(function(name) {
    if (payload[name] !== undefined) {
      message[keyMap[name]] = parseInt(payload[name], 10);
    }
  });

  Pebble.sendAppMessage(message,
    function() { console.log('Config sent OK'); },
    function(err) { console.log('Config send failed: ' + err); }
  );
});
