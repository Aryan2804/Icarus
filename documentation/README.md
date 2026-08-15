# DOCUMENTATION
Documentation for presentation of [EKLAVYA-SRA](https://eklavya.sravjti.in/projects/) project - [ICARUS](https://github.com/Aryan2804/Icarus) 

## **What is Icarus?**  
Icarus is a small, FPV drone quadcopter project which involves the use of an ESP32-S3_WROOM1 board and brushed coreless motors in order to achieve flight. We wanted to design a drone from scratch which could be controlled over wifi-bluetooth through a mobile phone.      

----

## Main Parts of the drone:
**Schematic**
We began by starting the schematic for our drone. Firstly observing and going through each datasheet for the sensors, we chose for the drone. Each component was carefully chosen and connected keeping in mind that, the schematic must look neat so that, it would be easy to read. Each component had been carefully checked and its wiring corrected on then did we move on to the making of the PCB.

**PCB**
Next step was the designing of the PCB. The PCB has been carefully placed keeping in mind reducing the noise which would be picked up by our IMU. Other parameters we kept in mind were that our IMU would have to be centralised, each wire of the routing would have to be at obtuse angles, vias must be reduced, and overheating must be prevented. We kept the PCB with minimal traces and all the connections were mostly kept in nets which helped increase the speed of the information which travelled throughout the board. Also our PCB does not involve a 7.4v supply so we have connected a buck convertor externally to satify our 5v need for PCB. Keeping all these contraints in mind we came up with the final PCB design for our drone which we termed "Icarus Mk9".

**Load Cell**
We simultaneously worked on the functioning of the load cell. The load cell is a device which uses the concept of a wheatstone bridge in oreder to measure thrust. We needed the readings for thrust in our code to basically judge the lift of the drone. So we designed the stand for the load cell and calibrated the values accordingly to minimise our errors. Then we finally placed the motor and tested it on the load cell. The issue we faced was that the thrust was so powerful that the motor kept flying out of the slot and ended up breaking our propellers and ruining the readings. A fix for that was to cover the motor in tape and wedge it in the slot of our load cell.

**Motor control**
We wrote a motor control code and implemented it on a perfboard where we soldered on our connection and components of the motor driver. The basic function of our code was to treat each motor as individual and take inputs for each of them and later run the motors according to our desired speed.

**Camera Control**
As we needed a small enough camera to be able to mount on the frame catering to our size and weight constraint we decided to go with the OV03660 camera by Xiao with comes with its own mini ESP32 board. The camera is directly integrated in the PCB but we have an external JST connector which will power the camera and help it run parallelly with the drone. I reduced the quality, increased the frames, reset exposure values to better set to quick changes, changed the code so that the cam picks up the latest footage and increased delays so that the cam will not get stuck. This code is uploaded on the main branch in Firmware.

## Firmware for the drone:
**BMI-088**
BMI-088 is the IMU of our quadcopter and also is the brain of our PCB. We selected the BMI088— high-performance, 6-degree-of-freedom Inertial Measurement Unit (IMU)—specifically for its excellent vibration suppression and optimization for aerial environments, which are critical for stable drone flight. The writing of our code was done by going through multiple resources which we listed down in the [Resources](https://github.com/Aryan2804/Icarus#resources). The code works in a standard workflow:
1. definition of all the pins
2. defining a header file
3. defining the inputs and their bits which we will take ie. accelerometer and gyroscopic
4. finally taking the reading for the BMI

**PID control**
PID is the part in our code which helps in the stabilising of the drone. It involves the use of 3 terms ie. proportionality, integral and differential which work together to help reduce the errors in the inputs which go to the motor control. The code works in this way:
1. definition of all the pins
2. defining the header file
3. defining two different parts to flight start and lift end
4. this is then integrated together so the transition is flush and smooth
5. lastly generating the readings and giving them to the motor control

**Motor control**
Motor control is the part which finally gives us our output. its simple just involves 4 calculations for each motor and gives the output.


## **Educational and Practical Value:- Domains explored**   
Icarus offers significant learning opportunities in embedded systems, control theory, robotics and mechanical design. It encourages to innovate, optimize PID, and build practical skills in programming and electronics. The competition’s real-world challenges, such as sensor noise, motor control, and algorithm efficiency, make it an excellent platform for learning and experimentation.  

----


# **Start of the project…..**

## **Perfboard Testing for Micromouse:**
Perfboard testing is an essential step in the development process of a Icarusrobot. A perfboard (short for perforated board) is a prototyping board used to build circuits without the need for designing and manufacturing a printed circuit board (PCB) right away. It allows developers to test the functionality of various components, such as sensors, motor drivers, and microcontrollers, before finalizing the design. This approach provides a flexible and cost-effective way to verify that all components work as intended when integrated.

### **Purpose of Perfboard Testing:**  
1. **Circuit Validation**: Ensures that all components are correctly wired and interact as expected. This step is crucial to identify any electrical issues, such as incorrect connections or voltage mismatches, which could lead to component failure.  
2.	**Functional Testing**: Allows testing of the Micromouse’s core functionalities—like motor control, sensor readings, and microcontroller processing—in a controlled environment. This helps in validating that the hardware and software communicate properly.  
3.	**Debugging and Optimization**: Provides a platform for real-time debugging. One can easily replace components, adjust connections, or tweak circuit parameters without the complexities associated with a final PCB layout.

### **Steps for Perfboard Testing:**  
1.  **Component Placement and Soldering**:  
    - Carefully place all components—such as microcontrollers (ESP32), motor drivers (DRV8833), IR sensors, and Time-of-Flight (ToF) sensors—on the perfboard in a logical layout that minimizes wire length and prevents cross-connections.  
	- Solder the components securely, ensuring no cold joints or short circuits between adjacent pads. Proper soldering techniques are crucial to avoid unstable connections.  
2.  **Power Supply Verification**:  
	- Connect the power source (e.g., 3.7V battery) to the perfboard and verify the voltage levels across different sections of the circuit. Use a multimeter to ensure that all components receive their required voltage and current levels.  
3.  **Initial Power-On Testing**:  
	- Perform a “smoke test” by powering on the circuit with minimal components to check for immediate signs of faults, such as overheating components, unusual noises, or visible smoke.  
4.  **Subsystem Testing**:  
	- Motor and Motor Driver Testing: Verify that the motors respond correctly to the control signals from the motor driver and microcontroller. Test different speeds and directions.  
	- Sensor Testing: Confirm that the sensors (IR sensors, VL53L0X ToF sensors) provide accurate readings and that data is correctly processed by the microcontroller.  
	- Microcontroller Functionality: Ensure the microcontroller executes the maze-solving algorithms correctly and interfaces with all other components without errors.  
5.	**Integration Testing**:  
	- Once all individual components and subsystems are tested, perform an integrated test to see how they work together. This step involves running simple maze-solving algorithms, like wall-following, to ensure the robot moves and navigates properly.


### **Benefits of Perfboard Testing:**  
- Flexibility and Easy Modifications: Components can be easily removed or replaced without the need to manufacture a new PCB.  
- Cost-Effective: Helps in identifying issues early in the development cycle, saving costs associated with faulty PCB design or incorrect component selection.  
- Prototyping Speed: Accelerates the prototyping process, allowing for rapid iterations and testing of different circuit designs or configurations.


## **What is PCB Design?**  
PCB (Printed Circuit Board) design involves creating a physical layout for a board that supports and electrically connects electronic components. The process includes schematic capture, layout design, component placement, routing, and verification.  

### **Key Components of PCB Design:**  
- **Schematic Design**: The first step in PCB design, where electronic circuits are made using CAD tools to define electrical connections between components.  

- **PCB Layout**: Involves designing the physical layout of the board, defining its shape, and placing components based on the schematic.  

- **Routing**: Establishing electrical paths (traces) that connect various components on the board, which is crucial for signal integrity and reducing noise.  

- **Verification and DFM Checks**: Ensures the design is functionally correct and manufacturable. DFM (Design for Manufacturing) checks help identify potential issues before production.


#### **Basic Steps in PCB Design Using KiCad:**  
1. **Create Schematic**: Define components and their connections.  

2. **Assign Footprints**: Link each schematic symbol to its corresponding physical footprint.  

3. **Design Board Layout**: Arrange components and define the board’s shape and size.  

4. **Route Traces**: Connect components following design rules to ensure electrical performance.  

5. **Run DFM Checks**: Verify that the design is free from errors that could cause manufacturing issues.

Finally implemented all of this and tested the pcb placed it on the frame and debugged the codes and other sensors which helped us achieve flight finally.