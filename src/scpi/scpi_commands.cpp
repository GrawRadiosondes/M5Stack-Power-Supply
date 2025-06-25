//
// Created by TGA on 13.06.25.
//

#include <Arduino.h>
#include <channel.hpp>
#include <main.hpp>
#include <scpi/scpi.h>

#include "scpi_client.hpp"

//index of the selected channel (0 or 1)
uint8_t selected_channel{};

//voltage step size
constexpr float voltage_step_default{1.0};
float voltage_step{voltage_step_default};

//current step size
constexpr float current_step_default{0.1};
float current_step{current_step_default};

// IEEE 488.2 Commands
scpi_result_t get_selftest(scpi_t *context);

//No Operation
scpi_result_t scpi_nop(scpi_t *context);

// System Commands
scpi_result_t set_beeper_state(scpi_t *context);

scpi_result_t get_beeper_state(scpi_t *context);

scpi_result_t beep_immediate(scpi_t *context);

// Display Commands
scpi_result_t set_display_text(scpi_t *context);

scpi_result_t display_text_clear(scpi_t *context);

scpi_result_t set_brightness(scpi_t *context);

scpi_result_t set_display_enabled(scpi_t *context);

//configuration commands
scpi_result_t set_instrument_select(scpi_t *context);

scpi_result_t get_instrument_select(scpi_t *context);

scpi_result_t set_voltage(scpi_t *context);

scpi_result_t get_voltage_setting(scpi_t *context);

scpi_result_t set_voltage_step(scpi_t *context);

scpi_result_t get_voltage_step(scpi_t *context);

scpi_result_t set_current(scpi_t *context);

scpi_result_t get_current_setting(scpi_t *context);

scpi_result_t set_current_step(scpi_t *context);

scpi_result_t get_current_step(scpi_t *context);

scpi_result_t apply(scpi_t *context);

scpi_result_t apply_query(scpi_t *context);

scpi_result_t set_channel_state(scpi_t *context);

scpi_result_t get_channel_state(scpi_t *context);

//Measurement Commands
scpi_result_t measure_voltage(scpi_t *context);

scpi_result_t measure_current(scpi_t *context);

scpi_result_t measure_power(scpi_t *context);

// setup helpers
scpi_result_t change_i2c_adr(scpi_t *context);

// clang-format off
const scpi_command_t scpi_commands[] = {
	// IEEE Mandated Commands (SCPI std V1999.0 4.1.1)
	{ .pattern = "*CLS", .callback = SCPI_CoreCls},
	{ .pattern = "*ESE", .callback = SCPI_CoreEse},
	{ .pattern = "*ESE?", .callback = SCPI_CoreEseQ},
	{ .pattern = "*ESR?", .callback = SCPI_CoreEsrQ},
	{ .pattern = "*IDN?", .callback = SCPI_CoreIdnQ},
	{ .pattern = "*OPC", .callback = SCPI_CoreOpc},
	{ .pattern = "*OPC?", .callback = SCPI_CoreOpcQ},
	{ .pattern = "*RST", .callback = SCPI_CoreRst},
	{ .pattern = "*SRE", .callback = SCPI_CoreSre},
	{ .pattern = "*SRE?", .callback = SCPI_CoreSreQ},
	{ .pattern = "*STB?", .callback = SCPI_CoreStbQ},
	{ .pattern = "*TST?", .callback = get_selftest},
	{ .pattern = "*WAI", .callback = SCPI_CoreWai},

	// Required SCPI commands (SCPI std V1999.0 4.2.1)
	{.pattern = "SYSTem:ERRor[:NEXT]?", .callback = SCPI_SystemErrorNextQ},
	{.pattern = "SYSTem:ERRor:COUNt?", .callback = SCPI_SystemErrorCountQ},
	{.pattern = "SYSTem:VERSion?", .callback = SCPI_SystemVersionQ},

	{.pattern = "STATus:PRESet", .callback = SCPI_StatusPreset},

	// System Commands
	{.pattern = "SYSTem:BEEPer:STATe", .callback = set_beeper_state},
	{.pattern = "SYSTem:BEEPer:STATe?", .callback = get_beeper_state},
	{.pattern = "SYSTem:BEEPer[:IMMediate]", .callback = beep_immediate},
	{.pattern = "SYSTem:LOCal", .callback = scpi_nop},
	{.pattern = "SYSTem:REMote", .callback = scpi_nop},
	{.pattern = "SYSTem:RWLock", .callback = scpi_nop},

	// Display Commands
	{.pattern = "DISPlay[:WINDow]:TEXT:CLEar", .callback = display_text_clear},
	{.pattern = "DISPlay[:WINDow]:TEXT[:DATA]", .callback = set_display_text},
	{.pattern = "DISPlay:BRIGhtness", .callback = set_brightness},
	{.pattern = "DISPlay:ENABle", .callback = set_display_enabled},

	//configuration commands
	{.pattern = "INSTrument[:SELect]", .callback = set_instrument_select},
	{.pattern = "INSTrument[:SELect]?", .callback = get_instrument_select},
	{.pattern = "INSTrument:NSELect", .callback = set_instrument_select},
	{.pattern = "INSTrument:NSELect?", .callback = get_instrument_select},

	{.pattern = "[SOURce]:VOLTage[:LEVel][:IMMediate][:AMPLitude]", .callback = set_voltage},
	{.pattern = "[SOURce]:VOLTage[:LEVel][:IMMediate][:AMPLitude]?", .callback = get_voltage_setting},
	{.pattern = "[SOURce]:VOLTage[:LEVel]:STEP[:INCRement]", .callback = set_voltage_step},
	{.pattern = "[SOURce]:VOLTage[:LEVel]:STEP[:INCRement]?", .callback = get_voltage_step},

	{.pattern = "[SOURce]:CURRent[:LEVel][:IMMediate][:AMPLitude]", .callback = set_current},
	{.pattern = "[SOURce]:CURRent[:LEVel][:IMMediate][:AMPLitude]?", .callback = get_current_setting},
	{.pattern = "[SOURce]:CURRent[:LEVel]:STEP[:INCRement]", .callback = set_current_step},
	{.pattern = "[SOURce]:CURRent[:LEVel]:STEP[:INCRement]?", .callback = get_current_step},

	{.pattern = "APPLy", .callback = apply},
	{.pattern = "APPLy?", .callback = apply_query},

	{.pattern = "OUTPut[:CHANnel][:STATe]", .callback = set_channel_state},
	{.pattern = "OUTPut[:CHANnel][:STATe]?", .callback = get_channel_state},

	//Measurement Commands
	{.pattern = "MEASure[:SCALar]:CURRent[:DC]?", .callback = measure_current},
	{.pattern = "MEASure[:SCALar]:VOLTage[:DC]?", .callback = measure_voltage},
	{.pattern = "MEASure[:SCALar]:POWer?", .callback = measure_power},

	{.pattern = "I2C:ADRess[:SET]", .callback = change_i2c_adr},

	SCPI_CMD_LIST_END
};
// clang-format on

//
// Implementations
//

scpi_result_t reset_callback(scpi_t *)
{
	selected_channel = 0;
	voltage_step = voltage_step_default;
	current_step = current_step_default;
	beeper_active = true;
	display_text[0] = 0;
	display.setBrightness(0xFF);
	channels[0].reset();
	channels[1].reset();
	return SCPI_RES_OK;
}

scpi_result_t get_selftest(scpi_t *context)
{
	uint32_t res{};
	if (!channels[0].is_connected())
		res |= (1 << 0);
	if (!channels[1].is_connected())
		res |= (1 << 1);

	SCPI_ResultUInt32(context, res);
	return SCPI_RES_OK;
}

scpi_result_t scpi_nop(scpi_t *)
{
	return SCPI_RES_OK;
}

scpi_result_t set_instrument_select(scpi_t *context)
{
	scpi_number_t number;
	constexpr scpi_choice_def_t special[] = {
		{"OUTPut1", 1}, {"OUT1", 1}, {"OUTPut2", 2}, {"OUT2", 2}, SCPI_CHOICE_LIST_END
	};

	if (!SCPI_ParamNumber(context, special, &number, true))
		return SCPI_RES_ERR;

	// don't allow any units
	if (number.unit != SCPI_UNIT_NONE)
	{
		SCPI_ErrorPush(context, SCPI_ERROR_INVALID_SUFFIX);
		return SCPI_RES_ERR;
	}

	// get value from tag for special cases or from numeric value
	const int32_t val{number.special ? number.content.tag : static_cast<int32_t>(number.content.value)};

	if (val < 1 || val > 2)
	{
		SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
		return SCPI_RES_ERR;
	}

	selected_channel = static_cast<uint8_t>(val) - 1;
	return SCPI_RES_OK;
}

scpi_result_t get_instrument_select(scpi_t *context)
{
	SCPI_ResultInt32(context, selected_channel + 1);
	return SCPI_RES_OK;
}

scpi_result_t set_voltage(scpi_t *context)
{
	scpi_number_t number;
	constexpr scpi_choice_def_t special[] = {{"MIN", 1}, {"MAX", 2}, {"UP", 3}, {"DOWN", 4},SCPI_CHOICE_LIST_END};

	if (!SCPI_ParamNumber(context, special, &number, true))
		return SCPI_RES_ERR;

	// allow volt or no unit
	if (number.unit != SCPI_UNIT_NONE && number.unit != SCPI_UNIT_VOLT)
	{
		SCPI_ErrorPush(context, SCPI_ERROR_INVALID_SUFFIX);
		return SCPI_RES_ERR;
	}

	double out_voltage{};
	// get value from tag for special cases or from numeric value
	if (number.special)
	{
		if (number.content.tag == 1)
			out_voltage = 0.0;
		else if (number.content.tag == 2)
			out_voltage = Channel::max_voltage;
		else if (number.content.tag == 3)
			out_voltage = channels[selected_channel].get_voltage() + voltage_step;
		else if (number.content.tag == 4)
			out_voltage = channels[selected_channel].get_voltage() - voltage_step;
	} else
	{
		out_voltage = number.content.value;
	}

	if (out_voltage < 0 || out_voltage > Channel::max_voltage)
	{
		SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
		return SCPI_RES_ERR;
	}

	channels[selected_channel].set_voltage(static_cast<float>(out_voltage));
	return SCPI_RES_OK;
}

scpi_result_t get_voltage_setting(scpi_t *context)
{
	constexpr scpi_choice_def_t special[] = {{"MIN", 1}, {"MAX", 2}, SCPI_CHOICE_LIST_END};
	int32_t tag_res{};
	if (SCPI_ParamChoice(context, special, &tag_res, false))
	{
		if (tag_res == 1)
			SCPI_ResultFloat(context, 0);
		else if (tag_res == 2)
			SCPI_ResultFloat(context, Channel::max_voltage);
	} else
		SCPI_ResultFloat(context, channels[selected_channel].get_voltage());
	return SCPI_RES_OK;
}

scpi_result_t set_voltage_step(scpi_t *context)
{
	scpi_number_t number;
	constexpr scpi_choice_def_t special[] = {{"DEFault", 1},SCPI_CHOICE_LIST_END};

	if (!SCPI_ParamNumber(context, special, &number, true))
		return SCPI_RES_ERR;

	// allow volt or no unit
	if (number.unit != SCPI_UNIT_NONE && number.unit != SCPI_UNIT_VOLT)
	{
		SCPI_ErrorPush(context, SCPI_ERROR_INVALID_SUFFIX);
		return SCPI_RES_ERR;
	}

	double volt_step{};
	// get value from tag for special cases or from numeric value
	if (number.special)
	{
		volt_step = voltage_step_default;
	} else
	{
		volt_step = number.content.value;
	}

	if (volt_step < 0 || volt_step > Channel::max_voltage)
	{
		SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
		return SCPI_RES_ERR;
	}

	voltage_step = static_cast<float>(volt_step);
	return SCPI_RES_OK;
}

scpi_result_t get_voltage_step(scpi_t *context)
{
	constexpr scpi_choice_def_t special[] = {{"DEFault", 1}, SCPI_CHOICE_LIST_END};
	int32_t tag_res{};
	if (SCPI_ParamChoice(context, special, &tag_res, false))
	{
		SCPI_ResultFloat(context, voltage_step_default);
	} else
		SCPI_ResultFloat(context, voltage_step);
	return SCPI_RES_OK;
}

scpi_result_t set_current(scpi_t *context)
{
	scpi_number_t number;
	constexpr scpi_choice_def_t special[] = {{"MIN", 1}, {"MAX", 2}, {"UP", 3}, {"DOWN", 4},SCPI_CHOICE_LIST_END};

	if (!SCPI_ParamNumber(context, special, &number, true))
		return SCPI_RES_ERR;

	// allow ampere or no unit
	if (number.unit != SCPI_UNIT_NONE && number.unit != SCPI_UNIT_AMPER)
	{
		SCPI_ErrorPush(context, SCPI_ERROR_INVALID_SUFFIX);
		return SCPI_RES_ERR;
	}

	double out_current{};
	// get value from tag for special cases or from numeric value
	if (number.special)
	{
		if (number.content.tag == 1)
			out_current = 0.0;
		else if (number.content.tag == 2)
			out_current = Channel::max_current;
		else if (number.content.tag == 3)
			out_current = channels[selected_channel].get_current() + current_step;
		else if (number.content.tag == 4)
			out_current = channels[selected_channel].get_current() - current_step;
	} else
	{
		out_current = number.content.value;
	}

	if (out_current < 0 || out_current > Channel::max_current)
	{
		SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
		return SCPI_RES_ERR;
	}

	channels[selected_channel].set_current(static_cast<float>(out_current));
	return SCPI_RES_OK;
}

scpi_result_t get_current_setting(scpi_t *context)
{
	constexpr scpi_choice_def_t special[] = {{"MIN", 1}, {"MAX", 2}, SCPI_CHOICE_LIST_END};
	int32_t tag_res{};
	if (SCPI_ParamChoice(context, special, &tag_res, false))
	{
		if (tag_res == 1)
			SCPI_ResultFloat(context, 0);
		else if (tag_res == 2)
			SCPI_ResultFloat(context, Channel::max_current);
	} else
		SCPI_ResultFloat(context, channels[selected_channel].get_current());
	return SCPI_RES_OK;
}

scpi_result_t set_current_step(scpi_t *context)
{
	scpi_number_t number;
	constexpr scpi_choice_def_t special[] = {{"DEFault", 1},SCPI_CHOICE_LIST_END};

	if (!SCPI_ParamNumber(context, special, &number, true))
		return SCPI_RES_ERR;

	// allow ampere or no unit
	if (number.unit != SCPI_UNIT_NONE && number.unit != SCPI_UNIT_AMPER)
	{
		SCPI_ErrorPush(context, SCPI_ERROR_INVALID_SUFFIX);
		return SCPI_RES_ERR;
	}

	double curr_step{};
	// get value from tag for special cases or from numeric value
	if (number.special)
	{
		curr_step = current_step_default;
	} else
	{
		curr_step = number.content.value;
	}

	if (curr_step < 0 || curr_step > Channel::max_current)
	{
		SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
		return SCPI_RES_ERR;
	}

	current_step = static_cast<float>(curr_step);
	return SCPI_RES_OK;
}

scpi_result_t get_current_step(scpi_t *context)
{
	constexpr scpi_choice_def_t special[] = {{"DEFault", 1}, SCPI_CHOICE_LIST_END};
	int32_t tag_res{};
	if (SCPI_ParamChoice(context, special, &tag_res, false))
	{
		SCPI_ResultFloat(context, current_step_default);
	} else
		SCPI_ResultFloat(context, current_step);
	return SCPI_RES_OK;
}

scpi_result_t apply(scpi_t *context)
{
	scpi_number_t number;
	constexpr scpi_choice_def_t special_volt[] = {{"MIN", 1}, {"MAX", 2}, {"DEFault", 3},SCPI_CHOICE_LIST_END};

	if (!SCPI_ParamNumber(context, special_volt, &number, true))
		return SCPI_RES_ERR;

	// allow volt or no unit
	if (number.unit != SCPI_UNIT_NONE && number.unit != SCPI_UNIT_VOLT)
	{
		SCPI_ErrorPush(context, SCPI_ERROR_INVALID_SUFFIX);
		return SCPI_RES_ERR;
	}

	double out_voltage{};
	// get value from tag for special cases or from numeric value
	if (number.special)
	{
		if (number.content.tag == 1)
			out_voltage = 0.0;
		else if (number.content.tag == 2)
			out_voltage = Channel::max_voltage;
		else if (number.content.tag == 3)
			out_voltage = 1.0;
	} else
	{
		out_voltage = number.content.value;
	}

	if (out_voltage < 0 || out_voltage > Channel::max_voltage)
	{
		SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
		return SCPI_RES_ERR;
	}

	constexpr scpi_choice_def_t special_curr[] = {{"MIN", 1}, {"MAX", 2}, {"DEFault", 3},SCPI_CHOICE_LIST_END};

	double out_current{NAN};
	if (SCPI_ParamNumber(context, special_curr, &number, false))
	{
		// allow ampere or no unit
		if (number.unit != SCPI_UNIT_NONE && number.unit != SCPI_UNIT_AMPER)
		{
			SCPI_ErrorPush(context, SCPI_ERROR_INVALID_SUFFIX);
			return SCPI_RES_ERR;
		}


		// get value from tag for special cases or from numeric value
		if (number.special)
		{
			if (number.content.tag == 1)
				out_current = 0.0;
			else if (number.content.tag == 2)
				out_current = Channel::max_current;
			else if (number.content.tag == 3)
				out_current = 0.1;
		} else
		{
			out_current = number.content.value;
		}

		if (out_current < 0 || out_current > Channel::max_current)
		{
			SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
			return SCPI_RES_ERR;
		}
	}

	constexpr scpi_choice_def_t special[] = {{"OUT1", 0}, {"OUT2", 1}, SCPI_CHOICE_LIST_END};
	int32_t tag_res{};
	if (SCPI_ParamChoice(context, special, &tag_res, false))
		selected_channel = tag_res;

	channels[selected_channel].set_voltage(static_cast<float>(out_voltage));

	if (!isnan(out_current))
		channels[selected_channel].set_current(static_cast<float>(out_current));

	return SCPI_RES_OK;
}

scpi_result_t apply_query(scpi_t *context)
{
	const float res[]{channels[selected_channel].get_voltage(), channels[selected_channel].get_current()};
	SCPI_ResultArrayFloat(context, res, 2, SCPI_FORMAT_ASCII);
	return SCPI_RES_OK;
}

scpi_result_t set_channel_state(scpi_t *context)
{
	bool res;
	if (!SCPI_ParamBool(context, &res, true))
		return SCPI_RES_ERR;
	channels[selected_channel].set_enabled(res);
	return SCPI_RES_OK;
}

scpi_result_t get_channel_state(scpi_t *context)
{
	SCPI_ResultBool(context, channels[selected_channel].is_enabled());
	return SCPI_RES_OK;
}

scpi_result_t measure_voltage(scpi_t *context)
{
	SCPI_ResultFloat(context, channels[selected_channel].get_voltage_measurement());
	return SCPI_RES_OK;
}

scpi_result_t measure_current(scpi_t *context)
{
	SCPI_ResultFloat(context, channels[selected_channel].get_current_measurement());
	return SCPI_RES_OK;
}

scpi_result_t measure_power(scpi_t *context)
{
	const float power{
		channels[selected_channel].get_current_measurement() * channels[selected_channel].get_voltage_measurement()
	};
	SCPI_ResultFloat(context, power);
	return SCPI_RES_OK;
}

scpi_result_t change_i2c_adr(scpi_t *context)
{
	uint32_t addr;
	if (!SCPI_ParamUInt32(context, &addr, true))
		return SCPI_RES_ERR;

	channels[selected_channel].set_address(addr);
	return SCPI_RES_OK;
}

scpi_result_t set_beeper_state(scpi_t *context)
{
	bool res;
	if (!SCPI_ParamBool(context, &res, true))
		return SCPI_RES_ERR;
	beeper_active = res;
	return SCPI_RES_OK;
}

scpi_result_t get_beeper_state(scpi_t *context)
{
	SCPI_ResultBool(context, beeper_active);
	return SCPI_RES_OK;
}

scpi_result_t beep_immediate(scpi_t *)
{
	beep();
	return SCPI_RES_OK;
}

scpi_result_t set_display_text(scpi_t *context)
{
	const char *text;
	size_t len;
	if (!SCPI_ParamCharacters(context, &text, &len, true))
		return SCPI_RES_ERR;
	strncpy(display_text, text, len);
	return SCPI_RES_OK;
}

scpi_result_t display_text_clear(scpi_t *)
{
	display_text[0] = 0;
	return SCPI_RES_OK;
}

scpi_result_t set_brightness(scpi_t *context)
{
	double res;
	if (!SCPI_ParamDouble(context, &res, true))
		return SCPI_RES_ERR;

	if (res < 0 || res > 1)
	{
		SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
		return SCPI_RES_ERR;
	}

	display.setBrightness(static_cast<uint8_t>(0xFF * res));
	return SCPI_RES_OK;
}

scpi_result_t set_display_enabled(scpi_t *context)
{
	bool res;
	if (!SCPI_ParamBool(context, &res, true))
		return SCPI_RES_ERR;
	display.setBrightness(res ? 0xFF : 0);
	return SCPI_RES_OK;
}
