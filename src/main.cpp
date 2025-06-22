#include <Arduino.h>
#include <M5GFX.h>
#include <M5Unified.hpp>

#include "main.hpp"
#include "channel.hpp"

M5GFX display;
M5Canvas canvas(&display);


void init_display()
{
	display.begin();
	display.setEpdMode(epd_fastest);
	canvas.setColorDepth(8);
	canvas.createSprite(display.width(), display.height());
	canvas.setTextSize(2);

	//Channel seperator line
	canvas.drawLine(0, 120, 320, 120, TFT_WHITE);
}

void setup()
{
	M5.begin();
	init_display();

	channels[0].set_voltage(5.0);
	channels[0].set_current(0.5);
	channels[0].set_enabled(true);
}

void loop()
{
	channels[0].loop();
	channels[1].loop();
	canvas.pushSprite(0, 0);
	delay(200);
}
