#include "num2words-en.h"
#include <string.h>
#include <stdio.h>

static const char* const ONES[] = {
  "o'clock","one","two","three","four","five","six","seven","eight","nine"
};

static const char* const TEENS[] = {
  "ten","eleven","twelve","thirteen","fourteen",
  "fifteen","sixteen","seventeen","eighteen","nineteen"
};

static const char* const TENS[] = {
  "","ten","twenty","thirty","forty","fifty"
};

// Word roots for splitting: "four" + "teen", "eigh" + "teen", etc.
// For narrow screens we split teen words as root + "teen" across two lines
static const char* const TEEN_ROOTS[] = {
  "ten","elev","twelve","thir","four","fif","six","seven","eigh","nine"
};

static void hour_to_line(int hours, char *line1, size_t length) {
  int h = hours % 12;
  if (h == 0) {
    strncpy(line1, "twelve", length - 1);
  } else if (h >= 10) {
    strncpy(line1, TEENS[h - 10], length - 1);
  } else {
    strncpy(line1, ONES[h], length - 1);
  }
}

// Returns lines used (1 or 2)
static int minutes_to_lines(int minutes, char *line2, char *line3,
                             size_t length, MinutePrefix prefix,
                             bool split_long_words) {
  memset(line2, 0, length);
  memset(line3, 0, length);

  if (minutes == 0) {
    strncpy(line2, "o'clock", length - 1);
    return 1;
  }

  int tens_val = minutes / 10;
  int ones_val = minutes % 10;

  // Teens 10-19
  if (tens_val == 1) {
    if (split_long_words && ones_val != 0 && ones_val != 2) {
      // Split: root on line2, "teen" on line3
      // Exceptions: "ten" (no teen suffix), "twelve" (irregular)
      // "eleven" -> "elev" + "en" doesn't work cleanly, keep as-is
      // Split candidates: thirteen, fourteen, fifteen, sixteen,
      //                   seventeen, eighteen, nineteen
      if (ones_val >= 3) {
        snprintf(line2, length, "%steen", TEEN_ROOTS[ones_val]);
        // Check if it's long enough to warrant splitting (>7 chars)
        if (strlen(line2) > 7) {
          strncpy(line2, TEEN_ROOTS[ones_val], length - 1);
          strncpy(line3, "teen", length - 1);
          return 2;
        }
      }
    }
    strncpy(line2, TEENS[ones_val], length - 1);
    return 1;
  }

  // 1-9 with prefix
  if (tens_val == 0) {
    if (prefix == PREFIX_OH) {
      strncpy(line2, "oh", length - 1);
      strncpy(line3, ONES[ones_val], length - 1);
      return 2;
    } else if (prefix == PREFIX_O) {
      snprintf(line2, length, "o' %s", ONES[ones_val]);
      return 1;
    } else {
      strncpy(line2, ONES[ones_val], length - 1);
      return 1;
    }
  }

  // 20-59: tens on line2, ones on line3
  strncpy(line2, TENS[tens_val], length - 1);
  if (ones_val > 0) {
    strncpy(line3, ONES[ones_val], length - 1);
    return 2;
  }
  return 1;
}

void time_to_words(int hours, int minutes, char *words, size_t length, MinutePrefix prefix) {
  memset(words, 0, length);
  char tmp[32];
  char l2[32] = {0}, l3[32] = {0};

  hour_to_line(hours, tmp, sizeof(tmp));
  strncat(words, tmp, length - 1);
  strncat(words, " ", length - 1);

  minutes_to_lines(minutes, l2, l3, sizeof(l2), prefix, false);
  strncat(words, l2, length - 1);
  if (l3[0]) { strncat(words, " ", length - 1); strncat(words, l3, length - 1); }
}

void time_to_3words(int hours, int minutes, char *line1, char *line2, char *line3,
                    size_t length, MinutePrefix prefix, bool split_long_words) {
  memset(line1, 0, length);
  memset(line2, 0, length);
  memset(line3, 0, length);

  hour_to_line(hours, line1, length);
  minutes_to_lines(minutes, line2, line3, length, prefix, split_long_words);
}
