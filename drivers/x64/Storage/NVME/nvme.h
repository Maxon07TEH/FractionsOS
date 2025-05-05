#ifndef NVME_H
#define NVME_H

#include <stdint.h>

typedef struct {
    uint64_t prp1;
    uint64_t prp2;
    uint32_t cmd_specific[2];
} nvme_command;

typedef struct {
    uint32_t result;
    uint32_t rsvd;
    uint16_t sq_head;
    uint16_t sq_id;
    uint16_t cid;
    uint16_t status;
} nvme_completion;

typedef struct {
    uint32_t vid;
    uint32_t ssvid;
    char     serial[20];
    char     model[40];
    uint64_t max_data_transfer;
    uint32_t controller_id;
    // ... другие поля ...
} nvme_controller_info;

void nvme_init();
bool nvme_identify(nvme_controller_info* info);
bool nvme_read(uint64_t lba, uint32_t count, void* buffer);
bool nvme_write(uint64_t lba, uint32_t count, void* buffer);

#endif