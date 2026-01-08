#define _DEFAULT_SOURCE
#include "level.h"
#include <dirent.h>
#include <ncurses.h>
#include <time.h>
#include <unistd.h>
#include <vlc/vlc.h>

#define LANE_WIDTH 5
#define HIT_LINE 22
#define MAX_HEALTH 100
#define LIGHT_DURATION 10 // frames (at ~100fps)

typedef struct {
  int score;
  int combo;
  int max_combo;
  int health;
  int perfects, greats, goods, misses;
  char last_rating[16];
  int lane_light[4];
  int lane_burst[4];
} GameState;

void init_colors() {
  if (has_colors()) {
    start_color();
    init_pair(1, COLOR_CYAN, COLOR_BLACK);   // Perfect
    init_pair(2, COLOR_GREEN, COLOR_BLACK);  // Great
    init_pair(3, COLOR_YELLOW, COLOR_BLACK); // Good
    init_pair(4, COLOR_RED, COLOR_BLACK);    // Miss
    init_pair(5, COLOR_WHITE, COLOR_RED);    // Health bar bg
    init_pair(6, COLOR_BLACK, COLOR_GREEN);  // Health bar fill
    init_pair(7, COLOR_BLACK, COLOR_CYAN);   // Lane light
  }
}

void draw_ui(GameState *state, Level *level, long current_time) {
  int term_h, term_w;
  getmaxyx(stdscr, term_h, term_w);

  int lane_start = (term_w - (LANE_WIDTH * 4 + 5)) / 2;

  // Draw lanes and lights
  for (int i = 0; i < 4; i++) {
    int x = lane_start + i * (LANE_WIDTH + 1);
    if (state->lane_light[i] > 0)
      attron(COLOR_PAIR(7));
    for (int y = 0; y < term_h; y++) {
      mvaddch(y, x, '|');
    }
    if (state->lane_light[i] > 0) {
      for (int y = 0; y < term_h; y++) {
        for (int k = 1; k <= LANE_WIDTH; k++)
          mvaddch(y, x + k, ' ');
      }
      attroff(COLOR_PAIR(7));
      state->lane_light[i]--;
    }

    // Draw hit key labels
    char labels[] = "DFJK";
    mvaddch(HIT_LINE + 1, x + LANE_WIDTH / 2 + 1, labels[i]);
  }
  mvaddch(0, lane_start + 4 * (LANE_WIDTH + 1), '|'); // Fix right border

  // Accuracy Bursts
  for (int i = 0; i < 4; i++) {
    if (state->lane_burst[i] > 0) {
      int x = lane_start + i * (LANE_WIDTH + 1) + 1;
      attron(A_BOLD);
      for (int k = 0; k < LANE_WIDTH; k++)
        mvaddch(HIT_LINE, x + k, ACS_BLOCK);
      state->lane_burst[i]--;
      attroff(A_BOLD);
    }
  }

  // Draw notes
  for (int i = 0; i < level->note_count; i++) {
    Note *n = &level->notes[i];
    if (n->processed)
      continue;

    long time_diff = n->timestamp_ms - current_time;
    if (time_diff > level->speed_ms || time_diff < -200)
      continue;

    int y = HIT_LINE - (int)((float)time_diff / level->speed_ms * HIT_LINE);

    if (y >= 0 && y <= HIT_LINE) {
      int x = lane_start + n->column * (LANE_WIDTH + 1) + 1;
      for (int k = 0; k < LANE_WIDTH; k++) {
        mvaddch(y, x + k, ACS_BLOCK);
      }
    }
  }

  // Hit line
  for (int i = 0; i <= 4 * (LANE_WIDTH + 1); i++) {
    mvaddch(HIT_LINE, lane_start + i, '-');
  }

  // Health Bar
  int bar_h = 20;
  int bar_x = lane_start - 4;
  int bar_y = HIT_LINE - bar_h;
  attron(COLOR_PAIR(5));
  for (int i = 0; i < bar_h; i++)
    mvaddch(bar_y + i, bar_x, ' ');
  attroff(COLOR_PAIR(5));
  int fill = (state->health * bar_h) / MAX_HEALTH;
  attron(COLOR_PAIR(6));
  for (int i = 0; i < fill; i++)
    mvaddch(bar_y + bar_h - 1 - i, bar_x, ' ');
  attroff(COLOR_PAIR(6));
  mvprintw(bar_y - 1, bar_x - 1, "HP");

  // Info
  mvprintw(2, 2, "Score: %06d", state->score);
  mvprintw(3, 2, "Combo: %d  (Max: %d)", state->combo, state->max_combo);

  int r_pair = 0;
  if (strcmp(state->last_rating, "PERFECT") == 0)
    r_pair = 1;
  else if (strcmp(state->last_rating, "GREAT") == 0)
    r_pair = 2;
  else if (strcmp(state->last_rating, "GOOD") == 0)
    r_pair = 3;
  else if (strcmp(state->last_rating, "MISS") == 0)
    r_pair = 4;

  if (r_pair)
    attron(COLOR_PAIR(r_pair) | A_BOLD);
  mvprintw(HIT_LINE / 2, (term_w - strlen(state->last_rating)) / 2, "%s",
           state->last_rating);
  if (r_pair)
    attroff(COLOR_PAIR(r_pair) | A_BOLD);
}

void show_results(GameState *state) {
  clear();
  int h, w;
  getmaxyx(stdscr, h, w);
  int mid = w / 2;
  int cur_y = h / 4;

  attron(A_BOLD | A_UNDERLINE);
  mvprintw(cur_y++, mid - 7, "RESULTS SCREEN");
  attroff(A_BOLD | A_UNDERLINE);
  cur_y++;

  mvprintw(cur_y++, mid - 10, "Final Score: %d", state->score);
  mvprintw(cur_y++, mid - 10, "Max Combo:   %d", state->max_combo);
  cur_y++;

  attron(COLOR_PAIR(1));
  mvprintw(cur_y++, mid - 10, "PERFECT: %d", state->perfects);
  attroff(COLOR_PAIR(1));
  attron(COLOR_PAIR(2));
  mvprintw(cur_y++, mid - 10, "GREAT:   %d", state->greats);
  attroff(COLOR_PAIR(2));
  attron(COLOR_PAIR(3));
  mvprintw(cur_y++, mid - 10, "GOOD:    %d", state->goods);
  attroff(COLOR_PAIR(3));
  attron(COLOR_PAIR(4));
  mvprintw(cur_y++, mid - 10, "MISS:    %d", state->misses);
  attroff(COLOR_PAIR(4));

  cur_y += 2;
  mvprintw(cur_y, mid - 15, "Press any key to return to menu");
  refresh();
  nodelay(stdscr, FALSE);
  getch();
  nodelay(stdscr, TRUE);
}

char *select_level() {
  DIR *d;
  struct dirent *dir;
  char *levels[20];
  int count = 0;

  d = opendir(".");
  if (d) {
    while ((dir = readdir(d)) != NULL) {
      if (strstr(dir->d_name, ".lvl")) {
        levels[count] = strdup(dir->d_name);
        count++;
        if (count >= 20)
          break;
      }
    }
    closedir(d);
  }

  if (count == 0)
    return NULL;

  int highlight = 0;
  while (1) {
    clear();
    mvprintw(2, 2, "Select a Mania Level:");
    for (int i = 0; i < count; i++) {
      if (i == highlight)
        attron(A_REVERSE);
      mvprintw(4 + i, 4, "%s", levels[i]);
      if (i == highlight)
        attroff(A_REVERSE);
    }
    int ch = getch();
    if (ch == KEY_UP || ch == 'w')
      highlight = (highlight > 0) ? highlight - 1 : count - 1;
    if (ch == KEY_DOWN || ch == 's')
      highlight = (highlight < count - 1) ? highlight + 1 : 0;
    if (ch == 10) { // Enter
      char *res = strdup(levels[highlight]);
      for (int i = 0; i < count; i++)
        free(levels[i]);
      return res;
    }
    if (ch == 'q')
      return NULL;
    usleep(10000);
  }
}

int main() {
  // Init Ncurses
  initscr();
  noecho();
  curs_set(FALSE);
  keypad(stdscr, TRUE);
  nodelay(stdscr, TRUE);
  init_colors();

  while (1) {
    char *lvl_path = select_level();
    if (!lvl_path)
      break;

    Level *level = load_level(lvl_path);
    free(lvl_path);
    if (!level)
      continue;

    libvlc_instance_t *inst = libvlc_new(0, NULL);
    libvlc_media_t *m = libvlc_media_new_path(inst, level->audio_path);
    libvlc_media_player_t *mp = libvlc_media_player_new_from_media(m);
    libvlc_media_release(m);

    GameState state = {0, 0, 0, MAX_HEALTH, 0, 0, 0, 0, "", {0}, {0}};
    libvlc_media_player_play(mp);

    while (1) {
      long current_time = libvlc_media_player_get_time(mp) - level->offset_ms;
      if (current_time < 0 && libvlc_media_player_is_playing(mp))
        current_time = 0;

      int ch = getch();
      if (ch == 'q')
        break;

      int hit_col = -1;
      if (ch == 'd')
        hit_col = 0;
      else if (ch == 'f')
        hit_col = 1;
      else if (ch == 'j')
        hit_col = 2;
      else if (ch == 'k')
        hit_col = 3;

      if (hit_col != -1) {
        state.lane_light[hit_col] = LIGHT_DURATION;
        int found = 0;
        for (int i = 0; i < level->note_count; i++) {
          Note *n = &level->notes[i];
          if (n->processed || n->column != hit_col)
            continue;
          long diff = labs(n->timestamp_ms - current_time);
          if (diff < 150) {
            n->processed = 1;
            state.lane_burst[hit_col] = 3;
            if (diff < 50) {
              state.score += 300;
              state.perfects++;
              state.health += 5;
              strcpy(state.last_rating, "PERFECT");
            } else if (diff < 100) {
              state.score += 100;
              state.greats++;
              state.health += 2;
              strcpy(state.last_rating, "GREAT");
            } else {
              state.score += 50;
              state.goods++;
              state.health += 1;
              strcpy(state.last_rating, "GOOD");
            }
            state.combo++;
            found = 1;
            break;
          }
        }
        if (!found) {
          state.combo = 0;
          state.health -= 5;
          strcpy(state.last_rating, "MISS");
          state.misses++;
        }
      }

      for (int i = 0; i < level->note_count; i++) {
        Note *n = &level->notes[i];
        if (!n->processed && current_time - n->timestamp_ms > 150) {
          n->processed = 1;
          state.combo = 0;
          state.health -= 10;
          state.misses++;
          strcpy(state.last_rating, "MISS");
        }
      }

      if (state.combo > state.max_combo)
        state.max_combo = state.combo;
      if (state.health > MAX_HEALTH)
        state.health = MAX_HEALTH;
      if (state.health <= 0) {
        strcpy(state.last_rating, "FAILED");
        draw_ui(&state, level, current_time);
        refresh();
        usleep(1000000);
        break;
      }

      clear();
      draw_ui(&state, level, current_time);
      refresh();
      if (!libvlc_media_player_is_playing(mp) && current_time > 1000)
        break;
      usleep(5000);
    }

    libvlc_media_player_stop(mp);
    libvlc_media_player_release(mp);
    libvlc_release(inst);
    if (state.health > 0)
      show_results(&state);
    free_level(level);
  }

  endwin();
  return 0;
}
