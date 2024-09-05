# SPDX-License-Identifier: GPL-2.0-or-later
#
# Broachlink Noah2/Noah3/Noah4/Noah5/Noah6 board platform 
# Pyhon Library for managing the front LEDs through the /dev/noah in python
#
# Copyright (c) 2024 Rack Matrix Technology <www.rack-matrix.com>
# Author: David RENÉ <developers_at_rack-matrix.com>

import os
import os.path

def is_Noah():
	return os.path.exists('/dev/noah/led1')

def to_String(status):
	if status == "1":
		return "pushed"
	elif status == "0":
		return "unpushed"
	else:
		return "Unknown status"
		
class Noah:
	def push_button(self):
		try:
			with open('/dev/noah/button', 'r') as f:
				return f.read(1)
		except FileNotFoundError:
			return None
			
	def led_ON_OFF(self, led, on_off):
		try:
			with open('/dev/noah/led%d'%led, 'w') as f:
				f.write('1' if on_off else '0')
		except FileNotFoundError:
			return None
				
	def led_OFF(self, led):
		self.led_ON_OFF(led, False)
		

	def led_ON(self, led):
		self.led_ON_OFF(led, True)

	LED1 = 1
	LED2 = 2
	LED3 = 3