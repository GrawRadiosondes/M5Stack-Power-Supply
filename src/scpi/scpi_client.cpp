//
// Created by TGA on 13.06.25.
//

#include "scpi_client.hpp"

#include <Arduino.h>
#include <array>
#include <scpi/scpi.h>

namespace scpi
{
	std::array<char, 256> scpi_input_buffer;
	std::array<scpi_error_t, 17> scpi_error_queue_data;
	scpi_t scpi_context;

	size_t write_callback(scpi_t *, const char *data, const size_t len)
	{
		Serial.print(data);
		return len;
	}

	scpi_result_t flush_callback(scpi_t *)
	{
		Serial.flush();
		return SCPI_RES_ERR;
	}

	int error_callback(scpi_t *, const int_fast16_t err)
	{
          //todo show on screen
		/*if (err)
		{
			Serial.print("SCPI Error: ");
			Serial.print(err);
			Serial.print(", ");
			Serial.println(SCPI_ErrorTranslate(static_cast<int16_t>(err)));
		}*/
		return 0;
	}

	scpi_interface_t scpi_interface = {
		.error = error_callback,
		.write = write_callback,
		.control = nullptr,
		.flush = flush_callback,
		.reset = reset_callback,
	};

	// assumes Serial is already started
	void begin(const char *serialNum, const char *swVersion, const char *device_type)
	{
		SCPI_Init(&scpi_context, scpi_commands, &scpi_interface, scpi_units_def, "Graw Radiosondes", device_type,
		          serialNum, swVersion, scpi_input_buffer.data(), scpi_input_buffer.size(),
		          scpi_error_queue_data.data(), scpi_error_queue_data.size());
	}

	void loop()
	{
		while (Serial.available() > 0)
		{
			const char c = Serial.read();
			SCPI_Input(&scpi_context, &c, 1);
		}
	}
} // namespace scpi

// error reporting for other source files
void report_error(const int error_num, const char *text)
{
	SCPI_ErrorPushEx(&scpi::scpi_context, static_cast<int16_t>(error_num), const_cast<char *>(text), 0);
}
