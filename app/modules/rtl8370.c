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


// ============== esp8266 control functions ===========

// Lua: status = rtl8370.smi(sda_pin, sck_pin)
static int rtl8370_smi(lua_State* L)
{
	uint8_t pin_SDA = luaL_checkinteger(L, 1);
	uint8_t pin_SCK = luaL_checkinteger(L, 2);

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

// Lua: status = rtl8370.softreset()
static int rtl8370_softreset(lua_State* L)
{
	uint32_t ret;
	
	ret = rtl8370_setAsicReg(RTL8370_REG_CHIP_RESET, (1<<RTL8370_CHIP_RST_OFFSET));
	
	lua_pushnumber(L, ret);
	return 1;
}


// ================= switch misc =====================

// Lua: status = rtl8370.switch_init()
static int rtl8370_switch_init(lua_State* L)
{
	uint32_t ret;

	ret = rtk_switch_init();

	lua_pushnumber(L, ret);
	return 1;
}

// Lua: status[, ret] = rtl8370.switch_maxPktLen(len)
static int rtl8370_switch_maxPktLen(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	rtk_api_ret_t ret;
	rtk_switch_maxPktLen_t len;

	if (argc == 0) {
		// get
		ret = rtk_switch_maxPktLen_get((rtk_switch_maxPktLen_t *)(&len));
		lua_pushnumber(L, ret);
		lua_pushnumber(L, len);
		return 2;
	}
	else if (argc == 1) {
		// set
		len = luaL_checkinteger(L, 1);
		ret = rtk_switch_maxPktLen_set(len);
		lua_pushnumber(L, ret);
		return 1;
	}
	else {
		lua_pushnumber(L, RT_ERR_INPUT);
		return 1;
	}
}

// Lua: status[, ret] = rtl8370.switch_greenEthernet(len)
static int rtl8370_switch_greenEthernet(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	rtk_api_ret_t ret;
	rtk_enable_t enable;

	if (argc == 0) {
		// get
		ret = rtk_switch_greenEthernet_get((rtk_enable_t *)(&enable));
		lua_pushnumber(L, ret);
		lua_pushnumber(L, enable);
		return 2;
	}
	else if (argc == 1) {
		// set
		enable = luaL_checkinteger(L, 1);
		ret = rtk_switch_greenEthernet_set(enable);
		lua_pushnumber(L, ret);
		return 1;
	}
	else {
		lua_pushnumber(L, RT_ERR_INPUT);
		return 1;
	}
}



// ===================== led =========================

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

// Lua: status = rtl8370.vlan_init()
static int rtl8370_vlan_init(lua_State* L)
{
	rtk_api_ret_t ret;
	
	ret = rtk_vlan_init();
	
	lua_pushnumber(L, ret);
	return 1;
}

// Lua: status[, member_mask, untag_mask, fid] = rtl8370.vlan(vid[, member_mask, untag_mask, fid])
static int rtl8370_vlan(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	rtk_api_ret_t ret;
	rtk_vlan_t vid;
	rtk_portmask_t member_mask;
	rtk_portmask_t untag_mask;
	rtk_fid_t fid;

	if (argc == 1) {
		// get
		vid = luaL_checkinteger(L, 1);
		ret = rtk_vlan_get(vid, (rtk_portmask_t *)&member_mask, 
						(rtk_portmask_t *)&untag_mask, (rtk_fid_t *)&fid);

		lua_pushnumber(L, ret);
		lua_pushnumber(L, member_mask.bits[0]);
		lua_pushnumber(L, untag_mask.bits[0]);
		lua_pushnumber(L, fid);
		
		return 4;
	}
	else if (argc == 4) {
		// set
		vid = luaL_checkinteger(L, 1);
		member_mask.bits[0] = luaL_checkinteger(L, 2);
		untag_mask.bits[0] = luaL_checkinteger(L, 3);
		fid = luaL_checkinteger(L, 4);
		
		ret = rtk_vlan_set(vid, member_mask, untag_mask, fid);
		
		lua_pushnumber(L, ret);
		return 1;
	}
	else {
		lua_pushnumber(L, RT_ERR_INPUT);
		return 1;
	}
}

// Lua: status[, pvid, priority] = rtl8370.vlan_portPvid(port[, pvid, priority])
static int rtl8370_vlan_portPvid(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	rtk_api_ret_t ret;
	rtk_port_t port;
	rtk_vlan_t pvid;
	rtk_pri_t priority;

	if (argc == 1) {
		// get
		port = luaL_checkinteger(L, 1);
		ret = rtk_vlan_portPvid_get(port, (rtk_vlan_t *)&pvid, 
						(rtk_pri_t *)&priority);

		lua_pushnumber(L, ret);
		lua_pushnumber(L, pvid);
		lua_pushnumber(L, priority);
		
		return 3;
	}
	else if (argc == 3) {
		// set
		port = luaL_checkinteger(L, 1);
		pvid = luaL_checkinteger(L, 2);
		priority = luaL_checkinteger(L, 3);
		
		ret = rtk_vlan_portPvid_set(port, pvid, priority);
		
		lua_pushnumber(L, ret);
		return 1;
	}
	else {
		lua_pushnumber(L, RT_ERR_INPUT);
		return 1;
	}
}

// Lua: status[, igr_filter] = rtl8370.vlan_portIgrFilterEnable(port[, igr_filter])
static int rtl8370_vlan_portIgrFilterEnable(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	rtk_api_ret_t ret;
	rtk_port_t port;
	rtk_enable_t igr_filter;

	if (argc == 1) {
		// get
		port = luaL_checkinteger(L, 1);
		ret = rtk_vlan_portIgrFilterEnable_get(port, 
						(rtk_enable_t *)&igr_filter);

		lua_pushnumber(L, ret);
		lua_pushnumber(L, igr_filter);

		return 2;
	}
	else if (argc == 2) {
		// set
		port = luaL_checkinteger(L, 1);
		igr_filter = luaL_checkinteger(L, 2);
		
		ret = rtk_vlan_portIgrFilterEnable_set(port, igr_filter);
		
		lua_pushnumber(L, ret);
		return 1;
	}
	else {
		lua_pushnumber(L, RT_ERR_INPUT);
		return 1;
	}
}

// Lua: status[, accept_frame_type] = rtl8370.vlan_portAcceptFrameType(port[, accept_frame_type])
static int rtl8370_vlan_portAcceptFrameType(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	rtk_api_ret_t ret;
	rtk_port_t port;
	rtk_vlan_acceptFrameType_t accept_frame_type;

	if (argc == 1) {
		// get
		port = luaL_checkinteger(L, 1);
		ret = rtk_vlan_portAcceptFrameType_get(port, 
						(rtk_vlan_acceptFrameType_t *)&accept_frame_type);

		lua_pushnumber(L, ret);
		lua_pushnumber(L, accept_frame_type);

		return 2;
	}
	else if (argc == 2) {
		// set
		port = luaL_checkinteger(L, 1);
		accept_frame_type = luaL_checkinteger(L, 2);
		
		ret = rtk_vlan_portAcceptFrameType_set(port, accept_frame_type);
		
		lua_pushnumber(L, ret);
		return 1;
	}
	else {
		lua_pushnumber(L, RT_ERR_INPUT);
		return 1;
	}
}

// Lua: status[, priority] = rtl8370.vlan_vlanBasedPriority(vid[, priority])
static int rtl8370_vlan_vlanBasedPriority(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	rtk_api_ret_t ret;
	rtk_vlan_t vid;
	rtk_pri_t priority;

	if (argc == 1) {
		// get
		vid = luaL_checkinteger(L, 1);
		ret = rtk_vlan_vlanBasedPriority_get(vid, 
						(rtk_pri_t *)&priority);

		lua_pushnumber(L, ret);
		lua_pushnumber(L, priority);

		return 2;
	}
	else if (argc == 2) {
		// set
		vid = luaL_checkinteger(L, 1);
		priority = luaL_checkinteger(L, 2);
		
		ret = rtk_vlan_vlanBasedPriority_set(vid, priority);
		
		lua_pushnumber(L, ret);
		return 1;
	}
	else {
		lua_pushnumber(L, RT_ERR_INPUT);
		return 1;
	}
}

// Lua: status[, proto_type, frame_type, cvid, cpri] = rtl8370.vlan_protoAndPortBasedVlan(operation, port[, proto_type, frame_type][, cvid, cpri])
static int rtl8370_vlan_protoAndPortBasedVlan(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	rtk_api_ret_t ret;
	
	rtk_port_t port;
	rtk_vlan_protoAndPortInfo_t info;
	
	rtk_vlan_proto_type_t proto_type;
	rtk_vlan_protoVlan_frameType_t frame_type;
	
	const char * operation;
	size_t operation_length;

	operation = lua_tolstring(L, 1, &operation_length);

	if (argc == 4 && (!strcmp(operation, "get"))) {
		// get
		port = luaL_checkinteger(L, 2);
		proto_type = luaL_checkinteger(L, 3);
		frame_type = luaL_checkinteger(L, 4);
		
		ret = rtk_vlan_protoAndPortBasedVlan_get(port, proto_type,
						frame_type, (rtk_vlan_protoAndPortInfo_t *)&info);

		lua_pushnumber(L, ret);
		lua_pushnumber(L, info.proto_type);
		lua_pushnumber(L, info.frame_type);
		lua_pushnumber(L, info.cvid);
		lua_pushnumber(L, info.cpri);

		return 5;
	}
	else if (argc == 4 && (!strcmp(operation, "del"))) {
		// del
		port = luaL_checkinteger(L, 2);
		proto_type = luaL_checkinteger(L, 3);
		frame_type = luaL_checkinteger(L, 4);
		
		ret = rtk_vlan_protoAndPortBasedVlan_del(port, proto_type, frame_type);

		lua_pushnumber(L, ret);

		return 1;
	}
	else if (argc == 6 && (!strcmp(operation, "add"))) {
		// add
		port = luaL_checkinteger(L, 2);
		info.proto_type = luaL_checkinteger(L, 3);
		info.frame_type = luaL_checkinteger(L, 4);
		info.cvid = luaL_checkinteger(L, 5);
		info.cpri = luaL_checkinteger(L, 6);
		
		ret = rtk_vlan_protoAndPortBasedVlan_add(port, info);
		
		lua_pushnumber(L, ret);
		return 1;
	}
	else if (argc == 2 && (!strcmp(operation, "delAll"))) {
		// delAll
		port = luaL_checkinteger(L, 2);
		
		ret = rtk_vlan_protoAndPortBasedVlan_delAll(port);
		
		lua_pushnumber(L, ret);
		return 1;
	}
	else {
		lua_pushnumber(L, RT_ERR_INPUT);
		return 1;
	}
}

// Lua: status[, enable, fid] = rtl8370.vlan_portFid(port[, enable, fid])
static int rtl8370_vlan_portFid(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	rtk_api_ret_t ret;
	rtk_port_t port;
	rtk_enable_t enable;
	rtk_fid_t fid;

	if (argc == 1) {
		// get
		port = luaL_checkinteger(L, 1);
		ret = rtk_vlan_portFid_get(port, (rtk_enable_t *)&enable, 
						(rtk_fid_t *)&fid);

		lua_pushnumber(L, ret);
		lua_pushnumber(L, enable);
		lua_pushnumber(L, fid);
		
		return 3;
	}
	else if (argc == 3) {
		// set
		port = luaL_checkinteger(L, 1);
		enable = luaL_checkinteger(L, 2);
		fid = luaL_checkinteger(L, 3);
		
		ret = rtk_vlan_portFid_set(port, enable, fid);
		
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
	
	// esp8266 control functions
	{ LSTRKEY( "smi" ), 						LFUNCVAL( rtl8370_smi )},
	{ LSTRKEY( "hardreset" ), 					LFUNCVAL( rtl8370_hardreset )},
	{ LSTRKEY( "softreset" ), 					LFUNCVAL( rtl8370_softreset )},
	
	
	// switch
	{ LSTRKEY( "switch_init" ), 				LFUNCVAL( rtl8370_switch_init )},
	{ LSTRKEY( "switch_maxPktLen" ), 			LFUNCVAL( rtl8370_switch_maxPktLen )},
	{ LSTRKEY( "switch_greenEthernet" ), 		LFUNCVAL( rtl8370_switch_greenEthernet )},
	
	
	// led functions
	{ LSTRKEY( "led_serialMode" ), 				LFUNCVAL ( rtl8370_led_serialMode ) },
	{ LSTRKEY( "led_groupConfig" ), 			LFUNCVAL ( rtl8370_led_groupConfig ) },
	{ LSTRKEY( "led_blinkRate" ), 				LFUNCVAL ( rtl8370_led_blinkRate ) },
	{ LSTRKEY( "led_modeForce" ), 				LFUNCVAL ( rtl8370_led_modeForce ) },
	{ LSTRKEY( "led_mode" ), 					LFUNCVAL ( rtl8370_led_mode ) },
	{ LSTRKEY( "led_operation" ), 				LFUNCVAL ( rtl8370_led_operation ) },
	{ LSTRKEY( "led_enable" ), 					LFUNCVAL ( rtl8370_led_enable ) },
	
	
	// vlan functions
	{ LSTRKEY( "vlan_init" ), 					LFUNCVAL( rtl8370_vlan_init )},
	{ LSTRKEY( "vlan" ), 						LFUNCVAL( rtl8370_vlan )},
	{ LSTRKEY( "vlan_portPvid" ), 				LFUNCVAL( rtl8370_vlan_portPvid )},
	{ LSTRKEY( "vlan_portIgrFilterEnable" ), 	LFUNCVAL( rtl8370_vlan_portIgrFilterEnable )},
	{ LSTRKEY( "vlan_portAcceptFrameType" ), 	LFUNCVAL( rtl8370_vlan_portAcceptFrameType )},
	{ LSTRKEY( "vlan_vlanBasedPriority" ), 		LFUNCVAL( rtl8370_vlan_vlanBasedPriority )},
	{ LSTRKEY( "vlan_protoAndPortBasedVlan" ), 	LFUNCVAL( rtl8370_vlan_protoAndPortBasedVlan )},
	{ LSTRKEY( "vlan_portFid" ), 				LFUNCVAL( rtl8370_vlan_portFid )},
	
	
	// Return numbers
	{ LSTRKEY( "RT_ERR_OK" ), 					LNUMVAL( RT_ERR_OK ) },
	{ LSTRKEY( "RT_ERR_FAILED" ), 				LNUMVAL( RT_ERR_FAILED ) },
	{ LSTRKEY( "RT_ERR_SMI" ), 					LNUMVAL( RT_ERR_SMI ) },
	{ LSTRKEY( "RT_ERR_INPUT" ), 				LNUMVAL( RT_ERR_INPUT ) },
	{ LSTRKEY( "RT_ERR_PORT_ID" ), 				LNUMVAL( RT_ERR_PORT_ID ) },
	{ LSTRKEY( "RT_ERR_VLAN_ACCEPT_FRAME_TYPE" ), LNUMVAL( RT_ERR_VLAN_ACCEPT_FRAME_TYPE ) },
	{ LSTRKEY( "RT_ERR_VLAN_VID" ), 			LNUMVAL( RT_ERR_VLAN_VID ) },
	{ LSTRKEY( "RT_ERR_VLAN_PRIORITY" ), 		LNUMVAL( RT_ERR_VLAN_PRIORITY ) },
	{ LSTRKEY( "RT_ERR_TBL_FULL" ), 			LNUMVAL( RT_ERR_TBL_FULL ) },
	{ LSTRKEY( "RT_ERR_OUT_OF_RANGE" ), 		LNUMVAL( RT_ERR_OUT_OF_RANGE ) },
	{ LSTRKEY( "RT_ERR_INPUT" ), 				LNUMVAL( RT_ERR_INPUT ) },
	
	
	// end
	{ LNILKEY, LNILVAL}
};

int luaopen_rtl8370(lua_State *L)
{
	// 

}

NODEMCU_MODULE(RTL8370, "rtl8370", rtl8370_map, luaopen_rtl8370);
