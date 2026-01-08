#ifndef LEVEL_H
#define LEVEL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NOTES 10000

typedef struct {
  long timestamp_ms;
  int column;    // 0-3
  int processed; // For hit detection
} Note;

typedef struct {
  char audio_path[256];
  Note notes[MAX_NOTES];
  int note_count;
  int speed_ms;  // Window duration
  int offset_ms; // Global sync offset
} Level;

Level *load_level(const char *filename);
void free_level(Level *level);

#endif
