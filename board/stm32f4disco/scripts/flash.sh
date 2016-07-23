#!/bin/bash

openocd -f /usr/share/openocd/scripts/board/stm32f429discovery.cfg -f board/stm32f4disco/scripts/flash.cfg
