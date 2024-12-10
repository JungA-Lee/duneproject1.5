/*
* display.c:
* 화면에 게임 정보를 출력하는 기능 구현
* 맵, 커서, 시스템 메시지, 정보창, 자원 상태 등 출력
* io.c에 정의된 출력 관련 함수들을 활용
*/

#define _CRT_SECURE_NO_WARNINGS
#include "display.h"
#include "io.h"
#include <string.h>

#define MAX_MESSAGES 5  // 시스템 메시지 창에 표시할 최대 메시지 수

// 메시지 로그 관리 변수
static char message_log[MAX_MESSAGES][50] = { 0 };  // 메시지 로그 버퍼
static int message_count = 0;               // 현재 메시지 수

// 출력 요소들의 좌상단 좌표
const POSITION resource_pos = { 0, 0 };     // 자원 표시 위치
const POSITION map_pos = { 1, 0 };          // 맵 표시 시작 위치

// 화면 버퍼
char backbuf[MAP_HEIGHT][MAP_WIDTH] = { 0 }; // 이전 상태 버퍼
char frontbuf[MAP_HEIGHT][MAP_WIDTH] = { 0 }; // 현재 상태 버퍼

// 함수 선언
void project(char src[N_LAYER][MAP_HEIGHT][MAP_WIDTH], char dest[MAP_HEIGHT][MAP_WIDTH]);
void display_resource(RESOURCE resource);
void display_map(char map[N_LAYER + 1][MAP_HEIGHT][MAP_WIDTH]);
void display_cursor(CURSOR cursor);
void display_object_info(POSITION selected_pos);
void display_commands(void);
void display_message_log(void);
void log_system_message(const char* message);
void display_units(void);

// 전체 화면 갱신
void display(RESOURCE resource, char map[N_LAYER + 1][MAP_HEIGHT][MAP_WIDTH], CURSOR cursor) {
    // 화면 버퍼 초기화
    memset(frontbuf, ' ', sizeof(frontbuf));
    memset(backbuf, ' ', sizeof(backbuf));

    display_resource(resource);     // 자원 상태 출력
    display_map(map);               // 맵 출력
    display_cursor(cursor);         // 커서 출력
    display_message_log();          // 시스템 메시지 출력
    display_object_info(cursor.current); // 선택된 오브젝트 정보 출력
    display_commands();             // 명령어 출력
}

// 메시지 로그 표시
void display_message_log(void) {
    set_color(COLOR_RESOURCE);

    for (int i = 0; i < MAX_MESSAGES; i++) {
        gotoxy((POSITION) { MAP_HEIGHT + 1 + i, 0 });
        printf("                                        ");
    }

    for (int i = 0; i < message_count; i++) {
        gotoxy((POSITION) { MAP_HEIGHT + 1 + i, 0 });
        printf("%s\n", message_log[i]);
    }
}

// 시스템 메시지 추가 및 관리
void log_system_message(const char* message) {
    if (strlen(message) >= 50) {
        printf("Warning: Message too long, truncating.\n");
    }
    if (message_count < MAX_MESSAGES) {
        strncpy(message_log[message_count++], message, 49);
    }
    else {
        for (int i = 1; i < MAX_MESSAGES; i++) {
            strncpy(message_log[i - 1], message_log[i], 49);
        }
        strncpy(message_log[MAX_MESSAGES - 1], message, 49);
    }
}

// 선택된 오브젝트 정보 표시
void display_object_info(POSITION selected_pos) {
    for (int i = 0; i < unit_count; i++) {
        if (units[i].pos.row == selected_pos.row && units[i].pos.column == selected_pos.column) {
            set_color(COLOR_CURSOR);
            gotoxy((POSITION) { MAP_HEIGHT + 2, 0 });
            printf("Unit: %c, Health: %d, Carry: %d\n", units[i].type, units[i].health, units[i].carry_spice);
            return;
        }
    }
    printf("선택된 위치에 유닛이 없습니다.\n");
}

// 명령어 표시
void display_commands(void) {
    set_color(COLOR_DEFAULT);
    gotoxy((POSITION) { MAP_HEIGHT + 3, 0 });
    printf("Commands: M - Move, P - Patrol\n");
}

// 맵 전체를 출력
void display_map(char map[N_LAYER + 1][MAP_HEIGHT][MAP_WIDTH]) {
    for (int i = 0; i < MAP_HEIGHT; i++) {
        for (int j = 0; j < MAP_WIDTH; j++) {
            char ch = map[0][i][j];  // 기본 레이어

            // 상위 레이어 확인
            for (int k = 1; k <= N_LAYER; k++) {
                if (map[k][i][j] != ' ') {
                    ch = map[k][i][j];
                    break;
                }
            }

            // 화면 갱신
            if (frontbuf[i][j] != ch) {
                POSITION pos = { i, j };
                printc(padd(map_pos, pos), ch, COLOR_DEFAULT);
                frontbuf[i][j] = ch;
            }
        }
    }
}

// 자원 상태 표시
void display_resource(RESOURCE resource) {
    set_color(COLOR_RESOURCE);
    gotoxy(resource_pos);
    printf("스파이스 = %d/%d, 인구 = %d/%d\n",
        resource.spice, resource.spice_max,
        resource.population, resource.population_max);
}

// 커서 위치 표시
void display_cursor(CURSOR cursor) {
    POSITION prev = cursor.previous;
    POSITION curr = cursor.current;

    // 이전 위치 복원
    char ch = frontbuf[prev.row][prev.column];
    printc(padd(map_pos, prev), ch, COLOR_DEFAULT);

    // 현재 위치를 강조
    ch = frontbuf[curr.row][curr.column];
    printc(padd(map_pos, curr), ch, COLOR_CURSOR);
}

// 유닛 표시
void display_units(void) {
    for (int i = 0; i < unit_count; i++) {
        UNIT* unit = &units[i];
        gotoxy(unit->pos);
        printf("%c", unit->type);
    }
}
