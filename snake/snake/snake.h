#ifndef SNAKE_H
#define SNAKE_H

struct point {
    int col, line;
};

void chase(struct point*, struct point*, struct point, struct point, struct point);

/**
 * Restore a point.
 * In case something was manually drawn at a given point (effect, UI element etc.),
 * the function erases it, revealing whatever is beneath.
 */
int chk(const struct point*, struct point, struct point);

/**
 * Draw the level/box/room/location.
 * Drawing it is more advanced than other stuff,
 * thus the dedicated function.
 */
void drawbox(struct point);

/**
 * Flush typeahead to keep from buffering a bunch of chars and then
 * overshooting.  This loses horribly at 9600 baud, but works nicely
 * if the terminal gets behind.
 */
void flushi(void);
void length(int);
void logit(const char*, struct point);

/**
 * Entry point for the application.
 */
int main(int, char**);
void mainloop(struct point, struct point, struct point*) __attribute__((__noreturn__));

/**
 * Shortcut to update a point with new coordinates.
 */
struct point* point(struct point*, int, int);

/**
 * Determine if previous highest score was bitten.
 * It makes sense to call the function at the end,
 * when current score is final.
 * If second argument (flag) is set, write more details.
 */
int post(int, int);
int pushsnake(struct point, struct point, struct point);

/**
 * Redraw (almost) everything.
 * It may come in handy when redrawing individual points isn't enough.
 */
void setup(struct point, struct point, struct point);

/**
 * Points in a direction a player might want to go.
 * Helps to orient a player relative to exit or a treasure.
 */
void snap(struct point, struct point, struct point);

/**
 * Move given point to random coordinates.
 * Pretty much all objects/creatures are placed randomly within a level.
 */
void snrand(struct point*, struct point, struct point, struct point);

/**
 * Move main character to a random position (with a penalty).
 * It's useful for escaping snakes clutches.
 */
void spacewarp(int, struct point, struct point, struct point);
void stop(int) __attribute__((__noreturn__));

/**
 * Helper for snap().
 */
int stretch(const struct point*, struct point, struct point, struct point);

/**
 * Play game over (loss) animation.
 * Makes things fancy.
 */
void surround(struct point*);
void suspend(void);

/**
 * Play game over (victory) animation.
 * Makes things fancy.
 */
void win(const struct point*);

/**
 * Score is rendered differently from other objects/creatures.
 * Thus the dedicated function.
 */
void winnings(int);

#endif // end of include guard