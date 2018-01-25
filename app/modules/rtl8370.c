#include "module.h"
#include "lauxlib.h"
#include "platform.h"
#include "c_stdlib.h"
#include "c_string.h"
#include "user_interface.h"
#include "rtk_api_ext.h"
#include "smi.h"

#include "rtl8370_asicdrv_acl.h"
#include "rtl8370_asicdrv.h"
#include "rtl8370_asicdrv_dot1x.h"
#include "rtl8370_asicdrv_qos.h"
#include "rtl8370_asicdrv_scheduling.h"
#include "rtl8370_asicdrv_fc.h"
#include "rtl8370_asicdrv_port.h"
#include "rtl8370_asicdrv_phy.h"
#include "rtl8370_asicdrv_igmp.h"
#include "rtl8370_asicdrv_unknownMulticast.h"
#include "rtl8370_asicdrv_rma.h"
#include "rtl8370_asicdrv_vlan.h"
#include "rtl8370_asicdrv_lut.h"
#include "rtl8370_asicdrv_led.h"
#include "rtl8370_asicdrv_svlan.h"
#include "rtl8370_asicdrv_meter.h"
#include "rtl8370_asicdrv_inbwctrl.h"
#include "rtl8370_asicdrv_storm.h"
#include "rtl8370_asicdrv_misc.h"
#include "rtl8370_asicdrv_portIsolation.h"
#include "rtl8370_asicdrv_cputag.h"
#include "rtl8370_asicdrv_trunking.h"
#include "rtl8370_asicdrv_mirror.h"
#include "rtl8370_asicdrv_mib.h"
#include "rtl8370_asicdrv_interrupt.h"

// Lua: status = rtl8370.init()
static int rtl8370_init(lua_State* L)
{
	uint32_t ret;

	ret = rtk_switch_init();

	lua_pushnumber(L, ret);
	return 1;
}

// Lua: status = rtl8370.smi(sda_pin, sck_pin)
static int rtl8370_smi(lua_State* L)
{
	uint8_t pin_SDA = luaL_checkinteger(L, 1);
	uint8_t pin_SCK = luaL_checkinteger(L, 2);
  //uint8_t pin_RST = luaL_checkinteger(L, 3);
	uint32_t ret;

	MOD_CHECK_ID( gpio, pin_SDA );
	MOD_CHECK_ID( gpio, pin_SCK );
	//MOD_CHECK_ID( gpio, pin_RST );

	ret = smi_init(pin_SCK, pin_SDA);

	lua_pushnumber(L, ret);
	return 1;
}

// Lua: status = rtl8370.hardreset(rst_pin)
static int rtl8370_hardreset(lua_State* L)
{
	uint32_t ret;
	
	uint8_t pin_RST = luaL_checkinteger(L, 1);
	MOD_CHECK_ID( gpio, pin_RST );
	
	ret = smi_reset(pin_RST);
	
	lua_pushnumber(L, ret);
	return 1;
}

// Lua: status[, ret] = rtl8370.led.enable(group[, port_mask])
static int rtl8370_led_enable(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	rtk_api_ret_t ret;
	rtk_led_group_t group;
	rtk_portmask_t port_mask;
	uint32_t mask;

	if (argc == 1) {
		// get
		group = luaL_checkinteger(L, 1);
		ret = rtk_led_enable_get(group, &port_mask);
		mask = port_mask.bits[0];	// default? 
		lua_pushnumber(L, ret);
		lua_pushnumber(L, mask);
		return 2;
	}
	else if (argc == 2) {
		// set
		group = luaL_checkinteger(L, 1);
		mask = (uint32_t)luaL_checkinteger(L, 2);
		port_mask.bits[0] = mask;
		ret = rtk_led_enable_set(group, port_mask);
		lua_pushnumber(L, ret);
		return 1;
	}
	else {
		lua_pushnumber(L, RT_ERR_INPUT);
		return 1;
	}
}

// Lua: status[, ret] = rtl8370.led.operation([mode])
static int rtl8370_led_operation(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	rtk_api_ret_t ret;
	rtk_led_operation_t mode;

	if (argc == 0) {
		// get
		ret = rtk_led_operation_get((rtk_data_t *)(&mode));
		lua_pushnumber(L, ret);
		lua_pushnumber(L, mode);
		return 2;
	}
	else if (argc == 1) {
		// set
		mode = luaL_checkinteger(L, 1);
		ret = rtk_led_operation_set(mode);
		lua_pushnumber(L, ret);
		return 1;
	}
	else {
		lua_pushnumber(L, RT_ERR_INPUT);
		return 1;
	}
}

// Lua: status[, ret] = rtl8370.led.mode([mode])
static int rtl8370_led_mode(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	rtk_api_ret_t ret;
	rtk_led_mode_t mode;

	if (argc == 0) {
		// get
		ret = rtk_led_mode_get(&mode);
		lua_pushnumber(L, ret);
		lua_pushnumber(L, mode);
		return 2;
	}
	else if (argc == 1) {
		// set
		mode = luaL_checkinteger(L, 1);
		ret = rtk_led_mode_set(mode);
		lua_pushnumber(L, ret);
		return 1;
	}
	else {
		lua_pushnumber(L, RT_ERR_INPUT);
		return 1;
	}
}

// Lua: status[, mode] = rtl8370.led.modeForce(group[, mode])
static int rtl8370_led_modeForce(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	rtk_api_ret_t ret;
	rtk_led_group_t group;
	rtk_led_force_mode_t mode;

	if (argc == 1) {
		// get
		group = luaL_checkinteger(L, 1);
		ret = rtk_led_modeForce_get(group, (rtk_data_t *)(&mode));
		lua_pushnumber(L, ret);
		lua_pushnumber(L, mode);
		return 2;
	}
	else if (argc == 2) {
		// set
		group = luaL_checkinteger(L, 1);
		mode = luaL_checkinteger(L, 2);
		ret = rtk_led_modeForce_set(group, mode);
		lua_pushnumber(L, ret);
		return 1;
	}
	else {
		lua_pushnumber(L, RT_ERR_INPUT);
		return 1;
	}
}

// Lua: status[, mode] = rtl8370.led.blinkRate([rate])
static int rtl8370_led_blinkRate(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	rtk_api_ret_t ret;
	rtk_led_blink_rate_t rate;

	if (argc == 0) {
		// get
		ret = rtk_led_blinkRate_get((rtk_led_blink_rate_t *)(&rate));
		lua_pushnumber(L, ret);
		lua_pushnumber(L, rate);
		return 2;
	}
	else if (argc == 1) {
		// set
		rate = luaL_checkinteger(L, 1);
		ret = rtk_led_blinkRate_set(rate);
		lua_pushnumber(L, ret);
		return 1;
	}
	else {
		lua_pushnumber(L, RT_ERR_INPUT);
		return 1;
	}
}

// Lua: status[, mode] = rtl8370.led.groupConfig(group[, mode])
static int rtl8370_led_groupConfig(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	rtk_api_ret_t ret;
	rtk_led_group_t group;
	rtk_led_congig_t config;

	if (argc == 1) {
		// get
		group = luaL_checkinteger(L, 1);
		ret = rtk_led_groupConfig_get(group, (rtk_data_t *)(&config));
		lua_pushnumber(L, ret);
		lua_pushnumber(L, config);
		return 2;
	}
	else if (argc == 2) {
		// set
		group = luaL_checkinteger(L, 1);
		config = luaL_checkinteger(L, 2);
		ret = rtk_led_groupConfig_set(group, config);
		lua_pushnumber(L, ret);
		return 1;
	}
	else {
		lua_pushnumber(L, RT_ERR_INPUT);
		return 1;
	}
}

// Lua: status[, mode] = rtl8370.led.serialMode([mode])
static int rtl8370_led_serialMode(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	rtk_api_ret_t ret;
	rtk_led_active_t active;

	if (argc == 0) {
		// get
		ret = rtk_led_serialMode_get((rtk_data_t *)(&active));
		lua_pushnumber(L, ret);
		lua_pushnumber(L, active);
		return 2;
	}
	else if (argc == 1) {
		// set
		active = luaL_checkinteger(L, 1);
		ret = rtk_led_serialMode_set(active);
		lua_pushnumber(L, ret);
		return 1;
	}
	else {
		lua_pushnumber(L, RT_ERR_INPUT);
		return 1;
	}
}



//=================== vlan ============================

// Lua: status = rtl8370.vlan.init()
static int rtl8370_vlan_init(lua_State* L)
{
	rtk_api_ret_t ret;
	
	ret = rtk_vlan_init();
	
	lua_pushnumber(L, ret);
	return 1;
}

// Lua: status[, mode] = rtl8370.vlan(vid, member_mask, )
static int rtl8370_vlan(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	rtk_api_ret_t ret;
	rtk_led_active_t active;

	if (argc == 0) {
		// get
		ret = rtk_led_serialMode_get((rtk_data_t *)(&active));
		lua_pushnumber(L, ret);
		lua_pushnumber(L, active);
		return 2;
	}
	else if (argc == 1) {
		// set
		active = luaL_checkinteger(L, 1);
		ret = rtk_led_serialMode_set(active);
		lua_pushnumber(L, ret);
		return 1;
	}
	else {
		lua_pushnumber(L, RT_ERR_INPUT);
		return 1;
	}
}






























// Module function map
static const LUA_REG_TYPE rtl8370_map[] = {
	{ LSTRKEY( "init" ), LFUNCVAL( rtl8370_init )},
	{ LSTRKEY( "smi" ), LFUNCVAL( rtl8370_smi )},
	
	// led
	{ LSTRKEY( "led_serialMode" ), LFUNCVAL ( rtl8370_led_serialMode ) },
	{ LSTRKEY( "led_groupConfig" ), LFUNCVAL ( rtl8370_led_groupConfig ) },
	{ LSTRKEY( "led_blinkRate" ), LFUNCVAL ( rtl8370_led_blinkRate ) },
	{ LSTRKEY( "led_modeForce" ), LFUNCVAL ( rtl8370_led_modeForce ) },
	{ LSTRKEY( "led_mode" ), LFUNCVAL ( rtl8370_led_mode ) },
	{ LSTRKEY( "led_operation" ), LFUNCVAL ( rtl8370_led_operation ) },
	{ LSTRKEY( "led_enable" ), LFUNCVAL ( rtl8370_led_enable ) },

	// Errors
	{ LSTRKEY( "RT_ERR_OK" ), LNUMVAL( RT_ERR_OK ) },
	{ LSTRKEY( "RT_ERR_FAILED" ), LNUMVAL( RT_ERR_FAILED ) },
	{ LSTRKEY( "RT_ERR_SMI" ), LNUMVAL( RT_ERR_SMI ) },
	{ LSTRKEY( "RT_ERR_INPUT" ), LNUMVAL( RT_ERR_INPUT ) },
	{ LNILKEY, LNILVAL}
};

int luaopen_rtl8370(lua_State *L)
{


}

NODEMCU_MODULE(RTL8370, "rtl8370", rtl8370_map, luaopen_rtl8370);
