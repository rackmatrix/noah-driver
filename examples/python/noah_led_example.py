#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
#
# Broachlink Noah2/Noah3/Noah4/Noah5/Noah6 board platform 
# Sample code for managing the front LEDs through the /dev/noah in python
#
# Copyright (c) 2024 Rack Matrix Technology <www.rack-matrix.com>
# Author: David RENÉ <developers_at_rack-matrix.com>
 
from sys import stderr, exit
from time import sleep
import os
from bl_noah import bl_noah
	
if __name__ == '__main__':
	print('Rack Matrix Technology samples code for Noah board')
	if not bl_noah.is_Noah():
		print('This is not a Noah Board or the Rack Matrix driver not loaded')
	else:
		print('Noah managing front LEDs and button example\n')
		noah = bl_noah.Noah()
		current_status = noah.push_button()
		print("Noah push button status : ",bl_noah.to_String(current_status))
		noah.led_OFF(noah.LED1)
		noah.led_OFF(noah.LED2)
		noah.led_OFF(noah.LED3)
		
		while True:
			noah.led_ON(noah.LED1)
			sleep(0.050000)
			noah.led_OFF(noah.LED1)

			noah.led_ON(noah.LED2)
			sleep(0.050000)
			noah.led_OFF(noah.LED2)

			noah.led_ON(noah.LED3)
			sleep(0.050000);
			noah.led_OFF(noah.LED3)
			
			new_status = noah.push_button()
			if current_status != new_status:
				print("button status changed to :",bl_noah.to_String(new_status))
				current_status = new_status
			
			

