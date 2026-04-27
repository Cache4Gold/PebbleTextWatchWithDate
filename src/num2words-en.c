#include "num2words-en.h"
#include <string.h>

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
  "",
  "eleven",
  "twelve",
  "thirteen",
  "fourteen",
  "fifteen",
  "sixteen",
  "seventeen",
  "eighteen",   // fixed typo from original ("eightteen")
  "nineteen"
};

static const char* const TENS[] = {
  "",           // placeholder for 0 (handled by prefix logic at runtime)
  "ten",
  "twenty",
  "thirty",
  "forty",
  "fifty",
  "sixty",
  "seventy",
  "eighty",
  "ninety"
};

static size_t append_string(char* buffer, const size_t length, const char* str) {
  strncat(buffer, str, length);
  size_t written = strlen(str);
  return (length > written) ? written : length;
}

// Appends a number (0-59) as words into `words`.
// isOh: whether this is the minutes field (affects "oh" prefix logic)
static size_t append_number(char* words, int num, int isMinutes, MinutePrefix prefix) {
  int tens_val = num / 10 % 10;
  int ones_val = num % 10;
  size_t len = 0;

  // Teens: 11-19 (but not 10, handled by tens/ones logic)
  if (tens_val == 1 && num != 10) {
    strcat(words, TEENS[ones_val]);
    return strlen(TEENS[ones_val]);
  }

  // For the tens place of minutes (e.g. 01-09), apply prefix if set
  if (isMinutes && tens_val == 0 && ones_val > 0) {
    if (prefix == PREFIX_OH) {
      strcat(words, "oh");
      len += 2;
      strcat(words, " ");
      len += 1;
    }
    // PREFIX_O is handled after line splitting in time_to_3words
    // PREFIX_NONE: no prefix, just fall through to ones
  } else if (tens_val != 0) {
    strcat(words, TENS[tens_val]);
    len += strlen(TENS[tens_val]);
    if (ones_val > 0) {
      strcat(words, " ");
      len += 1;
    }
  }

  if (ones_val > 0 || num == 0) {
    strcat(words, ONES[ones_val]);
    len += strlen(ONES[ones_val]);
  }

  return len;
}

void time_to_words(int hours, int minutes, char* words, size_t length, MinutePrefix prefix) {
  size_t remaining = length;
  memset(words, 0, length);

  // Hours
  if (hours == 0 || hours == 12) {
    remaining -= append_string(words, remaining, TEENS[2]); // "twelve"
  } else {
    remaining -= append_number(words, hours % 12, 0, prefix);
  }

  remaining -= append_string(words, remaining, " ");

  // Minutes
  remaining -= append_number(words, minutes, 1, prefix);

  remaining -= append_string(words, remaining, " ");
  (void)remaining;
}

void time_to_3words(int hours, int minutes, char *line1, char *line2, char *line3, size_t length, MinutePrefix prefix) {
  char value[64];
  time_to_words(hours, minutes, value, sizeof(value), prefix);

  memset(line1, 0, length);
  memset(line2, 0, length);
  memset(line3, 0, length);

  // Split the space-delimited string into up to 3 lines
  char *start = value;
  char *pch = strstr(start, " ");
  while (pch != NULL) {
    if (line1[0] == 0) {
      memcpy(line1, start, pch - start);
    } else if (line2[0] == 0) {
      memcpy(line2, start, pch - start);
    } else if (line3[0] == 0) {
      memcpy(line3, start, pch - start);
    }
    start += pch - start + 1;
    pch = strstr(start, " ");
  }

  // Truncate long teen values across lines (e.g. "seven" + "teen"), except "thirteen"
  if (strlen(line2) > 7 && minutes != 13) {
    char *teen = strstr(line2, "teen");
    if (teen) {
      memcpy(line3, teen, 4);
      teen[0] = 0;
    }
  }

  // Apply "o'" prefix at display time (goes on line2 since that's where minutes land)
  if (prefix == PREFIX_O && minutes > 0 && minutes < 10) {
    char new_line2[32] = "o' ";
    strncat(new_line2, line2, sizeof(new_line2) - 4);
    memcpy(line2, new_line2, strlen(new_line2) + 1);
  }
}
