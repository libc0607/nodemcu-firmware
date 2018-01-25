-- sda, sck connected to D5, D6 
-- With two external pull-up resistors.
-- IO must be 3.3v TTL
local sda_pin, sck_pin  = 5, 6

-- init switch
rtl8370.smi(sda_pin, sck_pin)
rtl8370.init()

-- Set LED to force mode
rtl8370.led_modeForce(0,3) 

-- On my switch, the LED is parallel mode (default) 
-- and all connected at group 0.
-- if your switch is scan mode, 
-- you should see led_operation()

-- Blink Port 4 LED
local v 
tmr.create():alarm(500, tmr.ALARM_AUTO, function()
  _, v = rtl8370.led_enable(0)
  if  v == 0 then
    rtl8370.led_enable(0,16)
  elseif v == 16 then
    rtl8370.led_enable(0,0)
  end
end)