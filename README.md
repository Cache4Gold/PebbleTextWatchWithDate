# Customizable Text Watch with Date and Weather

A clean, fully customizable text-based watch face for Pebble. Tells the time in words — the way it was always meant to be read.

Built originally over a decade ago and completely rebuilt from the ground up for 2025, this watch face works beautifully on every Pebble ever made, from the original all the way to the new Time 2 and Round 2.

---

## Features

### Time in Words
The time is displayed across three lines in plain English. "ten twenty four." Clean, readable, timeless.

### Two Flexible Info Lines
The bottom of the watch face has two independently configurable info lines. Each one can show any of the following:

- **Day of week** — long (monday) or short (mon)
- **Date** — US long, US short, EU long, EU short, numeric M/D/Y, D/M/Y, Y/M/D, or day number only
- **Weather** — temperature only, conditions only, temp + conditions, or conditions + temp
- **Hidden** — for a cleaner look

Each info line has its own color, alignment, and case setting.

### Weather
Live weather is fetched automatically using your phone's location — no account or API key needed. Temperature in °F or °C. Conditions are simple plain-English descriptions: clear, cloudy, rain, snow, storm, and so on. Weather refreshes on launch and every 30 minutes.

### Deep Customization
- **Colors** — background, hour text, minute text, and each info line independently, using the full 64-color Pebble palette
- **Alignment** — left, center, or right for the time and each info line independently
- **Case** — lowercase, UPPERCASE, or Proper Case for every text element independently
- **Minute prefix** — none ("four"), "oh" ("oh four"), or "o'" ("o' four") for single-digit minutes

### Works on Every Pebble
Tested and optimized for all five Pebble platforms. The Time 2 and Round 2 get the full large Bitham 42 layout. Older watches with smaller screens automatically adapt — long words split intelligently so nothing gets cut off.

---

## Configuration

Open the Pebble app on your phone, find the watch face in your locker, and tap the Settings icon. Changes apply to the watch instantly.

---

## Weather Details

Weather is powered by [Open-Meteo](https://open-meteo.com) — a free, open-source weather API with no sign-up required. Location is detected automatically from your phone. If location access is denied, weather slots will show "--" until access is granted.

---

## Credits

Developed by [Cache4Gold](https://github.com/Cache4Gold)

Original watch face concept by Saracco (2012). Completely rewritten and expanded in 2025.

---

## License

MIT License. See LICENSE file for details.
