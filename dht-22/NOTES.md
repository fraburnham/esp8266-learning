Basic steps for DHT-22 rea
https://github.com/adafruit/DHT-sensor-library/blob/master/DHT.cpp#L123
(It looks like there is more to do to get a temp value in the adafruit code, but the datasheet doesn't show anything else)

* set pullup for normal state (the line should be held high when idle)
  * maybe set as output & hold? Seems better to tie to the v+ w/ a pullup
  
* stop interrupts (timing critical code)
  * adafruit stops after the line is low for 20ms...
  
* pull line high for 250ms (this looks like a stability trick)
* low for 20ms (this looks like the real start of the signal)

* high for 40us

* switch to input w/ pullup
* 10us delay

* expect ~80us low then ~80us high

* begin read of 40 bits (by capturing high and low times, no time in this loop to process them)
  * each bit is a 50us low followed by a variable len high pusle
  * bit is 0 if high state time < low state time (~30us)
  * bit is 1 if high state time > low state time (~70us)

* process data struct
  * 16 bits of humidity (int, %RH)  ; looks like an unsigned? short int esp8266 is 32bit
  * 16 bits of temp (int, Deg C)  ; I bet this is a signed short int...
  * 8 bits of parity
    * result of adding 32 prev bits 8 at a time  ; so it is probably best to take them 8 bits at a time anyway

How do I read that data w/ interrupts?
Prep stays the same (basic hold and wait stuff, could use timers but this feels simpler (timers may be lower power when I want to go to battery...))
Save the state of interrupts now
Clear all of them (can't have the timing code interrupted, or can I? an interrupt will interrupt and interrupt... but that could cause me to write invalid time. I could disable interrupts _inside_ the interrupt, that seems most sensible and has least impact on other code, but what if they do that to me? or I forget and do it to myself?)
Have two arrays (maybe) high[] and low[] and a flag was_last_high then the main code loop checks if the last edge was falling from high to low, if so shift the data out
  What happens if _this_ process is interrupted? (it will be)
So instead try making the arrays big enough to hold all the data high[41][2] and low[42][2] (could stop at 41 and ignore the trailing low pulse) then math it out later?
  This seems most sensible. How will I track the bit offset? High to low edge will inc it, other code will read it
  Do I need two arrays? Could I do pulse_time[82][3] where the 0th index is the pin state? (seems pointless...)
