.. Flight Software Framework documentation master file, created by
   sphinx-quickstart on Tue Nov 30 10:56:03 2021.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Flight Software Framework (FSFW) documentation
================================================

.. image:: ../misc/logo/FSFW_Logo_V3_bw.png
   :alt: FSFW Logo

The Flight Software Framework is a C++ Object Oriented Framework for unmanned,
automated systems like Satellites. 

The initial version of the Flight Software Framework was developed during
the Flying Laptop Project by the University of Stuttgart in cooperation
with Airbus Defence and Space GmbH.

Quick facts
---------------

The framework is designed for systems, which communicate with external devices, perform control
loops, receive telecommands and send telemetry, and need to maintain a high level of availability.
Therefore, a mode and health system provides control over the states of the software and the
controlled devices. In addition, a simple mechanism of event based fault detection, isolation and
recovery is implemented as well.

The FSFW provides abstraction layers for operating systems to provide a uniform operating system
abstraction layer (OSAL). Some components of this OSAL are required internally by the FSFW but is
also very useful for developers to implement the same application logic on different operating
systems with a uniform interface.

Currently, the FSFW provides the following OSALs:

- Linux
- Host 
- FreeRTOS
- RTEMS

The recommended hardware is a microprocessor with more than 1 MB of RAM and 1 MB of non-volatile
memory. For reference, current applications use a Cobham Gaisler UT699 (LEON3FT), a
ISISPACE IOBC or a Zynq-7020 SoC. The ``fsfw`` was also successfully run on the
STM32H743ZI-Nucleo board and on a Raspberry Pi and is currently running on the active
satellite mission Flying Laptop.

Index
-------

.. toctree::
   :maxdepth: 2
   :caption: Contents:

   getting_started
   highlevel
   core
   config
   osal
   pus
   devicehandlers
   controllers
   localpools
   api

Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
