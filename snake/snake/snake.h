#ifndef SNAKE_H
#define SNAKE_H

struct point {
    int col, line;
};

void chase(struct point*, struct point*);
int chk(const struct point*);
void drawbox(void);
void flushi(void);
void home(void);
void length(int);
void logit(const char*);
int main(int, char**);
void mainloop(void) __attribute__((__noreturn__));
struct point* point(struct point*, int, int);
int post(int, int);
int pushsnake(void);
void right(const struct point*);
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