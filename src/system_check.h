#ifndef SYSTEM_CHECK_H
#define SYSTEM_CHECK_H

/**/
/* Checks if the /dev/kvm device is accessible to the current user. */
/* Returns 1 if available, 0 otherwise. */
/**/
int is_kvm_available();

#endif /**/ /* SYSTEM_CHECK_H */
