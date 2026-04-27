#pragma once
#include <string.h>

typedef enum {
  PREFIX_NONE = 0,
  PREFIX_OH   = 1,
  PREFIX_O    = 2,
} MinutePrefix;

// split_long_words: if true, splits teen words across line2/line3
// (e.g. "eighteen" -> line2="eight" line3="teen") for narrow screens
void time_to_words(int hours, int minutes, char* words, size_t length, MinutePrefix prefix);
void time_to_3words(int hours, int minutes, char *line1, char *line2, char *line3,
                    size_t length, MinutePrefix prefix, bool split_long_words);
