.. _osal:

Operating System Abstraction Layer (OSAL)
============================================

Some specific information on the provided OSALs are provided.

Linux
-------

This OSAL can be used to compile for Linux host systems like Ubuntu 20.04 or for
embedded Linux targets like the Raspberry Pi. This OSAL generally requires threading support
and real-time functionalities. For most UNIX systems, this is done by adding ``-lrt`` and
``-lpthread`` to the linked libraries in the compilation process. The CMake build support provided
will do this automatically for the ``fsfw`` target. It should be noted that most UNIX systems need
to be configured specifically to allow the real-time functionalities required by the FSFW.

Hosted OSAL
-------------------

This is the newest OSAL. Support for Semaphores has not been implemented yet and will propably be
implemented as soon as C++20 with Semaphore support has matured. This OSAL can be used to run the
FSFW on any host system, but currently has only been tested on Windows 10 and Ubuntu 20.04. Unlike
the other OSALs, it uses dynamic memory allocation (e.g. for the message queue implementation).
Cross-platform serial port (USB) support might be added soon.

FreeRTOS OSAL
------------------

FreeRTOS is not included and the developer needs to take care of compiling the FreeRTOS sources and
adding the  ``FreeRTOSConfig.h`` file location to the include path. This OSAL has only been tested
extensively with the pre-emptive scheduler configuration so far but it should in principle also be
possible to use a cooperative scheduler. It is recommended to use the `heap_4` allocation scheme.
When using newlib (nano), it is also recommended to add ``#define configUSE_NEWLIB_REENTRANT`` to
the FreeRTOS configuration file to ensure thread-safety.

When using this OSAL, developers also need to provide an implementation for the
``vRequestContextSwitchFromISR`` function. This has been done because the call to request a context
switch from an ISR is generally located in the ``portmacro.h`` header and is different depending on
the target architecture or device.

RTEMS OSAL
---------------

The RTEMS OSAL was the first implemented OSAL which is also used on the active satellite Flying Laptop.

TCP/IP socket abstraction
------------------------------

The Linux and Host OSAL provide abstraction layers for the socket API. Currently, only UDP sockets
have been imlemented. This is very useful to test TMTC handling either on the host computer
directly (targeting localhost with a TMTC application) or on embedded Linux devices, sending
TMTC packets via Ethernet.

Example Applications
----------------------

There are example applications available for each OSAL

- `Hosted OSAL <https://egit.irs.uni-stuttgart.de/fsfw/fsfw-example-hosted>`_
- `Linux OSAL for MCUs <https://egit.irs.uni-stuttgart.de/fsfw/fsfw-example-linux-mcu>`_
- `FreeRTOS OSAL on the STM32H743ZIT <https://egit.irs.uni-stuttgart.de/fsfw/fsfw-example-stm32h7-freertos>`_
- `RTEMS OSAL on the STM32H743ZIT <https://egit.irs.uni-stuttgart.de/fsfw/fsfw-example-stm32h7-rtems>`_
