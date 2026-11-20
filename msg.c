#include <stdio.h>
#include <stdarg.h>
#include <ncurses.h>

#include "msg.h"

static char log[MSG_HIST][MSG_LEN];
static int head = 0;    /* next slot to overwrite */
static int count = 0;   /* total messages ever recorded (<= MSG_HIST) */

void msg(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(log[head], MSG_LEN, fmt, ap);
  va_end(ap);

  head = (head + 1) % MSG_HIST;
  if (count < MSG_HIST) count++;
}

/* Message `back` from the newest (0 = newest). */
const char *msg_recent(int back)
{
  if (back < 0 || back >= count) return "";
  int idx = (head - 1 - back) % MSG_HIST;
  if (idx < 0) idx += MSG_HIST;
  return log[idx];
}

/* Message of age `a` (0 = oldest, count-1 = newest). */
static const char *msg_age(int a)
{
  if (a < 0 || a >= count) return NULL;
  int idx = (head - count + a) % MSG_HIST;
  if (idx < 0) idx += MSG_HIST;
  return log[idx];
}

void msg_show_banner(int row, int lines)
{
  int i;
  for (i = 0; i < lines; i++) {
    move(row + i, 0);
    clrtoeol();
    /* top row of the window shows the oldest of the retained messages */
    mvprintw(row + i, 0, "> %s", msg_recent(lines - 1 - i));
  }
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
      mvprintw(shown++, 0, " %s", m);
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