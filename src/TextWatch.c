#include <pebble.h>
#include <ctype.h>
#include <stdbool.h>
#include "num2words-en.h"

// -------------------------------------------------------------------------
// Persist keys
// -------------------------------------------------------------------------
#define PERSIST_KEY_BG_COLOR        1
#define PERSIST_KEY_TIME_COLOR      2  // unused, kept for legacy compat
#define PERSIST_KEY_DATE_COLOR      3  // slot2 color
#define PERSIST_KEY_DAY_COLOR       4  // slot1 color
#define PERSIST_KEY_TIME_ALIGN      5
#define PERSIST_KEY_DATE_ALIGN      6  // slot alignment (both)
#define PERSIST_KEY_PREFIX          7
#define PERSIST_KEY_DATE_FORMAT     8  // unused directly now
#define PERSIST_KEY_FONT_CHOICE     9  // unused, kept for compat
#define PERSIST_KEY_HOUR_COLOR      10
#define PERSIST_KEY_MIN_COLOR       11
#define PERSIST_KEY_DAY_FORMAT      12 // slot1 content
#define PERSIST_KEY_TIME_CASE       13 // unused
#define PERSIST_KEY_DATE_CASE       14 // slot2 case
#define PERSIST_KEY_DAY_CASE        15 // slot1 case
#define PERSIST_KEY_HOUR_CASE       16
#define PERSIST_KEY_MIN_CASE        17
#define PERSIST_KEY_SLOT1_CONTENT   18
#define PERSIST_KEY_SLOT2_CONTENT   19
#define PERSIST_KEY_SLOT1_COLOR     20
#define PERSIST_KEY_SLOT2_COLOR     21
#define PERSIST_KEY_SLOT1_ALIGN     22
#define PERSIST_KEY_SLOT2_ALIGN     23
#define PERSIST_KEY_SLOT1_CASE      24
#define PERSIST_KEY_SLOT2_CASE      25
#define PERSIST_KEY_WEATHER_UNIT    26
#define PERSIST_KEY_WEATHER_TEMP    27
#define PERSIST_KEY_WEATHER_COND    28

// AppMessage keys
#define MSG_KEY_BG_COLOR            1
#define MSG_KEY_HOUR_COLOR          10
#define MSG_KEY_MIN_COLOR           11
#define MSG_KEY_TIME_ALIGN          5
#define MSG_KEY_PREFIX              7
#define MSG_KEY_HOUR_CASE           16
#define MSG_KEY_MIN_CASE            17
#define MSG_KEY_SLOT1_CONTENT       18
#define MSG_KEY_SLOT2_CONTENT       19
#define MSG_KEY_SLOT1_COLOR         20
#define MSG_KEY_SLOT2_COLOR         21
#define MSG_KEY_SLOT1_ALIGN         22
#define MSG_KEY_SLOT2_ALIGN         23
#define MSG_KEY_SLOT1_CASE          24
#define MSG_KEY_SLOT2_CASE          25
#define MSG_KEY_WEATHER_UNIT        26
#define MSG_KEY_WEATHER_TEMP        27
#define MSG_KEY_WEATHER_COND        28

// -------------------------------------------------------------------------
// Enums
// -------------------------------------------------------------------------
typedef enum {
  ALIGN_LEFT   = 0,
  ALIGN_CENTER = 1,
  ALIGN_RIGHT  = 2,
} AlignOption;

typedef enum {
  CASE_LOWER  = 0,
  CASE_UPPER  = 1,
  CASE_PROPER = 2,
} CaseOption;

// Slot content options
typedef enum {
  SLOT_HIDDEN          = 0,
  SLOT_DAY_LONG        = 1,
  SLOT_DAY_SHORT       = 2,
  SLOT_DATE_LONG_US    = 3,
  SLOT_DATE_SHORT_US   = 4,
  SLOT_DATE_LONG_EU    = 5,
  SLOT_DATE_SHORT_EU   = 6,
  SLOT_DATE_NUM_MDY    = 7,
  SLOT_DATE_NUM_DMY    = 8,
  SLOT_DATE_NUM_YMD    = 9,
  SLOT_DATE_DAY_NUM    = 10,
  SLOT_WEATHER_TEMP    = 11,
  SLOT_WEATHER_COND    = 12,
  SLOT_WEATHER_TEMP_COND = 13,
  SLOT_WEATHER_COND_TEMP = 14,
} SlotContent;

typedef enum {
  UNIT_F = 0,
  UNIT_C = 1,
} WeatherUnit;

// -------------------------------------------------------------------------
// Settings struct
// -------------------------------------------------------------------------
typedef struct {
  GColor      bg_color;
  GColor      hour_color;
  GColor      min_color;
  AlignOption time_align;
  MinutePrefix prefix;
  CaseOption  hour_case;
  CaseOption  min_case;
  // Slot 1 (top info line)
  SlotContent slot1_content;
  GColor      slot1_color;
  AlignOption slot1_align;
  CaseOption  slot1_case;
  // Slot 2 (bottom info line)
  SlotContent slot2_content;
  GColor      slot2_color;
  AlignOption slot2_align;
  CaseOption  slot2_case;
  // Weather
  WeatherUnit weather_unit;
} Settings;

// -------------------------------------------------------------------------
// Globals
// -------------------------------------------------------------------------
#define BUFFER_SIZE 48

static Window    *s_window;
static Settings   s_settings;

typedef struct {
  TextLayer *currentLayer;
  TextLayer *nextLayer;
  PropertyAnimation *currentAnimation;
  PropertyAnimation *nextAnimation;
} Line;

static Line s_line1, s_line2, s_line3;
static TextLayer *s_slot1_layer;
static TextLayer *s_slot2_layer;

static char s_line1Str[2][BUFFER_SIZE];
static char s_line2Str[2][BUFFER_SIZE];
static char s_line3Str[2][BUFFER_SIZE];

static int s_screen_w;
static int s_screen_h;

// Weather data (received from phone)
static int   s_weather_temp = 9999;   // 9999 = not loaded
static char  s_weather_cond[32] = "";

// -------------------------------------------------------------------------
// Case transformation
// -------------------------------------------------------------------------
static void apply_case(char *str, CaseOption c) {
  if (!str || str[0] == 0) return;
  switch (c) {
    case CASE_UPPER:
      for (int i = 0; str[i]; i++) str[i] = (char)toupper((unsigned char)str[i]);
      break;
    case CASE_PROPER:
      str[0] = (char)toupper((unsigned char)str[0]);
      break;
    case CASE_LOWER:
    default:
      for (int i = 0; str[i]; i++) str[i] = (char)tolower((unsigned char)str[i]);
      break;
  }
}

// -------------------------------------------------------------------------
// Settings load/save
// -------------------------------------------------------------------------
static void prv_load_settings(void) {
  s_settings.bg_color    = persist_exists(PERSIST_KEY_BG_COLOR)
                           ? (GColor){ .argb = persist_read_int(PERSIST_KEY_BG_COLOR) }
                           : GColorBlack;
  s_settings.hour_color  = persist_exists(PERSIST_KEY_HOUR_COLOR)
                           ? (GColor){ .argb = persist_read_int(PERSIST_KEY_HOUR_COLOR) }
                           : GColorWhite;
  s_settings.min_color   = persist_exists(PERSIST_KEY_MIN_COLOR)
                           ? (GColor){ .argb = persist_read_int(PERSIST_KEY_MIN_COLOR) }
                           : GColorWhite;
  s_settings.time_align  = persist_exists(PERSIST_KEY_TIME_ALIGN)
                           ? persist_read_int(PERSIST_KEY_TIME_ALIGN) : ALIGN_LEFT;
  s_settings.prefix      = persist_exists(PERSIST_KEY_PREFIX)
                           ? persist_read_int(PERSIST_KEY_PREFIX) : PREFIX_O;
  s_settings.hour_case   = persist_exists(PERSIST_KEY_HOUR_CASE)
                           ? persist_read_int(PERSIST_KEY_HOUR_CASE) : CASE_LOWER;
  s_settings.min_case    = persist_exists(PERSIST_KEY_MIN_CASE)
                           ? persist_read_int(PERSIST_KEY_MIN_CASE) : CASE_LOWER;
  s_settings.slot1_content = persist_exists(PERSIST_KEY_SLOT1_CONTENT)
                           ? persist_read_int(PERSIST_KEY_SLOT1_CONTENT) : SLOT_DAY_LONG;
  s_settings.slot1_color = persist_exists(PERSIST_KEY_SLOT1_COLOR)
                           ? (GColor){ .argb = persist_read_int(PERSIST_KEY_SLOT1_COLOR) }
                           : GColorWhite;
  s_settings.slot1_align = persist_exists(PERSIST_KEY_SLOT1_ALIGN)
                           ? persist_read_int(PERSIST_KEY_SLOT1_ALIGN) : ALIGN_RIGHT;
  s_settings.slot1_case  = persist_exists(PERSIST_KEY_SLOT1_CASE)
                           ? persist_read_int(PERSIST_KEY_SLOT1_CASE) : CASE_LOWER;
  s_settings.slot2_content = persist_exists(PERSIST_KEY_SLOT2_CONTENT)
                           ? persist_read_int(PERSIST_KEY_SLOT2_CONTENT) : SLOT_DATE_LONG_US;
  s_settings.slot2_color = persist_exists(PERSIST_KEY_SLOT2_COLOR)
                           ? (GColor){ .argb = persist_read_int(PERSIST_KEY_SLOT2_COLOR) }
                           : GColorWhite;
  s_settings.slot2_align = persist_exists(PERSIST_KEY_SLOT2_ALIGN)
                           ? persist_read_int(PERSIST_KEY_SLOT2_ALIGN) : ALIGN_RIGHT;
  s_settings.slot2_case  = persist_exists(PERSIST_KEY_SLOT2_CASE)
                           ? persist_read_int(PERSIST_KEY_SLOT2_CASE) : CASE_LOWER;
  s_settings.weather_unit = persist_exists(PERSIST_KEY_WEATHER_UNIT)
                           ? persist_read_int(PERSIST_KEY_WEATHER_UNIT) : UNIT_F;

  // Restore cached weather
  if (persist_exists(PERSIST_KEY_WEATHER_TEMP)) {
    s_weather_temp = persist_read_int(PERSIST_KEY_WEATHER_TEMP);
  }
  if (persist_exists(PERSIST_KEY_WEATHER_COND)) {
    persist_read_string(PERSIST_KEY_WEATHER_COND, s_weather_cond, sizeof(s_weather_cond));
  }
}

static void prv_save_settings(void) {
  persist_write_int(PERSIST_KEY_BG_COLOR,      s_settings.bg_color.argb);
  persist_write_int(PERSIST_KEY_HOUR_COLOR,    s_settings.hour_color.argb);
  persist_write_int(PERSIST_KEY_MIN_COLOR,     s_settings.min_color.argb);
  persist_write_int(PERSIST_KEY_TIME_ALIGN,    s_settings.time_align);
  persist_write_int(PERSIST_KEY_PREFIX,        s_settings.prefix);
  persist_write_int(PERSIST_KEY_HOUR_CASE,     s_settings.hour_case);
  persist_write_int(PERSIST_KEY_MIN_CASE,      s_settings.min_case);
  persist_write_int(PERSIST_KEY_SLOT1_CONTENT, s_settings.slot1_content);
  persist_write_int(PERSIST_KEY_SLOT1_COLOR,   s_settings.slot1_color.argb);
  persist_write_int(PERSIST_KEY_SLOT1_ALIGN,   s_settings.slot1_align);
  persist_write_int(PERSIST_KEY_SLOT1_CASE,    s_settings.slot1_case);
  persist_write_int(PERSIST_KEY_SLOT2_CONTENT, s_settings.slot2_content);
  persist_write_int(PERSIST_KEY_SLOT2_COLOR,   s_settings.slot2_color.argb);
  persist_write_int(PERSIST_KEY_SLOT2_ALIGN,   s_settings.slot2_align);
  persist_write_int(PERSIST_KEY_SLOT2_CASE,    s_settings.slot2_case);
  persist_write_int(PERSIST_KEY_WEATHER_UNIT,  s_settings.weather_unit);
}

// -------------------------------------------------------------------------
// Font helpers
// -------------------------------------------------------------------------
static GFont prv_get_font_bold(void) {
  return fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD);
}

static GFont prv_get_font_light(void) {
  return fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT);
}

static int prv_line_height(void) {
  return 50;
}

static GTextAlignment prv_galign(AlignOption a) {
  switch (a) {
    case ALIGN_CENTER: return GTextAlignmentCenter;
    case ALIGN_RIGHT:  return GTextAlignmentRight;
    default:           return GTextAlignmentLeft;
  }
}

// -------------------------------------------------------------------------
// Slot content builder
// -------------------------------------------------------------------------
static void prv_build_slot_text(SlotContent content, struct tm *tm, char *buf, size_t len) {
  memset(buf, 0, len);

  int day = tm->tm_mday;
  const char *suffix;
  if (day == 1 || day == 21 || day == 31)    suffix = "st";
  else if (day == 2 || day == 22)             suffix = "nd";
  else if (day == 3 || day == 23)             suffix = "rd";
  else                                        suffix = "th";

  char month_long[16], month_short[8], year_str[8];
  strftime(month_long,  sizeof(month_long),  "%B", tm);
  strftime(month_short, sizeof(month_short), "%b", tm);
  strftime(year_str,    sizeof(year_str),    "%Y", tm);

  bool have_weather = (s_weather_temp != 9999);
  const char *unit_str = (s_settings.weather_unit == UNIT_C) ? "c" : "f";
  char temp_str[16] = "--";
  if (have_weather) {
    snprintf(temp_str, sizeof(temp_str), "%d°%s", s_weather_temp, unit_str);
  }

  switch (content) {
    case SLOT_HIDDEN:
      break;
    case SLOT_DAY_LONG:
      strftime(buf, len, "%A", tm);
      break;
    case SLOT_DAY_SHORT:
      strftime(buf, len, "%a", tm);
      break;
    case SLOT_DATE_LONG_US:
      snprintf(buf, len, "%s %d%s, %s", month_long, day, suffix, year_str);
      break;
    case SLOT_DATE_SHORT_US:
      snprintf(buf, len, "%s %d%s", month_short, day, suffix);
      break;
    case SLOT_DATE_LONG_EU:
      snprintf(buf, len, "%d%s %s %s", day, suffix, month_long, year_str);
      break;
    case SLOT_DATE_SHORT_EU:
      snprintf(buf, len, "%d%s %s", day, suffix, month_short);
      break;
    case SLOT_DATE_NUM_MDY:
      strftime(buf, len, "%m/%d/%Y", tm);
      break;
    case SLOT_DATE_NUM_DMY:
      strftime(buf, len, "%d/%m/%Y", tm);
      break;
    case SLOT_DATE_NUM_YMD:
      strftime(buf, len, "%Y/%m/%d", tm);
      break;
    case SLOT_DATE_DAY_NUM:
      snprintf(buf, len, "%d", day);
      break;
    case SLOT_WEATHER_TEMP:
      strncpy(buf, temp_str, len - 1);
      break;
    case SLOT_WEATHER_COND:
      strncpy(buf, have_weather ? s_weather_cond : "--", len - 1);
      break;
    case SLOT_WEATHER_TEMP_COND:
      if (have_weather) snprintf(buf, len, "%s %s", temp_str, s_weather_cond);
      else strncpy(buf, "--", len - 1);
      break;
    case SLOT_WEATHER_COND_TEMP:
      if (have_weather) snprintf(buf, len, "%s %s", s_weather_cond, temp_str);
      else strncpy(buf, "--", len - 1);
      break;
  }
}

static void prv_update_slots(struct tm *tm) {
  static char s1[40], s2[40];

  prv_build_slot_text(s_settings.slot1_content, tm, s1, sizeof(s1));
  prv_build_slot_text(s_settings.slot2_content, tm, s2, sizeof(s2));

  apply_case(s1, s_settings.slot1_case);
  apply_case(s2, s_settings.slot2_case);

  text_layer_set_text(s_slot1_layer, s1);
  text_layer_set_text(s_slot2_layer, s2);

  layer_set_hidden((Layer*)s_slot1_layer, s_settings.slot1_content == SLOT_HIDDEN);
  layer_set_hidden((Layer*)s_slot2_layer, s_settings.slot2_content == SLOT_HIDDEN);
}

// -------------------------------------------------------------------------
// Animation
// -------------------------------------------------------------------------
static void prv_destroy_property_animation(PropertyAnimation **prop_animation) {
  if (*prop_animation == NULL) return;
  if (animation_is_scheduled((Animation*)*prop_animation)) {
    animation_unschedule((Animation*)*prop_animation);
    property_animation_destroy(*prop_animation);
  }
  *prop_animation = NULL;
}

static void prv_animation_stopped(struct Animation *animation, bool finished, void *context) {
  Layer *current = (Layer *)context;
  GRect rect = layer_get_frame(current);
  rect.origin.x = s_screen_w;
  layer_set_frame(current, rect);
}

static void prv_animate_line(Line *line, TextLayer *current, TextLayer *next) {
  GRect rect = layer_get_frame((Layer*)next);
  rect.origin.x -= s_screen_w;

  prv_destroy_property_animation(&line->nextAnimation);
  line->nextAnimation = property_animation_create_layer_frame((Layer*)next, NULL, &rect);
  animation_set_duration((Animation*)line->nextAnimation, 400);
  animation_set_curve((Animation*)line->nextAnimation, AnimationCurveEaseOut);
  animation_schedule((Animation*)line->nextAnimation);

  GRect rect2 = layer_get_frame((Layer*)current);
  rect2.origin.x -= s_screen_w;

  prv_destroy_property_animation(&line->currentAnimation);
  line->currentAnimation = property_animation_create_layer_frame((Layer*)current, NULL, &rect2);
  animation_set_duration((Animation*)line->currentAnimation, 400);
  animation_set_curve((Animation*)line->currentAnimation, AnimationCurveEaseOut);
  animation_set_handlers((Animation*)line->currentAnimation, (AnimationHandlers){
    .stopped = (AnimationStoppedHandler)prv_animation_stopped
  }, current);
  animation_schedule((Animation*)line->currentAnimation);
}

static void prv_update_line(Line *line, char lineStr[2][BUFFER_SIZE], char *value) {
  TextLayer *next, *current;
  GRect rect = layer_get_frame((Layer*)line->currentLayer);
  current = (rect.origin.x == 0) ? line->currentLayer : line->nextLayer;
  next    = (current == line->currentLayer) ? line->nextLayer : line->currentLayer;

  if (current == line->currentLayer) {
    memset(lineStr[1], 0, BUFFER_SIZE);
    memcpy(lineStr[1], value, strlen(value));
    text_layer_set_text(next, lineStr[1]);
  } else {
    memset(lineStr[0], 0, BUFFER_SIZE);
    memcpy(lineStr[0], value, strlen(value));
    text_layer_set_text(next, lineStr[0]);
  }

  prv_animate_line(line, current, next);
}

static bool prv_needs_update(Line *line, char lineStr[2][BUFFER_SIZE], char *nextValue) {
  char *currentStr;
  GRect rect = layer_get_frame((Layer*)line->currentLayer);
  currentStr = (rect.origin.x == 0) ? lineStr[0] : lineStr[1];
  return (memcmp(currentStr, nextValue, strlen(nextValue)) != 0 ||
          (strlen(nextValue) == 0 && strlen(currentStr) != 0));
}

// -------------------------------------------------------------------------
// Display time
// -------------------------------------------------------------------------
// Forward declaration
static void prv_update_line(Line *line, char lineStr[2][BUFFER_SIZE], char *value);

static void prv_force_clear_line(Line *line, char lineStr[2][BUFFER_SIZE]) {
  // Use the normal update mechanism to animate the line out with empty content.
  // This keeps the animation system consistent and avoids layer state confusion.
  prv_update_line(line, lineStr, "");
}

static void prv_display_time(struct tm *t) {
  char l1[BUFFER_SIZE], l2[BUFFER_SIZE], l3[BUFFER_SIZE];
  time_to_3words(t->tm_hour, t->tm_min, l1, l2, l3, BUFFER_SIZE,
                 s_settings.prefix, s_screen_w <= 144);

  apply_case(l1, s_settings.hour_case);
  apply_case(l2, s_settings.min_case);
  apply_case(l3, s_settings.min_case);

  if (prv_needs_update(&s_line1, s_line1Str, l1)) prv_update_line(&s_line1, s_line1Str, l1);
  if (prv_needs_update(&s_line2, s_line2Str, l2)) prv_update_line(&s_line2, s_line2Str, l2);

  // For line3: if it should be empty but currently shows content, animate it away.
  // Check the ACTUAL visible layer text, not just the string buffers.
  if (l3[0] == 0) {
    GRect rect3 = layer_get_frame((Layer*)s_line3.currentLayer);
    TextLayer *visible3 = (rect3.origin.x == 0) ? s_line3.currentLayer : s_line3.nextLayer;
    const char *visible_text = text_layer_get_text(visible3);
    if (visible_text && visible_text[0] != 0) {
      prv_update_line(&s_line3, s_line3Str, "");
    }
  } else if (prv_needs_update(&s_line3, s_line3Str, l3)) {
    prv_update_line(&s_line3, s_line3Str, l3);
  }
}

static void prv_display_initial_time(struct tm *t) {
  time_to_3words(t->tm_hour, t->tm_min,
                 s_line1Str[0], s_line2Str[0], s_line3Str[0],
                 BUFFER_SIZE, s_settings.prefix, s_screen_w <= 144);

  apply_case(s_line1Str[0], s_settings.hour_case);
  apply_case(s_line2Str[0], s_settings.min_case);
  apply_case(s_line3Str[0], s_settings.min_case);

  text_layer_set_text(s_line1.currentLayer, s_line1Str[0]);
  text_layer_set_text(s_line2.currentLayer, s_line2Str[0]);
  text_layer_set_text(s_line3.currentLayer, s_line3Str[0]);
  prv_update_slots(t);
}

// -------------------------------------------------------------------------
// Apply settings to layers
// -------------------------------------------------------------------------
static void prv_apply_settings(void) {
  window_set_background_color(s_window, s_settings.bg_color);

  GFont bold_font  = prv_get_font_bold();
  GFont light_font = prv_get_font_light();
  GTextAlignment time_align = prv_galign(s_settings.time_align);

  text_layer_set_font(s_line1.currentLayer, bold_font);
  text_layer_set_font(s_line1.nextLayer,    bold_font);
  text_layer_set_text_color(s_line1.currentLayer, s_settings.hour_color);
  text_layer_set_text_color(s_line1.nextLayer,    s_settings.hour_color);
  text_layer_set_background_color(s_line1.currentLayer, GColorClear);
  text_layer_set_background_color(s_line1.nextLayer,    GColorClear);
  text_layer_set_text_alignment(s_line1.currentLayer, time_align);
  text_layer_set_text_alignment(s_line1.nextLayer,    time_align);

  text_layer_set_font(s_line2.currentLayer, light_font);
  text_layer_set_font(s_line2.nextLayer,    light_font);
  text_layer_set_font(s_line3.currentLayer, light_font);
  text_layer_set_font(s_line3.nextLayer,    light_font);
  text_layer_set_text_color(s_line2.currentLayer, s_settings.min_color);
  text_layer_set_text_color(s_line2.nextLayer,    s_settings.min_color);
  text_layer_set_text_color(s_line3.currentLayer, s_settings.min_color);
  text_layer_set_text_color(s_line3.nextLayer,    s_settings.min_color);
  text_layer_set_background_color(s_line2.currentLayer, GColorClear);
  text_layer_set_background_color(s_line2.nextLayer,    GColorClear);
  text_layer_set_background_color(s_line3.currentLayer, GColorClear);
  text_layer_set_background_color(s_line3.nextLayer,    GColorClear);
  text_layer_set_text_alignment(s_line2.currentLayer, time_align);
  text_layer_set_text_alignment(s_line2.nextLayer,    time_align);
  text_layer_set_text_alignment(s_line3.currentLayer, time_align);
  text_layer_set_text_alignment(s_line3.nextLayer,    time_align);

  // Slot layers
  text_layer_set_text_color(s_slot1_layer, s_settings.slot1_color);
  text_layer_set_text_alignment(s_slot1_layer, prv_galign(s_settings.slot1_align));
  text_layer_set_text_color(s_slot2_layer, s_settings.slot2_color);
  text_layer_set_text_alignment(s_slot2_layer, prv_galign(s_settings.slot2_align));

  // Layout
  int lh = prv_line_height();
  int inset = PBL_IF_ROUND_ELSE(14, 0);
  int content_w = s_screen_w - (inset * 2);

  int y1, y2, y3, slot1_top, slot2_top;

  if (s_screen_h <= 168) {
    y1 = 10; y2 = 47; y3 = 84;
    slot1_top = 135;
    slot2_top = 150;
  } else {
    bool both_hidden = (s_settings.slot1_content == SLOT_HIDDEN &&
                        s_settings.slot2_content == SLOT_HIDDEN);
    int info_area_h = both_hidden ? 22 : 50;
    int available_h = s_screen_h - info_area_h;
    // Tighter line spacing on large screens (44px steps vs 50px font height)
    int line_step = lh - 6;
    int total_text_h = line_step * 2 + lh; // two gaps + last line
    int top_margin = (available_h - total_text_h) / 2;
    if (top_margin < 8) top_margin = 8;
    // Push down slightly so it's not too close to the top
    top_margin += 12;
    y1 = top_margin;
    y2 = top_margin + line_step;
    y3 = top_margin + (line_step * 2);
    slot2_top = s_screen_h - 26;
    slot1_top = s_screen_h - 52;
  }

  layer_set_frame((Layer*)s_line1.currentLayer, GRect(inset, y1, content_w, lh + 8));
  layer_set_frame((Layer*)s_line1.nextLayer,    GRect(s_screen_w, y1, content_w, lh + 8));
  layer_set_frame((Layer*)s_line2.currentLayer, GRect(inset, y2, content_w, lh + 8));
  layer_set_frame((Layer*)s_line2.nextLayer,    GRect(s_screen_w, y2, content_w, lh + 8));
  layer_set_frame((Layer*)s_line3.currentLayer, GRect(inset, y3, content_w, lh + 8));
  layer_set_frame((Layer*)s_line3.nextLayer,    GRect(s_screen_w, y3, content_w, lh + 8));

  layer_set_frame((Layer*)s_slot1_layer, GRect(inset, slot1_top, content_w, 20));
  layer_set_frame((Layer*)s_slot2_layer, GRect(inset, slot2_top, content_w, 20));
}

// -------------------------------------------------------------------------
// AppMessage
// -------------------------------------------------------------------------
static void prv_inbox_received(DictionaryIterator *iter, void *context) {
  Tuple *t;

  t = dict_find(iter, MSG_KEY_BG_COLOR);
  if (t) s_settings.bg_color = (GColor){ .argb = (uint8_t)t->value->int32 };

  t = dict_find(iter, MSG_KEY_HOUR_COLOR);
  if (t) s_settings.hour_color = (GColor){ .argb = (uint8_t)t->value->int32 };

  t = dict_find(iter, MSG_KEY_MIN_COLOR);
  if (t) s_settings.min_color = (GColor){ .argb = (uint8_t)t->value->int32 };

  t = dict_find(iter, MSG_KEY_TIME_ALIGN);
  if (t) s_settings.time_align = (AlignOption)t->value->int32;

  t = dict_find(iter, MSG_KEY_PREFIX);
  if (t) s_settings.prefix = (MinutePrefix)t->value->int32;

  t = dict_find(iter, MSG_KEY_HOUR_CASE);
  if (t) s_settings.hour_case = (CaseOption)t->value->int32;

  t = dict_find(iter, MSG_KEY_MIN_CASE);
  if (t) s_settings.min_case = (CaseOption)t->value->int32;

  t = dict_find(iter, MSG_KEY_SLOT1_CONTENT);
  if (t) s_settings.slot1_content = (SlotContent)t->value->int32;

  t = dict_find(iter, MSG_KEY_SLOT1_COLOR);
  if (t) s_settings.slot1_color = (GColor){ .argb = (uint8_t)t->value->int32 };

  t = dict_find(iter, MSG_KEY_SLOT1_ALIGN);
  if (t) s_settings.slot1_align = (AlignOption)t->value->int32;

  t = dict_find(iter, MSG_KEY_SLOT1_CASE);
  if (t) s_settings.slot1_case = (CaseOption)t->value->int32;

  t = dict_find(iter, MSG_KEY_SLOT2_CONTENT);
  if (t) s_settings.slot2_content = (SlotContent)t->value->int32;

  t = dict_find(iter, MSG_KEY_SLOT2_COLOR);
  if (t) s_settings.slot2_color = (GColor){ .argb = (uint8_t)t->value->int32 };

  t = dict_find(iter, MSG_KEY_SLOT2_ALIGN);
  if (t) s_settings.slot2_align = (AlignOption)t->value->int32;

  t = dict_find(iter, MSG_KEY_SLOT2_CASE);
  if (t) s_settings.slot2_case = (CaseOption)t->value->int32;

  t = dict_find(iter, MSG_KEY_WEATHER_UNIT);
  if (t) s_settings.weather_unit = (WeatherUnit)t->value->int32;

  // Weather data (sent separately from phone on refresh)
  t = dict_find(iter, MSG_KEY_WEATHER_TEMP);
  if (t) {
    s_weather_temp = t->value->int32;
    persist_write_int(PERSIST_KEY_WEATHER_TEMP, s_weather_temp);
  }

  t = dict_find(iter, MSG_KEY_WEATHER_COND);
  if (t) {
    strncpy(s_weather_cond, t->value->cstring, sizeof(s_weather_cond) - 1);
    persist_write_string(PERSIST_KEY_WEATHER_COND, s_weather_cond);
  }

  prv_save_settings();
  prv_apply_settings();

  time_t now = time(NULL);
  prv_display_initial_time(localtime(&now));
}

static void prv_inbox_dropped(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "AppMessage dropped: %d", (int)reason);
}

// -------------------------------------------------------------------------
// Tick handler
// -------------------------------------------------------------------------
static void prv_handle_tick(struct tm *tick_time, TimeUnits units_changed) {
  prv_display_time(tick_time);
  prv_update_slots(tick_time);
}

// -------------------------------------------------------------------------
// Window lifecycle
// -------------------------------------------------------------------------
static void prv_window_load(Window *window) {
  Layer *root = window_get_root_layer(window);
  s_screen_w = layer_get_bounds(root).size.w;
  s_screen_h = layer_get_bounds(root).size.h;

  s_line1.currentLayer = text_layer_create(GRect(0, 0, s_screen_w, 60));
  s_line1.nextLayer    = text_layer_create(GRect(s_screen_w, 0, s_screen_w, 60));
  s_line2.currentLayer = text_layer_create(GRect(0, 0, s_screen_w, 60));
  s_line2.nextLayer    = text_layer_create(GRect(s_screen_w, 0, s_screen_w, 60));
  s_line3.currentLayer = text_layer_create(GRect(0, 0, s_screen_w, 60));
  s_line3.nextLayer    = text_layer_create(GRect(s_screen_w, 0, s_screen_w, 60));

  // Use larger info fonts on big screens, smaller on narrow screens
  GFont slot1_font = (s_screen_h > 168)
    ? fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD)
    : fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);
  GFont slot2_font = (s_screen_h > 168)
    ? fonts_get_system_font(FONT_KEY_GOTHIC_18)
    : fonts_get_system_font(FONT_KEY_GOTHIC_14);
  int slot_h = (s_screen_h > 168) ? 24 : 20;

  s_slot1_layer = text_layer_create(GRect(0, 0, s_screen_w, slot_h));
  text_layer_set_background_color(s_slot1_layer, GColorClear);
  text_layer_set_font(s_slot1_layer, slot1_font);

  s_slot2_layer = text_layer_create(GRect(0, 0, s_screen_w, slot_h));
  text_layer_set_background_color(s_slot2_layer, GColorClear);
  text_layer_set_font(s_slot2_layer, slot2_font);

  prv_apply_settings();

  layer_add_child(root, (Layer*)s_line1.currentLayer);
  layer_add_child(root, (Layer*)s_line1.nextLayer);
  layer_add_child(root, (Layer*)s_line2.currentLayer);
  layer_add_child(root, (Layer*)s_line2.nextLayer);
  layer_add_child(root, (Layer*)s_line3.currentLayer);
  layer_add_child(root, (Layer*)s_line3.nextLayer);
  layer_add_child(root, (Layer*)s_slot1_layer);
  layer_add_child(root, (Layer*)s_slot2_layer);

  time_t now = time(NULL);
  prv_display_initial_time(localtime(&now));
}

static void prv_window_unload(Window *window) {
  prv_destroy_property_animation(&s_line1.currentAnimation);
  prv_destroy_property_animation(&s_line1.nextAnimation);
  prv_destroy_property_animation(&s_line2.currentAnimation);
  prv_destroy_property_animation(&s_line2.nextAnimation);
  prv_destroy_property_animation(&s_line3.currentAnimation);
  prv_destroy_property_animation(&s_line3.nextAnimation);

  text_layer_destroy(s_line1.currentLayer);
  text_layer_destroy(s_line1.nextLayer);
  text_layer_destroy(s_line2.currentLayer);
  text_layer_destroy(s_line2.nextLayer);
  text_layer_destroy(s_line3.currentLayer);
  text_layer_destroy(s_line3.nextLayer);
  text_layer_destroy(s_slot1_layer);
  text_layer_destroy(s_slot2_layer);
}

// -------------------------------------------------------------------------
// Main
// -------------------------------------------------------------------------
int main(void) {
  prv_load_settings();

  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers){
    .load   = prv_window_load,
    .unload = prv_window_unload,
  });
  window_stack_push(s_window, true);

  app_message_open(256, 64);
  app_message_register_inbox_received(prv_inbox_received);
  app_message_register_inbox_dropped(prv_inbox_dropped);

  tick_timer_service_subscribe(MINUTE_UNIT, prv_handle_tick);

  app_event_loop();

  tick_timer_service_unsubscribe();
  window_destroy(s_window);

  return 0;
}
