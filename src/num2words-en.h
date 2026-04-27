#pragma once
#include <string.h>

// Minute prefix style options
typedef enum {
  PREFIX_NONE = 0,  // "one five"
  PREFIX_OH   = 1,  // "oh one"
  PREFIX_O    = 2,  // "o' one"
} MinutePrefix;

void time_to_words(int hours, int minutes, char* words, size_t length, MinutePrefix prefix);
void time_to_3words(int hours, int minutes, char *line1, char *line2, char *line3, size_t length, MinutePrefix prefix);
