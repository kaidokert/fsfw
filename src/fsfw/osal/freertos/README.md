FreeRTOS Readme
=

## Main.cpp Notices

### Tasks

A FreeRTOS application needs to start 

> vTaskStartScheduler() 

before creating Tasks.
Keep this in mind for the mission dependent code!
This has to be done before the Task Factory is used. 