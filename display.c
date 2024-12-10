/*
* display.c:
* ȭ�鿡 ���� ������ ����ϴ� ��� ����
* ��, Ŀ��, �ý��� �޽���, ����â, �ڿ� ���� �� ���
* io.c�� ���ǵ� ��� ���� �Լ����� Ȱ��
*/

#define _CRT_SECURE_NO_WARNINGS
#include "display.h"
#include "io.h"
#include <string.h>

#define MAX_MESSAGES 5  // �ý��� �޽��� â�� ǥ���� �ִ� �޽��� ��

// �޽��� �α� ���� ����
static char message_log[MAX_MESSAGES][50] = { 0 };  // �޽��� �α� ����
static int message_count = 0;               // ���� �޽��� ��

// ��� ��ҵ��� �»�� ��ǥ
const POSITION resource_pos = { 0, 0 };     // �ڿ� ǥ�� ��ġ
const POSITION map_pos = { 1, 0 };          // �� ǥ�� ���� ��ġ

// ȭ�� ����
char backbuf[MAP_HEIGHT][MAP_WIDTH] = { 0 }; // ���� ���� ����
char frontbuf[MAP_HEIGHT][MAP_WIDTH] = { 0 }; // ���� ���� ����

// �Լ� ����
void project(char src[N_LAYER][MAP_HEIGHT][MAP_WIDTH], char dest[MAP_HEIGHT][MAP_WIDTH]);
void display_resource(RESOURCE resource);
void display_map(char map[N_LAYER + 1][MAP_HEIGHT][MAP_WIDTH]);
void display_cursor(CURSOR cursor);
void display_object_info(POSITION selected_pos);
void display_commands(void);
void display_message_log(void);
void log_system_message(const char* message);
void display_units(void);

// ��ü ȭ�� ����
void display(RESOURCE resource, char map[N_LAYER + 1][MAP_HEIGHT][MAP_WIDTH], CURSOR cursor) {
    // ȭ�� ���� �ʱ�ȭ
    memset(frontbuf, ' ', sizeof(frontbuf));
    memset(backbuf, ' ', sizeof(backbuf));

    display_resource(resource);     // �ڿ� ���� ���
    display_map(map);               // �� ���
    display_cursor(cursor);         // Ŀ�� ���
    display_message_log();          // �ý��� �޽��� ���
    display_object_info(cursor.current); // ���õ� ������Ʈ ���� ���
    display_commands();             // ��ɾ� ���
}

// �޽��� �α� ǥ��
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

// �ý��� �޽��� �߰� �� ����
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

// ���õ� ������Ʈ ���� ǥ��
void display_object_info(POSITION selected_pos) {
    for (int i = 0; i < unit_count; i++) {
        if (units[i].pos.row == selected_pos.row && units[i].pos.column == selected_pos.column) {
            set_color(COLOR_CURSOR);
            gotoxy((POSITION) { MAP_HEIGHT + 2, 0 });
            printf("Unit: %c, Health: %d, Carry: %d\n", units[i].type, units[i].health, units[i].carry_spice);
            return;
        }
    }
    printf("���õ� ��ġ�� ������ �����ϴ�.\n");
}

// ��ɾ� ǥ��
void display_commands(void) {
    set_color(COLOR_DEFAULT);
    gotoxy((POSITION) { MAP_HEIGHT + 3, 0 });
    printf("Commands: M - Move, P - Patrol\n");
}

// �� ��ü�� ���
void display_map(char map[N_LAYER + 1][MAP_HEIGHT][MAP_WIDTH]) {
    for (int i = 0; i < MAP_HEIGHT; i++) {
        for (int j = 0; j < MAP_WIDTH; j++) {
            char ch = map[0][i][j];  // �⺻ ���̾�

            // ���� ���̾� Ȯ��
            for (int k = 1; k <= N_LAYER; k++) {
                if (map[k][i][j] != ' ') {
                    ch = map[k][i][j];
                    break;
                }
            }

            // ȭ�� ����
            if (frontbuf[i][j] != ch) {
                POSITION pos = { i, j };
                printc(padd(map_pos, pos), ch, COLOR_DEFAULT);
                frontbuf[i][j] = ch;
            }
        }
    }
}

// �ڿ� ���� ǥ��
void display_resource(RESOURCE resource) {
    set_color(COLOR_RESOURCE);
    gotoxy(resource_pos);
    printf("�����̽� = %d/%d, �α� = %d/%d\n",
        resource.spice, resource.spice_max,
        resource.population, resource.population_max);
}

// Ŀ�� ��ġ ǥ��
void display_cursor(CURSOR cursor) {
    POSITION prev = cursor.previous;
    POSITION curr = cursor.current;

    // ���� ��ġ ����
    char ch = frontbuf[prev.row][prev.column];
    printc(padd(map_pos, prev), ch, COLOR_DEFAULT);

    // ���� ��ġ�� ����
    ch = frontbuf[curr.row][curr.column];
    printc(padd(map_pos, curr), ch, COLOR_CURSOR);
}

// ���� ǥ��
void display_units(void) {
    for (int i = 0; i < unit_count; i++) {
        UNIT* unit = &units[i];
        gotoxy(unit->pos);
        printf("%c", unit->type);
    }
}
