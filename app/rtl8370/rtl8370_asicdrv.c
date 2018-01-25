/*
 * Copyright (C) 2009 Realtek Semiconductor Corp. 
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated, 
 * modified or distributed under the authorized license from Realtek. 
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER 
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED. 
 *
 * $Revision: 6658 $
 * $Date: 2009-10-30 14:49:58 +0800 (Fri, 30 Oct 2009) $
 *
 * Purpose : RTL8370 switch high-level API for RTL8367B
 * Feature : 
 *
 */
#include <rtl8370_asicdrv.h>
#include <smi.h>

/*=======================================================================
 * 1. Asic read/write driver through SMI
 *========================================================================*/
/*
@func ret_t | rtl8370_setAsicRegBit | Set a bit value of a specified register.
@parm uint32 | reg | Register's address.
@parm uint32 | bit | Bit location. For 16-bits register only. Maximun value is 15 for MSB location.
@parm uint32 | value | Value to set. It can be value 0 or 1.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_INPUT | Invalid input parameter. 
@comm
    Set a bit of a specified register to 1 or 0. It is 16-bits system of RTL8366s chip.
    
*/
ret_t rtl8370_setAsicRegBit(uint32 reg, uint32 bit, uint32 value)
{
    uint32 regData;
    ret_t retVal;
    
    if(bit>=RTL8370_REGBITLENGTH)
        return RT_ERR_INPUT;

    retVal = smi_read(reg, &regData);
    if(retVal != RT_ERR_OK) 
		return RT_ERR_SMI;
    if (value) 
        regData = regData | (1<<bit);
    else
        regData = regData & (~(1<<bit));
    
    retVal = smi_write(reg, regData);
	
    if (retVal != RT_ERR_OK) 
		return RT_ERR_SMI;
	
    return RT_ERR_OK;
}

ret_t rtl8370_getAsicRegBit(uint32 reg, uint32 bit, uint32 *value)
{

    uint32 regData;
    ret_t retVal;

    retVal = smi_read(reg, &regData);
    if (retVal != RT_ERR_OK) 
		return RT_ERR_SMI;
    
    *value = (regData & (0x1 << bit)) >> bit;    

    return RT_ERR_OK;
}


/*
@func ret_t | rtl8370_setAsicRegBits | Set bits value of a specified register.
@parm uint32 | reg | Register's address.
@parm uint32 | bits | Bits mask for setting. 
@parm uint32 | value | Bits value for setting. Value of bits will be set with mapping mask bit is 1.   
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_INPUT | Invalid input parameter. 
@comm
    Set bits of a specified register to value. Both bits and value are be treated as bit-mask.
    
*/
ret_t rtl8370_setAsicRegBits(uint32 reg, uint32 bits, uint32 value)
{
    uint32 regData;    
    ret_t retVal;    
    uint32 bitsShift;    
    uint32 valueShifted;        

    if(bits>= (1<<RTL8370_REGBITLENGTH) )
        return RT_ERR_INPUT;

    bitsShift = 0;
    while(!(bits & (1 << bitsShift)))
    {
        bitsShift++;
        if(bitsShift >= RTL8370_REGBITLENGTH)
            return RT_ERR_INPUT;
    }
    valueShifted = value << bitsShift;

    if(valueShifted > RTL8370_REGDATAMAX)
        return RT_ERR_INPUT;

    retVal = smi_read(reg, &regData);
    if (retVal != RT_ERR_OK) 
		return RT_ERR_SMI;

    regData = regData & (~bits);
    regData = regData | (valueShifted & bits);

    retVal = smi_write(reg, regData);
    if (retVal != RT_ERR_OK) 
		return RT_ERR_SMI;

    return RT_ERR_OK;
}


ret_t rtl8370_getAsicRegBits(uint32 reg, uint32 bits, uint32 *value)
{
    
    uint32 regData;    
    ret_t retVal;    
    uint32 bitsShift;    

    if(bits>= (1<<RTL8370_REGBITLENGTH) )
        return RT_ERR_INPUT;    

    bitsShift = 0;
    while(!(bits & (1 << bitsShift)))
    {
        bitsShift++;
        if(bitsShift >= RTL8370_REGBITLENGTH)
            return RT_ERR_INPUT;
    }
    
    retVal = smi_read(reg, &regData);
    if (retVal != RT_ERR_OK) 
		return RT_ERR_SMI;

    *value = (regData & bits) >> bitsShift;
	
    return RT_ERR_OK;
}

/*
@func ret_t | rtl8370_setAsicReg | Set content of asic register.
@parm uint32 | reg | Register's address.
@parm uint32 | value | Value setting to register.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@comm
    The value will be set to ASIC mapping address only and it is always return RT_ERR_OK while setting un-mapping address registers.
    
*/
ret_t rtl8370_setAsicReg(uint32 reg, uint32 value)
{
    ret_t retVal;

    retVal = smi_write(reg, value);
    if (retVal != RT_ERR_OK) 
		return RT_ERR_SMI;
	
    return RT_ERR_OK;
}


/*
@func ret_t | rtl8370_getAsicReg | Get content of register.
@parm uint32 | reg | Register's address.
@parm uint32* | value | Value of register.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@comm
     Value 0x0000 will be returned for ASIC un-mapping address.
    
*/
ret_t rtl8370_getAsicReg(uint32 reg, uint32 *value)
{

    uint32 regData;
    ret_t retVal;

    retVal = smi_read(reg, &regData);
    if (retVal != RT_ERR_OK) 
		return RT_ERR_SMI;

    *value = regData;

    return RT_ERR_OK;
}


