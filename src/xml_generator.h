#ifndef XML_GENERATOR_H
#define XML_GENERATOR_H

#include "vm_config.h"

/**/
/* Generates a libvirt XML configuration from a VMConfig struct. */
/* The caller is responsible for freeing the allocated xml_buffer. */
/**/
int generate_xml(VMConfig *config, char **xml_buffer);

#endif /**/ /* XML_GENERATOR_H */
