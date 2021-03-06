
![Banner](https://d.wattpad.com/story_parts/488157334/images/14f211bab33f56fc351331339007.gif)
=====

![License](https://img.shields.io/badge/License-AGPL3.0-blue.svg)![Version](https://img.shields.io/badge/mariOS-V1-green.svg)


mariOS
=====
mariOS Answers the Request for an Implementation of OS

---------
## About
The project implements a very easy and tiny core that supports:
  * tasks creation and local stack definition
  * fixed-priority scheduling
  * preemption and explicit task yield
  * task preemptive delay function
  * blocking and non-blocking queue-based tasks communication
  
Actually, it supports exclusively the ARM Cortex M3/M4 through the definition of two interrupt handlers and some other helpful machine-dependent functions.
Take the project as it is: easy to comprehend, small, ready-for-compiling over a STM32 toolchain (even though easily portable over others toolchains), ready for future extensions.

## Documentation
A draft of documentation is available from internal code documentation by doxygen.
For your comodity, check it out the docs folder: https://mariobarbareschi.github.io/mariOS/docs/html.

---------

### LICENSE
* [GPLV3.0](https://www.gnu.org/licenses/licenses.html)

### Contributing
Github is for social coding.
If you want to write code, I encourage contributions through pull requests from forks of this repository.
 

