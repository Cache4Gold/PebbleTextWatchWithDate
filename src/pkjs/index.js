// src/pkjs/index.js
// This runs on the phone inside the Pebble app.
// It opens the config page and forwards the result to the watch.

var CONFIG_URL = 'https://cache4gold.github.io/PebbleTextWatchWithDate/config/config.html';

Pebble.addEventListener('ready', function() {
  // Phone-side JS is ready
});

Pebble.addEventListener('showConfiguration', function() {
  // Build URL with current config so the page can pre-populate fields
  var currentConfig = {};
  var keys = [
    'bg_color', 'time_color', 'date_color', 'day_color',
    'time_align', 'date_align', 'prefix', 'date_format', 'font_choice'
  ];
  keys.forEach(function(key) {
    var val = localStorage.getItem(key);
    if (val !== null) {
      currentConfig[key] = parseInt(val, 10);
    }
  });

  var url = CONFIG_URL + '?config=' + encodeURIComponent(JSON.stringify(currentConfig));
  Pebble.openURL(url);
});

Pebble.addEventListener('webviewclosed', function(e) {
  if (!e || !e.response || e.response === 'CANCELLED') {
    return;
  }

  var payload;
  try {
    payload = JSON.parse(decodeURIComponent(e.response));
  } catch (err) {
    console.log('Error parsing config response: ' + err);
    return;
  }

  // Save to localStorage so we can restore on next config open
  Object.keys(payload).forEach(function(key) {
    localStorage.setItem(key, payload[key]);
  });

  // Map key names to numeric AppMessage keys (must match appinfo.json appKeys)
  var keyMap = {
    bg_color:    1,
    time_color:  2,
    date_color:  3,
    day_color:   4,
    time_align:  5,
    date_align:  6,
    prefix:      7,
    date_format: 8,
    font_choice: 9
  };

  var message = {};
  Object.keys(keyMap).forEach(function(name) {
    if (payload[name] !== undefined) {
      message[keyMap[name]] = parseInt(payload[name], 10);
    }
  });

  Pebble.sendAppMessage(message, function() {
    console.log('Config sent to watch successfully');
  }, function(err) {
    console.log('Failed to send config to watch: ' + err);
  });
});
