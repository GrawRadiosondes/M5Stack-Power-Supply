#include <Arduino.h>
#include <M5GFX.h>
#include <M5ModulePPS.h>
#include <M5Unified.hpp>

M5GFX display;
M5Canvas canvas(&display);

std::array<uint8_t, 2> module_adr{MODULE_POWER_ADDR,MODULE_POWER_ADDR + 1};
std::array<M5ModulePPS, module_adr.size()> pps;
std::array<bool, module_adr.size()> module_found{};

void init_display()
{
	display.begin();
	display.setEpdMode(epd_fastest);
	canvas.setColorDepth(8);
	canvas.setFont(&efontCN_12);
	canvas.createSprite(display.width(), display.height());
	canvas.setTextSize(2);
	canvas.setTextScroll(true);
}

void setup()
{
	M5.begin();
	init_display();

	for (int i = 0; i < module_adr.size(); i++)
		module_found[i] = pps[i].begin(&Wire, M5.In_I2C.getSDA(), M5.In_I2C.getSCL(), module_adr[i], 400000U);

	pps[0].setOutputVoltage(5.0);
	pps[0].setOutputCurrent(0.5);
	pps[0].setPowerEnable(true);
}

void loop()
{
	const float voltage_readback = pps[0].getReadbackVoltage();
	const float current_readback = pps[0].getReadbackCurrent();
	//float vin = pps[0].getVIN();
	//int mode = pps[0].getMode();

	canvas.printf("%.3fV %fmA\r\n", voltage_readback, current_readback * 1000);
	canvas.pushSprite(0, 0);
	delay(500);
}
