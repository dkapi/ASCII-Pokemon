#ifndef MSG_H
#define MSG_H

/* Classic roguelike message log: a ring buffer of recent events, a banner of
 * the last few messages, and a scrollable history viewer. */

#define MSG_LEN 96      /* max chars per message */
#define MSG_HIST 200    /* ring buffer depth */

/* printf-style: append a message to the log. */
void msg(const char *fmt, ...);

/* Return the message `back` entries from the newest (0 = newest, 1 = one
 * before, ...), or "" when there is no such entry. */
const char *msg_recent(int back);

/* Draw the last `lines` messages starting at screen row `row`. */
void msg_show_banner(int row, int lines);

/* Draw a full-width log pane: a divider line at `row`, then the last
 * `height - 1` messages.  No-op when `height` is too small. */
void msg_show_log(int row, int height);

/* Full-screen, scrollable history.  Consumes keys via `getkey` until the
 * player dismisses it (q / space / enter / EOF). */
void msg_hist_view(int (*getkey)(void));

#endif /* MSG_H */