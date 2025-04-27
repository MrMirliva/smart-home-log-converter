#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "setupParams.h"
#include "binaryReader.h"
#include "sort_records.h"
#include "xmlWriter.h"

int keyStart = 0;
int keyEnd = 7;
int orderAsc = 1;

// Fonksiyon prototipleri
char get_separator(int separator_arg);
void trim_newline(char *str);
void parse_csv_line(char *line, Record *record, char separator);

// gcc main.c setupParams.c binaryReader.c sort_records.c compare_records.c xmlWriter.c -I/usr/include/libxml2 -ljson-c -lxml2 -o converter
// ./converter -separator 1 -opsys 2

int main(int argc, char *argv[]) {
    if (argc < 5) {
        printf("Kullanım: %s -separator <1|2|3> -opsys <1|2|3>\n", argv[0]);
        return 1;
    }

    int separator_arg = 0;
    int opsys_arg = 0;

    // Komut satırı argümanlarını parse et
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-separator") == 0 && i + 1 < argc) {
            separator_arg = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-opsys") == 0 && i + 1 < argc) {
            opsys_arg = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-h") == 0) {
            printf("Kullanım: %s -separator <1=virgul,2=tab,3=noktalı-virgul> -opsys <1=windows,2=linux,3=macos>\n", argv[0]);
            return 0;
        }
    }

    char separator = get_separator(separator_arg);
    if (separator == '\0') {
        printf("Geçersiz separator değeri! 1, 2 veya 3 olmalı.\n");
        return 1;
    }

    
    SetupParams params;
    if (readSetupParams("setupParams.json", &params)) {
        printf("\n✅ setupParams.json dosyasından okunan değerler:\n");
        printf("Dosya Adı: %s\n", params.dataFileName);
        printf("Key Start: %d\n", params.keyStart);
        printf("Key End: %d\n", params.keyEnd);
        printf("Order: %s\n", params.order);

        // <<< BURAYA EKLİYORSUN >>>
        keyStart = params.keyStart;
        keyEnd = params.keyEnd;
        orderAsc = (strcmp(params.order, "ASC") == 0) ? 1 : 0;

    } else {
        printf("❌ setupParams.json dosyası okunamadı!\n");
    }


    // CSV dosyasını oku
    FILE *csv = fopen("smartlogs.csv", "r");
    if (!csv) {
        perror("CSV dosyası açılamadı");
        return 1;
    }

    // Binary dosyasını aç
    FILE *bin = fopen("logdata.dat", "wb");
    if (!bin) {
        perror("Binary dosyası açılamadı");
        fclose(csv);
        return 1;
    }

    char line[512]; // Bir satır için buffer
    fgets(line, sizeof(line), csv); // İlk satır başlık, atla

    while (fgets(line, sizeof(line), csv)) {
        //printf("Satır okundu: %s\n", line);
        trim_newline(line);

        Record rec;
        memset(&rec, 0, sizeof(Record));

        parse_csv_line(line, &rec, separator);

        fwrite(&rec, sizeof(Record), 1, bin);
    }

    fclose(csv);
    fclose(bin);

    printf("\n✅ CSV'den binary dosyaya başarıyla dönüştürüldü.\n");

    // Binary dosyasını oku ve belleğe al
    int record_count = 0;
    Record *records = read_binary_file("logdata.dat", &record_count);

    if (records == NULL) {
        printf("❌ Binary dosyadan veri okunamadı!\n");
        return 1;
    }

    printf("✅ Toplam %d kayıt yüklendi!\n", record_count);

    // <<< BURADA SIRALAMA YAPILMALI >>>
    sort_records(records, record_count);
    printf("✅ Kayıtlar sıralandı!\n");

    // <<< BURADA XML DOSYASI OLUŞTURULMALI >>>
    create_xml(records, record_count, "logdata.xml");
    printf("✅ XML dosyası oluşturuldu: logdata.xml\n");

    // Sonunda belleği temizle
    free(records);

    return 0;
}

// Separator değerine göre ayırıcı karakter döndürür
char get_separator(int separator_arg) {
    switch (separator_arg) {
        case 1: return ','; // Virgül
        case 2: return '\t'; // Tab
        case 3: return ';';  // Noktalı virgül
        default: return '\0';
    }
}

// Satır sonlarındaki '\n', '\r' karakterlerini temizler
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
