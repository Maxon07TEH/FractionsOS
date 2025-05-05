#include "nvme.h"
#include "pci.h"
#include "mmio.h"
#include "memory.h"

#define NVME_REG_CAP 0x00
#define NVME_REG_VS 0x08
#define NVME_REG_INTMS 0x0C
#define NVME_REG_SQ0TDBL 0x1000

static uint32_t* nvme_regs;
static uint64_t admin_queue_phys;
static nvme_command* admin_sq;
static nvme_completion* admin_cq;

void nvme_init() {
    // Поиск NVMe контроллера через PCI
    pci_device_t dev = pci_find_class(0x010802);
    if(dev.vendor_id == 0xFFFF) return;
    
    // Включение MMIO
    nvme_regs = (uint32_t*)pci_map_bar(dev, 0);
    
    // Инициализация админских очередей
    admin_sq = (nvme_command*)AllocateAligned(4096, 4096);
    admin_cq = (nvme_completion*)AllocateAligned(4096, 4096);
    admin_queue_phys = (uint64_t)VirtualToPhysical(admin_sq);
    
    // Настройка регистров контроллера
    mmio_write32(nvme_regs + NVME_REG_SQ0TDBL, 0);
    // ... полная инициализация ...
}

bool nvme_read(uint64_t lba, uint32_t count, void* buffer) {
    nvme_command cmd = {
        .prp1 = (uint64_t)buffer,
        .cmd_specific = {count, (uint32_t)lba, (uint32_t)(lba >> 32)}
    };
    
    // Добавление команды в очередь
    admin_sq[0] = cmd;
    mmio_write32(nvme_regs + NVME_REG_SQ0TDBL, 1);
    
    // Ожидание завершения
    while(admin_cq[0].status == 0xFFFF);
    return (admin_cq[0].status & 0x1) == 0;
}