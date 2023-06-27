.. _alma-gen:

Geryon G071
######

Overview
********

The Geryon sample generates 3 PWM output with shift 120 degree for each channels.


.. _alma-gen-requirements:

Building and Running
********************

Build and flash Blinky as follows, changing ``geryon_g071`` for your board:

.. zephyr-app-commands::
   :zephyr-app: .
   :board: geryon_g071
   :goals: build flash
   :compact:

After flashing, you can check the output 3 pins PB0, PB3 and PA15
