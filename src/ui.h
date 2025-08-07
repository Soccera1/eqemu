#ifndef UI_H
#define UI_H

#include <ncurses.h>
#include "vm_config.h"

void draw_main_menu(int highlight);
void handle_main_menu();
void handle_create_vm_menu(VMConfig *config);
int show_selection_menu(const char *title, const char **options, int num_options, int current_selection_index);
void get_string_from_user(int y, int x, char *target, int max_len);
int get_int_from_user(int y, int x, int current_value);

#endif /**/ /* UI_H */
