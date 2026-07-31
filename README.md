# UAV Black Box & Telemetry system
## Description
A robust embedded firmware project for a UAV flight recorder (Black Box) built on modern **C++** and **FreeRTOS**. The system concurrently collects high-frequency sensor data, structures it, and logs telemetry SD card.
## Key Fetures and Hardware
-   **Microcontroller:** WeAct STM32F411CEU6 ("Black Pill") featuring an ARM Cortex-M4 core.
-   **Real-Time Operating System (FreeRTOS):** Multitasking architecture with dedicated tasks for sensor polling, inter-task communication (queues), and background SD card logging.    
-   **Peripherals & Interfaces:**    
    -   **BMP280** (Barometric Pressure / Altitude Sensor) via **I2C**.
    -   **NEO-6M** (GPS Module) parsing NMEA sentences via **UART**.        
    -   **SD Card Module** interfacing over **SPI** with **FatFS** integration for reliable file system operations.
## Software Engineering Practices:
-   Object-Oriented Programming (OOP) in C++ with hardware abstraction layers and dependency injection.
-   Comprehensive unit testing of Sensor and Logger functionality using **Google Test (gtest)** and **Google Mock (gmock)** running on the host machine via **CMake**.