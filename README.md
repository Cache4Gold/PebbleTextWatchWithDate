# Customizable Text Watch with Date and Weather

A clean, fully customizable text-based watch face for Pebble. Tells the time in words, the way it was always meant to be read.

Built originally over a decade ago and completely rebuilt from the ground up for 2025, this watch face is designed primarily for the Pebble Time 2, with support for the original Pebble and Pebble Time as well.

---

## Features

### Time in Words
The time is displayed across three lines in plain English. "ten twenty four." Clean, readable, timeless.

### Two Flexible Info Lines
The bottom of the watch face has two independently configurable info lines. Each one can show any of the following:

- Day of week: long (monday) or short (mon)
- Date: US long, US short, EU long, EU short, numeric M/D/Y, D/M/Y, Y/M/D, or day number only
- Weather: temperature only, conditions only, temp + conditions, or conditions + temp
- Hidden: for a cleaner look

Each info line has its own color, alignment, and case setting.

### Weather
Live weather is fetched automatically using your phone's location — no account or API key needed. Temperature in °F or °C. Conditions are simple plain-English descriptions: clear, cloudy, rain, snow, storm, and so on. Weather refreshes on launch and every 30 minutes.

### Deep Customization
- Colors: background, hour text, minute text, and each info line independently, using the full 64-color Pebble palette
- Alignment: left, center, or right for the time and each info line independently
- Case: lowercase, UPPERCASE, or Proper Case for every text element independently
- Minute prefix: none ("four"), "oh" ("oh four"), or "o'" ("o' four") for single-digit minutes

### Screen Compatibility
Optimized for the Pebble Time 2. Also works on the original Pebble and Pebble Time, where the layout automatically adapts and long words split intelligently so nothing gets cut off.

---

## Weather Details

Weather is powered by [Open-Meteo](https://open-meteo.com) — a free, open-source weather API with no sign-up required. Location is detected automatically from your phone. If location access is denied, weather slots will show "--" until access is granted.


