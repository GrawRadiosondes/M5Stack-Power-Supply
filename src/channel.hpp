//
// Created by TGA on 22.06.25.
//

#pragma once

#include <M5ModulePPS.h>

class Channel
{
	uint8_t module_adr;
	bool connected{false};
	M5ModulePPS module{};
	uint8_t display_offset;

	//settings
	float voltage_target{0.0};
	float current_target{0.0};
	bool enabled{false};

	//measurements
	float voltage_is{0.0};
	float current_is{0.0};
	bool in_cc_mode{false};

public:
	constexpr float max_voltage {12.0};
	constexpr float max_current {5.0};

	Channel(const uint8_t module_adr, const uint8_t display_offset): module_adr(module_adr),
																	 display_offset(display_offset) {}

	//refresh measurement data
	void loop();

	//refresh gui output
	void draw() const;

	void set_voltage(float voltage);

	void set_current(float current);

	void set_enabled(bool in);

	void set_address(uint8_t addr);

	void reset();
};

extern std::array<Channel, 2> channels;
