#ifndef FAT32_H
#define FAT32_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint8_t  jump[3];
    char     oem[8];
    uint16_t bytes_per_sector;
    uint8_t  sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t  fat_count;
    uint16_t root_entries;
    uint16_t total_sectors16;
    uint8_t  media_type;
    uint16_t sectors_per_fat16;
    uint16_t sectors_per_track;
    uint16_t head_count;
    uint32_t hidden_sectors;
    uint32_t total_sectors32;
    uint32_t sectors_per_fat;
    uint16_t flags;
    uint16_t version;
    uint32_t root_cluster;
    uint16_t fs_info_sector;
    uint16_t backup_boot_sector;
    uint8_t  reserved[12];
    uint8_t  drive_number;
    uint8_t  nt_flags;
    uint8_t  signature;
    uint32_t volume_id;
    char     volume_label[11];
    char     fs_type[8];
} __attribute__((packed)) fat32_boot_sector;

typedef struct {
    char     name[11];
    uint8_t  attr;
    uint8_t  nt_reserved;
    uint8_t  creation_time_tenth;
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t last_access_date;
    uint16_t cluster_high;
    uint16_t modified_time;
    uint16_t modified_date;
    uint16_t cluster_low;
    uint32_t file_size;
} __attribute__((packed)) fat32_dir_entry;

typedef struct {
    uint32_t cluster;
    uint32_t size;
    uint32_t position;
    uint32_t current_cluster;
    uint32_t current_sector;
    uint32_t sector_offset;
} fat32_file;

bool fat32_init(uint8_t drive);
bool fat32_open(const char* path, fat32_file* file);
bool fat32_read(fat32_file* file, void* buffer, uint32_t size);
bool fat32_close(fat32_file* file);

#endif