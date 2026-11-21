#include <stdio.h>
#include <stdarg.h>
#include <ncurses.h>

#include "msg.h"
#include "ui.h"

static char log[MSG_HIST][MSG_LEN];
static char sev[MSG_HIST];
static int head = 0;    /* next slot to overwrite */
static int count = 0;   /* total messages ever recorded (<= MSG_HIST) */

/* Color pair for a severity, or 0 for the terminal default. */
static int sev_pair(int s)
{
  if (s == MSG_GOOD) return CP_GREEN;
  if (s == MSG_BAD)  return CP_RED;
  return 0;
}

static void msg_at(int s, const char *fmt, va_list ap)
{
  vsnprintf(log[head], MSG_LEN, fmt, ap);
  sev[head] = (char) s;
  head = (head + 1) % MSG_HIST;
  if (count < MSG_HIST) count++;
}

void msg(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  msg_at(MSG_NORMAL, fmt, ap);
  va_end(ap);
}

void msg_good(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  msg_at(MSG_GOOD, fmt, ap);
  va_end(ap);
}

void msg_bad(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  msg_at(MSG_BAD, fmt, ap);
  va_end(ap);
}

/* Ring index of the message `back` from the newest. */
static int idx_back(int back)
{
  int idx = (head - 1 - back) % MSG_HIST;
  if (idx < 0) idx += MSG_HIST;
  return idx;
}

const char *msg_recent(int back)
{
  if (back < 0 || back >= count) return "";
  return log[idx_back(back)];
}

int msg_recent_sev(int back)
{
  if (back < 0 || back >= count) return MSG_NORMAL;
  return sev[idx_back(back)];
}

/* Ring index of the message of age `a` (0 = oldest, count-1 = newest). */
static int idx_age(int a)
{
  int idx = (head - count + a) % MSG_HIST;
  if (idx < 0) idx += MSG_HIST;
  return idx;
}

static const char *msg_age(int a)
{
  if (a < 0 || a >= count) return NULL;
  return log[idx_age(a)];
}

static int msg_age_sev(int a)
{
  if (a < 0 || a >= count) return MSG_NORMAL;
  return sev[idx_age(a)];
}

void msg_draw_line(int y, int x, const char *m, int s)
{
  int pair = sev_pair(s);
  move(y, x);
  clrtoeol();
  if (pair) attron(COLOR_PAIR(pair));
  mvprintw(y, x, "%s", m);
  if (pair) attroff(COLOR_PAIR(pair));
}

void msg_show_banner(int row, int lines)
{
  for (int i = 0; i < lines; i++) {
    int back = lines - 1 - i;   /* top row shows the oldest retained message */
    char line[MSG_LEN + 4];
    snprintf(line, sizeof line, "> %s", msg_recent(back));
    msg_draw_line(row + i, 0, line, msg_recent_sev(back));
  }
}

void msg_show_log(int row, int height)
{
  if (height < 2) {
    return;
  }
  mvaddch(row, 0, '+');
  mvhline(row, 1, '-', COLS - 2);
  mvaddch(row, COLS - 1, '+');
  msg_show_banner(row + 1, height - 1);
}

void msg_hist_view(int (*getkey)(void))
{
  if (count == 0) {
    mvprintw(LINES - 1, 0, "no messages yet  (press any key)");
    refresh();
    getkey();
    return;
  }

  int page = (LINES >= 4) ? LINES - 2 : 1;   /* viewport rows */
  int lo = (count < page) ? count : page;    /* smallest end offset */
  int end = count;                           /* show this many, from oldest */

  for (;;) {
    clear();
    int start = end - page;
    if (start < 0) start = 0;
    int shown = 0;
    for (int a = start; a < end; a++) {
      const char *m = msg_age(a);
      if (!m) break;
      char line[MSG_LEN + 4];
      snprintf(line, sizeof line, " %s", m);
      msg_draw_line(shown++, 0, line, msg_age_sev(a));
    }
    mvprintw(LINES - 1, 0,
             "message history  (%d-%d of %d)   up/down scroll, q done",
             start + 1, end, count);
    refresh();

    int k = getkey();
    if (k == 'q' || k == 'Q' || k == ' ' || k == '\n' || k == ERR) {
      break;
    }
    if ((k == KEY_UP || k == 'k') && end > lo) {
      end--;
    } else if ((k == KEY_DOWN || k == 'j') && end < count) {
      end++;
    }
  }
}