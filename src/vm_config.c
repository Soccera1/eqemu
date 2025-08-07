#include "vm_config.h"
#include <string.h>

void init_vm_config(VMConfig *config) {
    strcpy(config->name, "new-vm");
    config->cpus = 1;
    config->memory = 1024;
    strcpy(config->storage_path, "/var/lib/libvirt/images/new-vm.qcow2");
    config->storage_size = 20;
    strcpy(config->chipset, "q35");
    strcpy(config->firmware, "uefi");
}
