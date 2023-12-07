#pragma once

#include <stdint.h>

namespace pic {
    void init();

    void set_mask(uint8_t irq);

    void clear_mask(uint8_t irq);

    void send_eoi(uint8_t irq);

    uint8_t get_interrupt_vector(uint8_t irq);
}
