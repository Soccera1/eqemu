#ifndef XML_PARSER_H
#define XML_PARSER_H

#include "vm_config.h"

/**/
/* Parses a libvirt XML file and populates a VMConfig struct. */
/**/
int parse_xml_file(const char *filepath, VMConfig *config);

/**/
/* Parses a libvirt XML buffer and populates a VMConfig struct. */
/**/
int parse_xml_buffer(const char *xml_buffer, VMConfig *config);


#endif /**/ /* XML_PARSER_H */
