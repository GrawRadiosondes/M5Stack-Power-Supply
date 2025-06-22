//
// Created by TGA on 13.06.25.
//

#include <Arduino.h>
#include <channel.hpp>
#include <scpi/scpi.h>

#include "scpi_client.hpp"

//index of the selected channel (0 or 1)
uint8_t selected_channel{};

// IEEE 488.2 Commands
scpi_result_t get_selftest(scpi_t *context);

//No Operation
scpi_result_t scpi_nop(scpi_t *context);

//configuration commands
scpi_result_t set_instrument_select(scpi_t *context);

scpi_result_t get_instrument_select(scpi_t *context);

scpi_result_t set_voltage(scpi_t *context);
scpi_result_t get_voltage_setting(scpi_t *context);


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
	{.pattern = "SYSTem:LOCal", .callback = scpi_nop},
	{.pattern = "SYSTem:REMote", .callback = scpi_nop},
	{.pattern = "SYSTem:RWLock", .callback = scpi_nop},

	{.pattern = "STATus:PRESet", .callback = SCPI_StatusPreset},

	//configuration commands
	{.pattern = "INSTrument[:SELect]", .callback = set_instrument_select},
	{.pattern = "INSTrument[:SELect]?", .callback = get_instrument_select},
	{.pattern = "INSTrument:NSELect", .callback = set_instrument_select},
	{.pattern = "INSTrument:NSELect?", .callback = get_instrument_select},

	SCPI_CMD_LIST_END
};
// clang-format on

//
// Implementations
//

scpi_result_t reset_callback(scpi_t *)
{
	channels[0].reset();
	channels[1].reset();
	return SCPI_RES_OK;
}

scpi_result_t get_selftest(scpi_t *context)
{
	SCPI_ResultInt32(context, 0);
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
	SCPI_ResultInt32(context, selected_channel+1);
	return SCPI_RES_OK;
}
