#include "level.h"

Level *load_level(const char *filename) {
  FILE *f = fopen(filename, "r");
  if (!f)
    return NULL;

  Level *level = (Level *)malloc(sizeof(Level));
  level->note_count = 0;
  level->speed_ms = 2000;
  level->offset_ms = 0;
  memset(level->audio_path, 0, sizeof(level->audio_path));

  char line[256];
  while (fgets(line, sizeof(line), f)) {
    if (line[0] == '#' || line[0] == '\n')
      continue;

    if (strncmp(line, "audio,", 6) == 0) {
      sscanf(line, "audio,%255s", level->audio_path);
    } else if (strncmp(line, "speed,", 6) == 0) {
      sscanf(line, "speed,%d", &level->speed_ms);
    } else if (strncmp(line, "offset,", 7) == 0) {
      sscanf(line, "offset,%d", &level->offset_ms);
    } else {
      long ts;
      int col;
      if (sscanf(line, "%ld,%d", &ts, &col) == 2) {
        if (level->note_count < MAX_NOTES) {
          level->notes[level->note_count].timestamp_ms = ts;
          level->notes[level->note_count].column = col;
          level->notes[level->note_count].processed = 0;
          level->note_count++;
        }
      }
    }
  }

  fclose(f);
  return level;
}

void free_level(Level *level) {
  if (level)
    free(level);
}
