#include <Arduino.h>
#include <M5GFX.h>
#include <M5Unified.hpp>

#include "main.hpp"
#include "channel.hpp"
#include "scpi/scpi_client.hpp"

M5GFX display;
M5Canvas canvas(&display);

char serial_num_str[18]{};

bool beeper_active{true};
char display_text[64]{};

void init_display();

void draw_display_text();

void setup()
{
	Serial.begin(115200);
	Serial.flush();
	Serial.setDebugOutput(false);
	// uncomment this line for debug output on boot
	//while (!Serial) {}
	delay(100);

	//read serial number
	const uint64_t chip_id = ESP.getEfuseMac();
	snprintf(serial_num_str, sizeof serial_num_str, "%012llX", chip_id);

	// start scpi interface
	scpi::begin(serial_num_str, "1.0.0", "M5-PSU 2");

	M5.begin();
	init_display();

	M5.Speaker.setAllChannelVolume(255);
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
		canvas.clear();
		last_display_refresh = millis();
		//Channel seperator line
		canvas.drawLine(0, 120, 320, 120, TFT_WHITE);
		//Channels
		channels[0].draw("CH1");
		channels[1].draw("CH2");
		//text box
		draw_display_text();

		canvas.pushSprite(0, 0);
	}
}

void beep()
{
	if (beeper_active)
		M5.Speaker.tone(2000, 500);
}

void init_display()
{
	display.begin();
	display.setEpdMode(epd_fastest);
	canvas.setColorDepth(8);
	canvas.createSprite(display.width(), display.height());
	canvas.setTextSize(2);
}

void draw_display_text()
{
	if (display_text[0] == '\0')
		return;
	canvas.fillRect(20, 50, 280, 140, TFT_DARKGREY);

	canvas.setTextColor(TFT_WHITE);
	canvas.setFont(&efontCN_12);
	canvas.setCursor(40, 130);
	canvas.print(display_text);
}
