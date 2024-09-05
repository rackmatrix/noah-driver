# SPDX-License-Identifier: GPL-2.0-or-later
#
# Broachlink Noah2/Noah3/Noah4/Noah5/Noah6 board platform 
# Pyhon Library for managing the front LEDs through the /dev/noah in python
#
# Copyright (c) 2024 Rack Matrix Technology <www.rack-matrix.com>
# Author: David RENÉ <developers_at_rack-matrix.com>

from setuptools import setup, find_packages

setup(
    name='bl_noah',
    version='0.1',
    description='A Python library for managing front LEDs & push button of Broachlink Noah boards series.',
    author='Rack Matrix Technology (David RENÉ)',
    packages=find_packages(),
)
