#define _POSIX_C_SOURCE 200809L
#include "xml_parser.h"
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <string.h>
#include <stdlib.h>

static char* get_xpath_content(xmlDocPtr doc, const xmlChar* xpath_expr) {
    xmlXPathContextPtr context = xmlXPathNewContext(doc);
    xmlXPathObjectPtr result = xmlXPathEvalExpression(xpath_expr, context);
    xmlXPathFreeContext(context);

    if (!result || xmlXPathNodeSetIsEmpty(result->nodesetval)) {
        if (result) xmlXPathFreeObject(result);
        return NULL;
    }

    xmlNodeSetPtr nodeset = result->nodesetval;
    xmlChar *content = xmlNodeGetContent(nodeset->nodeTab[0]);
    char *ret = strdup((char*)content);

    xmlFree(content);
    xmlXPathFreeObject(result);
    return ret;
}

int parse_xml_buffer(const char *xml_buffer, VMConfig *config) {
    xmlDocPtr doc = xmlReadMemory(xml_buffer, strlen(xml_buffer), "noname.xml", NULL, 0);
    if (doc == NULL) {
        return -1;
    }

    init_vm_config(config); /* Initialize with defaults first */

    char *content;

    content = get_xpath_content(doc, (const xmlChar*)"/domain/name");
    if (content) { strncpy(config->name, content, sizeof(config->name) - 1); free(content); }

    content = get_xpath_content(doc, (const xmlChar*)"/domain/vcpu");
    if (content) { config->cpus = atoi(content); free(content); }

    content = get_xpath_content(doc, (const xmlChar*)"/domain/memory");
    if (content) { config->memory = atoi(content) / 1024; free(content); }

    content = get_xpath_content(doc, (const xmlChar*)"/domain/devices/disk/source/@file");
    if (content) { strncpy(config->storage_path, content, sizeof(config->storage_path) - 1); free(content); }
    
    content = get_xpath_content(doc, (const xmlChar*)"/domain/os/loader[@type='pflash']");
    if (content) {
        strcpy(config->firmware, "uefi");
        free(content);
    } else {
        strcpy(config->firmware, "bios");
    }

    xmlFreeDoc(doc);
    xmlCleanupParser();
    return 0;
}

int parse_xml_file(const char *filepath, VMConfig *config) {
    xmlDocPtr doc = xmlReadFile(filepath, NULL, 0);
    if (doc == NULL) {
        return -1;
    }

    /* For simplicity, we can just get the buffer and reuse the other function */
    xmlChar *buf;
    int size;
    xmlDocDumpMemory(doc, &buf, &size);
    
    int result = parse_xml_buffer((const char*)buf, config);

    xmlFree(buf);
    xmlFreeDoc(doc);

    return result;
}
