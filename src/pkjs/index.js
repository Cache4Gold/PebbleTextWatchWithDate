var CONFIG_URL = 'https://cache4gold.github.io/PebbleTextWatchWithDate/config/config.html';

// WMO weather code -> simple condition string
function wmoToCondition(code) {
  if (code === 0)                return 'clear';
  if (code <= 2)                 return 'partly cloudy';
  if (code === 3)                return 'cloudy';
  if (code <= 49)                return 'foggy';
  if (code <= 59)                return 'drizzle';
  if (code <= 69)                return 'rain';
  if (code <= 79)                return 'snow';
  if (code <= 84)                return 'rain';
  if (code <= 86)                return 'snow';
  if (code <= 99)                return 'storm';
  return 'unknown';
}

function fetchWeather(lat, lon, unit) {
  var tempUnit = (unit === 1) ? 'celsius' : 'fahrenheit';
  var url = 'https://api.open-meteo.com/v1/forecast' +
    '?latitude=' + lat +
    '&longitude=' + lon +
    '&current_weather=true' +
    '&temperature_unit=' + tempUnit +
    '&forecast_days=1';

  var req = new XMLHttpRequest();
  req.open('GET', url, true);
  req.onload = function() {
    try {
      var data = JSON.parse(req.responseText);
      var temp = Math.round(data.current_weather.temperature);
      var cond = wmoToCondition(data.current_weather.weathercode);

      console.log('Weather: ' + temp + ' ' + cond);

      Pebble.sendAppMessage(
        { 27: temp, 28: cond },
        function() { console.log('Weather sent OK'); },
        function(e) { console.log('Weather send failed: ' + JSON.stringify(e)); }
      );
    } catch(e) {
      console.log('Weather parse error: ' + e);
    }
  };
  req.onerror = function() { console.log('Weather request failed'); };
  req.send();
}

function getWeather() {
  var unit = parseInt(localStorage.getItem('weather_unit') || '0', 10);
  navigator.geolocation.getCurrentPosition(
    function(pos) {
      fetchWeather(pos.coords.latitude, pos.coords.longitude, unit);
    },
    function(err) {
      console.log('Geolocation error: ' + err.message);
    },
    { enableHighAccuracy: false, maximumAge: 300000, timeout: 10000 }
  );
}

Pebble.addEventListener('ready', function() {
  getWeather();
  // Refresh every 30 minutes
  setInterval(getWeather, 30 * 60 * 1000);
});

Pebble.addEventListener('showConfiguration', function() {
  var keys = [
    'bg_color','hour_color','min_color','time_align','prefix',
    'hour_case','min_case',
    'slot1_content','slot1_color','slot1_align','slot1_case',
    'slot2_content','slot2_color','slot2_align','slot2_case',
    'weather_unit'
  ];
  var currentConfig = {};
  keys.forEach(function(key) {
    var val = localStorage.getItem(key);
    if (val !== null) currentConfig[key] = parseInt(val, 10);
  });
  Pebble.openURL(CONFIG_URL + '?config=' + encodeURIComponent(JSON.stringify(currentConfig)));
});

Pebble.addEventListener('webviewclosed', function(e) {
  if (!e || !e.response || e.response === 'CANCELLED') return;
  var payload;
  try { payload = JSON.parse(decodeURIComponent(e.response)); }
  catch(err) { console.log('Config parse error: ' + err); return; }

  Object.keys(payload).forEach(function(key) { localStorage.setItem(key, payload[key]); });

  // Update weather unit cache for next fetch
  if (payload.weather_unit !== undefined) {
    localStorage.setItem('weather_unit', payload.weather_unit);
  }

  var keyMap = {
    bg_color:1, hour_color:10, min_color:11,
    time_align:5, prefix:7, hour_case:16, min_case:17,
    slot1_content:18, slot1_color:20, slot1_align:22, slot1_case:24,
    slot2_content:19, slot2_color:21, slot2_align:23, slot2_case:25,
    weather_unit:26,
  };

  var message = {};
  Object.keys(keyMap).forEach(function(name) {
    if (payload[name] !== undefined) message[keyMap[name]] = parseInt(payload[name], 10);
  });

  Pebble.sendAppMessage(message,
    function() {
      console.log('Config sent OK');
      // Re-fetch weather in case unit changed
      getWeather();
    },
    function(err) { console.log('Config send failed: ' + err); }
  );
});
