#define _DEFAULT_SOURCE
#include <dirent.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_GAMES 20
#define MAX_NAME_LEN 50

typedef struct {
  char name[MAX_NAME_LEN];
  char path[256];
} Game;

Game games[MAX_GAMES];
int game_count = 0;

void find_games() {
  DIR *d;
  struct dirent *dir;
  struct stat st;

  d = opendir(".");
  if (d) {
    while ((dir = readdir(d)) != NULL) {
      if (dir->d_type == DT_DIR && strcmp(dir->d_name, ".") != 0 &&
          strcmp(dir->d_name, "..") != 0 && dir->d_name[0] != '.') {
        char executable_path[256];
        snprintf(executable_path, sizeof(executable_path), "./%s/%s",
                 dir->d_name, dir->d_name);

        // Check if the executable exists and is executable
        if (stat(executable_path, &st) == 0 && (st.st_mode & S_IXUSR)) {
          if (game_count < MAX_GAMES) {
            strncpy(games[game_count].name, dir->d_name, MAX_NAME_LEN - 1);
            games[game_count].name[MAX_NAME_LEN - 1] = '\0';
            strncpy(games[game_count].path, executable_path, 255);
            games[game_count].path[255] = '\0';
            game_count++;
          }
        }
      }
    }
    closedir(d);
  }
}

void draw_menu(int highlight) {
  int x, y, i;

  x = 2;
  y = 2;
  box(stdscr, 0, 0);

  attron(A_BOLD);
  mvprintw(0, 2, " CLIG Launcher ");
  attroff(A_BOLD);

  mvprintw(y++, x, "Select a game to play:");
  y++;

  for (i = 0; i < game_count; i++) {
    if (highlight == i) {
      attron(A_REVERSE);
      mvprintw(y, x, "%s", games[i].name);
      attroff(A_REVERSE);
    } else {
      mvprintw(y, x, "%s", games[i].name);
    }
    y++;
  }

  if (highlight == game_count) {
    attron(A_REVERSE);
    mvprintw(y + 1, x, "Quit");
    attroff(A_REVERSE);
  } else {
    mvprintw(y + 1, x, "Quit");
  }

  refresh();
}

int main() {
  int highlight = 0;
  int choice = -1;
  int c;

  initscr();
  clear();
  noecho();
  cbreak();
  curs_set(0);
  keypad(stdscr, TRUE);

  find_games();

  if (game_count == 0) {
    endwin();
    printf("No games found.\n");
    return 1;
  }

  while (1) {
    draw_menu(highlight);
    c = getch();

    switch (c) {
    case 'w':
    case 'k':
    case KEY_UP:
      if (highlight > 0)
        highlight--;
      else
        highlight = game_count;
      break;
    case 's':
    case 'j':
    case KEY_DOWN:
      if (highlight < game_count)
        highlight++;
      else
        highlight = 0;
      break;
    case 'q':
    case 'Q':
      choice = game_count;
      break;
    case 10: // Enter
      choice = highlight;
      break;
    }

    if (choice != -1) {
      if (choice == game_count)
        break;

      def_prog_mode(); // Save curses terminal state
      endwin();        // Temporarily leave curses mode

      system(games[choice].path);

      reset_prog_mode(); // Restore curses terminal state
      refresh();         // Redraw the screen

      choice = -1; // Reset choice
    }
  }

  endwin();
  return 0;
}
