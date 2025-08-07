#ifndef VM_CONFIG_H
#define VM_CONFIG_H

typedef struct {
    char name[256];
    int cpus;
    int memory; /* In MB */
    char storage_path[1024];
    int storage_size; /* In GB */
    char chipset[64];
    char firmware[16]; /* "bios" or "uefi" */
} VMConfig;

void init_vm_config(VMConfig *config);

#endif /**/ /* VM_CONFIG_H */
