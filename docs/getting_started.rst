Getting Started
================


Getting started
----------------

The `Hosted FSFW example`_ provides a good starting point and a demo to see the FSFW capabilities.
It is recommended to get started by building and playing around with the demo application.
There are also other examples provided for all OSALs using the popular embedded platforms
Raspberry Pi, Beagle Bone Black and STM32H7.

Generally, the FSFW is included in a project by providing
a configuration folder, building the static library and linking against it. 
There are some functions like ``printChar`` which are different depending on the target architecture
and need to be implemented by the mission developer.

A template configuration folder was provided and can be copied into the project root to have
a starting point. The [configuration section](docs/README-config.md#top) provides more specific 
information about the possible options.

Prerequisites
-------------------

The Embedded Template Library (etl) is a dependency of the FSFW which is automatically
installed and provided by the build system unless the correction version was installed.
The current recommended version can be found inside the fsfw ``CMakeLists.txt`` file or by using
``ccmake`` and looking up the ``FSFW_ETL_LIB_MAJOR_VERSION`` variable.

You can install the ETL library like this. On Linux, it might be necessary to add ``sudo`` before
the install call:

.. code-block:: console

   git clone https://github.com/ETLCPP/etl
   cd etl
   git checkout <currentRecommendedVersion>
   mkdir build && cd build
   cmake ..
   cmake --install .

It is recommended to install ``20.27.2`` or newer for the package version handling of
ETL to work.

Adding the library
-------------------

The following steps show how to add and use FSFW components. It is still recommended to
try out the example mentioned above to get started, but the following steps show how to
add and link against the FSFW library in general.

1. Add this repository as a submodule

   .. code-block:: console

      git submodule add https://egit.irs.uni-stuttgart.de/fsfw/fsfw.git fsfw

2. Add the following directive inside the uppermost ``CMakeLists.txt`` file of your project

   .. code-block:: cmake

      add_subdirectory(fsfw)

3. Make sure to provide a configuration folder and supply the path to that folder with
   the `FSFW_CONFIG_PATH` CMake variable from the uppermost `CMakeLists.txt` file.
   It is also necessary to provide the `printChar` function. You can find an example
   implementation for a hosted build
   `here <https://egit.irs.uni-stuttgart.de/fsfw/fsfw-example-hosted/src/branch/master/bsp_hosted/utility/printChar.c>`_.

4. Link against the FSFW library

   .. code-block:: cmake

      target_link_libraries(<YourProjectName> PRIVATE fsfw)


5. It should now be possible use the FSFW as a static library from the user code.

Building the unittests
-------------------------

The FSFW also has unittests which use the `Catch2 library`_.
These are built by setting the CMake option ``FSFW_BUILD_UNITTESTS`` to ``ON`` or `TRUE`
from your project `CMakeLists.txt` file or from the command line.

You can install the Catch2 library, which prevents the build system to avoid re-downloading
the dependency if the unit tests are completely rebuilt. The current recommended version
can be found inside the fsfw ``CMakeLists.txt`` file or by using ``ccmake`` and looking up
the ``FSFW_CATCH2_LIB_VERSION`` variable.

.. code-block:: console

   git clone https://github.com/catchorg/Catch2.git
   cd Catch2
   git checkout <currentRecommendedVersion>
   cmake -Bbuild -H. -DBUILD_TESTING=OFF
   sudo cmake --build build/ --target install


The fsfw-tests binary will be built as part of the static library and dropped alongside it.
If the unittests are built, the library and the tests will be built with coverage information by
default. This can be disabled by setting the `FSFW_TESTS_COV_GEN` option to `OFF` or `FALSE`.

You can use the following commands inside the ``fsfw`` folder to set up the build system

.. code-block:: console

   mkdir build-tests && cd build-tests
   cmake -DFSFW_BUILD_TESTS=ON -DFSFW_OSAL=host ..


You can also use ``-DFSFW_OSAL=linux`` on Linux systems.

Coverage data in HTML format can be generated using the `Code coverage`_ CMake module.
To build the unittests, run them and then generare the coverage data in this format,
the following command can be used inside the build directory after the build system was set up

.. code-block:: console

   cmake --build . -- fsfw-tests_coverage -j


The ``helper.py`` script located in the ``script`` folder can also be used to create, build
and open the unittests conveniently. Try ``helper.py -h`` for more information.

Building the documentation
----------------------------

The FSFW documentation is built using the tools Sphinx, doxygen and breathe based on the
instructions provided in `this blogpost <https://devblogs.microsoft.com/cppblog/clear-functional-c-documentation-with-sphinx-breathe-doxygen-cmake/>`_. If you
want to do this locally, set up the prerequisites first. This requires a ``python3``
installation as well. Example here is for Ubuntu.

.. code-block:: console

   sudo apt-get install doxygen graphviz

And the following Python packages

.. code-block:: console

   python3 -m pip install sphinx breathe

You can set up a documentation build system using the following commands

.. code-block:: bash

   mkdir build-docs && cd build-docs
   cmake -DFSFW_BUILD_DOCS=ON -DFSFW_OSAL=host ..

Then you can generate the documentation using

.. code-block:: bash

   cmake --build . -j

You can find the generated documentation inside the ``docs/sphinx`` folder inside the build
folder. Simply open the ``index.html`` in the webbrowser of your choice.

The ``helper.py`` script located in the ``script`` folder can also be used to create, build
and open the documentation conveniently. Try ``helper.py -h`` for more information.

Formatting the source
-----------------------

The formatting is done by the ``clang-format`` tool. The configuration is contained within the
``.clang-format`` file in the repository root. As long as ``clang-format`` is installed, you
can run the ``apply-clang-format.sh`` helper script to format all source files consistently.


.. _`Hosted FSFW example`: https://egit.irs.uni-stuttgart.de/fsfw/fsfw-example-hosted
.. _`Catch2 library`: https://github.com/catchorg/Catch2
.. _`Code coverage`: https://github.com/bilke/cmake-modules/tree/master
