//
// Created by TGA on 22.06.25.
//

#include <M5Unified.hpp>

#include "channel.hpp"
#include "main.hpp"

std::array<Channel, 2> channels{{{MODULE_POWER_ADDR, 0}, {MODULE_POWER_ADDR + 1, 120}}};

void Channel::loop()
{
	if (!connected)
	{
		connected = module.begin(&Wire, M5.In_I2C.getSDA(), M5.In_I2C.getSCL(), module_adr, 100000U);
		if (connected)
		{
			module.setPowerEnable(enabled);
			module.setOutputVoltage(voltage_target);
			module.setOutputCurrent(current_target);
		}
		return;
	}

	if (!enabled)
		return;

	voltage_is = module.getReadbackVoltage();
	current_is = module.getReadbackCurrent();
	in_cc_mode = !module.getMode();
}

void Channel::set_voltage(const float voltage)
{
	voltage_target = voltage;
	if (connected)
		module.setOutputVoltage(voltage_target);
}

void Channel::set_current(const float current)
{
	current_target = current;
	if (connected)
		module.setOutputCurrent(current_target);
}

void Channel::set_enabled(const bool in)
{
	enabled = in;
	if (connected)
	{
		module.setPowerEnable(in);
		module.setOutputVoltage(voltage_target);
		module.setOutputCurrent(current_target);
	}
	if (!enabled)
	{
		voltage_is = 0;
		current_is = 0;
		in_cc_mode = false;
	}
}

void Channel::set_address(const uint8_t addr)
{
	if (connected)
		module.setI2CAddress(addr);
	connected = false;
}

void Channel::draw(const char* name) const
{
	//try to connect module if not already connected
	if (!connected)
	{
		canvas.setTextColor(TFT_RED, TFT_BLACK);
		canvas.setFont(&efontCN_16);
		canvas.setCursor(30, display_offset + 40);
		canvas.print("Module not found");
		return;
	}
	canvas.fillRect(30, display_offset + 40, 280, 30, TFT_BLACK);

	//channel Label
	canvas.setTextColor(TFT_YELLOW, TFT_BLACK);
	canvas.setFont(&efontCN_12);
	canvas.setCursor(16, display_offset + 8);
	canvas.print(name);

	//cv/cc mode
	canvas.setTextColor(in_cc_mode ? TFT_RED : TFT_YELLOW, TFT_BLACK);
	canvas.setCursor(80, display_offset + 8);
	if (enabled)
		canvas.print(in_cc_mode ? "CC" : "CV");
	else
		canvas.print("  ");

	//power
	canvas.setTextColor(enabled ? TFT_GREEN : TFT_YELLOW, TFT_BLACK);
	canvas.setCursor(170, display_offset + 8);
	const float power = enabled ? voltage_is * current_is : voltage_target * current_target;
	canvas.printf("%3.1f W ", power);

	//voltage measurement/setting
	canvas.setFont(&efontCN_16);
	canvas.setCursor(20, display_offset + 60);
	canvas.printf("%4.2f V ", enabled ? voltage_is : voltage_target);

	//current measurement/setting
	canvas.setCursor(170, display_offset + 60);
	canvas.printf("%4.1f mA  ", (enabled ? current_is : current_target) * 1000);
}

void Channel::reset()
{
	set_enabled(false);
	set_voltage(0);
	set_current(0);
}
