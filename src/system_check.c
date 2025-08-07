#include "system_check.h"
#include <unistd.h>

int is_kvm_available() {
    /**/
    /* Check for read and write access to the KVM device. */
    /**/
    if (access("/dev/kvm", R_OK | W_OK) == 0) {
        return 1; /* KVM is available and accessible. */
    }
    return 0; /* KVM is not available or not accessible. */
}
