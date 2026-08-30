# Icarus
Design an FPV quadcopter from scratch using brushed coreless motors
| ![](https://github.com/Aryan2804/Icarus/blob/main/assets/icarus-main.jpeg)  |
:-------------------------:|

## Table of Contents:
- [About The Project](#about-the-project)
- [Components](#components)
- [Perfboard Testing](#perfboard-testing)
- [PCB Model](#pcb-model)
- [Software Used](#software-used)
- [Resources](#resources)
- [Contributors](#contributors)


## About the Project:
The aim of this project is to design and assemble a full size fpv drone operating over bluetooth which will use firmware that is written from scratch



## Repository structure:

```
Icarus
├── Docs
│   ├── components.md
│   └── resources.md
├── Firmware                      # ESP-IDF firmware, written from scratch
│   ├── IMU                       # BMI088 driver + sensor fusion
│   ├── Motor_Control             # PWM/ESC logic for 8520 brushed motors
│   ├── Camera                    # Xiao-ESP32-S3-Sense (OV3660) interface
│   └── Main                      # control loop
├── Hardware
│   ├── PCB                       # KiCad 10.0 files
│   │   ├── Schematics
│   │   ├── Gerbers
│   │   └── 3D_Views
│   └── Mechanical                # Onshape exports (frame, mounts)
│       └── STL
├── Testing
│   ├── Perfboard                 # Perfboard prototype 
│   └── Loadcell                  # Motor thrust test 
├── assets                        # Images used in README
└── README.md
```



## Components:

|     Components        |          Description          |
| --------------------- | ----------------------------- |
|ESP32-S3-Wroom-1 board |Microcontroller|
|8520 Brushed motors |Motors for Propulsion|
|BMI 088 |6-DoF Inertial Measurement Unit|
|Xiao-ESP32-S3-sense OV3660 |Camera Module|
|USB- C Receptacle |Power and Programming Interface|
|AMS1117 |Voltage regulator|
|Load cell |Thrust measurement|



## Perfboard and Load cell Testing:
| ![](https://github.com/Aryan2804/Icarus/blob/main/assets/icarus-perfboard.jpeg)  | ![](https://github.com/Aryan2804/Icarus/blob/main/assets/icarus-loadcell.jpeg) | 
:-------------------------:|:-------------------------:|

## PCB Model:
- View of the PCB Model front and back:       
  
| ![pcb_model_front](https://github.com/Aryan2804/Icarus/blob/main/assets/icarus-3D-view-front.png)            | ![pcb_model_back](https://github.com/Aryan2804/Icarus/blob/main/assets/icarus-3D-view-back.png) |    
:-------------------------:|:-------------------------:|

- View of the PCB Routing - front and back:      
  
| ![pcb_routing_front](https://github.com/Aryan2804/Icarus/blob/main/assets/icarus-routing-front.png)            | ![pcb_routing_back](https://github.com/Aryan2804/Icarus/blob/main/assets/icarus-routing-back.png) |  
:-------------------------:|:-------------------------:|



## Software Used: 
- Mechanical Design - [Onshape](https://www.onshape.com/en/)
- PCB Design - [KiCad 10.0](https://www.kicad.org/)
- Embedded C and Firmware - [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/index.html)
- Simulation - [Ardupilot](https://ardupilot.org/planner/docs/mission-planner-installation.html)


## Resources:
- [Sergio Virahonda's blog](https://medium.com/@svirahonda/building-a-drone-flight-controller-from-scratch-a-software-engineers-guide-to-clean-c-644a2bd392c4) 
- [Understanding SPI protocol](https://www.youtube.com/watch?v=0nVNwozXsIc&t=119s)
- [Undestanding the IMU](https://www.kynix.com/components/the-ultimate-guide-to-the-bosch-bmi088-sensor-7-key-insights.html)
- [Understanding KiCad](https://www.youtube.com/watch?v=XiLnHYSrNHw) 



## Contributors:
- [Roshan Castelino](https://github.com/roshan-castelino)
- [Aryan Ghughare](https://github.com/Aryan2804)
- [Astha Sharma ](https://github.com/asthasharma020406-stack)

## License:
[MIT License](https://opensource.org/license/mit/)