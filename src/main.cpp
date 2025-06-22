#include <Arduino.h>
#include <M5GFX.h>
#include <M5Unified.hpp>

#include "main.hpp"
#include "channel.hpp"
#include "scpi/scpi_client.hpp"

M5GFX display;
M5Canvas canvas(&display);

char serial_num_str[18]{};


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
	Serial.begin(921600);
	Serial.flush();
	Serial.setDebugOutput(false);
	// uncomment this line for debug output on boot
	//while (!Serial) {}
	delay(100);

	//read serial number
	const uint64_t chip_id = ESP.getEfuseMac();
	snprintf(serial_num_str, sizeof serial_num_str, "%012llX", chip_id);

	// start scpi interface
	scpi::begin(serial_num_str, "1.0.0", "PSU 2");

	M5.begin();
	init_display();

	channels[0].set_voltage(8);
	channels[0].set_current(0.5);
	channels[0].set_enabled(true);
}

void loop()
{
	scpi::loop();

	// poll measurements
	channels[0].loop();
	channels[1].loop();

	//refresh displayed data at 4 HZ
	//don't use a timer here because scpi commands should take priority over this
	static unsigned long last_display_refresh{};
	if (millis() - last_display_refresh > 250)
	{
		last_display_refresh = millis();
		channels[0].draw();
		channels[1].draw();
		canvas.pushSprite(0, 0);
	}
}
