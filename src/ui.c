#define _GNU_SOURCE
#include "ui.h"
#include "vm_config.h"
#include "xml_generator.h"
#include "xml_parser.h"
#include "editor.h"
#include "command.h"
#include "system_check.h"
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <unistd.h>

#define NUM_CHOICES 3
#define NUM_VM_FIELDS 9

char *choices[] = {
    "Create a new VM",
    "Edit an existing VM",
    "Exit",
};

char *vm_fields[] = {
    "Name",
    "CPUs",
    "Memory (MB)",
    "Storage Path",
    "Storage Size (GB)",
    "Chipset",
    "Firmware",
    "Save & Create",
    "Cancel",
};

const char *firmware_options[] = {"uefi", "bios"};
const char *chipset_options[] = {"q35", "i440fx"};

void show_transient_message(const char *message) {
    int msg_len = strlen(message);
    int start_y = LINES - 1;
    int start_x = (COLS - msg_len) / 2;
    
    move(start_y, start_x);
    attron(A_REVERSE);
    printw("%s", message);
    attroff(A_REVERSE);
    refresh();
    napms(2000); /* Wait 2 seconds */
    move(start_y, 0);
    clrtoeol();
}

void draw_main_menu(int highlight) {
    int x, y, i;

    clear();
    x = 2;
    y = 2;
    box(stdscr, 0, 0);

    for (i = 0; i < NUM_CHOICES; i++) {
        if (highlight == i + 1) {
            wattron(stdscr, A_REVERSE);
            mvwprintw(stdscr, y, x, "%s", choices[i]);
            wattroff(stdscr, A_REVERSE);
        } else {
            mvwprintw(stdscr, y, x, "%s", choices[i]);
        }
        y++;
    }
    refresh();
}

void draw_create_vm_menu(VMConfig *config, int highlight, const char* title) {
    clear();
    box(stdscr, 0, 0);
    mvwprintw(stdscr, 2, 2, "%s", title);

    int y = 4;
    for (int i = 0; i < NUM_VM_FIELDS; i++) {
        if (highlight == i) {
            wattron(stdscr, A_REVERSE);
        }
        mvwprintw(stdscr, y + i, 2, "%s", vm_fields[i]);
        wattroff(stdscr, A_REVERSE);
    }

    mvwprintw(stdscr, 4, 25, ": %s", config->name);
    mvwprintw(stdscr, 5, 25, ": %d", config->cpus);
    mvwprintw(stdscr, 6, 25, ": %d", config->memory);
    mvwprintw(stdscr, 7, 25, ": %s", config->storage_path);
    mvwprintw(stdscr, 8, 25, ": %d", config->storage_size);
    mvwprintw(stdscr, 9, 25, ": %s", config->chipset);
    mvwprintw(stdscr, 10, 25, ": %s", config->firmware);

    mvwprintw(stdscr, LINES - 2, 2, "Press 'e' to edit the raw XML. Press 's' to Save & Create.");
    refresh();
}

void get_string_from_user(int y, int x, char *target, int max_len) {
    char buffer[max_len];
    strncpy(buffer, target, max_len);
    
    echo();
    curs_set(1);
    move(y, x);
    clrtoeol();
    mvwgetnstr(stdscr, y, x, buffer, max_len -1);
    curs_set(0);
    noecho();

    if (strlen(buffer) > 0) {
        strncpy(target, buffer, max_len);
    }
}

int get_int_from_user(int y, int x, int current_value) {
    char buffer[12];
    snprintf(buffer, 12, "%d", current_value);

    echo();
    curs_set(1);
    move(y, x);
    clrtoeol();
    mvwgetnstr(stdscr, y, x, buffer, 11);
    curs_set(0);
    noecho();

    if (strlen(buffer) > 0) {
        char *endptr;
        long val = strtol(buffer, &endptr, 10);
        if (*endptr == '\0') {
            return (int)val;
        }
    }
    return current_value;
}


int show_selection_menu(const char *title, const char **options, int num_options, int current_selection_index) {
    int win_height = num_options + 4;
    int win_width = 40;
    int start_y = (LINES - win_height) / 2;
    int start_x = (COLS - win_width) / 2;

    WINDOW *win = newwin(win_height, win_width, start_y, start_x);
    box(win, 0, 0);
    mvwprintw(win, 1, 2, "%s", title);

    keypad(win, TRUE);
    int highlight = current_selection_index;
    int choice = -1;

    while (choice == -1) {
        for (int i = 0; i < num_options; i++) {
            if (i == highlight) {
                wattron(win, A_REVERSE);
            }
            mvwprintw(win, i + 2, 2, "%s", options[i]);
            wattroff(win, A_REVERSE);
        }
        wrefresh(win);

        int c = wgetch(win);
        switch (c) {
            case KEY_UP:
                highlight = (highlight == 0) ? num_options - 1 : highlight - 1;
                break;
            case KEY_DOWN:
                highlight = (highlight == num_options - 1) ? 0 : highlight + 1;
                break;
            case 10: /* Enter */
                choice = highlight;
                break;
            case 27: /* Escape */
                choice = -2; /* Indicates cancellation */
                break;
        }
    }

    delwin(win);
    return choice;
}

void run_virsh_define(VMConfig *config) {
    if (!is_kvm_available()) {
        show_transient_message("Warning: KVM not found. Falling back to slower QEMU emulation.");
    }

    char *xml_buffer = NULL;
    generate_xml(config, &xml_buffer);
    if (!xml_buffer) {
        show_transient_message("Failed to generate XML.");
        return;
    }

    char tmp_path[] = "/tmp/eqemu_vm_XXXXXX.xml";
    int fd = mkstemps(tmp_path, 4);
    if (fd == -1) {
        show_transient_message("Failed to create temporary file.");
        free(xml_buffer);
        return;
    }

    write(fd, xml_buffer, strlen(xml_buffer));
    close(fd);
    free(xml_buffer);

    char command[1024];
    snprintf(command, sizeof(command), "virsh define %s", tmp_path);

    endwin();
    printf("Running command: %s\n", command);
    int result = system(command);
    printf("\nPress enter to continue...");
    
    char ch;
    while((ch = getchar()) != '\n' && ch != EOF);

    initscr();
    refresh();

    if (result == 0) {
        show_transient_message("VM definition updated successfully!");
    } else {
        show_transient_message("Failed to update VM definition. Check output.");
    }

    unlink(tmp_path); /* Clean up the temporary file */
}

void handle_edit_xml(VMConfig *config) {
    char *xml_buffer = NULL;
    generate_xml(config, &xml_buffer);
    if (!xml_buffer) {
        show_transient_message("Failed to generate XML.");
        return;
    }

    char tmp_path[] = "/tmp/eqemu_vm_XXXXXX.xml";
    int fd = mkstemps(tmp_path, 4);
    if (fd == -1) {
        show_transient_message("Failed to create temporary file.");
        free(xml_buffer);
        return;
    }

    write(fd, xml_buffer, strlen(xml_buffer));
    close(fd);
    free(xml_buffer);

    endwin();
    open_editor(tmp_path);
    initscr();
    refresh();

    parse_xml_file(tmp_path, config);
    unlink(tmp_path);
}


void handle_vm_form(VMConfig *config, const char* title) {
    int highlight = 0;
    int c;

    while (1) {
        draw_create_vm_menu(config, highlight, title);
        c = getch();
        switch (c) {
        case KEY_UP:
            highlight = (highlight == 0) ? NUM_VM_FIELDS - 1 : highlight - 1;
            break;
        case KEY_DOWN:
            highlight = (highlight == NUM_VM_FIELDS - 1) ? 0 : highlight + 1;
            break;
        case 'e':
        case 'E':
            handle_edit_xml(config);
            break;
        case 's':
        case 'S':
            run_virsh_define(config);
            return;
        case 10: { /* Enter key */
            if (highlight == NUM_VM_FIELDS - 1) return; /* Cancel */
            if (highlight == NUM_VM_FIELDS - 2) { /* Save */
                run_virsh_define(config);
                return;
            }

            int y = 4 + highlight;
            int x = 27;

            switch (highlight) {
                case 0: /* Name */
                    get_string_from_user(y, x, config->name, sizeof(config->name));
                    break;
                case 1: /* CPUs */
                    {
                        int new_cpus = get_int_from_user(y, x, config->cpus);
                        if (new_cpus >= 1) config->cpus = new_cpus;
                    }
                    break;
                case 2: /* Memory */
                     {
                        int new_mem = get_int_from_user(y, x, config->memory);
                        if (new_mem >= 128) config->memory = new_mem;
                    }
                    break;
                case 3: /* Storage Path */
                    get_string_from_user(y, x, config->storage_path, sizeof(config->storage_path));
                    break;
                case 4: /* Storage Size */
                    {
                        int new_size = get_int_from_user(y, x, config->storage_size);
                        if (new_size >= 1) config->storage_size = new_size;
                    }
                    break;
                case 5: /* Chipset */
                    {
                        int current_idx = (strcmp(config->chipset, "i440fx") == 0) ? 1 : 0;
                        int choice = show_selection_menu("Chipset", chipset_options, 2, current_idx);
                        if (choice >= 0) strcpy(config->chipset, chipset_options[choice]);
                    }
                    break;
                case 6: /* Firmware */
                    {
                        int current_idx = (strcmp(config->firmware, "bios") == 0) ? 1 : 0;
                        int choice = show_selection_menu("Firmware", firmware_options, 2, current_idx);
                        if (choice >= 0) strcpy(config->firmware, firmware_options[choice]);
                    }
                    break;
            }
            break; 
        }
        }
    }
}

void handle_edit_vm_menu() {
    char *vm_list_str = execute_command("virsh list --all --name");
    if (!vm_list_str || strlen(vm_list_str) == 0) {
        show_transient_message("No VMs found.");
        free(vm_list_str);
        return;
    }

    int count = 0;
    char *temp = strdup(vm_list_str);
    char *token = strtok(temp, "\n");
    while (token) {
        if (strlen(token) > 0) count++;
        token = strtok(NULL, "\n");
    }
    free(temp);

    if (count == 0) {
        show_transient_message("No VMs found.");
        free(vm_list_str);
        return;
    }

    const char **vm_options = calloc(count, sizeof(char*));
    temp = strdup(vm_list_str);
    token = strtok(temp, "\n");
    int i = 0;
    while (token) {
        if (strlen(token) > 0) {
            vm_options[i++] = token;
        }
        token = strtok(NULL, "\n");
    }

    int choice = show_selection_menu("Select a VM to Edit", vm_options, count, 0);

    if (choice >= 0) {
        char command[512];
        snprintf(command, sizeof(command), "virsh dumpxml %s", vm_options[choice]);
        char *xml_buffer = execute_command(command);

        if (xml_buffer) {
            VMConfig config;
            if (parse_xml_buffer(xml_buffer, &config) == 0) {
                handle_vm_form(&config, "Edit Virtual Machine");
            } else {
                show_transient_message("Failed to parse VM XML.");
            }
            free(xml_buffer);
        } else {
            show_transient_message("Failed to get VM XML.");
        }
    }

    free(vm_list_str);
    free(vm_options);
}


void handle_main_menu() {
    int highlight = 1;
    int choice = 0;
    int c;

    while (choice != 3) {
        draw_main_menu(highlight);
        c = getch();
        switch (c) {
        case KEY_UP:
            if (highlight == 1)
                highlight = NUM_CHOICES;
            else
                --highlight;
            break;
        case KEY_DOWN:
            if (highlight == NUM_CHOICES)
                highlight = 1;
            else
                ++highlight;
            break;
        case 10: /**/ /* Enter key */
            choice = highlight;
            
            switch(choice) {
                case 1:
                    {
                        VMConfig config;
                        init_vm_config(&config);
                        handle_vm_form(&config, "Create a New Virtual Machine");
                    }
                    break;
                case 2:
                    handle_edit_vm_menu();
                    break;
                case 3:
                    break;
            }

            break;
        default:
            break;
        }
    }
}

