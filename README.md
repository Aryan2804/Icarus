Icarus is a quadcopter project by Roshan Castelino, Astha Sharma and Aryan Ghughare.
Our main goal for this project is to create a drone which uses 8520 brushed motors and A ESP32-S3-Wroom-1 board as these were the specifications provided by our mentors.
## Components
We initially decided to finalise our components and structure which we will use in the project in order to have a base for the project,
**Our main components and sensors decided are:**
1. ESP32-S3-Wroom-1 board (Main Microcontroller)
2. 8520 Brushed motors (Propulsion)
3. BMI 088 (6-DoF Inertial Measurement Unit)
4. USB- C Receptacle (Power and Programming Interface)


We selected the BMI088—a high-performance, 6-degree-of-freedom Inertial Measurement Unit (IMU)—specifically for its excellent vibration suppression and optimization for aerial environments, which are critical for stable drone flight.
## Goals
To bring Project Icarus to completion, we mapped out the following milestones:
**Hardware & Structural Design:**
1. Designing the system schematic.
2. Layout and routing of the custom PCB.
3. Designing a custom test frame integrated with a load cell to measure motor thrust.
4. Designing the final, corrected quadcopter frame tailored to fit the fabricated PCB.

**Firmware & Flight Control:**
1. Writing low-level drivers and firmware for the onboard sensors (IMU).
2. Developing and optimizing the flight control loop algorithms to achieve stable flight.
