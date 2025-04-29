// setupParams.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <json-c/json.h>
#include "setupParams.h"

// setupParams.c
// JSON dosyasını oku ve SetupParams yapısını doldur

int readSetupParams(const char *filename, SetupParams *params) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("JSON dosyası açılamadı");
        return 0;
    }

    fseek(fp, 0, SEEK_END);
    long fsize = ftell(fp);
    rewind(fp);

    char *buffer = (char *)malloc(fsize + 1);
    fread(buffer, 1, fsize, fp);
    buffer[fsize] = '\0';
    fclose(fp);

    struct json_object *parsed_json;
    parsed_json = json_tokener_parse(buffer);

    if (!parsed_json) {
        fprintf(stderr, "JSON parse hatası!\n");
        free(buffer);
        return 0;
    }

    struct json_object *dataFileNameObj;
    struct json_object *keyStartObj;
    struct json_object *keyEndObj;
    struct json_object *orderObj;

    if (json_object_object_get_ex(parsed_json, "dataFileName", &dataFileNameObj))
        strncpy(params->dataFileName, json_object_get_string(dataFileNameObj), sizeof(params->dataFileName)-1);

    if (json_object_object_get_ex(parsed_json, "keyStart", &keyStartObj))
        params->keyStart = json_object_get_int(keyStartObj);

    if (json_object_object_get_ex(parsed_json, "keyEnd", &keyEndObj))
        params->keyEnd = json_object_get_int(keyEndObj);

    if (json_object_object_get_ex(parsed_json, "order", &orderObj))
        strncpy(params->order, json_object_get_string(orderObj), sizeof(params->order)-1);

    json_object_put(parsed_json);
    free(buffer);

    return 1;
}
