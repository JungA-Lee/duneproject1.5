#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdio.h>
#include <stdbool.h>
#include <Windows.h>
#include <conio.h>
#include <assert.h>

#define TICK 10
#define MAX_UNITS 10
#define N_LAYER 3
#define MAP_WIDTH 60
#define MAP_HEIGHT 18
#define MAX_HEALTH 100

typedef struct {
    int row, column;
} POSITION;

typedef struct {
    POSITION previous;
    POSITION current;
    int size;
} CURSOR;

typedef struct {
    int spice;
    int spice_max;
    int population;
    int population_max;
} RESOURCE;

typedef struct {
    POSITION pos;
    POSITION dest;
    POSITION target;
    char type;
    int health;
    int attack;
    int range;
    int move_speed;
    int attack_speed;
    int next_action_time;
    int carry_spice;
    bool is_patrolling;
    POSITION patrol_point;
    bool in_action;
} UNIT;

typedef struct {
    bool in_progress;
    int start_time;
    int production_time;
    char unit_type;
} PRODUCTION;

typedef struct {
    POSITION pos;
    int length;
    int move_period;
    int next_move_time;
} SANDWORM;

typedef struct {
    POSITION pos;
    int move_period;
    int next_move_time;
} EAGLE;

typedef struct {
    POSITION pos[4];
    int move_period;
    int next_move_time;
    int lifespan;
} SANDSTORM;

typedef enum {
    d_stay = 0, d_up, d_right, d_left, d_down
} DIRECTION;

typedef enum {
    k_none = 0, k_up, k_right, k_left, k_down,
    k_quit, k_space, k_escape, k_undef
} KEY;

#define is_arrow_key(k) (k_up <= (k) && (k) <= k_down)
#define ktod(k) (DIRECTION)(k)

extern UNIT units[MAX_UNITS];
extern int unit_count;

inline POSITION padd(POSITION p1, POSITION p2) {
    return (POSITION) { p1.row + p2.row, p1.column + p2.column };
}

inline POSITION psub(POSITION p1, POSITION p2) {
    return (POSITION) { p1.row - p2.row, p1.column - p2.column };
}

inline POSITION dtop(DIRECTION d) {
    static POSITION direction_vector[] = { {0, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 0} };
    return direction_vector[d];
}

#define pmove(p, d) (padd((p), dtop(d)))

#endif
