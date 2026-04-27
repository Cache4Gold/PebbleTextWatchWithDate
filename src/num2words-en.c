#include "num2words-en.h"
#include <string.h>
#include <stdio.h>

static const char* const ONES[] = {
  "o'clock",
  "one",
  "two",
  "three",
  "four",
  "five",
  "six",
  "seven",
  "eight",
  "nine"
};

static const char* const TEENS[] = {
  "ten",
  "eleven",
  "twelve",
  "thirteen",
  "fourteen",
  "fifteen",
  "sixteen",
  "seventeen",
  "eighteen",
  "nineteen"
};

static const char* const TENS[] = {
  "",
  "ten",
  "twenty",
  "thirty",
  "forty",
  "fifty"
};

// Write hour (1-12) as a word into line1.
static void hour_to_line(int hours, char *line1, size_t length) {
  int h = hours % 12;
  if (h == 0) {
    strncpy(line1, "twelve", length - 1);
  } else if (h >= 10) {
    // 10-12 are teens
    strncpy(line1, TEENS[h - 10], length - 1);
  } else {
    strncpy(line1, ONES[h], length - 1);
  }
}

// Write minutes (0-59) into line2 and optionally line3.
// Returns number of lines used (1 or 2).
static int minutes_to_lines(int minutes, char *line2, char *line3,
                             size_t length, MinutePrefix prefix) {
  memset(line2, 0, length);
  memset(line3, 0, length);

  if (minutes == 0) {
    strncpy(line2, "o'clock", length - 1);
    return 1;
  }

  int tens_val = minutes / 10;
  int ones_val = minutes % 10;

  if (tens_val == 1) {
    // 10-19: single teen word goes on line2
    strncpy(line2, TEENS[ones_val], length - 1);
    return 1;
  }

  if (tens_val == 0) {
    // 1-9: apply prefix on line2, word on line2
    if (prefix == PREFIX_OH) {
      strncpy(line2, "oh", length - 1);
      strncpy(line3, ONES[ones_val], length - 1);
      return 2;
    } else if (prefix == PREFIX_O) {
      // "o' five" fits on one line
      snprintf(line2, length, "o' %s", ONES[ones_val]);
      return 1;
    } else {
      strncpy(line2, ONES[ones_val], length - 1);
      return 1;
    }
  }

  // 20-59: tens word on line2, ones word on line3 (if any)
  strncpy(line2, TENS[tens_val], length - 1);
  if (ones_val > 0) {
    strncpy(line3, ONES[ones_val], length - 1);
    return 2;
  }
  return 1;
}

void time_to_words(int hours, int minutes, char *words, size_t length, MinutePrefix prefix) {
  // Build a flat string representation for compatibility
  char l1[32] = {0}, l2[32] = {0}, l3[32] = {0};
  hour_to_line(hours, l1, sizeof(l1));
  minutes_to_lines(minutes, l2, l3, sizeof(l2), prefix);
  memset(words, 0, length);
  strncat(words, l1, length - 1);
  if (l2[0]) { strncat(words, " ", length - 1); strncat(words, l2, length - 1); }
  if (l3[0]) { strncat(words, " ", length - 1); strncat(words, l3, length - 1); }
}

void time_to_3words(int hours, int minutes, char *line1, char *line2, char *line3,
                    size_t length, MinutePrefix prefix) {
  memset(line1, 0, length);
  memset(line2, 0, length);
  memset(line3, 0, length);

  hour_to_line(hours, line1, length);
  minutes_to_lines(minutes, line2, line3, length, prefix);
}
