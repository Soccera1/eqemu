#include "xml_generator.h"
#include "system_check.h"
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int generate_xml(VMConfig *config, char **xml_buffer) {
    xmlDocPtr doc = NULL;
    xmlNodePtr root_node = NULL, node = NULL, sub_node = NULL;
    char str_buffer[1024];
    const char *domain_type = is_kvm_available() ? "kvm" : "qemu";

    doc = xmlNewDoc(BAD_CAST "1.0");
    root_node = xmlNewNode(NULL, BAD_CAST "domain");
    xmlSetProp(root_node, BAD_CAST "type", BAD_CAST domain_type);
    xmlDocSetRootElement(doc, root_node);

    /**/
    /* VM Name */
    /**/
    xmlNewChild(root_node, NULL, BAD_CAST "name", BAD_CAST config->name);

    /**/
    /* Memory */
    /**/
    snprintf(str_buffer, sizeof(str_buffer), "%d", config->memory * 1024); /* Memory in KiB */
    node = xmlNewChild(root_node, NULL, BAD_CAST "memory", BAD_CAST str_buffer);
    xmlSetProp(node, BAD_CAST "unit", BAD_CAST "KiB");

    /**/
    /* vCPUs */
    /**/
    snprintf(str_buffer, sizeof(str_buffer), "%d", config->cpus);
    xmlNewChild(root_node, NULL, BAD_CAST "vcpu", BAD_CAST str_buffer);

    /**/
    /* OS Type */
    /**/
    node = xmlNewChild(root_node, NULL, BAD_CAST "os", NULL);
    sub_node = xmlNewChild(node, NULL, BAD_CAST "type", BAD_CAST "hvm");
    xmlSetProp(sub_node, BAD_CAST "arch", BAD_CAST "x86_64");
    xmlSetProp(sub_node, BAD_CAST "machine", BAD_CAST config->chipset);

    if (strcmp(config->firmware, "uefi") == 0) {
        sub_node = xmlNewChild(node, NULL, BAD_CAST "loader", NULL);
        xmlSetProp(sub_node, BAD_CAST "readonly", BAD_CAST "yes");
        xmlSetProp(sub_node, BAD_CAST "type", BAD_CAST "pflash");
        xmlSetProp(sub_node, BAD_CAST "path", BAD_CAST "/usr/share/OVMF/OVMF_CODE.fd");

        snprintf(str_buffer, sizeof(str_buffer), "/var/lib/libvirt/qemu/nvram/%s_VARS.fd", config->name);
        sub_node = xmlNewChild(node, NULL, BAD_CAST "nvram", BAD_CAST str_buffer);
        xmlSetProp(sub_node, BAD_CAST "template", BAD_CAST "/usr/share/OVMF/OVMF_VARS.fd");
    }


    /**/
    /* Features */
    /**/
    node = xmlNewChild(root_node, NULL, BAD_CAST "features", NULL);
    xmlNewChild(node, NULL, BAD_CAST "acpi", NULL);
    xmlNewChild(node, NULL, BAD_CAST "apic", NULL);

    /**/
    /* Devices */
    /**/
    node = xmlNewChild(root_node, NULL, BAD_CAST "devices", NULL);

    /**/
    /* Disk */
    /**/
    sub_node = xmlNewChild(node, NULL, BAD_CAST "disk", NULL);
    xmlSetProp(sub_node, BAD_CAST "type", BAD_CAST "file");
    xmlSetProp(sub_node, BAD_CAST "device", BAD_CAST "disk");
    xmlNewChild(sub_node, NULL, BAD_CAST "driver", NULL);
    xmlSetProp(xmlGetLastChild(sub_node), BAD_CAST "name", BAD_CAST "qemu");
    xmlSetProp(xmlGetLastChild(sub_node), BAD_CAST "type", BAD_CAST "qcow2");
    xmlNewChild(sub_node, NULL, BAD_CAST "source", NULL);
    xmlSetProp(xmlGetLastChild(sub_node), BAD_CAST "file", BAD_CAST config->storage_path);
    xmlNewChild(sub_node, NULL, BAD_CAST "target", NULL);
    xmlSetProp(xmlGetLastChild(sub_node), BAD_CAST "dev", BAD_CAST "vda");
    xmlSetProp(xmlGetLastChild(sub_node), BAD_CAST "bus", BAD_CAST "virtio");

    /**/
    /* Graphics */
    /**/
    sub_node = xmlNewChild(node, NULL, BAD_CAST "graphics", NULL);
    xmlSetProp(sub_node, BAD_CAST "type", BAD_CAST "spice");
    xmlSetProp(sub_node, BAD_CAST "autoport", BAD_CAST "yes");


    /**/
    /* Dump XML to buffer */
    /**/
    xmlBufferPtr buf = xmlBufferCreate();
    xmlNodeDump(buf, doc, root_node, 0, 1);
    *xml_buffer = (char *)xmlBufferContent(buf);
    /* Do not free buf here, the content is pointed to by *xml_buffer */

    xmlFreeDoc(doc);
    xmlCleanupParser();

    return 0;
}
