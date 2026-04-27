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

// Write a number 0-59 as words into buf.
static int number_to_words(char *buf, int num, int is_minutes, MinutePrefix prefix) {
  int tens_val = num / 10;
  int ones_val = num % 10;

  // Teens 10-19: single complete word
  if (tens_val == 1) {
    strcpy(buf, TEENS[ones_val]);
    return (int)strlen(buf);
  }

  int len = 0;

  // For minutes 1-9, optionally prepend "oh"
  if (is_minutes && tens_val == 0 && ones_val > 0) {
    if (prefix == PREFIX_OH) {
      strcpy(buf, "oh ");
      len = 3;
    }
  } else if (tens_val > 0) {
    strcpy(buf, TENS[tens_val]);
    len = (int)strlen(TENS[tens_val]);
    if (ones_val > 0) {
      buf[len++] = ' ';
    }
  }

  if (ones_val > 0 || num == 0) {
    strcpy(buf + len, ONES[ones_val]);
    len += (int)strlen(ONES[ones_val]);
  }

  return len;
}

void time_to_words(int hours, int minutes, char *words, size_t length, MinutePrefix prefix) {
  memset(words, 0, length);
  char tmp[32];

  if (hours == 0 || hours == 12) {
    strncat(words, "twelve", length - 1);
  } else {
    number_to_words(tmp, hours % 12, 0, prefix);
    strncat(words, tmp, length - 1);
  }
  strncat(words, " ", length - 1);

  number_to_words(tmp, minutes, 1, prefix);
  strncat(words, tmp, length - 1);
  strncat(words, " ", length - 1);
}

void time_to_3words(int hours, int minutes, char *line1, char *line2, char *line3, size_t length, MinutePrefix prefix) {
  memset(line1, 0, length);
  memset(line2, 0, length);
  memset(line3, 0, length);

  char value[64];
  time_to_words(hours, minutes, value, sizeof(value), prefix);

  // Tokenize into up to 3 lines
  char copy[64];
  strncpy(copy, value, sizeof(copy) - 1);

  char *token = strtok(copy, " ");
  int line = 1;
  while (token != NULL && line <= 3) {
    if (line == 1)      strncpy(line1, token, length - 1);
    else if (line == 2) strncpy(line2, token, length - 1);
    else if (line == 3) strncpy(line3, token, length - 1);
    line++;
    token = strtok(NULL, " ");
  }

  // Apply "o'" prefix to line2 for minutes 1-9
  if (prefix == PREFIX_O && minutes > 0 && minutes < 10) {
    char tmp[32];
    snprintf(tmp, sizeof(tmp), "o' %s", line2);
    strncpy(line2, tmp, length - 1);
  }
}
