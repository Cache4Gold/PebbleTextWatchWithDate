#include <pebble.h>
#include <ctype.h>
#include "num2words-en.h"

// -------------------------------------------------------------------------
// Persistent storage keys
// -------------------------------------------------------------------------
#define PERSIST_KEY_BG_COLOR        1
#define PERSIST_KEY_TIME_COLOR      2
#define PERSIST_KEY_DATE_COLOR      3
#define PERSIST_KEY_DAY_COLOR       4
#define PERSIST_KEY_TIME_ALIGN      5
#define PERSIST_KEY_DATE_ALIGN      6
#define PERSIST_KEY_PREFIX          7
#define PERSIST_KEY_DATE_FORMAT     8
#define PERSIST_KEY_FONT_CHOICE     9
#define PERSIST_KEY_HOUR_COLOR      10
#define PERSIST_KEY_MIN_COLOR       11
#define PERSIST_KEY_DAY_FORMAT      12
#define PERSIST_KEY_TIME_CASE       13
#define PERSIST_KEY_DATE_CASE       14
#define PERSIST_KEY_DAY_CASE        15

// AppMessage keys
#define MSG_KEY_BG_COLOR            1
#define MSG_KEY_TIME_COLOR          2
#define MSG_KEY_DATE_COLOR          3
#define MSG_KEY_DAY_COLOR           4
#define MSG_KEY_TIME_ALIGN          5
#define MSG_KEY_DATE_ALIGN          6
#define MSG_KEY_PREFIX              7
#define MSG_KEY_DATE_FORMAT         8
#define MSG_KEY_FONT_CHOICE         9
#define MSG_KEY_HOUR_COLOR          10
#define MSG_KEY_MIN_COLOR           11
#define MSG_KEY_DAY_FORMAT          12
#define MSG_KEY_TIME_CASE           13
#define MSG_KEY_DATE_CASE           14
#define MSG_KEY_DAY_CASE            15

// -------------------------------------------------------------------------
// Enums
// -------------------------------------------------------------------------
typedef enum {
  ALIGN_LEFT   = 0,
  ALIGN_CENTER = 1,
  ALIGN_RIGHT  = 2,
} AlignOption;

typedef enum {
  DATE_FORMAT_LONG_US   = 0,  // monday / april 27th, 2026
  DATE_FORMAT_SHORT_US  = 1,  // mon / apr 27th
  DATE_FORMAT_LONG_EU   = 2,  // monday / 27th april 2026
  DATE_FORMAT_SHORT_EU  = 3,  // mon / 27th apr
  DATE_FORMAT_NUM_MDY   = 4,  // 4/27/2026
  DATE_FORMAT_NUM_DMY   = 5,  // 27/4/2026
  DATE_FORMAT_NUM_YMD   = 6,  // 2026/4/27
} DateFormat;

typedef enum {
  DAY_FORMAT_LONG  = 0,  // monday
  DAY_FORMAT_SHORT = 1,  // mon
  DAY_FORMAT_NONE  = 2,  // hidden
} DayFormat;

typedef enum {
  FONT_BITHAM_42  = 0,
  FONT_ROBOTO_49  = 1,
  FONT_BITHAM_34  = 2,
  FONT_GOTHIC_24  = 3,
  FONT_BITHAM_30  = 4,
} FontChoice;

typedef enum {
  CASE_LOWER  = 0,
  CASE_UPPER  = 1,
  CASE_PROPER = 2,
} CaseOption;

// -------------------------------------------------------------------------
// Settings struct
// -------------------------------------------------------------------------
typedef struct {
  GColor      bg_color;
  GColor      hour_color;
  GColor      min_color;
  GColor      date_color;
  GColor      day_color;
  AlignOption time_align;
  AlignOption date_align;
  MinutePrefix prefix;
  DateFormat  date_format;
  DayFormat   day_format;
  FontChoice  font_choice;
  CaseOption  time_case;
  CaseOption  date_case;
  CaseOption  day_case;
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
static TextLayer *s_date_layer;
static TextLayer *s_day_layer;

static char s_line1Str[2][BUFFER_SIZE];
static char s_line2Str[2][BUFFER_SIZE];
static char s_line3Str[2][BUFFER_SIZE];

static int s_screen_w;
static int s_screen_h;

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
  s_settings.date_color  = persist_exists(PERSIST_KEY_DATE_COLOR)
                           ? (GColor){ .argb = persist_read_int(PERSIST_KEY_DATE_COLOR) }
                           : GColorWhite;
  s_settings.day_color   = persist_exists(PERSIST_KEY_DAY_COLOR)
                           ? (GColor){ .argb = persist_read_int(PERSIST_KEY_DAY_COLOR) }
                           : GColorWhite;
  s_settings.time_align  = persist_exists(PERSIST_KEY_TIME_ALIGN)
                           ? persist_read_int(PERSIST_KEY_TIME_ALIGN) : ALIGN_LEFT;
  s_settings.date_align  = persist_exists(PERSIST_KEY_DATE_ALIGN)
                           ? persist_read_int(PERSIST_KEY_DATE_ALIGN) : ALIGN_RIGHT;
  s_settings.prefix      = persist_exists(PERSIST_KEY_PREFIX)
                           ? persist_read_int(PERSIST_KEY_PREFIX) : PREFIX_NONE;
  s_settings.date_format = persist_exists(PERSIST_KEY_DATE_FORMAT)
                           ? persist_read_int(PERSIST_KEY_DATE_FORMAT) : DATE_FORMAT_LONG_US;
  s_settings.day_format  = persist_exists(PERSIST_KEY_DAY_FORMAT)
                           ? persist_read_int(PERSIST_KEY_DAY_FORMAT) : DAY_FORMAT_LONG;
  s_settings.font_choice = persist_exists(PERSIST_KEY_FONT_CHOICE)
                           ? persist_read_int(PERSIST_KEY_FONT_CHOICE) : FONT_BITHAM_42;
  s_settings.time_case   = persist_exists(PERSIST_KEY_TIME_CASE)
                           ? persist_read_int(PERSIST_KEY_TIME_CASE) : CASE_LOWER;
  s_settings.date_case   = persist_exists(PERSIST_KEY_DATE_CASE)
                           ? persist_read_int(PERSIST_KEY_DATE_CASE) : CASE_LOWER;
  s_settings.day_case    = persist_exists(PERSIST_KEY_DAY_CASE)
                           ? persist_read_int(PERSIST_KEY_DAY_CASE) : CASE_LOWER;
}

static void prv_save_settings(void) {
  persist_write_int(PERSIST_KEY_BG_COLOR,    s_settings.bg_color.argb);
  persist_write_int(PERSIST_KEY_HOUR_COLOR,  s_settings.hour_color.argb);
  persist_write_int(PERSIST_KEY_MIN_COLOR,   s_settings.min_color.argb);
  persist_write_int(PERSIST_KEY_DATE_COLOR,  s_settings.date_color.argb);
  persist_write_int(PERSIST_KEY_DAY_COLOR,   s_settings.day_color.argb);
  persist_write_int(PERSIST_KEY_TIME_ALIGN,  s_settings.time_align);
  persist_write_int(PERSIST_KEY_DATE_ALIGN,  s_settings.date_align);
  persist_write_int(PERSIST_KEY_PREFIX,      s_settings.prefix);
  persist_write_int(PERSIST_KEY_DATE_FORMAT, s_settings.date_format);
  persist_write_int(PERSIST_KEY_DAY_FORMAT,  s_settings.day_format);
  persist_write_int(PERSIST_KEY_FONT_CHOICE, s_settings.font_choice);
  persist_write_int(PERSIST_KEY_TIME_CASE,   s_settings.time_case);
  persist_write_int(PERSIST_KEY_DATE_CASE,   s_settings.date_case);
  persist_write_int(PERSIST_KEY_DAY_CASE,    s_settings.day_case);
}

// -------------------------------------------------------------------------
// Font helpers
// -------------------------------------------------------------------------
static GFont prv_get_font_bold(void) {
  switch (s_settings.font_choice) {
    case FONT_ROBOTO_49: return fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49);
    case FONT_BITHAM_34: return fonts_get_system_font(FONT_KEY_BITHAM_34_MEDIUM_NUMBERS);
    case FONT_GOTHIC_24: return fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
    case FONT_BITHAM_30: return fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK);
    default:             return fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD);
  }
}

static GFont prv_get_font_light(void) {
  switch (s_settings.font_choice) {
    case FONT_ROBOTO_49: return fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49);
    case FONT_BITHAM_34: return fonts_get_system_font(FONT_KEY_BITHAM_34_MEDIUM_NUMBERS);
    case FONT_GOTHIC_24: return fonts_get_system_font(FONT_KEY_GOTHIC_24);
    case FONT_BITHAM_30: return fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK);
    default:             return fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT);
  }
}

static int prv_line_height(void) {
  switch (s_settings.font_choice) {
    case FONT_ROBOTO_49: return 52;
    case FONT_BITHAM_34: return 38;
    case FONT_GOTHIC_24: return 30;
    case FONT_BITHAM_30: return 34;
    default:             return 50;
  }
}

// -------------------------------------------------------------------------
// Alignment helper
// -------------------------------------------------------------------------
static GTextAlignment prv_galign(AlignOption a) {
  switch (a) {
    case ALIGN_CENTER: return GTextAlignmentCenter;
    case ALIGN_RIGHT:  return GTextAlignmentRight;
    default:           return GTextAlignmentLeft;
  }
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
// Date formatting
// -------------------------------------------------------------------------
static void prv_set_date(struct tm *tm) {
  static char date_str[40];
  static char day_str[16];

  memset(date_str, 0, sizeof(date_str));
  memset(day_str, 0, sizeof(day_str));

  int day = tm->tm_mday;
  const char *suffix;
  if (day == 1 || day == 21 || day == 31)    suffix = "st";
  else if (day == 2 || day == 22)             suffix = "nd";
  else if (day == 3 || day == 23)             suffix = "rd";
  else                                        suffix = "th";

  switch (s_settings.date_format) {
    case DATE_FORMAT_LONG_US: {
      char month_str[16], year_str[8];
      strftime(month_str, sizeof(month_str), "%B", tm);
      strftime(year_str, sizeof(year_str), "%Y", tm);
      snprintf(date_str, sizeof(date_str), "%s %d%s, %s", month_str, day, suffix, year_str);
      break;
    }
    case DATE_FORMAT_SHORT_US: {
      char month_str[8];
      strftime(month_str, sizeof(month_str), "%b", tm);
      snprintf(date_str, sizeof(date_str), "%s %d%s", month_str, day, suffix);
      break;
    }
    case DATE_FORMAT_LONG_EU: {
      char month_str[16], year_str[8];
      strftime(month_str, sizeof(month_str), "%B", tm);
      strftime(year_str, sizeof(year_str), "%Y", tm);
      snprintf(date_str, sizeof(date_str), "%d%s %s %s", day, suffix, month_str, year_str);
      break;
    }
    case DATE_FORMAT_SHORT_EU: {
      char month_str[8];
      strftime(month_str, sizeof(month_str), "%b", tm);
      snprintf(date_str, sizeof(date_str), "%d%s %s", day, suffix, month_str);
      break;
    }
    case DATE_FORMAT_NUM_MDY:
      strftime(date_str, sizeof(date_str), "%m/%d/%Y", tm);
      break;
    case DATE_FORMAT_NUM_DMY:
      strftime(date_str, sizeof(date_str), "%d/%m/%Y", tm);
      break;
    case DATE_FORMAT_NUM_YMD:
      strftime(date_str, sizeof(date_str), "%Y/%m/%d", tm);
      break;
  }

  // Day of week
  switch (s_settings.day_format) {
    case DAY_FORMAT_LONG:
      strftime(day_str, sizeof(day_str), "%A", tm);
      break;
    case DAY_FORMAT_SHORT:
      strftime(day_str, sizeof(day_str), "%a", tm);
      break;
    case DAY_FORMAT_NONE:
    default:
      day_str[0] = 0;
      break;
  }

  // Apply case
  apply_case(date_str, s_settings.date_case);
  apply_case(day_str, s_settings.day_case);

  text_layer_set_text(s_date_layer, date_str);
  text_layer_set_text(s_day_layer, day_str);

  // Show/hide day layer
  layer_set_hidden((Layer*)s_day_layer, s_settings.day_format == DAY_FORMAT_NONE);
}

// -------------------------------------------------------------------------
// Display time
// -------------------------------------------------------------------------
static void prv_display_time(struct tm *t) {
  char l1[BUFFER_SIZE], l2[BUFFER_SIZE], l3[BUFFER_SIZE];
  time_to_3words(t->tm_hour, t->tm_min, l1, l2, l3, BUFFER_SIZE, s_settings.prefix);

  apply_case(l1, s_settings.time_case);
  apply_case(l2, s_settings.time_case);
  apply_case(l3, s_settings.time_case);

  if (prv_needs_update(&s_line1, s_line1Str, l1)) prv_update_line(&s_line1, s_line1Str, l1);
  if (prv_needs_update(&s_line2, s_line2Str, l2)) prv_update_line(&s_line2, s_line2Str, l2);
  if (prv_needs_update(&s_line3, s_line3Str, l3)) prv_update_line(&s_line3, s_line3Str, l3);
}

static void prv_display_initial_time(struct tm *t) {
  time_to_3words(t->tm_hour, t->tm_min,
                 s_line1Str[0], s_line2Str[0], s_line3Str[0],
                 BUFFER_SIZE, s_settings.prefix);

  apply_case(s_line1Str[0], s_settings.time_case);
  apply_case(s_line2Str[0], s_settings.time_case);
  apply_case(s_line3Str[0], s_settings.time_case);

  text_layer_set_text(s_line1.currentLayer, s_line1Str[0]);
  text_layer_set_text(s_line2.currentLayer, s_line2Str[0]);
  text_layer_set_text(s_line3.currentLayer, s_line3Str[0]);
  prv_set_date(t);
}

// -------------------------------------------------------------------------
// Apply settings to layers
// -------------------------------------------------------------------------
static void prv_apply_settings(void) {
  window_set_background_color(s_window, s_settings.bg_color);

  GFont bold_font  = prv_get_font_bold();
  GFont light_font = prv_get_font_light();
  GTextAlignment time_align = prv_galign(s_settings.time_align);

  // Line 1 = hour (bold font, hour color)
  text_layer_set_font(s_line1.currentLayer, bold_font);
  text_layer_set_font(s_line1.nextLayer,    bold_font);
  text_layer_set_text_color(s_line1.currentLayer, s_settings.hour_color);
  text_layer_set_text_color(s_line1.nextLayer,    s_settings.hour_color);
  text_layer_set_background_color(s_line1.currentLayer, GColorClear);
  text_layer_set_background_color(s_line1.nextLayer,    GColorClear);
  text_layer_set_text_alignment(s_line1.currentLayer, time_align);
  text_layer_set_text_alignment(s_line1.nextLayer,    time_align);

  // Lines 2 & 3 = minutes (light font, minute color)
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

  // Date layers
  GTextAlignment date_align = prv_galign(s_settings.date_align);
  text_layer_set_text_color(s_date_layer, s_settings.date_color);
  text_layer_set_text_alignment(s_date_layer, date_align);
  text_layer_set_text_color(s_day_layer, s_settings.day_color);
  text_layer_set_text_alignment(s_day_layer, date_align);

  // Reposition time lines for current font
  int lh = prv_line_height();
  int top_margin = (s_screen_h <= 168) ? 6 : 16;
  int inset = PBL_IF_ROUND_ELSE(14, 0);
  int content_w = s_screen_w - (inset * 2);

  layer_set_frame((Layer*)s_line1.currentLayer, GRect(inset, top_margin,            content_w, lh + 8));
  layer_set_frame((Layer*)s_line1.nextLayer,    GRect(s_screen_w, top_margin,       content_w, lh + 8));
  layer_set_frame((Layer*)s_line2.currentLayer, GRect(inset, top_margin + lh,       content_w, lh + 8));
  layer_set_frame((Layer*)s_line2.nextLayer,    GRect(s_screen_w, top_margin + lh,  content_w, lh + 8));
  layer_set_frame((Layer*)s_line3.currentLayer, GRect(inset, top_margin + (lh * 2), content_w, lh + 8));
  layer_set_frame((Layer*)s_line3.nextLayer,    GRect(s_screen_w, top_margin+(lh*2),content_w, lh + 8));

  // Date area: bottom of screen
  // If day is hidden, give date a bit more room
  bool day_hidden = (s_settings.day_format == DAY_FORMAT_NONE);
  int date_top = day_hidden ? (s_screen_h - 20) : (s_screen_h - 32);
  int day_top  = s_screen_h - 48;
  layer_set_frame((Layer*)s_date_layer, GRect(inset, date_top, content_w, 20));
  layer_set_frame((Layer*)s_day_layer,  GRect(inset, day_top,  content_w, 20));
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

  t = dict_find(iter, MSG_KEY_DATE_COLOR);
  if (t) s_settings.date_color = (GColor){ .argb = (uint8_t)t->value->int32 };

  t = dict_find(iter, MSG_KEY_DAY_COLOR);
  if (t) s_settings.day_color = (GColor){ .argb = (uint8_t)t->value->int32 };

  t = dict_find(iter, MSG_KEY_TIME_ALIGN);
  if (t) s_settings.time_align = (AlignOption)t->value->int32;

  t = dict_find(iter, MSG_KEY_DATE_ALIGN);
  if (t) s_settings.date_align = (AlignOption)t->value->int32;

  t = dict_find(iter, MSG_KEY_PREFIX);
  if (t) s_settings.prefix = (MinutePrefix)t->value->int32;

  t = dict_find(iter, MSG_KEY_DATE_FORMAT);
  if (t) s_settings.date_format = (DateFormat)t->value->int32;

  t = dict_find(iter, MSG_KEY_DAY_FORMAT);
  if (t) s_settings.day_format = (DayFormat)t->value->int32;

  t = dict_find(iter, MSG_KEY_FONT_CHOICE);
  if (t) s_settings.font_choice = (FontChoice)t->value->int32;

  t = dict_find(iter, MSG_KEY_TIME_CASE);
  if (t) s_settings.time_case = (CaseOption)t->value->int32;

  t = dict_find(iter, MSG_KEY_DATE_CASE);
  if (t) s_settings.date_case = (CaseOption)t->value->int32;

  t = dict_find(iter, MSG_KEY_DAY_CASE);
  if (t) s_settings.day_case = (CaseOption)t->value->int32;

  prv_save_settings();
  prv_apply_settings();

  time_t now = time(NULL);
  struct tm *tm_now = localtime(&now);
  prv_display_initial_time(tm_now);
}

static void prv_inbox_dropped(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "AppMessage dropped: %d", (int)reason);
}

// -------------------------------------------------------------------------
// Tick handler
// -------------------------------------------------------------------------
static void prv_handle_tick(struct tm *tick_time, TimeUnits units_changed) {
  prv_display_time(tick_time);
  if (units_changed & DAY_UNIT) {
    prv_set_date(tick_time);
  }
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

  s_date_layer = text_layer_create(GRect(0, 0, s_screen_w, 20));
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));

  s_day_layer = text_layer_create(GRect(0, 0, s_screen_w, 20));
  text_layer_set_background_color(s_day_layer, GColorClear);
  text_layer_set_font(s_day_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));

  prv_apply_settings();

  layer_add_child(root, (Layer*)s_line1.currentLayer);
  layer_add_child(root, (Layer*)s_line1.nextLayer);
  layer_add_child(root, (Layer*)s_line2.currentLayer);
  layer_add_child(root, (Layer*)s_line2.nextLayer);
  layer_add_child(root, (Layer*)s_line3.currentLayer);
  layer_add_child(root, (Layer*)s_line3.nextLayer);
  layer_add_child(root, (Layer*)s_date_layer);
  layer_add_child(root, (Layer*)s_day_layer);

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
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_day_layer);
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

  app_message_open(512, 64);
  app_message_register_inbox_received(prv_inbox_received);
  app_message_register_inbox_dropped(prv_inbox_dropped);

  tick_timer_service_subscribe(MINUTE_UNIT, prv_handle_tick);

  app_event_loop();

  tick_timer_service_unsubscribe();
  window_destroy(s_window);

  return 0;
}
