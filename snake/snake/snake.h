#ifndef SNAKE_H
#define SNAKE_H

struct point {
    int col, line;
};

void chase(struct point*, struct point*);

/**
 * Restore a point.
 * In case something was manually drawn at a given point (effect, UI element etc.),
 * the function erases it, revealing whatever is beneath.
 */
int chk(const struct point*);

/**
 * Draw the level/box/room/location.
 * Drawing it is more advanced than other stuff,
 * thus the dedicated function.
 */
void drawbox(void);

/**
 * Flush typeahead to keep from buffering a bunch of chars and then
 * overshooting.  This loses horribly at 9600 baud, but works nicely
 * if the terminal gets behind.
 */
void flushi(void);
void length(int);
void logit(const char*);

/**
 * Entry point for the application.
 */
int main(int, char**);
void mainloop(void) __attribute__((__noreturn__));

/**
 * Shortcut to update a point with new coordinates.
 */
struct point* point(struct point*, int, int);
int post(int, int);
int pushsnake(void);
void right(const struct point*);

/**
 * Redraw (almost) everything.
 * May come in handy when redrawing individual points isn't enough.
 */
void setup(void);
void snap(void);
void snrand(struct point*);
void spacewarp(int);
void stop(int) __attribute__((__noreturn__));
int stretch(const struct point*);
void surround(struct point*);
void suspend(void);
void win(const struct point*);
void winnings(int);

#endif // end of include guard