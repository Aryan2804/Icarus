# Icarus:
Design an FPV quadcopter from scratch using brushed coreless motors

## Table of Contents
- [About The Project](#about-the-project)
- [Goals](#goals)
- [Project Workflow](#project-workflow)
- [Components](#components)
- [Perfboard Testing](#perfboard-testing)
- [PCB Model](#pcb-model)
- [Software Used](#software-used)
- [Resources](#resources)
- [Contributors](#contributors)
- [Acknowledgements](#acknowledgements)


## About the Project
The aim of this project is to design and assemble a full size fpv drone operating over bluetooth which will use firmware that is written from scratch


## Goals
To bring Project Icarus to completion, we mapped out the following milestones:
### Hardware & Structural Design:
1. Designing the system schematic.
2. Positioning each component and sensor to secure maximum efficiency.
3. Layout and routing of the custom PCB.
4. Making a custom perfboard of the Motor control to understand its working and make it efficient.
5. Designing a custom test frame integrated with a load cell to measure motor thrust.
6. Designing the final, corrected quadcopter frame tailored to fit the fabricated PCB.

### Firmware & Flight Control:
1. Writing low-level drivers and firmware for the onboard sensors (IMU).
2. Understanding and implementing how each part of the firmware will relate to each other and give a desirable feedback to control the drone.
3. Developing and optimizing the flight control loop algorithms to achieve stable flight.
4. Lastly, attaching the camera module to achieve an fpv drone which gives live feed as it moves along.


## Project Workflow
#### Research
- Learning and understanding Embedded C by reading blogs and existing pre-requisites on github in order to get a firm understanding of the project goals for firmware
- Understanding the desired size and dimensions in order to achieve flight
- Deciding the constraints for Icarus and selecting hardware and electronic componenets accordingly. We did a detailed review of existing fpv drone designs and features and arrived at our present design
#### Frame and PCB Design
- Designing a frame on [Onshape.com](https://www.onshape.com/en/). We went through various iterations and design options. After long testing we came up with our final design which completely covers the pcb and locks in the motors perfectly
- PCB Design involved a series of contraints to overcome. Size and noise needed to be reduced in order to make a brushed motor drone fly. Our final PCB satisfied all the contraints and its dimensions were 55mmx35mm
#### Perfboard Testing 
- Testing our pcb schematic on a perfboard and recreating the final motor driver design on a perfboard.
#### PCB Soldering Assembly and Testing
- Once the PCB was delivered, we soldered the components onto the PCB and assembleed the drone with the 3D-printed frame
- We then tested the sensors and the control system on the pcb until we were satisfied with the output
#### Calibration of the drone
- Then we calibrated the drone until we were satisfied with the stability of the flight after which we implemented our cam making it FPV finally 


## Components

|     Components        |          Description          |
| --------------------- | ----------------------------- |
|ESP32-S3-Wroom-1 board |Microcontroller|
|8520 Brushed motors |Motors for Propulsion|
|BMI 088 |6-DoF Inertial Measurement Unit|
|Xiao-ESP32-S3-sense OV3660 |Camera Module|
|USB- C Receptacle |Power and Programming Interface|


## Perfboard and Load cell Testing
| ![](https://github.com/Aryan2804/Icarus/blob/main/assets/icarus-perfboard.jpeg)  | ![](https://github.com/Aryan2804/Icarus/blob/main/assets/icarus-loadcell.jpeg) | 
:-------------------------:|:-------------------------:|
![](https://github.com/Aryan2804/Icarus/blob/main/assets/icarus-loadcell-testing.mp4)
:-------------------------:|


## PCB Model
- View of the PCB Model front and back:       
  
| ![pcb_model_front](https://github.com/Aryan2804/Icarus/blob/main/assets/icarus-3D-view-front.png)            | ![pcb_model_back](https://github.com/Aryan2804/Icarus/blob/main/assets/icarus-3D-view-back.png) |    
:-------------------------:|:-------------------------:|

- View of the PCB Routing - front and back:      
  
| ![pcb_routing_front](https://github.com/Aryan2804/Icarus/blob/main/assets/icarus-routing-front.png)            | ![pcb_routing_back](https://github.com/Aryan2804/Icarus/blob/main/assets/icarus-routing-back.png) |  
:-------------------------:|:-------------------------:|


## Software Used 
- Mechanical Design - [Onshape](https://www.onshape.com/en/)
- PCB Design - [KiCad 10.0](https://www.kicad.org/)
- Embedded C and Firmware - [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/index.html)
- Simulation - [Ardupilot](https://ardupilot.org/planner/docs/mission-planner-installation.html)

## Resources
- [Sergio Virahonda's blog](https://medium.com/@svirahonda/building-a-drone-flight-controller-from-scratch-a-software-engineers-guide-to-clean-c-644a2bd392c4) 
- [Understanding SPI protocol](https://www.youtube.com/watch?v=0nVNwozXsIc&t=119s)
- [Undestanding the IMU](https://www.kynix.com/components/the-ultimate-guide-to-the-bosch-bmi088-sensor-7-key-insights.html)
- [Understanding KiCad](https://www.youtube.com/watch?v=XiLnHYSrNHw) 



## Contributors
- [Roshan Castelino](https://github.com/roshan-castelino)
- [Aryan Ghughare](https://github.com/Aryan2804)
- [Astha Sharma ](https://github.com/asthasharma020406-stack)

## Acknowledgements 
- [SRA VJTI](https://sravjti.in/) Eklavya 2024
- Special thanks to our mentors [Aryan Vyapari](https://github.com/4rynv), [Bhakti Assar](https://github.com/Bhakti-A), and all the seniors at SRA, VJTI for their constant support and guidance throughout the project.

## License
[MIT License](https://opensource.org/license/mit/)