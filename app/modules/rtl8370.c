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


// ================== 抄来的 Tools =======================

// Function:
// 		void rtl8370_tools_strmac_to_byte(char * str, uint8 * mac)
// Input: 
//		str = "00:11:22:33:44:55";
// Output: 
//		mac[6] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};
void rtl8370_tools_strmac_to_byte(char * str, uint8 * mac) 
{
	uint8 *p;
	int32 count = 0;
	p = strtok(str, ":");
	if (p != NULL) {
		mac[count++] = strtoul(p,0,16);
		while(1) {
			p = strtok(NULL, ":");
			if (p == NULL) {
				break;
			}
			else {
				mac[count++] = strtoul(p,0,16);
			}
		}
	}
}

// Function:
// 		void rtl8370_tools_strip_to_byte(char * str, uint8 * ip)
// Input: 
//		str = "192.168.0.1";
// Output: 
//		ip[4] = {0xC0, 0xa8, 0x00, 0x01};
void rtl8370_tools_strip_to_byte(char * str, uint8 * ip) 
{
	uint8 *p;
	int32 count = 0;
	p = strtok(str, ".");
	if (p != NULL) {
		ip[count++] = atol(p);
		while(1) {
			p = strtok(NULL, ".");
			if (p == NULL) {
				break;
			}
			else {
				ip[count++] = atol(p);
			}
		}
	}
}



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



// ================= spanning tree =====================

// Lua: status = rtl8370.stp_init()
static int rtl8370_stp_init(lua_State* L)
{
	uint32_t ret;

	ret = rtk_stp_init();

	lua_pushnumber(L, ret);
	return 1;
}

// Lua: status[, stp_state] = rtl8370.stp_mstpState(msti, port[, stp_state])
static int rtl8370_stp_mstpState(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	rtk_api_ret_t ret;
	rtk_stp_msti_id_t msti;
	rtk_port_t port;
	rtk_stp_state_t stp_state;

	if (argc == 2) {
		// get
		msti = luaL_checkinteger(L, 1);
		port = luaL_checkinteger(L, 2);
		
		ret = rtk_stp_mstpState_get(msti, port, 
						(rtk_stp_state_t *)&stp_state);

		lua_pushnumber(L, ret);
		lua_pushnumber(L, stp_state);
		
		return 2;
	}
	else if (argc == 3) {
		// set
		msti = luaL_checkinteger(L, 1);
		port = luaL_checkinteger(L, 2);
		stp_state = luaL_checkinteger(L, 3);
		
		ret = rtk_stp_mstpState_set(msti, port, stp_state);
		
		lua_pushnumber(L, ret);
		return 1;
	}
	else {
		lua_pushnumber(L, RT_ERR_INPUT);
		return 1;
	}
}



// ================= port mirroring =====================

// Lua: status[, mirrored_rx_portmask, mirrored_tx_portmask] = rtl8370.mirror_portBased(mirroring_port[, mirrored_rx_portmask, mirrored_tx_portmask])
static int rtl8370_mirror_portBased(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	rtk_api_ret_t ret;
	rtk_port_t port;
	rtk_portmask_t mirrored_rx_portmask;
	rtk_portmask_t mirrored_tx_portmask;

	if (argc == 1) {
		// get
		port = luaL_checkinteger(L, 1);
		
		ret = rtk_mirror_portBased_get((rtk_port_t *)&port, 
						(rtk_portmask_t *)&mirrored_rx_portmask, 
						(rtk_portmask_t *)&mirrored_tx_portmask);

		lua_pushnumber(L, ret);
		lua_pushnumber(L, mirrored_rx_portmask.bits[0]);
		lua_pushnumber(L, mirrored_tx_portmask.bits[0]);
		
		return 3;
	}
	else if (argc == 3) {
		// set
		port = luaL_checkinteger(L, 1);
		mirrored_rx_portmask.bits[0] = luaL_checkinteger(L, 2);
		mirrored_tx_portmask.bits[0] = luaL_checkinteger(L, 3);
		
		ret = rtk_mirror_portBased_set(port, &mirrored_rx_portmask, &mirrored_tx_portmask);
		
		lua_pushnumber(L, ret);
		return 1;
	}
	else {
		lua_pushnumber(L, RT_ERR_INPUT);
		return 1;
	}
}

// Lua: status[, enable] = rtl8370.mirror_portIso([enable])
static int rtl8370_mirror_portIso(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	rtk_api_ret_t ret;
	rtk_enable_t enable;


	if (argc == 0) {
		// get
		
		ret = rtk_mirror_portIso_get((rtk_enable_t *)&enable);

		lua_pushnumber(L, ret);
		lua_pushnumber(L, enable);
		
		return 2;
	}
	else if (argc == 1) {
		// set
		enable = luaL_checkinteger(L, 1);
		
		ret = rtk_mirror_portIso_set(enable);
		
		lua_pushnumber(L, ret);
		return 1;
	}
	else {
		lua_pushnumber(L, RT_ERR_INPUT);
		return 1;
	}
}



// ================= port trunk =====================

// Lua: status[, trunk_member_portmask] = rtl8370.trunk_port(trk_gid[, trunk_member_portmask])
static int rtl8370_trunk_port(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	rtk_api_ret_t ret;
	rtk_trunk_group_t trk_gid;
	rtk_portmask_t trunk_member_portmask;

	if (argc == 1) {
		// get
		trk_gid = luaL_checkinteger(L, 1);
		
		ret = rtk_trunk_port_get(trk_gid, (rtk_portmask_t *)&trunk_member_portmask);

		lua_pushnumber(L, ret);
		lua_pushnumber(L, trunk_member_portmask.bits[0]);
		
		return 2;
	}
	else if (argc == 2) {
		// set
		trk_gid = luaL_checkinteger(L, 1);
		trunk_member_portmask.bits[0] = luaL_checkinteger(L, 2);
		
		ret = rtk_trunk_port_set(trk_gid, trunk_member_portmask);
		
		lua_pushnumber(L, ret);
		return 1;
	}
	else {
		lua_pushnumber(L, RT_ERR_INPUT);
		return 1;
	}
}

// Lua: status[, algo_bitmask] = rtl8370.trunk_distributionAlgorithm(trk_gid[, algo_bitmask])
static int rtl8370_trunk_distributionAlgorithm(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	rtk_api_ret_t ret;
	rtk_trunk_group_t trk_gid;
	rtk_trunk_hashVal2Port_t algo_bitmask;

	if (argc == 1) {
		// get
		trk_gid = luaL_checkinteger(L, 1);
		
		ret = rtk_trunk_distributionAlgorithm_get(trk_gid, (rtk_trunk_hashVal2Port_t *)&algo_bitmask);

		lua_pushnumber(L, ret);
		lua_pushnumber(L, algo_bitmask.value[0]);
		
		return 2;
	}
	else if (argc == 2) {
		// set
		trk_gid = luaL_checkinteger(L, 1);
		algo_bitmask.value[0] = luaL_checkinteger(L, 2);
		
		ret = rtk_trunk_distributionAlgorithm_set(trk_gid, algo_bitmask);
		
		lua_pushnumber(L, ret);
		return 1;
	}
	else {
		lua_pushnumber(L, RT_ERR_INPUT);
		return 1;
	}
}

// Lua: status, portmask = rtl8370.trunk_queueEmptyStatus()
static int rtl8370_trunk_queueEmptyStatus(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	rtk_api_ret_t ret;
	rtk_portmask_t portmask;

	if (argc == 0) {
		// get
		// qeueu? 这个干啥用的啊
		ret = rtk_trunk_qeueuEmptyStatus_get((rtk_portmask_t *)&portmask);
		
		lua_pushnumber(L, ret);
		lua_pushnumber(L, portmask.bits[0]);
		
		return 2;
	}
	else {
		lua_pushnumber(L, RT_ERR_INPUT);
		return 1;
	}
}



// ================= Rate =====================

// Lua: status[, rate, ifg_include] = rtl8370.rate_shareMeter(index[, rate, ifg_include])
static int rtl8370_rate_shareMeter(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	rtk_api_ret_t ret;
	rtk_meter_id_t index;
	rtk_rate_t rate;
	rtk_enable_t ifg_include;

	if (argc == 1) {
		// get
		index = luaL_checkinteger(L, 1);
		
		ret = rtk_rate_shareMeter_get(index, (rtk_rate_t *)&rate, 
						(rtk_enable_t *)&ifg_include);

		lua_pushnumber(L, ret);
		lua_pushnumber(L, rate);
		lua_pushnumber(L, ifg_include);
		
		return 3;
	}
	else if (argc == 3) {
		// set
		index = luaL_checkinteger(L, 1);
		rate = luaL_checkinteger(L, 2);
		ifg_include = luaL_checkinteger(L, 3);
		
		ret = rtk_rate_shareMeter_set(index, rate, ifg_include);
		
		lua_pushnumber(L, ret);
		return 1;
	}
	else {
		lua_pushnumber(L, RT_ERR_INPUT);
		return 1;
	}
}

// Lua: status[, rate, ifg_include, fc_enable] = rtl8370.rate_igrBandwidthCtrlRate(port[, rate, ifg_include, fc_enable])
static int rtl8370_rate_igrBandwidthCtrlRate(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	rtk_api_ret_t ret;
	rtk_port_t port;
	rtk_rate_t rate;
	rtk_enable_t ifg_include;
	rtk_enable_t fc_enable;

	if (argc == 1) {
		// get
		port = luaL_checkinteger(L, 1);
		
		ret = rtk_rate_igrBandwidthCtrlRate_get(port, (rtk_rate_t *)&rate, 
						(rtk_enable_t *)&ifg_include, (rtk_enable_t *)&fc_enable);

		lua_pushnumber(L, ret);
		lua_pushnumber(L, rate);
		lua_pushnumber(L, ifg_include);
		lua_pushnumber(L, fc_enable);
		
		return 4;
	}
	else if (argc == 4) {
		// set
		port = luaL_checkinteger(L, 1);
		rate = luaL_checkinteger(L, 2);
		ifg_include = luaL_checkinteger(L, 3);
		fc_enable = luaL_checkinteger(L, 4);
		
		ret = rtk_rate_igrBandwidthCtrlRate_set(port, rate, ifg_include, fc_enable);
		
		lua_pushnumber(L, ret);
		return 1;
	}
	else {
		lua_pushnumber(L, RT_ERR_INPUT);
		return 1;
	}
}

// Lua: status[, rate, ifg_include] = rtl8370.rate_egrBandwidthCtrlRate(port[, rate, ifg_include])
static int rtl8370_rate_egrBandwidthCtrlRate(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	rtk_api_ret_t ret;
	rtk_port_t port;
	rtk_rate_t rate;
	rtk_enable_t ifg_include;

	if (argc == 1) {
		// get
		port = luaL_checkinteger(L, 1);
		
		ret = rtk_rate_egrBandwidthCtrlRate_get(port, (rtk_rate_t *)&rate, 
						(rtk_enable_t *)&ifg_include);

		lua_pushnumber(L, ret);
		lua_pushnumber(L, rate);
		lua_pushnumber(L, ifg_include);
		
		return 3;
	}
	else if (argc == 3) {
		// set
		port = luaL_checkinteger(L, 1);
		rate = luaL_checkinteger(L, 2);
		ifg_include = luaL_checkinteger(L, 3);
		
		ret = rtk_rate_egrBandwidthCtrlRate_set(port, rate, ifg_include);
		
		lua_pushnumber(L, ret);
		return 1;
	}
	else {
		lua_pushnumber(L, RT_ERR_INPUT);
		return 1;
	}
}

// Lua: status[, enable]= rtl8370.rate_egrQueueBwCtrlEnable(port, queue[, enable])
static int rtl8370_rate_egrQueueBwCtrlEnable(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	rtk_api_ret_t ret;
	rtk_port_t port;
	rtk_qid_t queue;
	rtk_enable_t enable;

	if (argc == 2) {
		// get
		port = luaL_checkinteger(L, 1);
		queue = luaL_checkinteger(L, 2);
		
		ret = rtk_rate_egrQueueBwCtrlEnable_get(port, queue, 
						(rtk_enable_t *)&enable);

		lua_pushnumber(L, ret);
		lua_pushnumber(L, enable);
		
		return 2;
	}
	else if (argc == 3) {
		// set
		port = luaL_checkinteger(L, 1);
		queue = luaL_checkinteger(L, 2);
		enable = luaL_checkinteger(L, 3);
		
		ret = rtk_rate_egrQueueBwCtrlEnable_set(port, queue, enable);
		
		lua_pushnumber(L, ret);
		return 1;
	}
	else {
		lua_pushnumber(L, RT_ERR_INPUT);
		return 1;
	}
}

// Lua: status[, index]= rtl8370.rate_egrQueueBwCtrlRate(port, queue[, index])
static int rtl8370_rate_egrQueueBwCtrlRate(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	rtk_api_ret_t ret;
	rtk_port_t port;
	rtk_qid_t queue;
	rtk_data_t index;

	if (argc == 2) {
		// get
		port = luaL_checkinteger(L, 1);
		queue = luaL_checkinteger(L, 2);
		
		ret = rtk_rate_egrQueueBwCtrlRate_get(port, queue, 
						(rtk_data_t *)&index);

		lua_pushnumber(L, ret);
		lua_pushnumber(L, index);
		
		return 2;
	}
	else if (argc == 3) {
		// set
		port = luaL_checkinteger(L, 1);
		queue = luaL_checkinteger(L, 2);
		index = luaL_checkinteger(L, 3);
		
		ret = rtk_rate_egrQueueBwCtrlRate_set(port, queue, index);
		
		lua_pushnumber(L, ret);
		return 1;
	}
	else {
		lua_pushnumber(L, RT_ERR_INPUT);
		return 1;
	}
}



//=================== EEE ============================

// Lua: status = rtl8370.eee_init()
static int rtl8370_eee_init(lua_State* L)
{
	rtk_api_ret_t ret;
	
	ret = rtk_eee_init();
	
	lua_pushnumber(L, ret);
	return 1;
}

// Lua: status[, enable]= rtl8370.eee_portEnable(port[, enable])
static int rtl8370_eee_portEnable(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	rtk_api_ret_t ret;
	rtk_port_t port;
	rtk_enable_t enable;

	if (argc == 1) {
		// get
		port = luaL_checkinteger(L, 1);
		
		ret = rtk_eee_portEnable_get(port, (rtk_data_t *)&enable);

		lua_pushnumber(L, ret);
		lua_pushnumber(L, enable);
		
		return 2;
	}
	else if (argc == 2) {
		// set
		port = luaL_checkinteger(L, 1);
		enable = luaL_checkinteger(L, 2);
		
		ret = rtk_eee_portEnable_set(port, enable);
		
		lua_pushnumber(L, ret);
		return 1;
	}
	else {
		lua_pushnumber(L, RT_ERR_INPUT);
		return 1;
	}
}



//=================== ALDP ============================

// Lua: status = rtl8370.aldp_init()
static int rtl8370_aldp_init(lua_State* L)
{
	rtk_api_ret_t ret;
	
	ret = rtk_aldp_init();
	
	lua_pushnumber(L, ret);
	return 1;
}

// Lua: status[, enable]= rtl8370.aldp_enable([enable])
static int rtl8370_aldp_enable(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	rtk_api_ret_t ret;
	rtk_enable_t enable;

	if (argc == 0) {
		// get
		
		ret = rtk_aldp_enable_get((rtk_data_t *)&enable);

		lua_pushnumber(L, ret);
		lua_pushnumber(L, enable);
		
		return 2;
	}
	else if (argc == 1) {
		// set
		enable = luaL_checkinteger(L, 1);
		
		ret = rtk_aldp_enable_set(enable);
		
		lua_pushnumber(L, ret);
		return 1;
	}
	else {
		lua_pushnumber(L, RT_ERR_INPUT);
		return 1;
	}
}



// ================= Storm =====================

// Lua: status[, rate, ifg_include] = rtl8370.storm_controlRate(port, storm_type, mode[, rate, ifg_include])
static int rtl8370_storm_controlRate(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	rtk_api_ret_t ret;
	rtk_port_t port;
	rtk_rate_storm_group_t storm_type;
	rtk_rate_t rate;
	rtk_enable_t ifg_include;
	uint32 mode;

	if (argc == 3) {
		// get
		port = luaL_checkinteger(L, 1);
		storm_type = luaL_checkinteger(L, 2);
		mode = luaL_checkinteger(L, 3);
		
		ret = rtk_storm_controlRate_get(port, storm_type, (rtk_rate_t *)&rate, 
						(rtk_enable_t *)&ifg_include, mode);

		lua_pushnumber(L, ret);
		lua_pushnumber(L, rate);
		lua_pushnumber(L, ifg_include);
		
		return 3;
	}
	else if (argc == 5) {
		// set
		port = luaL_checkinteger(L, 1);
		storm_type = luaL_checkinteger(L, 2);
		mode = luaL_checkinteger(L, 3);
		rate = luaL_checkinteger(L, 4);
		ifg_include = luaL_checkinteger(L, 5);
		
		ret = rtk_storm_controlRate_set(port, storm_type, rate, ifg_include, mode);
		
		lua_pushnumber(L, ret);
		return 1;
	}
	else {
		lua_pushnumber(L, RT_ERR_INPUT);
		return 1;
	}
}

// Lua: status[, enable]= rtl8370.storm_bypass(type[, enable])
static int rtl8370_storm_bypass(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	rtk_api_ret_t ret;
	rtk_storm_bypass_t type;
	rtk_enable_t enable;

	if (argc == 1) {
		// get
		type = luaL_checkinteger(L, 1);
		
		ret = rtk_storm_bypass_get(type, &enable);

		lua_pushnumber(L, ret);
		lua_pushnumber(L, enable);
		
		return 2;
	}
	else if (argc == 2) {
		// set
		type = luaL_checkinteger(L, 1);
		enable = luaL_checkinteger(L, 2);
		
		ret = rtk_storm_bypass_set(type, enable);
		
		lua_pushnumber(L, ret);
		return 1;
	}
	else {
		lua_pushnumber(L, RT_ERR_INPUT);
		return 1;
	}
}



//=================== Stat ============================

// Lua: status = rtl8370.stat_globalReset()
static int rtl8370_stat_globalReset(lua_State* L)
{
	rtk_api_ret_t ret;
	
	ret = rtk_stat_global_reset();
	
	lua_pushnumber(L, ret);
	return 1;
}

// Lua: status = rtl8370.stat_portReset(port)
static int rtl8370_stat_portReset(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	rtk_api_ret_t ret;
	rtk_port_t port;

	if (argc == 1) {
		// get
		port = luaL_checkinteger(L, 1);
	
		ret = rtk_stat_port_reset(port);

		lua_pushnumber(L, ret);
		return 1;
	}
	else {
		lua_pushnumber(L, RT_ERR_INPUT);
		return 1;
	}
}

// Lua: status, counter = rtl8370.stat_global()		
// Note: cntr_idx only can be DOT1D_TP_LEARNED_ENTRY_DISCARDS_INDEX
// so argc should be 0
static int rtl8370_stat_global(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	rtk_api_ret_t ret;
	rtk_stat_counter_t cntr_idx;
	
	if (argc == 0) {
		// get		
		ret = rtk_stat_global_get(DOT1D_TP_LEARNED_ENTRY_DISCARDS_INDEX, &cntr_idx);

		lua_pushnumber(L, cntr_idx);
		return 1;
	}
	else {
		lua_pushnumber(L, RT_ERR_INPUT);
		return 1;
	}
}

// Lua: status, cntr = rtl8370.stat_port(port, cntr_idx)		
static int rtl8370_stat_port(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	rtk_api_ret_t ret;
	rtk_port_t port;
	rtk_stat_port_type_t cntr_idx;
	rtk_stat_counter_t cntr;
	
	if (argc == 2) {
		// get		
		port = luaL_checkinteger(L, 1);
		cntr_idx = luaL_checkinteger(L, 2);
		
		ret = rtk_stat_port_get(port, cntr_idx, &cntr);

		lua_pushnumber(L, cntr);
		return 1;
	}
	else {
		lua_pushnumber(L, RT_ERR_INPUT);
		return 1;
	}
}



//=================== Leaky ============================

// Lua: status[, enable]= rtl8370.leaky_vlan(type[, enable])
static int rtl8370_leaky_vlan(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	rtk_api_ret_t ret;
	rtk_leaky_type_t type;
	rtk_enable_t enable;

	if (argc == 1) {
		// get
		type = luaL_checkinteger(L, 1);
		
		ret = rtk_leaky_vlan_get(type, &enable);

		lua_pushnumber(L, ret);
		lua_pushnumber(L, enable);
		
		return 2;
	}
	else if (argc == 2) {
		// set
		type = luaL_checkinteger(L, 1);
		enable = luaL_checkinteger(L, 2);
		
		ret = rtk_leaky_vlan_set(type, enable);
		
		lua_pushnumber(L, ret);
		return 1;
	}
	else {
		lua_pushnumber(L, RT_ERR_INPUT);
		return 1;
	}
}

// Lua: status[, enable]= rtl8370.leaky_portIsolation(type[, enable])
static int rtl8370_leaky_portIsolation(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	rtk_api_ret_t ret;
	rtk_leaky_type_t type;
	rtk_enable_t enable;

	if (argc == 1) {
		// get
		type = luaL_checkinteger(L, 1);
		
		ret = rtk_leaky_portIsolation_get(type, &enable);

		lua_pushnumber(L, ret);
		lua_pushnumber(L, enable);
		
		return 2;
	}
	else if (argc == 2) {
		// set
		type = luaL_checkinteger(L, 1);
		enable = luaL_checkinteger(L, 2);
		
		ret = rtk_leaky_portIsolation_set(type, enable);
		
		lua_pushnumber(L, ret);
		return 1;
	}
	else {
		lua_pushnumber(L, RT_ERR_INPUT);
		return 1;
	}
}



//=================== CPU Port ============================

// Lua: status[, enable]= rtl8370.cpu_enable([enable])
static int rtl8370_cpu_enable(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	rtk_api_ret_t ret;
	rtk_enable_t enable;

	if (argc == 0) {
		// get
		enable = luaL_checkinteger(L, 1);
		
		ret = rtk_cpu_enable_get(&enable);

		lua_pushnumber(L, ret);
		lua_pushnumber(L, enable);
		
		return 2;
	}
	else if (argc == 1) {
		// set
		enable = luaL_checkinteger(L, 1);
		
		ret = rtk_cpu_enable_set(enable);
		
		lua_pushnumber(L, ret);
		return 1;
	}
	else {
		lua_pushnumber(L, RT_ERR_INPUT);
		return 1;
	}
}

// Lua: status[, port, mode]= rtl8370.cpu_tagPort([port, mode])
static int rtl8370_cpu_tagPort(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	rtk_api_ret_t ret;
	rtk_port_t port;
	rtk_cpu_insert_t mode;

	if (argc == 0) {
		// get
		
		ret = rtk_cpu_tagPort_get(&port, &mode);

		lua_pushnumber(L, ret);
		lua_pushnumber(L, port);
		lua_pushnumber(L, mode);
		
		return 3;
	}
	else if (argc == 2) {
		// set
		port = luaL_checkinteger(L, 1);
		mode = luaL_checkinteger(L, 2);
		
		ret = rtk_cpu_tagPort_set(port, mode);
		
		lua_pushnumber(L, ret);
		return 1;
	}
	else {
		lua_pushnumber(L, RT_ERR_INPUT);
		return 1;
	}
}



//=================== QoS ============================

// Lua: status = rtl8370.qos_init([queueNum])
static int rtl8370_qos_init(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	rtk_api_ret_t ret;
	rtk_queue_num_t queueNum;

	if (argc == 1) {
		
		queueNum = luaL_checkinteger(L, 1);
		
		ret = rtk_qos_init(queueNum);

		lua_pushnumber(L, ret);
		
		return 1;
	}
	else {
		lua_pushnumber(L, RT_ERR_INPUT);
		return 1;
	}
}

// Lua: status[, {port, dot1q, acl, dscp, cvlan, svlan, dmac, smac}] 
//			= rtl8370.qos_priSel({port, dot1q, acl, dscp, cvlan, svlan, dmac, smac})
static int rtl8370_qos_priSel(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	rtk_api_ret_t ret;
	rtk_priority_select_t priDec;

	if (argc == 0) {
		// get
		ret = rtk_qos_priSel_get(&priDec);

		lua_pushnumber(L, ret);
		lua_newtable(L);
		
		// To-do: optimize these shit code
		lua_pushstring(L, "port");	// key
		lua_pushnumber(L, priDec.port_pri);	// value
		lua_settable(L, -3);
		lua_pushstring(L, "dot1q");	
		lua_pushnumber(L, priDec.dot1q_pri);	
		lua_settable(L, -3);
		lua_pushstring(L, "acl");	
		lua_pushnumber(L, priDec.acl_pri);	
		lua_settable(L, -3);
		lua_pushstring(L, "dscp");	
		lua_pushnumber(L, priDec.dscp_pri);	
		lua_settable(L, -3);
		lua_pushstring(L, "cvlan");	
		lua_pushnumber(L, priDec.cvlan_pri);	
		lua_settable(L, -3);
		lua_pushstring(L, "svlan");	
		lua_pushnumber(L, priDec.svlan_pri);	
		lua_settable(L, -3);
		lua_pushstring(L, "dmac");	
		lua_pushnumber(L, priDec.dmac_pri);	
		lua_settable(L, -3);
		lua_pushstring(L, "smac");	
		lua_pushnumber(L, priDec.smac_pri);	
		lua_settable(L, -3);
		
		return 2;
	}
	else if (argc == 2) {
		// set
		// To-do: optimize these shit code
		lua_pushstring(L, "port");
		lua_gettable(L, -2);
		priDec.port_pri = luaL_checkinteger(L, 1);
		lua_pop(L, 1);
		
		lua_pushstring(L, "dot1q");
		lua_gettable(L, -2);
		priDec.dot1q_pri = luaL_checkinteger(L, 1);
		lua_pop(L, 1);
		
		lua_pushstring(L, "acl");
		lua_gettable(L, -2);
		priDec.acl_pri = luaL_checkinteger(L, 1);
		lua_pop(L, 1);
		
		lua_pushstring(L, "dscp");	
		lua_gettable(L, -2);
		priDec.dscp_pri = luaL_checkinteger(L, 1);
		lua_pop(L, 1);
		
		lua_pushstring(L, "cvlan");
		lua_gettable(L, -2);
		priDec.cvlan_pri = luaL_checkinteger(L, 1);
		lua_pop(L, 1);
		
		lua_pushstring(L, "svlan");
		lua_gettable(L, -2);
		priDec.svlan_pri = luaL_checkinteger(L, 1);
		lua_pop(L, 1);
		
		lua_pushstring(L, "dmac");
		lua_gettable(L, -2);
		priDec.dmac_pri = luaL_checkinteger(L, 1);
		lua_pop(L, 1);
		
		lua_pushstring(L, "smac");	
		lua_gettable(L, -2);
		priDec.smac_pri = luaL_checkinteger(L, 1);
		lua_pop(L, 1);
		
		ret = rtk_qos_priSel_set(&priDec);
		
		lua_pushnumber(L, ret);
		return 1;
	}
	else {
		lua_pushnumber(L, RT_ERR_INPUT);
		return 1;
	}
}

// Lua: status[, int_pri]= rtl8370.qos_1pPriRemap(dot1p_pri[, int_pri])
static int rtl8370_qos_1pPriRemap(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	rtk_api_ret_t ret;
	rtk_pri_t dot1p_pri;
	rtk_pri_t int_pri;

	if (argc == 1) {
		// get
		dot1p_pri = luaL_checkinteger(L, 1);
		
		ret = rtk_qos_1pPriRemap_get(dot1p_pri, &int_pri);

		lua_pushnumber(L, ret);
		lua_pushnumber(L, int_pri);
		
		return 2;
	}
	else if (argc == 2) {
		// set
		dot1p_pri = luaL_checkinteger(L, 1);
		int_pri = luaL_checkinteger(L, 2);
		
		ret = rtk_qos_1pPriRemap_set(dot1p_pri, int_pri);
		
		lua_pushnumber(L, ret);
		return 1;
	}
	else {
		lua_pushnumber(L, RT_ERR_INPUT);
		return 1;
	}
}

// Lua: status[, int_pri]= rtl8370.qos_dscpPriRemap(dscp[, int_pri])
static int rtl8370_qos_dscpPriRemap(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	rtk_api_ret_t ret;
	rtk_dscp_t dscp;
	rtk_pri_t int_pri;

	if (argc == 1) {
		// get
		dscp = luaL_checkinteger(L, 1);
		
		ret = rtk_qos_dscpPriRemap_get(dscp, &int_pri);

		lua_pushnumber(L, ret);
		lua_pushnumber(L, int_pri);
		
		return 2;
	}
	else if (argc == 2) {
		// set
		dscp = luaL_checkinteger(L, 1);
		int_pri = luaL_checkinteger(L, 2);
		
		ret = rtk_qos_dscpPriRemap_set(dscp, int_pri);
		
		lua_pushnumber(L, ret);
		return 1;
	}
	else {
		lua_pushnumber(L, RT_ERR_INPUT);
		return 1;
	}
}

// Lua: status[, int_pri]= rtl8370.qos_portPri(port[, int_pri])
static int rtl8370_qos_portPri(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	rtk_api_ret_t ret;
	rtk_port_t port;
	rtk_pri_t int_pri;

	if (argc == 1) {
		// get
		port = luaL_checkinteger(L, 1);
		
		ret = rtk_qos_portPri_get(port, &int_pri);

		lua_pushnumber(L, ret);
		lua_pushnumber(L, int_pri);
		
		return 2;
	}
	else if (argc == 2) {
		// set
		port = luaL_checkinteger(L, 1);
		int_pri = luaL_checkinteger(L, 2);
		
		ret = rtk_qos_portPri_set(port, int_pri);
		
		lua_pushnumber(L, ret);
		return 1;
	}
	else {
		lua_pushnumber(L, RT_ERR_INPUT);
		return 1;
	}
}

// Lua: status[, queue_num]= rtl8370.qos_queueNum(port[, queue_num])
static int rtl8370_qos_queueNum(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	rtk_api_ret_t ret;
	rtk_port_t port;
	rtk_queue_num_t queue_num;

	if (argc == 1) {
		// get
		port = luaL_checkinteger(L, 1);
		
		ret = rtk_qos_queueNum_get(port, &queue_num);

		lua_pushnumber(L, ret);
		lua_pushnumber(L, queue_num);
		
		return 2;
	}
	else if (argc == 2) {
		// set
		port = luaL_checkinteger(L, 1);
		queue_num = luaL_checkinteger(L, 2);
		
		ret = rtk_qos_queueNum_set(port, queue_num);
		
		lua_pushnumber(L, ret);
		return 1;
	}
	else {
		lua_pushnumber(L, RT_ERR_INPUT);
		return 1;
	}
}

// Lua: status[, {queue1_pri, ..., queue8_pri}] 
//			= rtl8370.qos_priMap(queue_num[, {queue1_pri, ..., queue8_pri}])
static int rtl8370_qos_priMap(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	rtk_api_ret_t ret;
	rtk_queue_num_t queue_num;
	rtk_qos_pri2queue_t pri2qid;

	if (argc == 1) {
		// get
		queue_num = luaL_checkinteger(L, 1);
		ret = rtk_qos_priMap_get(queue_num, &pri2qid);

		lua_pushnumber(L, ret);
		
		lua_newtable(L); 
		for (int i=0; i<8; i++) {
			lua_pushnumber(L, i+1);					// key
			lua_pushnumber(L, pri2qid.pri2queue[i]);// value
			lua_settable(L, -3);
		} 
		
		return 2;
	}
	else if (argc == 2) {
		// set
		queue_num = luaL_checkinteger(L, 1);
		
		for (int i=0; i<8; i++) {
			lua_pushnumber(L, i+1);		// push key
			lua_gettable(L, -2);		// get value at -1 from table at -2
			pri2qid.pri2queue[i] = luaL_checkinteger(L, -1);
			lua_pop(L, 1);
		}
		
		ret = rtk_qos_priMap_set(queue_num, &pri2qid);
		
		lua_pushnumber(L, ret);
		return 1;
	}
	else {
		lua_pushnumber(L, RT_ERR_INPUT);
		return 1;
	}
}

// Lua: status[, {queue1_weight, ..., queue8_weight}] 
//         	= rtl8370.qos_schedulingQueue(port[, {queue1_weight, ..., queue8_weight}])
static int rtl8370_qos_schedulingQueue(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	rtk_api_ret_t ret;
	rtk_port_t port;
	rtk_qos_queue_weights_t qWeights;

	if (argc == 1) {
		// get
		port = luaL_checkinteger(L, 1);
		ret = rtk_qos_schedulingQueue_get(port, &qWeights);

		lua_pushnumber(L, ret);
		
		lua_newtable(L); 
		for (int i=0; i<8; i++) {
			lua_pushnumber(L, i+1);					// key
			lua_pushnumber(L, qWeights.weights[i]);// value
			lua_settable(L, -3);
		} 
		
		return 2;
	}
	else if (argc == 2) {
		// set
		port = luaL_checkinteger(L, 1);
		
		for (int i=0; i<8; i++) {
			lua_pushnumber(L, i+1);		// push key
			lua_gettable(L, -2);		// get value at -1 from table at -2
			qWeights.weights[i] = luaL_checkinteger(L, -1);
			lua_pop(L, 1);
		}
		
		ret = rtk_qos_schedulingQueue_set(port, &qWeights);
		
		lua_pushnumber(L, ret);
		return 1;
	}
	else {
		lua_pushnumber(L, RT_ERR_INPUT);
		return 1;
	}
}

// Lua: status[, enable]= rtl8370.qos_1pRemarkEnable(port[, enable])
static int rtl8370_qos_1pRemarkEnable(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	rtk_api_ret_t ret;
	rtk_port_t port;
	rtk_enable_t enable;

	if (argc == 1) {
		// get
		port = luaL_checkinteger(L, 1);
		
		ret = rtk_qos_1pRemarkEnable_get(port, &enable);

		lua_pushnumber(L, ret);
		lua_pushnumber(L, enable);
		
		return 2;
	}
	else if (argc == 2) {
		// set
		port = luaL_checkinteger(L, 1);
		enable = luaL_checkinteger(L, 2);
		
		ret = rtk_qos_1pRemarkEnable_set(port, enable);
		
		lua_pushnumber(L, ret);
		return 1;
	}
	else {
		lua_pushnumber(L, RT_ERR_INPUT);
		return 1;
	}
}

// Lua: status[, dot1p_pri]= rtl8370.qos_1pRemark(int_pri[, dot1p_pri])
static int rtl8370_qos_1pRemark(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	rtk_api_ret_t ret;
	rtk_pri_t int_pri;
	rtk_pri_t dot1p_pri;

	if (argc == 1) {
		// get
		int_pri = luaL_checkinteger(L, 1);
		
		ret = rtk_qos_1pRemark_get(int_pri, &dot1p_pri);

		lua_pushnumber(L, ret);
		lua_pushnumber(L, dot1p_pri);
		
		return 2;
	}
	else if (argc == 2) {
		// set
		int_pri = luaL_checkinteger(L, 1);
		dot1p_pri = luaL_checkinteger(L, 2);
		
		ret = rtk_qos_1pRemark_set(int_pri, dot1p_pri);
		
		lua_pushnumber(L, ret);
		return 1;
	}
	else {
		lua_pushnumber(L, RT_ERR_INPUT);
		return 1;
	}
}

// Lua: status[, enable]= rtl8370.qos_dscpRemarkEnable(port[, enable])
static int rtl8370_qos_dscpRemarkEnable(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	rtk_api_ret_t ret;
	rtk_port_t port;
	rtk_enable_t enable;

	if (argc == 1) {
		// get
		port = luaL_checkinteger(L, 1);
		
		ret = rtk_qos_dscpRemarkEnable_get(port, &enable);

		lua_pushnumber(L, ret);
		lua_pushnumber(L, enable);
		
		return 2;
	}
	else if (argc == 2) {
		// set
		port = luaL_checkinteger(L, 1);
		enable = luaL_checkinteger(L, 2);
		
		ret = rtk_qos_dscpRemarkEnable_set(port, enable);
		
		lua_pushnumber(L, ret);
		return 1;
	}
	else {
		lua_pushnumber(L, RT_ERR_INPUT);
		return 1;
	}
}

// Lua: status[, dscp]= rtl8370.qos_dscpRemark(int_pri[, dscp])
static int rtl8370_qos_dscpRemark(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	rtk_api_ret_t ret;
	rtk_pri_t int_pri;
	rtk_dscp_t dscp;

	if (argc == 1) {
		// get
		int_pri = luaL_checkinteger(L, 1);
		
		ret = rtk_qos_dscpRemark_get(int_pri, &dscp);

		lua_pushnumber(L, ret);
		lua_pushnumber(L, dscp);
		
		return 2;
	}
	else if (argc == 2) {
		// set
		int_pri = luaL_checkinteger(L, 1);
		dscp = luaL_checkinteger(L, 2);
		
		ret = rtk_qos_dscpRemark_set(int_pri, dscp);
		
		lua_pushnumber(L, ret);
		return 1;
	}
	else {
		lua_pushnumber(L, RT_ERR_INPUT);
		return 1;
	}
}



//=================== Interrupt ============================
// Note: Only RTL8370M has external interrupt pin.

// Lua: status[, type] = rtl8370.int_polarity([type])
static int rtl8370_int_polarity(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	rtk_api_ret_t ret;
	rtk_int_polarity_t type;

	if (argc == 0) {
		// get
		ret = rtk_int_polarity_get(&type);

		lua_pushnumber(L, ret);
		lua_pushnumber(L, type);
		
		return 2;
	}
	else if (argc == 1) {
		//set
		type = luaL_checkinteger(L, 1);
		
		ret = rtk_int_polarity_set(type);

		lua_pushnumber(L, ret);
		
		return 1;
	}
	else {
		lua_pushnumber(L, RT_ERR_INPUT);
		return 1;
	}
}

// Lua: status[, enable]= rtl8370.int_control(type[, enable])
static int rtl8370_int_control(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	rtk_api_ret_t ret;
	rtk_int_type_t type;
	rtk_enable_t enable;

	if (argc == 1) {
		// get
		type = luaL_checkinteger(L, 1);
		
		ret = rtk_int_control_get(type, &enable);

		lua_pushnumber(L, ret);
		lua_pushnumber(L, enable);
		
		return 2;
	}
	else if (argc == 2) {
		// set
		type = luaL_checkinteger(L, 1);
		enable = luaL_checkinteger(L, 2);
		
		ret = rtk_int_control_set(type, enable);
		
		lua_pushnumber(L, ret);
		return 1;
	}
	else {
		lua_pushnumber(L, RT_ERR_INPUT);
		return 1;
	}
}

// Lua: status[, statusMask] = rtl8370.int_status([statusMask])
static int rtl8370_int_status(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	rtk_api_ret_t ret;
	rtk_int_status_t statusMask;

	if (argc == 0) {
		// get

		ret = rtk_int_status_get(&statusMask);

		lua_pushnumber(L, ret);
		lua_pushnumber(L, statusMask.value[0]);
		
		return 2;
	}
	else if (argc == 1) {
		//set
		statusMask.value[0] = luaL_checkinteger(L, 1);
		
		ret = rtk_int_status_set(statusMask);

		lua_pushnumber(L, ret);
		
		return 1;
	}
	else {
		lua_pushnumber(L, RT_ERR_INPUT);
		return 1;
	}
}

// Lua: status[, enable]= rtl8370.int_advanceInfo(type[, enable])
static int rtl8370_int_advanceInfo(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	rtk_api_ret_t ret;
	rtk_int_advType_t adv_type;
	rtk_int_info_t info;

	if (argc == 1) {
		// get
		adv_type = luaL_checkinteger(L, 1);
		
		ret = rtk_int_advanceInfo_get(adv_type, &info);

		lua_pushnumber(L, ret);
		lua_pushnumber(L, info);
		
		return 2;
	}
	else {
		lua_pushnumber(L, RT_ERR_INPUT);
		return 1;
	}
}



//============== Trap & Reserved Multicast Address =============

// Lua: status[, ucast_action]= rtl8370.trap_unknownUnicastPktAction(type[, ucast_action])
static int rtl8370_trap_unknownUnicastPktAction(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	rtk_api_ret_t ret;
	rtk_trap_ucast_type_t type;
	rtk_trap_ucast_action_t ucast_action;

	if (argc == 1) {
		// get
		type = luaL_checkinteger(L, 1);
		
		ret = rtk_trap_unknownUnicastPktAction_get(type, &ucast_action);

		lua_pushnumber(L, ret);
		lua_pushnumber(L, ucast_action);
		
		return 2;
	}
	else if (argc == 2) {
		// set
		type = luaL_checkinteger(L, 1);
		ucast_action = luaL_checkinteger(L, 2);
		
		ret = rtk_trap_unknownUnicastPktAction_set(type, ucast_action);

		lua_pushnumber(L, ret);
		return 1;
	}
	else {
		lua_pushnumber(L, RT_ERR_INPUT);
		return 1;
	}
}

// Lua: status[, mcast_action]= rtl8370.trap_unknownMcastPktAction(port, type[, mcast_action])
static int rtl8370_trap_unknownMcastPktAction(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	rtk_api_ret_t ret;
	rtk_port_t port;
	rtk_mcast_type_t type;
	rtk_trap_mcast_action_t mcast_action;

	if (argc == 2) {
		// get
		port = luaL_checkinteger(L, 1);
		type = luaL_checkinteger(L, 2);
		
		ret = rtk_trap_unknownMcastPktAction_get(port, type, &mcast_action);

		lua_pushnumber(L, ret);
		lua_pushnumber(L, mcast_action);
		return 2;
	}
	else if (argc == 3) {
		// set
		port = luaL_checkinteger(L, 1);
		type = luaL_checkinteger(L, 2);
		mcast_action = luaL_checkinteger(L, 3);
		
		ret = rtk_trap_unknownMcastPktAction_set(port, type, mcast_action);

		lua_pushnumber(L, ret);
		return 1;
	}
	else {
		lua_pushnumber(L, RT_ERR_INPUT);
		return 1;
	}
}

// Lua: status[, igmp_action]= rtl8370.trap_igmpCtrlPktAction(type[, igmp_action])
static int rtl8370_trap_igmpCtrlPktAction(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	rtk_api_ret_t ret;
	rtk_igmp_type_t type;
	rtk_trap_igmp_action_t igmp_action;

	if (argc == 1) {
		// get
		type = luaL_checkinteger(L, 1);
		
		ret = rtk_trap_igmpCtrlPktAction_get(type, &igmp_action);

		lua_pushnumber(L, ret);
		lua_pushnumber(L, igmp_action);
		
		return 2;
	}
	else if (argc == 2) {
		// set
		type = luaL_checkinteger(L, 1);
		igmp_action = luaL_checkinteger(L, 2);
		
		ret = rtk_trap_igmpCtrlPktAction_set(type, igmp_action);

		lua_pushnumber(L, ret);
		return 1;
	}
	else {
		lua_pushnumber(L, RT_ERR_INPUT);
		return 1;
	}
}

// Lua: status[, rma_action]= rtl8370.trap_rmaAction(mac[, rma_action])
static int rtl8370_trap_rmaAction(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	rtk_api_ret_t ret;
	rtk_mac_t rma_frame;
	rtk_trap_rma_action_t rma_action;
	rtk_mac_t * pRma_frame = &rma_frame;
	uint32 length;
	const char * rma_frame_char;
	char * rma_frame_char_buf;			// ......
	
	rma_frame_char = lua_tolstring(L, 1, &length);		// it's a const pointer
	os_memcpy(rma_frame_char_buf, rma_frame_char, length);
	rma_frame_char_buf[length] = '\0';
	rtl8370_tools_strmac_to_byte(rma_frame_char_buf, pRma_frame->octet);
	
	if (argc == 1) {
		// get
		ret = rtk_trap_rmaAction_get(&rma_frame, &rma_action);

		lua_pushnumber(L, ret);
		lua_pushnumber(L, rma_action);
		return 2;
	}
	else if (argc == 2) {
		// set
		rma_action = luaL_checkinteger(L, 2);
		ret = rtk_trap_rmaAction_set(&rma_frame, rma_action);

		lua_pushnumber(L, ret);
		return 1;
	}
	else {
		lua_pushnumber(L, RT_ERR_INPUT);
		return 1;
	}
}

// Lua: status[, enable]= rtl8370.trap_ethernetAv([enable])
static int rtl8370_trap_ethernetAv(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	rtk_api_ret_t ret;
	rtk_enable_t enable;

	if (argc == 0) {
		// get
		enable = luaL_checkinteger(L, 1);
		
		ret = rtk_trap_ethernetAv_get(&enable);

		lua_pushnumber(L, ret);
		lua_pushnumber(L, enable);
		
		return 2;
	}
	else if (argc == 1) {
		// set
		enable = luaL_checkinteger(L, 1);
		
		ret = rtk_trap_ethernetAv_set(enable);
		
		lua_pushnumber(L, ret);
		return 1;
	}
	else {
		lua_pushnumber(L, RT_ERR_INPUT);
		return 1;
	}
}



//=================== ACL ============================

// Lua: status = rtl8370.acl_init()
static int rtl8370_acl_init(lua_State* L)
{
	rtk_api_ret_t ret;
	
	ret = rtk_filter_igrAcl_init();
	
	lua_pushnumber(L, ret);
	return 1;
}

// Lua: status[, pCfg] = rtl8370.acl_createCfg(activePort, activePortMask, invert)
// return a pointer of config (lightuserdata in Lua)
static int rtl8370_acl_createCfg(lua_State* L)
{
	rtk_api_ret_t ret;
	rtk_filter_cfg_t * pCfg;
	
	pCfg = (rtk_filter_cfg_t *)os_malloc(sizeof(rtk_filter_cfg_t));
	
	if (NULL == pCfg) {
		ret = RT_ERR_INPUT;			// todo
		lua_pushnumber(L, ret);
		return 1;
	}	
	

	pCfg->activeport.dataType = FILTER_FIELD_DATA_MASK;		// default
	pCfg->activeport.value = luaL_checkinteger(L, 1);
	pCfg->activeport.mask = luaL_checkinteger(L, 2);
	pCfg->invert = luaL_checkinteger(L, 3);
	
	ret = RT_ERR_OK;
	lua_pushnumber(L, ret);
	lua_pushlightuserdata(L, (void *)pCfg);
	return 2;

		
}

// Lua: status = rtl8370.acl_freeCfg(pCfg)
static int rtl8370_acl_freeCfg(lua_State* L)
{
	rtk_api_ret_t ret = RT_ERR_OK;
	rtk_filter_cfg_t * pCfg;
	
	pCfg = (rtk_filter_cfg_t *)luaL_checkinteger(L, 1);
	
	if (NULL == pCfg) {
		ret = RT_ERR_INPUT;	
	}	
	
	rtk_filter_field_t * pFieldTemp;
	while (NULL != pCfg->fieldHead) {
		pFieldTemp = pCfg->fieldHead->next;
		os_free(pCfg->fieldHead);
		pCfg->fieldHead = pFieldTemp;
	}
	
	os_free(pCfg);
	pCfg = NULL;
	
	lua_pushnumber(L, ret);
	return 1;	
}

// Lua: status = rtl8370.acl_freeAction(Action)
static int rtl8370_acl_freeAction(lua_State* L)
{
	rtk_api_ret_t ret = RT_ERR_OK;
	rtk_filter_action_t * pAction;
	
	pAction = (rtk_filter_action_t *)luaL_checkinteger(L, 1);
	
	if (NULL == pAction) {
		ret = RT_ERR_INPUT;	
	}	
	
	os_free(pAction);
	pAction = NULL;
	
	lua_pushnumber(L, ret);
	return 1;	
}

// Lua: status, pAction = rtl8370.acl_createAction()
// return a pointer of action (lightuserdata in Lua)
static int rtl8370_acl_createAction(lua_State* L)
{
	rtk_api_ret_t ret;
	rtk_filter_action_t * pAction;
	
	pAction = (rtk_filter_action_t *)os_malloc(sizeof(rtk_filter_action_t));
	
	if (NULL == pAction) {
		ret = RT_ERR_INPUT;			// todo
		lua_pushnumber(L, ret);
		return 1;
	}	
	os_memset(pAction, 0, sizeof(rtk_filter_action_t));
	ret = RT_ERR_OK;
	lua_pushnumber(L, ret);
	lua_pushlightuserdata(L, (void *)pAction);
	return 2;	
}

// Lua: status = rtl8370.acl_addAction(pAction, act[, args ...])
/*
	status = rtl8370.acl_addAction(pAction, "cIndex", cidx)
	status = rtl8370.acl_addAction(pAction, "cVid", cidx, cvid)
	status = rtl8370.acl_addAction(pAction, "sIndex", sidx)
	--status = rtl8370.acl_addAction(pAction, "policing", idx)	-- ?
	status = rtl8370.acl_addAction(pAction, "trap")
	status = rtl8370.acl_addAction(pAction, "copy")
	status = rtl8370.acl_addAction(pAction, "redirect", portmask)
	status = rtl8370.acl_addAction(pAction, "drop")
	status = rtl8370.acl_addAction(pAction, "mirror")
	status = rtl8370.acl_addAction(pAction, "dstPort", portmask)
	status = rtl8370.acl_addAction(pAction, "priority", priority)	-- 0~7, dot1p priority
	status = rtl8370.acl_addAction(pAction, "sVid", svid)
*/
CONST_T static char * rtl8370_acl_action_type_char[] = {
	"cIndex", "cVid", "sIndex", "policing", "trap", 	// 5
	"copy", "redirect", "drop", "mirror", "dstPort", 		// 10
	"priority", "sVid", NULL
};
static int rtl8370_acl_addAction(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	rtk_api_ret_t ret;
	rtk_filter_action_t * pAction;
	
	const char * buf_char;
	size_t buf_char_length;
	
	// init pAction
	pAction = (rtk_filter_action_t *) luaL_checkinteger(L, 1);
	
	// switch action type
	rtk_filter_act_enable_t i = 0;
	buf_char = lua_tolstring(L, 2, &buf_char_length);

	while (rtl8370_acl_action_type_char[i] != NULL) {
		if (0 == strcmp(rtl8370_acl_action_type_char[i], buf_char)) 
			break;	
		i++;
	}
	
	// rtl8370_acl_action_type_char[i] is type in rtk_filter_act_enable_t
	// get args
	pAction->actEnable[i] = TRUE;
	switch(i) {
		case FILTER_ENACT_INGRESS_CVLAN_INDEX:		 	
			pAction->filterIngressCvlanIdx = luaL_checkinteger(L, 3);
		break;
		case FILTER_ENACT_INGRESS_CVLAN_VID:	
			pAction->filterIngressCvlanVid = luaL_checkinteger(L, 3);		
			pAction->filterIngressCvlanIdx = luaL_checkinteger(L, 4);
		break;
		case FILTER_ENACT_EGRESS_SVLAN_INDEX:		
			pAction->filterIngressCvlanIdx = luaL_checkinteger(L, 3);
		break;
		case FILTER_ENACT_POLICING_0:		
			// To-do: How to use it?
		break;
		case FILTER_ENACT_REDIRECT:		
			pAction->filterRedirectPortmask = luaL_checkinteger(L, 3);
		break;
		case FILTER_ENACT_ADD_DSTPORT:		
			pAction->filterAddDstPortmask = luaL_checkinteger(L, 3);
		break;
		case FILTER_ENACT_PRIORITY:	
			pAction->filterPriority = luaL_checkinteger(L, 3);
		break;
		case FILTER_ENACT_EGRESS_SVLAN_VID:	
			pAction->filterEgressSvlanVid = luaL_checkinteger(L, 3);
		break;
		
		// No arg
		case FILTER_ENACT_TRAP_CPU:		
		case FILTER_ENACT_COPY_CPU:		
		case FILTER_ENACT_DROP:		
		case FILTER_ENACT_MIRROR:		
		
		break;
		
		// Wrong args
		default:
			ret = RT_ERR_FILTER_INACL_ACT_NOT_SUPPORT;
			lua_pushnumber(L, ret);
			return 1;
		break;	
	}
	
	lua_pushnumber(L, ret);
	return 1;	
}

// Lua: status= rtl8370.acl_addField2Cfg(pCfg, fieldType[, options ...])
/*
	I'll try these at first
	l2: dmac, smac
	l3: sip, dip
	l4: tcpsport, tcpdport
	
	Maybe later：
	l2: stag (SVLAN tag), etype (EtherType), ctag (CVLAN tag)
	l3: tos, flag, protocol, offset (?), 
	    sip6 (IPv6 Source Address), dip6 (IPv6 Destination Address), 
		6tclass (IPv6 Traffic Class), 6nheader (IPv6 Next Header)
	l4: tcpflag, udpsport, udpdport, icmpcode, icmptype, igmptype
	
	
	Lua: 
	-- L2:
	--							 config		mac					maskbit from start, 1~48
	status = rtl8370.acl_addField2Cfg(cfg, "dmac", "00:90:f5:23:73:85"[, 48])
	status = rtl8370.acl_addField2Cfg(cfg, "smac", "00:90:f5:23:73:86"[, 48])
	--------						
	status = rtl8370.acl_addField2Cfg(cfg, "stag", pri, cfi, vid)
	status = rtl8370.acl_addField2Cfg(cfg, "ctag", pri, cfi, vid)
	status = rtl8370.acl_addField2Cfg(cfg, "etype", "0800")			-- IPv4 Packet 
	
	-- L3:
	--							 config		ip address		maskbit from start, 1~32
	status = rtl8370.acl_addField2Cfg(cfg, "sip", "192.168.0.111"[, 32])
	status = rtl8370.acl_addField2Cfg(cfg, "dip", "192.168.0.112"[, 32])
	--------
	status = rtl8370.acl_addField2Cfg(cfg, "tos", tos)
	status = rtl8370.acl_addField2Cfg(cfg, "flag", flag)
	status = rtl8370.acl_addField2Cfg(cfg, "protocol", protocol)
	status = rtl8370.acl_addField2Cfg(cfg, "offset", offset)
	status = rtl8370.acl_addField2Cfg(cfg, "sip6", "2001:250:3000:3ca0::55"[, 128])
	status = rtl8370.acl_addField2Cfg(cfg, "dip6", "2001:250:3000:3ca0::55"[, 128])
	status = rtl8370.acl_addField2Cfg(cfg, "6tclass", class)
	status = rtl8370.acl_addField2Cfg(cfg, "6nheader", nheader)
	
	-- L4:
	status = rtl8370.acl_addField2Cfg(cfg, "tcpsport", 443[, 0xffff])	-- port, mask
	status = rtl8370.acl_addField2Cfg(cfg, "tcpdport", 22[, 0xfff0])
	--------
	status = rtl8370.acl_addField2Cfg(cfg, "tcpflag", tcpflag)
	status = rtl8370.acl_addField2Cfg(cfg, "udpsport", udpsport)
	status = rtl8370.acl_addField2Cfg(cfg, "udpdport", udpdport)
	status = rtl8370.acl_addField2Cfg(cfg, "icmpcode", icmpcode)
	status = rtl8370.acl_addField2Cfg(cfg, "icmptype", icmptype)
	status = rtl8370.acl_addField2Cfg(cfg, "igmptype", igmptype)
	
	
*/
CONST_T static char * rtl8370_acl_field_type_char[] = {
	"dmac", "smac", "etype", "stag", "ctag", //4
	"sip", "dip", "tos", "protocol", "flag", "offset",//10
	"sip6", "dip6", "6tclass", "6nheader",	//14
	"tcpsport", "tcpdport", "tcpflag", "udpsport", "udpdport",//19
	"icmpcode", "icmptype", "igmptype", NULL
};
static int rtl8370_acl_addField2Cfg(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	rtk_api_ret_t ret;
	rtk_filter_cfg_t * pCfg;
	rtk_filter_field_t * newField;
	const char * field_type_char;
	size_t field_type_char_length;
	size_t length;	
	const char * mac_char;
	char * mac_char_buf;			// ......
	
	// init cfg and field
	pCfg = (rtk_filter_cfg_t *) luaL_checkinteger(L, 1);
	newField = (rtk_filter_field_t *)os_malloc(sizeof(rtk_filter_field_t));
	
	// switch field type
	rtk_filter_field_type_t i = 0;
	field_type_char = lua_tolstring(L, 2, &field_type_char_length);

	while (rtl8370_acl_field_type_char[i] != NULL) {
		if (0 == strcmp(rtl8370_acl_field_type_char[i], field_type_char)) 
			break;	
		i++;
	}
	
	// rtl8370_acl_field_type_char[i] is type
	newField->fieldType = i;
	switch(i) {
		case FILTER_FIELD_DMAC:		// smac and dmac are same rtk_mac_t 				
		case FILTER_FIELD_SMAC:		
			newField->filter_pattern_union.smac.dataType = FILTER_FIELD_DATA_MASK;
			mac_char = lua_tolstring(L, 3, &length);		
			os_memcpy(mac_char_buf, mac_char, length);
			mac_char_buf[length] = '\0';
			rtl8370_tools_strmac_to_byte(mac_char_buf, newField->filter_pattern_union.smac.value.octet);
			if (argc == 4) {
				// specified mask
				int mask_bits = luaL_checkinteger(L, 4);
				os_memset(newField->filter_pattern_union.smac.mask.octet, 0, ETHER_ADDR_LEN);
				for (int j=0; j<mask_bits; j++) {
					*((newField->filter_pattern_union.smac.mask.octet)+(j/8)) &= (1<<(7-(j%8)));
				}
			} else {
				// default mask all
				os_memset(newField->filter_pattern_union.smac.mask.octet, 0xff, ETHER_ADDR_LEN);
			}	
		break;
		case FILTER_FIELD_IPV4_SIP:		// sip and dip are same rtk_filter_ip_t
		case FILTER_FIELD_IPV4_DIP:		
			newField->filter_pattern_union.dip.dataType = FILTER_FIELD_DATA_MASK;
			mac_char = lua_tolstring(L, 3, &length);		
			os_memcpy(mac_char_buf, mac_char, length);
			mac_char_buf[length] = '\0';
			rtl8370_tools_strip_to_byte(mac_char_buf, (uint8 *)(&(newField->filter_pattern_union.dip.value)));
			if (argc == 4) {
				// specified mask
				int mask_bits = luaL_checkinteger(L, 4);
				os_memset(&(newField->filter_pattern_union.dip.mask), 0, 4);		// clear
				for (int j=0; j<mask_bits; j++) {
					*((uint8 *)((&(newField->filter_pattern_union.dip.mask))+(j/8))) &= (1<<(7-(j%8)));
				}
			} else {
				// default mask all
				os_memset(&(newField->filter_pattern_union.dip.mask), 0xff, 4);
			}
		break;
		case FILTER_FIELD_TCP_SPORT:	
		case FILTER_FIELD_TCP_DPORT:	
			newField->filter_pattern_union.tcpSrcPort.dataType = FILTER_FIELD_DATA_MASK;
			newField->filter_pattern_union.tcpSrcPort.value = luaL_checkinteger(L, 3);	
			if (argc == 4) {
				// specified mask
				newField->filter_pattern_union.tcpSrcPort.mask = luaL_checkinteger(L, 4);
			} else {
				// default mask all
				newField->filter_pattern_union.tcpSrcPort.mask = 0xffff;
			}
		break;		
		default:
			// 因为懒还没写的23333
			ret = RT_ERR_FILTER_INACL_RULE_NOT_SUPPORT;
			lua_pushnumber(L, ret);
			return 1;
		break;	
	}
	
	// ready to link newField to cfg
	ret = rtk_filter_igrAcl_field_add(pCfg, newField);
	lua_pushnumber(L, ret);
	return 1;	
}

// Lua: status, ruleNum = rtl8370.acl_cfg("add", filter_id, cfg, action)
/* 
	status, ruleNum = rtl8370.acl_cfg("add", filter_id, cfg, action)
	status = rtl8370.acl_cfg("del", filter_id)
	status = rtl8370.acl_cfg("delall")
	-- status, cfg, action = rtl8370.acl_cfg("get", filter_id)	-- 可能没啥用
	--status = rtl8370.acl_cfg("set", filter_id, template_index, cfg, action)
*/
CONST_T static char * rtl8370_acl_cfg_char[6] = {
	"add", "del", "delall", "get", "set", NULL
};
static int rtl8370_acl_cfg(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	const char * buf_char;
	size_t buf_char_length;
	int i;
	buf_char = lua_tolstring(L, 1, &buf_char_length);
	
	rtk_api_ret_t ret = RT_ERR_OK;
	rtk_filter_id_t filter_id;
	rtk_filter_cfg_t * pCfg;
	rtk_filter_action_t * pAction;
	rtk_filter_number_t ruleNum;
	
	
	while (rtl8370_acl_cfg_char[i] != NULL) {
		if (0 == strcmp(rtl8370_acl_cfg_char[i], buf_char)) 
			break;	
		i++;
	}
	
	switch(i) {
		case 0:	// add		 		
			filter_id = luaL_checkinteger(L, 2); 
			pCfg = (rtk_filter_cfg_t *)luaL_checkinteger(L, 3);
			pAction = (rtk_filter_action_t *)luaL_checkinteger(L, 4);
			ret = rtk_filter_igrAcl_cfg_add(filter_id, pCfg, pAction, &ruleNum);
			lua_pushnumber(L, ret);
			lua_pushnumber(L, ruleNum);
			return 2;
		break;	
		case 1:	// del
			filter_id = luaL_checkinteger(L, 2); 
			ret = rtk_filter_igrAcl_cfg_del(filter_id);
			lua_pushnumber(L, ret);
			return 1;
		break;
		case 2:	// delall	
			ret = rtk_filter_igrAcl_cfg_delAll();
			lua_pushnumber(L, ret);
			return 1;
		break;
		case 3:	// get	
		case 4:	// set	
			// PASS
			lua_pushnumber(L, ret);
			return 1;
		break;
		// Wrong args
		default:
			ret = RT_ERR_INPUT;
			lua_pushnumber(L, ret);
			return 1;
		break;	
	}
	
	lua_pushnumber(L, ret);
	return 1;	
}

// Lua: status = rtl8370.acl_unmatchAction(port, action)
/* 
	status = rtl8370.acl_unmatchAction(port, action)
	status, action = rtl8370.acl_unmatchAction(port)
*/
static int rtl8370_acl_unmatchAction(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	rtk_api_ret_t ret;
	rtk_port_t port;
	rtk_filter_unmatch_action_t action;

	if (argc == 1) {
		// get
		port = luaL_checkinteger(L, 1);
		
		ret = rtk_filter_igrAcl_unmatchAction_get(port, &action);

		lua_pushnumber(L, ret);
		lua_pushnumber(L, action);
		
		return 2;
	}
	else if (argc == 2) {
		// set
		port = luaL_checkinteger(L, 1);
		action = luaL_checkinteger(L, 2);
		
		ret = rtk_filter_igrAcl_unmatchAction_set(port, action);
		
		lua_pushnumber(L, ret);
		return 1;
	}
	else {
		lua_pushnumber(L, RT_ERR_INPUT);
		return 1;
	}
}

// Lua: status = rtl8370.acl_state(port, state)
/* 
	status = rtl8370.acl_state(port, state)
	status, state = rtl8370.acl_state(port)
*/
static int rtl8370_acl_state(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	rtk_api_ret_t ret;
	rtk_port_t port;
	rtk_filter_state_t state;

	if (argc == 1) {
		// get
		port = luaL_checkinteger(L, 1);
		
		ret = rtk_filter_igrAcl_state_get(port, &state);

		lua_pushnumber(L, ret);
		lua_pushnumber(L, state);
		
		return 2;
	}
	else if (argc == 2) {
		// set
		port = luaL_checkinteger(L, 1);
		state = luaL_checkinteger(L, 2);
		
		ret = rtk_filter_igrAcl_state_set(port, state);
		
		lua_pushnumber(L, ret);
		return 1;
	}
	else {
		lua_pushnumber(L, RT_ERR_INPUT);
		return 1;
	}
}

// Lua: status = rtl8370.acl_template(index, template_table)
/* 
	status = rtl8370.acl_template(index, template_table)
	status, template_table = rtl8370.acl_template(index)
	index: 0~4
	template_table[7]: {0,1,2,3,4,5,6}	
*/
static int rtl8370_acl_template(lua_State* L)
{
	uint32_t argc = lua_gettop(L);
	rtk_api_ret_t ret;
	uint32 index;
	rtk_filter_template_t aclTemplate;
	aclTemplate.index = luaL_checkinteger(L, 1);
	
	if (argc == 1) {
		// get

		ret = rtk_filter_igrAcl_template_get(&aclTemplate);
		lua_pushnumber(L, ret);
		
		lua_newtable(L); 
		for (int i=0; i<RTK_MAX_NUM_OF_FILTER_FIELD; i++) {
			lua_pushnumber(L, i+1);					// key
			lua_pushnumber(L, aclTemplate.fieldType[i]);// value
			lua_settable(L, -3);
		} 
		return 2;
	}
	else if (argc == 2) {
		// set
		for (int i=0; i<RTK_MAX_NUM_OF_FILTER_FIELD; i++) {
			lua_pushnumber(L, i+1);		// push key
			lua_gettable(L, -2);		// get value at -1 from table at -2
			aclTemplate.fieldType[i] = luaL_checkinteger(L, -1);
			lua_pop(L, 1);
		}
		
		ret = rtk_filter_igrAcl_template_set(&aclTemplate);
		
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
	
	
	// spanning tree
	{ LSTRKEY( "stp_init" ), 					LFUNCVAL( rtl8370_stp_init )},
	{ LSTRKEY( "stp_mstpState" ), 				LFUNCVAL( rtl8370_stp_mstpState )},
	
	
	// port mirroring
	{ LSTRKEY( "mirror_portBased" ), 			LFUNCVAL( rtl8370_mirror_portBased )},
	{ LSTRKEY( "mirror_portIso" ), 				LFUNCVAL( rtl8370_mirror_portIso )},


	// Port Trunk
	{ LSTRKEY( "trunk_port" ), 					LFUNCVAL( rtl8370_trunk_port )},
	{ LSTRKEY( "trunk_distributionAlgorithm" ), LFUNCVAL( rtl8370_trunk_distributionAlgorithm )},
	{ LSTRKEY( "trunk_queueEmptyStatus" ), 		LFUNCVAL( rtl8370_trunk_queueEmptyStatus )},
	
	
	// Rate
	{ LSTRKEY( "rate_shareMeter" ), 			LFUNCVAL( rtl8370_rate_shareMeter )},
	{ LSTRKEY( "rate_igrBandwidthCtrlRate" ), 	LFUNCVAL( rtl8370_rate_igrBandwidthCtrlRate )},
	{ LSTRKEY( "rate_egrBandwidthCtrlRate" ), 	LFUNCVAL( rtl8370_rate_egrBandwidthCtrlRate )},
	{ LSTRKEY( "rate_egrQueueBwCtrlEnable" ), 	LFUNCVAL( rtl8370_rate_egrQueueBwCtrlEnable )},
	{ LSTRKEY( "rate_egrQueueBwCtrlRate" ), 	LFUNCVAL( rtl8370_rate_egrQueueBwCtrlRate )},

	
	// EEE
	{ LSTRKEY( "eee_init" ), 					LFUNCVAL( rtl8370_eee_init )},
	{ LSTRKEY( "eee_portEnable" ), 				LFUNCVAL( rtl8370_eee_portEnable )},


	// ALDP
	{ LSTRKEY( "aldp_init" ), 					LFUNCVAL( rtl8370_aldp_init )},
	{ LSTRKEY( "aldp_enable" ), 				LFUNCVAL( rtl8370_aldp_enable )},

	
	// Storm
	{ LSTRKEY( "storm_controlRate" ), 			LFUNCVAL( rtl8370_storm_controlRate )},
	{ LSTRKEY( "storm_bypass" ), 				LFUNCVAL( rtl8370_storm_bypass )},

	
	// Statistics
	{ LSTRKEY( "stat_globalReset" ), 			LFUNCVAL( rtl8370_stat_globalReset )},
	{ LSTRKEY( "stat_portReset" ), 				LFUNCVAL( rtl8370_stat_portReset )},
	{ LSTRKEY( "stat_global" ), 				LFUNCVAL( rtl8370_stat_global )},
	{ LSTRKEY( "stat_port" ), 					LFUNCVAL( rtl8370_stat_port )},

	
	// Leaky
	{ LSTRKEY( "leaky_vlan" ), 					LFUNCVAL( rtl8370_leaky_vlan )},
	{ LSTRKEY( "leaky_portIsolation" ), 		LFUNCVAL( rtl8370_leaky_portIsolation )},
	
	
	// CPU Port
	{ LSTRKEY( "cpu_enable" ), 					LFUNCVAL( rtl8370_cpu_enable )},
	{ LSTRKEY( "cpu_tagPort" ), 				LFUNCVAL( rtl8370_cpu_tagPort )},
	
	
	// QoS
	{ LSTRKEY( "qos_init" ), 					LFUNCVAL( rtl8370_qos_init )},
	{ LSTRKEY( "qos_priSel" ), 					LFUNCVAL( rtl8370_qos_priSel )},
	{ LSTRKEY( "qos_1pPriRemap" ), 				LFUNCVAL( rtl8370_qos_1pPriRemap )},
	{ LSTRKEY( "qos_dscpPriRemap" ), 			LFUNCVAL( rtl8370_qos_dscpPriRemap )},
	{ LSTRKEY( "qos_portPri" ), 				LFUNCVAL( rtl8370_qos_portPri )},
	{ LSTRKEY( "qos_queueNum" ), 				LFUNCVAL( rtl8370_qos_queueNum )},
	{ LSTRKEY( "qos_priMap" ), 					LFUNCVAL( rtl8370_qos_priMap )},
	{ LSTRKEY( "qos_schedulingQueue" ), 		LFUNCVAL( rtl8370_qos_schedulingQueue )},
	{ LSTRKEY( "qos_1pRemarkEnable" ), 			LFUNCVAL( rtl8370_qos_1pRemarkEnable )},
	{ LSTRKEY( "qos_1pRemark" ), 				LFUNCVAL( rtl8370_qos_1pRemark )},
	{ LSTRKEY( "qos_dscpRemarkEnable" ), 		LFUNCVAL( rtl8370_qos_dscpRemarkEnable )},
	{ LSTRKEY( "qos_dscpRemark" ), 				LFUNCVAL( rtl8370_qos_dscpRemark )},
	
	
	// Interrupt
	{ LSTRKEY( "int_polarity" ), 				LFUNCVAL( rtl8370_int_polarity )},
	{ LSTRKEY( "int_control" ), 				LFUNCVAL( rtl8370_int_control )},
	{ LSTRKEY( "int_status" ), 					LFUNCVAL( rtl8370_int_status )},
	{ LSTRKEY( "int_advanceInfo" ), 			LFUNCVAL( rtl8370_int_advanceInfo )},

	
	// Trap & Reserved Multicast Address
	{ LSTRKEY( "trap_unknownUnicastPktAction" ),LFUNCVAL( rtl8370_trap_unknownUnicastPktAction )},
	{ LSTRKEY( "trap_unknownMcastPktAction" ), 	LFUNCVAL( rtl8370_trap_unknownMcastPktAction )},
	{ LSTRKEY( "trap_igmpCtrlPktAction" ), 		LFUNCVAL( rtl8370_trap_igmpCtrlPktAction )},
	{ LSTRKEY( "trap_rmaAction" ), 				LFUNCVAL( rtl8370_trap_rmaAction )},
	{ LSTRKEY( "trap_ethernetAv" ), 			LFUNCVAL( rtl8370_trap_ethernetAv )},
		
		
	// ACL
	{ LSTRKEY( "acl_init" ), 					LFUNCVAL( rtl8370_acl_init )},
	{ LSTRKEY( "acl_createCfg" ), 				LFUNCVAL( rtl8370_acl_createCfg )},
	{ LSTRKEY( "acl_createAction" ), 			LFUNCVAL( rtl8370_acl_createAction )},
	{ LSTRKEY( "acl_freeCfg" ), 				LFUNCVAL( rtl8370_acl_freeCfg )},
	{ LSTRKEY( "acl_freeAction" ), 				LFUNCVAL( rtl8370_acl_freeAction )},
	{ LSTRKEY( "acl_addAction" ), 				LFUNCVAL( rtl8370_acl_addAction )},
	{ LSTRKEY( "acl_addField2Cfg" ), 			LFUNCVAL( rtl8370_acl_addField2Cfg )},
	{ LSTRKEY( "acl_cfg" ), 					LFUNCVAL( rtl8370_acl_cfg )},
	{ LSTRKEY( "acl_unmatchAction" ), 			LFUNCVAL( rtl8370_acl_unmatchAction )},
	{ LSTRKEY( "acl_state" ), 					LFUNCVAL( rtl8370_acl_state )},
	{ LSTRKEY( "acl_template" ), 				LFUNCVAL( rtl8370_acl_template )},
	
		
	
	
	// Return numbers
	{ LSTRKEY( "RT_ERR_OK" ), 					LNUMVAL( RT_ERR_OK ) },
	{ LSTRKEY( "RT_ERR_FAILED" ), 				LNUMVAL( RT_ERR_FAILED ) },
	{ LSTRKEY( "RT_ERR_SMI" ), 					LNUMVAL( RT_ERR_SMI ) },
	{ LSTRKEY( "RT_ERR_INPUT" ), 				LNUMVAL( RT_ERR_INPUT ) },
	{ LSTRKEY( "RT_ERR_PORT_ID" ), 				LNUMVAL( RT_ERR_PORT_ID ) },
	{ LSTRKEY( "RT_ERR_VLAN_ACCEPT_FRAME_TYPE"),LNUMVAL( RT_ERR_VLAN_ACCEPT_FRAME_TYPE ) },
	{ LSTRKEY( "RT_ERR_VLAN_VID" ), 			LNUMVAL( RT_ERR_VLAN_VID ) },
	{ LSTRKEY( "RT_ERR_VLAN_PRIORITY" ), 		LNUMVAL( RT_ERR_VLAN_PRIORITY ) },
	{ LSTRKEY( "RT_ERR_TBL_FULL" ), 			LNUMVAL( RT_ERR_TBL_FULL ) },
	{ LSTRKEY( "RT_ERR_OUT_OF_RANGE" ), 		LNUMVAL( RT_ERR_OUT_OF_RANGE ) },
	{ LSTRKEY( "RT_ERR_MSTI" ), 				LNUMVAL( RT_ERR_MSTI ) },
	{ LSTRKEY( "RT_ERR_MSTP_STATE" ), 			LNUMVAL( RT_ERR_MSTP_STATE ) },
	{ LSTRKEY( "RT_ERR_PORT_MASK" ), 			LNUMVAL( RT_ERR_PORT_MASK ) },
	{ LSTRKEY( "RT_ERR_LA_TRUNK_ID" ), 			LNUMVAL( RT_ERR_LA_TRUNK_ID ) },
	{ LSTRKEY( "RT_ERR_LA_HASHMASK" ), 			LNUMVAL( RT_ERR_LA_HASHMASK ) },
	{ LSTRKEY( "RT_ERR_FILTER_METER_ID" ), 		LNUMVAL( RT_ERR_FILTER_METER_ID ) },
	{ LSTRKEY( "RT_ERR_RATE" ), 				LNUMVAL( RT_ERR_RATE ) },
	{ LSTRKEY( "RT_ERR_ENABLE" ), 				LNUMVAL( RT_ERR_ENABLE ) },
	{ LSTRKEY( "RT_ERR_INBW_RATE" ), 			LNUMVAL( RT_ERR_INBW_RATE ) },
	{ LSTRKEY( "RT_ERR_QUEUE_ID" ), 			LNUMVAL( RT_ERR_QUEUE_ID ) },
	{ LSTRKEY( "RT_ERR_NULL_POINTER" ), 		LNUMVAL( RT_ERR_NULL_POINTER ) },
	{ LSTRKEY( "RT_ERR_SFC_UNKNOWN_GROUP" ), 	LNUMVAL( RT_ERR_SFC_UNKNOWN_GROUP ) },
	{ LSTRKEY( "RT_ERR_QUEUE_NUM" ), 			LNUMVAL( RT_ERR_QUEUE_NUM ) },
	{ LSTRKEY( "RT_ERR_QOS_SEL_PRI_SOURCE" ), 	LNUMVAL( RT_ERR_QOS_SEL_PRI_SOURCE ) },
	{ LSTRKEY( "RT_ERR_QOS_INT_PRIORITY" ), 	LNUMVAL( RT_ERR_QOS_INT_PRIORITY ) },
	{ LSTRKEY( "RT_ERR_QOS_DSCP_VALUE" ), 		LNUMVAL( RT_ERR_QOS_DSCP_VALUE ) },
	{ LSTRKEY( "RT_ERR_QOS_SEL_PORT_PRI" ), 	LNUMVAL( RT_ERR_QOS_SEL_PORT_PRI ) },
	{ LSTRKEY( "RT_ERR_QOS_QUEUE_WEIGHT" ), 	LNUMVAL( RT_ERR_QOS_QUEUE_WEIGHT ) },
	{ LSTRKEY( "RT_ERR_NOT_ALLOWED"),			LNUMVAL( RT_ERR_NOT_ALLOWED ) },
	{ LSTRKEY( "RT_ERR_ENTRY_INDEX"),			LNUMVAL( RT_ERR_ENTRY_INDEX ) },
	{ LSTRKEY( "RT_ERR_FILTER_INACL_ACT_NOT_SUPPORT"),	LNUMVAL( RT_ERR_FILTER_INACL_ACT_NOT_SUPPORT ) },
	{ LSTRKEY( "RT_ERR_FILTER_INACL_RULE_NOT_SUPPORT"),	LNUMVAL( RT_ERR_FILTER_INACL_RULE_NOT_SUPPORT ) },
	{ LSTRKEY( "RT_ERR_FILTER_ENTRYIDX"),		LNUMVAL( RT_ERR_FILTER_ENTRYIDX ) },
	

	
	
	// end
	{ LNILKEY, LNILVAL}
};

int luaopen_rtl8370(lua_State *L)
{
	// 

}

NODEMCU_MODULE(RTL8370, "rtl8370", rtl8370_map, luaopen_rtl8370);
