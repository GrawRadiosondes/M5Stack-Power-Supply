//
// Created by TGA on 13.06.2025.
//

#pragma once
#include <scpi/scpi.h>

// must be implemented externally and contain the supported scpi commands
extern const scpi_command_t scpi_commands[];
scpi_result_t reset_callback(scpi_t *context);

namespace scpi {

void begin(const char *serialNum, const char *swVersion, const char *device_type);

void loop();
} // namespace scpi