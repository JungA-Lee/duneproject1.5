#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include "common.h"
#include "io.h"
#include "display.h"

/* 함수 선언 */
void init(void);
void intro(void);
void outro(void);
void cursor_move(DIRECTION dir, bool fast_move);
void update_status(POSITION pos);
void clear_status(void);

void sandworm_move(void);
void eagle_move(void);
void sandstorm_move(void);

void start_build_mode(void);
void cancel_build_mode(void);
void handle_build_key(KEY key);
void build_current_position(void);

void start_production(void);
void check_production_status(void);
void cancel_production(void);

void add_unit(char type, POSITION pos);
void remove_unit(int index);
void command_harvest(int unit_index);
void command_move(int unit_index, POSITION target);
void update_units(void);
void update_combat_and_units(void);
void command_unit(KEY key);

void patrol_unit(int unit_index);
void move_unit(int unit_index);
void combat_unit(int attacker_index);


/* 전역 변수 */
int sys_clock = 0;
CURSOR cursor = { {1, 1}, {1, 1}, 1 };
POSITION selected_pos = { -1, -1 };

POSITION target = { 10, 10 };
POSITION pos = { .row = 10, .column = 15 };

char map[N_LAYER + 1][MAP_HEIGHT][MAP_WIDTH] = { 0 };

RESOURCE resource = { 100, 100, 0, 10 };

bool build_mode = false;
char selected_building = '\0';

UNIT units[MAX_UNITS] = { 0 };
int unit_count = 0;

SANDWORM sandworm = { {5, 5}, 500, 500 };
EAGLE eagle = { {10, 10}, 300, 300 };
SANDSTORM sandstorm = {
    .pos = { {15, 15}, {15, 16}, {16, 15}, {16, 16} },
    .move_period = 100,
    .next_move_time = 100,
    .lifespan = 5000
};

PRODUCTION current_production = { false, 0, 5000, 'H' };

/* main() 함수 */
int main(void) {
    srand((unsigned int)time(NULL));

    intro();
    init();
    display(resource, map, cursor);

    while (1) {
        KEY key = get_key();

        if (is_arrow_key(key)) {
            cursor_move(ktod(key), false);
        }
        else if (key == k_space) {
            if (build_mode && selected_building != '\0') {
                build_current_position();
            }
            else {
                update_status(cursor.current);
            }
        }
        else if (key == k_escape) {
            if (build_mode) cancel_build_mode();
            else clear_status();
        }
        else if ((key == 'B' || key == 'b') && !build_mode) {
            start_build_mode();
        }
        else if (build_mode && (key == 'P' || key == 'p' || key == 'B' || key == 'b')) {
            handle_build_key(key);
        }
        else if (key == 'H') {
            start_production();
        }
        else if (key == 'M' || key == 'P') {
            command_unit(key);
        }

        if (current_production.in_progress) {
            check_production_status();
        }

        sandworm_move();
        eagle_move();
        sandstorm_move();

        update_units();
        update_combat_and_units();

        display(resource, map, cursor);

        Sleep(TICK);
        sys_clock += TICK;
    }
}

/* 초기화 */
void init(void) {
    for (int i = 0; i < MAP_HEIGHT; i++) {
        for (int j = 0; j < MAP_WIDTH; j++) {
            if (i == 0 || i == MAP_HEIGHT - 1 || j == 0 || j == MAP_WIDTH - 1) {
                map[0][i][j] = '#';
            }
            else {
                map[0][i][j] = ' ';
            }
        }
    }

    map[0][MAP_HEIGHT - 3][1] = 'B';
    map[0][MAP_HEIGHT - 3][2] = 'B';
    map[0][MAP_HEIGHT - 2][1] = 'B';
    map[0][MAP_HEIGHT - 2][2] = 'B';

    map[0][1][MAP_WIDTH - 3] = 'H';
    map[0][1][MAP_WIDTH - 4] = 'H';
    map[0][2][MAP_WIDTH - 3] = 'H';
    map[0][2][MAP_WIDTH - 4] = 'H';

    map[0][3][3] = '5';

    map[1][5][5] = 'W';
    map[2][10][10] = 'E';

    log_system_message("맵 초기화 완료.");
}

/* 상태창 업데이트 */
void update_status(POSITION pos) {
    selected_pos = pos;
    char obj = map[0][pos.row][pos.column];

    switch (obj) {
    case 'B': log_system_message("아트레이디스 본부"); break;
    case 'P': log_system_message("장판"); break;
    case ' ': log_system_message("사막 지형"); break;
    case 'S': log_system_message("스파이스 매장지"); break;
    default: log_system_message("알 수 없는 지형"); break;
    }
}

void clear_status(void) {
    selected_pos.row = -1;
    selected_pos.column = -1;
    log_system_message("상태창 비우기");
}

/* 건설 모드 */
void start_build_mode(void) {
    build_mode = true;
    selected_building = '\0';
    cursor.size = 1;
    log_system_message("Build Mode activated. Press P or B to select.");
}

void cancel_build_mode(void) {
    build_mode = false;
    selected_building = '\0';
    cursor.size = 1;
    log_system_message("Build Mode canceled.");
}

void handle_build_key(KEY key) {
    if (!build_mode) return;

    switch (key) {
    case 'P':
    case 'p':
        selected_building = 'P';
        cursor.size = 2;
        log_system_message("Selected: Plate (2x2)");
        break;
    case 'B':
    case 'b':
        if (selected_building == 'P') {
            selected_building = 'B';
            cursor.size = 2;
            log_system_message("Selected: Base (2x2)");
        }
        else {
            log_system_message("Error: Plates must be built before building a base.");
        }
        break;
    default:
        log_system_message("Invalid key in Build Mode.");
        break;
    }
}

void build_current_position(void) {
    if (!build_mode || selected_building == '\0') {
        log_system_message("Build Mode not active or no building selected.");
        return;
    }

    POSITION pos = cursor.current;
    bool can_build = true;

    for (int i = 0; i < cursor.size; i++) {
        for (int j = 0; j < cursor.size; j++) {
            int row = pos.row + i;
            int col = pos.column + j;

            if (row >= MAP_HEIGHT || col >= MAP_WIDTH) {
                can_build = false;
                log_system_message("Error: Out of bounds.");
                break;
            }
            if (selected_building == 'B' && map[0][row][col] != 'P') {
                can_build = false;
                log_system_message("Error: Base must be built on a Plate.");
                break;
            }
            if (map[0][row][col] != ' ' && map[0][row][col] != 'P') {
                can_build = false;
                log_system_message("Error: Space occupied.");
                break;
            }
        }
        if (!can_build) break;
    }

    if (can_build) {
        for (int i = 0; i < cursor.size; i++) {
            for (int j = 0; j < cursor.size; j++) {
                map[0][pos.row + i][pos.column + j] = selected_building;
            }
        }
        log_system_message("Construction complete!");
        cancel_build_mode();
    }
    else {
        log_system_message("Cannot build here.");
    }
}

/* 생산 명령 처리 */
void start_production(void) {
    if (selected_pos.row < 0 || selected_pos.column < 0) {
        log_system_message("No valid position selected.");
        return;
    }

    if (map[0][selected_pos.row][selected_pos.column] != 'B') {
        log_system_message("No base at selected position.");
        return;
    }

    if (resource.spice < 10) {
        log_system_message("Not enough spice to produce a harvester.");
        return;
    }

    resource.spice -= 10;

    int target_row = selected_pos.row + 1;
    int target_col = selected_pos.column;

    if (map[1][target_row][target_col] == ' ') {
        map[1][target_row][target_col] = 'H';
        log_system_message("Harvester production started.");
    }
    else {
        log_system_message("No space available for harvester.");
    }
}

void cancel_production(void) {
    if (current_production.in_progress) {
        current_production.in_progress = false;
        log_system_message("Production canceled.");
    }
}

void check_production_status(void) {
    if (current_production.in_progress) {
        int elapsed_time = sys_clock - current_production.start_time;
        if (elapsed_time >= current_production.production_time) {
            current_production.in_progress = false;
            log_system_message("Harvester ready.");
        }
    }
}

/* 샌드웜 이동 */
void sandworm_move(void) {
    if (sys_clock < sandworm.next_move_time) return;

    POSITION closest_pos = { -1, -1 };
    int min_distance = MAP_WIDTH * MAP_HEIGHT;

    for (int i = 0; i < MAP_HEIGHT; i++) {
        for (int j = 0; j < MAP_WIDTH; j++) {
            if (map[1][i][j] != ' ' && map[1][i][j] != 'W') {
                int dist = abs(sandworm.pos.row - i) + abs(sandworm.pos.column - j);
                if (dist < min_distance) {
                    min_distance = dist;
                    closest_pos = (POSITION){ i, j };
                }
            }
        }
    }

    if (closest_pos.row != -1) {
        POSITION direction = psub(closest_pos, sandworm.pos);
        DIRECTION move_dir = (abs(direction.row) > abs(direction.column)) ?
            (direction.row > 0 ? d_down : d_up) :
            (direction.column > 0 ? d_right : d_left);
        POSITION new_pos = pmove(sandworm.pos, move_dir);

        map[1][sandworm.pos.row][sandworm.pos.column] = ' ';
        sandworm.pos = new_pos;
        map[1][sandworm.pos.row][sandworm.pos.column] = 'W';
    }

    sandworm.next_move_time = sys_clock + sandworm.move_period;
}

/* 독수리 이동 */
void eagle_move(void) {
    if (sys_clock < eagle.next_move_time) return;

    DIRECTION move_dir = (DIRECTION)(rand() % 4 + 1);
    POSITION new_pos = pmove(eagle.pos, move_dir);

    if (new_pos.row > 0 && new_pos.row < MAP_HEIGHT - 1 &&
        new_pos.column > 0 && new_pos.column < MAP_WIDTH - 1) {
        map[2][eagle.pos.row][eagle.pos.column] = ' ';
        eagle.pos = new_pos;
        map[2][eagle.pos.row][eagle.pos.column] = 'E';
    }

    eagle.next_move_time = sys_clock + eagle.move_period;
}

/* 모래폭풍 이동 및 소멸 */
void sandstorm_move(void) {
    if (sys_clock < sandstorm.next_move_time) return;

    for (int i = 0; i < 4; i++) {
        map[2][sandstorm.pos[i].row][sandstorm.pos[i].column] = ' ';
    }

    DIRECTION move_dir = (DIRECTION)(rand() % 4 + 1);
    POSITION offset = dtop(move_dir);
    for (int i = 0; i < 4; i++) {
        sandstorm.pos[i].row += offset.row;
        sandstorm.pos[i].column += offset.column;

        if (sandstorm.pos[i].row < 1 || sandstorm.pos[i].row >= MAP_HEIGHT - 1 ||
            sandstorm.pos[i].column < 1 || sandstorm.pos[i].column >= MAP_WIDTH - 1) {
            sandstorm.pos[i] = (POSITION){ rand() % MAP_HEIGHT, rand() % MAP_WIDTH };
        }

        map[2][sandstorm.pos[i].row][sandstorm.pos[i].column] = 'S';
    }

    sandstorm.lifespan -= sandstorm.move_period;
    if (sandstorm.lifespan <= 0) {
        for (int i = 0; i < 4; i++) {
            map[2][sandstorm.pos[i].row][sandstorm.pos[i].column] = ' ';
        }
        sandstorm.lifespan = 5000;
    }

    sandstorm.next_move_time = sys_clock + sandstorm.move_period;
}

/* 유닛 상태 업데이트 */
void update_units(void) {
    for (int i = 0; i < unit_count; i++) {
        UNIT* unit = &units[i];
        if (unit->is_patrolling) {
            patrol_unit(i);
        }
        else {
            move_unit(i);
        }
    }
}

/* 유닛 전투 및 상태 업데이트 */
void update_combat_and_units(void) {
    for (int i = 0; i < unit_count; i++) {
        combat_unit(i);
        if (!units[i].is_patrolling) {
            move_unit(i);
        }
    }
}

/* 커서 이동 */
void cursor_move(DIRECTION dir, bool fast_move) {
    int move_steps = fast_move ? 3 : 1;

    for (int i = 0; i < move_steps; i++) {
        POSITION curr = cursor.current;
        POSITION new_pos = pmove(curr, dir);

        // 맵 경계 검사 추가
        if (new_pos.row >= 1 && new_pos.row < MAP_HEIGHT - 1 &&
            new_pos.column >= 1 && new_pos.column < MAP_WIDTH - 1) {
            cursor.previous = cursor.current;
            cursor.current = new_pos;
        }
        else {
            log_system_message("Cursor cannot move out of bounds.");
        }
    }
}

/* 유닛 추가 */
void add_unit(char type, POSITION pos) {
    if (unit_count < MAX_UNITS) {
        units[unit_count++] = (UNIT){
            .pos = pos,
            .dest = pos,
            .target = pos,
            .type = type,
            .health = MAX_HEALTH,
            .attack = 10,
            .range = 1,
            .move_speed = 200,
            .attack_speed = 500,
            .next_action_time = 0,
            .carry_spice = 0,
            .is_patrolling = false,
            .patrol_point = { -1, -1 },
            .in_action = false
        };
        log_system_message("Unit added.");
    }
    else {
        log_system_message("Cannot add unit: supply limit reached.");
    }
}

/* 유닛 삭제 */
void remove_unit(int index) {
    if (index < 0 || index >= unit_count) {
        log_system_message("Invalid unit index.");
        return;
    }
    for (int i = index; i < unit_count - 1; i++) {
        units[i] = units[i + 1];
    }
    unit_count--;
    log_system_message("Unit removed.");
}

/* 하베스터 수확 명령 */
void command_harvest(int unit_index) {
    UNIT* unit = &units[unit_index];
    if (map[0][unit->pos.row][unit->pos.column] == 'S') { // 스파이스 매장지 여부 확인
        log_system_message("Harvesting spice...");
        int spice_amount = rand() % 3 + 2; // 2~4 스파이스
        resource.spice += spice_amount;

        if (--map[0][unit->pos.row][unit->pos.column] == '0') { // 매장량 감소
            map[0][unit->pos.row][unit->pos.column] = ' '; // 매장량 소진 시 지형 비움
        }

        unit->in_action = false; // 동작 완료
    }
    else {
        log_system_message("No spice at target.");
    }
}

/* 유닛 이동 명령 */
void command_move(int unit_index, POSITION target) {
    UNIT* unit = &units[unit_index];
    unit->dest = target;
    unit->in_action = true;

    POSITION diff = psub(target, unit->pos);
    DIRECTION move_dir = (abs(diff.row) > abs(diff.column)) ?
        (diff.row > 0 ? d_down : d_up) :
        (diff.column > 0 ? d_right : d_left);

    POSITION next_pos = pmove(unit->pos, move_dir);

    if (next_pos.row >= 0 && next_pos.row < MAP_HEIGHT &&
        next_pos.column >= 0 && next_pos.column < MAP_WIDTH &&
        map[1][next_pos.row][next_pos.column] == ' ') {
        map[1][unit->pos.row][unit->pos.column] = ' ';
        unit->pos = next_pos;
        map[1][unit->pos.row][unit->pos.column] = unit->type;
        log_system_message("Unit moving...");
    }

    if (unit->pos.row == target.row && unit->pos.column == target.column) {
        unit->in_action = false;
        log_system_message("Unit arrived at target.");
    }
}

/* 유닛 명령 처리 */
void command_unit(KEY key) {
    for (int i = 0; i < unit_count; i++) {
        if (units[i].pos.row == cursor.current.row && units[i].pos.column == cursor.current.column) {
            if (key == 'M') {
                POSITION target = cursor.current;
                command_move(i, target);
            }
            else if (key == 'H') {
                command_harvest(i);
            }
            else {
                log_system_message("Invalid command for unit.");
            }
            return;
        }
    }
    log_system_message("No unit at selected position.");
}


/* 게임 시작 메시지 */
void intro(void) {
    printf("=== Welcome to DUNE 1.5 ===\n");
    printf("Prepare for battle!\n");
    Sleep(2000);
    system("cls");
}

/* 게임 종료 메시지 */
void outro(void) {
    printf("Exiting game...\n");
    Sleep(1000);
    system("cls");
    printf("Goodbye, warrior of Arrakis.\n");
    exit(0);
}

/* 유닛 순찰 */
void patrol_unit(int unit_index) {
    UNIT* unit = &units[unit_index];

    if (sys_clock < unit->next_action_time) return;

    if (psub(unit->pos, unit->patrol_point).row == 0 &&
        psub(unit->pos, unit->patrol_point).column == 0) {
        POSITION temp = unit->dest;
        unit->dest = unit->patrol_point;
        unit->patrol_point = temp;
    }

    move_unit(unit_index);
}


/* 유닛 이동 */
void move_unit(int unit_index) {
    UNIT* unit = &units[unit_index];

    if (sys_clock < unit->next_action_time) return;

    POSITION diff = psub(unit->dest, unit->pos);
    if (diff.row == 0 && diff.column == 0) {
        unit->in_action = false;  // 도착 시 동작 상태 해제
        log_system_message("Unit has arrived at the destination.");
        return;
    }

    DIRECTION dir = (abs(diff.row) > abs(diff.column))
        ? (diff.row > 0 ? d_down : d_up)
        : (diff.column > 0 ? d_right : d_left);

    POSITION next_pos = pmove(unit->pos, dir);

    if (map[1][next_pos.row][next_pos.column] == ' ') {
        map[1][unit->pos.row][unit->pos.column] = ' ';
        unit->pos = next_pos;
        map[1][unit->pos.row][unit->pos.column] = unit->type;
    }
    else {
        log_system_message("Cannot move to the target position.");
    }

    unit->next_action_time = sys_clock + unit->move_speed;
}


/* 유닛 전투 */
void combat_unit(int attacker_index) {
    UNIT* attacker = &units[attacker_index];

    for (int i = 0; i < unit_count; i++) {
        if (i == attacker_index) continue;

        UNIT* target = &units[i];
        int distance = abs(attacker->pos.row - target->pos.row) + abs(attacker->pos.column - target->pos.column);

        if (distance <= attacker->range && target->health > 0) {
            target->health -= attacker->attack;
            log_system_message("Unit attacked!");

            if (target->health <= 0) {
                log_system_message("Unit destroyed!");
                map[1][target->pos.row][target->pos.column] = ' ';
                for (int j = i; j < unit_count - 1; j++) {
                    units[j] = units[j + 1];
                }
                unit_count--;
            }
            return;
        }
    }
}
