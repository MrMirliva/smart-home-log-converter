#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libxml/xmlschemas.h> 

//
#include "setupParams.h"
#include "binaryReader.h"
#include "sort_records.h"
#include "xmlWriter.h"

// Global değişkenler
int keyStart = 0;
int keyEnd = 7;
int orderAsc = 1;

// Fonksiyon prototipleri
void csv_to_binary(const char* input_csv, const char* output_binary, int separator, int opsys);
void binary_to_xml(const char* output_xml, int separator, int opsys);
void validate_xml(const char* input_xml, const char* xsd_file);
void display_help();

// csv_to_binary fonksiyonunda kullanılacak küçük yardımcı fonksiyonlar
char get_separator(int separator_arg);
void trim_newline(char *str);
void parse_csv_line(char *line, Record *record, char separator);

// gcc main.c setupParams.c binaryReader.c sort_records.c compare_records.c xmlWriter.c -I/usr/include/libxml2 -ljson-c -lxml2 -o deviceTool

int main(int argc, char *argv[]) {
    if (argc < 6) {
        printf("Eksik argüman!n");
        display_help();
        return 1;
    }

    // Argümanları oku
    char* input_file = argv[1];
    char* output_file = argv[2];
    int conversion_type = atoi(argv[3]);
    int separator = -1;
    int opsys = -1;

    for (int i = 4; i < argc; i++) {
        if (strcmp(argv[i], "-separator") == 0 && (i + 1) < argc) {
            separator = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-opsys") == 0 && (i + 1) < argc) {
            opsys = atoi(argv[++i]);
        }
    }

    if (separator == -1 || opsys == -1) {
        printf("Hata: -separator ve -opsys parametreleri eksik!\n");
        return 1;
    }

    // İşlem tipi seçimi
    switch (conversion_type) {
        case 1:
            csv_to_binary(input_file, output_file, separator, opsys);
            break;
        case 2:
            binary_to_xml(output_file, separator, opsys); // output XML dosyası
            break;
        case 3:
            validate_xml(input_file, output_file);
            break;
        default:
            printf("Hata: Geçersiz conversion_type! (1, 2 veya 3 olmalı)\n");
            return 1;
    }

    return 0;
}

// CSV to Binary fonksiyonu
void csv_to_binary(const char* input_csv, const char* output_binary, int separator_arg, int opsys_arg) {
    char separator = get_separator(separator_arg);
    if (separator == '\0') {
        printf("Geçersiz separator değeri! 1, 2 veya 3 olmalı.\n");
        return;
    }

    SetupParams params;
    if (readSetupParams("setupParams.json", &params)) {
        printf("\n✅ setupParams.json dosyasından okunan değerler:\n");
        printf("Dosya Adı: %s\n", params.dataFileName);
        printf("Key Start: %d\n", params.keyStart);
        printf("Key End: %d\n", params.keyEnd);
        printf("Order: %s\n", params.order);

        keyStart = params.keyStart;
        keyEnd = params.keyEnd;
        orderAsc = (strcmp(params.order, "ASC") == 0) ? 1 : 0;

    } else {
        printf("❌ setupParams.json dosyası okunamadı!\n");
        return;
    }

    // CSV dosyasını oku
    FILE *csv = fopen(input_csv, "r");
    if (!csv) {
        perror("CSV dosyası açılamadı");
        return;
    }

    // Binary dosyasını aç
    FILE *bin = fopen(output_binary, "wb");
    if (!bin) {
        perror("Binary dosyası açılamadı");
        fclose(csv);
        return;
    }

    char line[512]; // Bir satır için buffer
    fgets(line, sizeof(line), csv); // İlk satır başlık, atla

    while (fgets(line, sizeof(line), csv)) {
        trim_newline(line);

        Record rec;
        memset(&rec, 0, sizeof(Record));

        parse_csv_line(line, &rec, separator);

        fwrite(&rec, sizeof(Record), 1, bin);
    }

    fclose(csv);
    fclose(bin);

    printf("\n✅ CSV'den binary dosyaya başarıyla dönüştürüldü.\n");
}

// Binary to XML fonksiyonu
void binary_to_xml(const char* output_xml, int separator, int opsys) {
    // 1. SetupParams.json dosyasını oku
    SetupParams params;
    if (!readSetupParams("setupParams.json", &params)) {
        printf("❌ setupParams.json dosyası okunamadı!\n");
        return;
    }
    keyStart = params.keyStart;
    keyEnd = params.keyEnd;
    orderAsc = (strcmp(params.order, "ASC") == 0) ? 1 : 0;

    // 2. Binary dosyasını oku
    int record_count = 0;
    Record* records = read_binary_file("logdata.dat", &record_count);
    if (records == NULL) {
        printf("❌ Binary dosyadan veri okunamadı!\n");
        return;
    }

    printf("✅ Toplam %d kayıt yüklendi!\n", record_count);

    // 3. RAM'deki kayıtları sırala
    sort_records(records, record_count);
    printf("✅ Kayıtlar sıralandı!\n");

    // 4. Sıralı kayıtları XML dosyasına yaz
    create_xml(records, record_count, output_xml);
    printf("✅ XML dosyası oluşturuldu: %s\n", output_xml);

    // 5. Belleği temizle
    free(records);
}

// XML Validation fonksiyonu
void validate_xml(const char* input_xml, const char* xsd_file) {
    xmlDocPtr doc;
    xmlSchemaPtr schema = NULL;
    xmlSchemaParserCtxtPtr ctxt;

    // XML dosyasını oku
    doc = xmlReadFile(input_xml, NULL, 0);
    if (doc == NULL) {
        printf("❌ XML dosyası okunamadı: %s\n", input_xml);
        return;
    }

    // XSD dosyasını oku
    ctxt = xmlSchemaNewParserCtxt(xsd_file);
    if (ctxt == NULL) {
        printf("❌ XSD dosyası açılamadı: %s\n", xsd_file);
        xmlFreeDoc(doc);
        return;
    }

    schema = xmlSchemaParse(ctxt);
    xmlSchemaFreeParserCtxt(ctxt);

    if (schema == NULL) {
        printf("❌ XSD şeması parse edilemedi.\n");
        xmlFreeDoc(doc);
        return;
    }

    xmlSchemaValidCtxtPtr valid_ctxt = xmlSchemaNewValidCtxt(schema);
    if (valid_ctxt == NULL) {
        printf("❌ Validation context oluşturulamadı.\n");
        xmlSchemaFree(schema);
        xmlFreeDoc(doc);
        return;
    }

    // Validate işlemi
    int ret = xmlSchemaValidateDoc(valid_ctxt, doc);
    if (ret == 0) {
        printf("✅ XML dosyası başarıyla doğrulandı!\n");
    } else if (ret > 0) {
        printf("❌ XML dosyası şemaya uygun DEĞİL!\n");
    } else {
        printf("❌ Validation işlemi sırasında hata oluştu.\n");
    }

    // Bellek temizliği
    xmlSchemaFreeValidCtxt(valid_ctxt);
    xmlSchemaFree(schema);
    xmlFreeDoc(doc);
    xmlCleanupParser();
}

// separator değerine göre ayırıcı döndürür
char get_separator(int separator_arg) {
    switch (separator_arg) {
        case 1: return ','; // Virgül
        case 2: return '\t'; // Tab
        case 3: return ';';  // Noktalı virgül
        default: return '\0';
    }
}

// Satır sonlarındaki '\n' veya '\r' karakterlerini temizler
void trim_newline(char *str) {
    int len = strlen(str);
    while (len > 0 && (str[len-1] == '\n' || str[len-1] == '\r')) {
        str[--len] = '\0';
    }
}

// CSV satırını Record yapısına parse eder
void parse_csv_line(char *line, Record *record, char separator) {
    char *token;
    int field = 0;

    token = strtok(line, &separator);
    while (token != NULL) {
        switch (field) {
            case 0: strncpy(record->device_id, token, sizeof(record->device_id)-1); break;
            case 1: strncpy(record->timestamp, token, sizeof(record->timestamp)-1); break;
            case 2: record->temperature = atof(token); break;
            case 3: record->humidity = atoi(token); break;
            case 4: record->status = token[0]; break;
            case 5: strncpy(record->location, token, sizeof(record->location)-1); break;
            case 6: strncpy(record->alert_level, token, sizeof(record->alert_level)-1); break;
            case 7: record->battery = atoi(token); break;
            case 8: strncpy(record->firmware_ver, token, sizeof(record->firmware_ver)-1); break;
            case 9: record->event_code = atoi(token); break;
        }
        field++;
        token = strtok(NULL, &separator);
    }
}

// Yardım mesajı fonksiyonu
void display_help() {
    printf("\n# Usage:\n");
    printf("./deviceTool <input_file> <output_file> <conversion_type> -separator <1|2|3> -opsys <1|2|3> [-h]\n");
    printf("\n# Arguments:\n");
    printf("<input_file>   = Kaynak dosya\n");
    printf("<output_file>  = Hedef dosya\n");
    printf("<conversion_type> = 1: CSV->Binary, 2: Binary->XML, 3: XML Validation\n");
    printf("-separator <1|2|3>  = Alan ayırıcı (1=virgül, 2=tab, 3=semicolon)\n");
    printf("-opsys <1|2|3>      = Satır sonu karakteri (1=Windows, 2=Linux, 3=MacOS)\n");
    printf("\n# Examples:\n");
    printf("./deviceTool smartlogs.csv logdata.dat 1 -separator 1 -opsys 2\n");
    printf("./deviceTool smartlogs.dat smartlogs.xml 2 -separator 1 -opsys 2\n");
    printf("./deviceTool smartlogs.xml smartlogs.xsd 3 -separator 1 -opsys 2\n");
}
