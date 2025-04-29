#include <libxml/parser.h>
#include <libxml/tree.h>
#include <stdio.h>
#include "binaryReader.h"
#include "xmlWriter.h"

// Helper: event_code'dan hexBig ve hexLittle endian string üret
void format_event_code(int event_code, char *hexBig, char *hexLittle, char *convertedDecimal) {
    // 32 bit tam sayı gibi ele al
    unsigned int value = (unsigned int)event_code;

    // hexBig = 8 karakterlik büyük endian formatı
    sprintf(hexBig, "%08X", value);

    // hexLittle = küçük endian formata çevirmek gerek
    unsigned int b0 = (value & 0xFF000000) >> 24;
    unsigned int b1 = (value & 0x00FF0000) >> 8;
    unsigned int b2 = (value & 0x0000FF00) << 8;
    unsigned int b3 = (value & 0x000000FF) << 24;
    unsigned int littleEndian = b0 | b1 | b2 | b3;

    sprintf(hexLittle, "%08X", littleEndian);

    // convertedDecimal = littleEndian'ın decimal hali
    sprintf(convertedDecimal, "%u", littleEndian);
}


void create_xml(const Record *records, int record_count, const char *xml_filename) {
    xmlDocPtr doc = NULL;
    xmlNodePtr root_node = NULL;

    // 1. XML dokümanı oluştur
    doc = xmlNewDoc(BAD_CAST "1.0");
    root_node = xmlNewNode(NULL, BAD_CAST "smartlogs");
    xmlDocSetRootElement(doc, root_node);

    // 2. Her kayıt için <entry>
    for (int i = 0; i < record_count; i++) {
        char id_str[10];
        sprintf(id_str, "%d", i + 1);

        xmlNodePtr entry = xmlNewChild(root_node, NULL, BAD_CAST "entry", NULL);
        xmlNewProp(entry, BAD_CAST "id", BAD_CAST id_str);

        // <device> bloğu
        xmlNodePtr device_node = xmlNewChild(entry, NULL, BAD_CAST "device", NULL);
        xmlNewChild(device_node, NULL, BAD_CAST "device_id", BAD_CAST records[i].device_id);
        xmlNewChild(device_node, NULL, BAD_CAST "location", BAD_CAST records[i].location);
        xmlNewChild(device_node, NULL, BAD_CAST "firmware_ver", BAD_CAST records[i].firmware_ver);

        // <metrics> bloğu
        xmlNodePtr metrics_node = xmlNewChild(entry, NULL, BAD_CAST "metrics", NULL);
        xmlNewProp(metrics_node, BAD_CAST "status", BAD_CAST records[i].status);
        xmlNewProp(metrics_node, BAD_CAST "alert_level", BAD_CAST records[i].alert_level);

        char temp_str[20];
        sprintf(temp_str, "%.2f", records[i].temperature);
        xmlNewChild(metrics_node, NULL, BAD_CAST "temperature", BAD_CAST temp_str);

        char hum_str[10];
        sprintf(hum_str, "%d", records[i].humidity);
        xmlNewChild(metrics_node, NULL, BAD_CAST "humidity", BAD_CAST hum_str);

        char batt_str[10];
        sprintf(batt_str, "%d", records[i].battery);
        xmlNewChild(metrics_node, NULL, BAD_CAST "battery", BAD_CAST batt_str);

        // <timestamp>
        xmlNewChild(entry, NULL, BAD_CAST "timestamp", BAD_CAST records[i].timestamp);

        // <event_code>
        char eventCode_str[12];
        sprintf(eventCode_str, "%d", records[i].event_code);

        char hexBig[12], hexLittle[12], convertedDecimal[12];
        format_event_code(records[i].event_code, hexBig, hexLittle, convertedDecimal);

        xmlNodePtr event_node = xmlNewChild(entry, NULL, BAD_CAST "event_code", BAD_CAST eventCode_str);
        xmlNewProp(event_node, BAD_CAST "hexBig", BAD_CAST hexBig);
        xmlNewProp(event_node, BAD_CAST "hexLittle", BAD_CAST hexLittle);
        xmlNewProp(event_node, BAD_CAST "decimal", BAD_CAST convertedDecimal);
    }

    // 3. XML dosyasını kaydet
    xmlSaveFormatFileEnc(xml_filename, doc, "UTF-8", 1);

    // 4. Temizlik
    xmlFreeDoc(doc);
    xmlCleanupParser();
}
