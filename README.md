# 🤖 Spider-E

## Description
A four-legged robot designed and built by a team of four students. \
Controlled wirelessly from an iPhone via Bluetooth, capable of walking, turning, waving, and dancing.


## Project Overview
We built this quadruped robot as a learning project. This gives us an understanding of a complete engineering pipeline-- from CAD design to prototyping to electronics integration to software. 

Starting from Arijit1080's open-source spider robot as a reference, we re-CADed the frame entirely, partially in both OnShape and SolidWorks, went through 3-4 iterations for the leg prototype, and modified electronics to support iOS bluetooth control using HM-10 BLE module instead of the original HC-05. The result is a fully functional crawling robot that we understand at every level - mechanical, electrical, and software.

<img width="2989" height="2391" alt="IMG_3076" src="https://github.com/user-attachments/assets/58d90d22-11be-4057-996a-c1c22b24511e" />

Fig. Hello from Spider-E

<img width="302" height="1701" alt="IMG_306" src="https://github.com/user-attachments/assets/b8cf3d11-4b92-46ce-9546-732dbba47a7f" />

Fig. Hello again

## Demo 
** video **



## Modification
| Area | Original | Our version |
| ---- | ---- | ---- |
| CAD | Used author's existing STL files | Re-designed from scratch in OnShape and SolidWorks |
| Frame Size | Original dimensions | Scaled up 115% |
| Leg design | Orignial geometry | 3-4 iterations, optimized fit, structural strength and servo clearance |
| Bluetooth | HC-05 (Android compatible) | HM-10 BLE (iOS compatible) |
| Wiring | Direct Serial Connection | Voltage divider on HM-10 BLE on RX line with 1k + 2k ohm resistors |
| Battery | Origninal set up with breakout board | 2S LiPo 7.4V 3000mAh with Dean connector |
| Power regulation | Original set-up | LM2596 buck converter calibrated to 5V |
| Software pins | Hardware serial | SoftwareSerial on A0 / A1 |

## Hardware 

### Parts List

| Component | Quantity | Notes |
|---|---|---|
| Arduino Nano | 1 | compact design |
| Sg90 9 g Servo motor | 12 | 3 per leg |
| Arduino Nano I/O expansion sensor shield | 1 | Pin access for servo motors |
| DSD TECH HM-10 BLE module | 1 | iOS compatible, BLE 4.0 |
| LM2596 buck converter | 1 | Adjustable, convert from 7.4V battery to 5V output |
| 2S LiPo battery 7.4V 3000mAh | 1 | With Dean/T connector and charger |
| Dean T-plug adapter with leads | 1 | Safe battery wiring without cutting pack |
| Toggle switch | 1 | Main power switch |
| 18 AWG silicone wire | ~3ft | Red and black, power circuit |
| 1K and 2K ohm resistors | 1 each | HM-10 voltage divider |
| Shrink tube | | Safe protection after soldering
| 3D printed frame | 1 set | PLA, designed in OnShape and SolidWorks |

<img width="4032" height="3024" alt="IMG_3035" src="https://github.com/user-attachments/assets/1fc88f7a-a0cb-4377-a605-2ee20c31dbdd" />

Fig. Hardware components 


## Robot Dimension
| Parameter | Value |
| ---- | ---- |
| Coax length | 50.40 mm |
| Tipia length | 80.50 mm |
| Femur length | 63.00 mm |
| Body dimension | 120 mm x 70 mm |


## Prototypes
Material is PLA. The earlier prototypes break easily at the joint of thin walls. The last version is supported with stronger walls while clearing enough space for servo wires.

<img width="2699" height="2699" alt="IMG_2911" src="https://github.com/user-attachments/assets/2a2cd25a-7944-4e8f-96c4-85ffff9586b0" />

Fig. First complete leg

<img width="3024" height="4032" alt="IMG_3018" src="https://github.com/user-attachments/assets/02213b8b-44f8-4c1d-8c4e-b20dff58f9a2" />

Fig. Second print with stronger walls

<img width="3024" height="4032" alt="IMG_3020" src="https://github.com/user-attachments/assets/eae0e8e8-545b-438f-a6a8-7cfdc1a86c67" />

Fig. Chasis with a leg

<img width="3024" height="4032" alt="IMG_3065" src="https://github.com/user-attachments/assets/4087d521-4a89-4984-abc4-4405933c8950" />

Fig. Four sets of legs

## Wiring
Connection is made with 18 AWG tinned copper wires. 
Battery -> switch -> buck converter (calibration necessary) -> Arduino nano expansion board -> servo motors & bluetooth module
<img width="3024" height="4032" alt="IMG_3042" src="https://github.com/user-attachments/assets/71be8035-ee78-4fc1-b289-c53e35d08573" />

Fig. Wire connection lay-out

<img width="1788" height="946" alt="Image 4-9-26 at 10 18 AM" src="https://github.com/user-attachments/assets/671b8331-8afd-4d0c-a484-1e6c51ab77ea" />

Fig. Original connection from Youtube

### HM-10 Bluetooth Wiring

```
HM-10 VCC  → Nano 5V       (direct)
HM-10 GND  → Nano GND      (direct)
HM-10 TX   → Nano A0       (direct, no resistors needed)
HM-10 RX   → junction      (voltage divider output)

Voltage divider:
Nano A1 → [1K resistor] → junction → [2K resistor] → GND
                               ↓
                          HM-10 RX pin

Result: 5V stepped down to 3.3V for HM-10 RX logic level
```

### Leg Layout (Top View)

```
        FRONT (eyes)
   2(FL)     0(FR)
      [body]
   3(BL)     1(BR)
        BACK
```
<img width="1914" height="1178" alt="IMG_3688" src="https://github.com/user-attachments/assets/d3eafaeb-df07-4369-b2d1-5b629db5942a" /> 

Fig. Leg position sketch

<img width="931" height="784" alt="IMG_3690" src="https://github.com/user-attachments/assets/c9c834af-4d8a-40ae-bd07-693403551b98" />

Fig. Each leg part

---

## Software Setup

### Prerequisites
- Arduino IDE 2.x
- FlexiTimer2 Library
- SoftwareSerial (included in Arduino IDE)
- DSD TECH Bluetooth app (iOS app store)

### Installing FlexiTimer2
```
1. Download ZIP: https://github.com/wimleers/flexitimer2
2. Arduino IDE -> Sketch -> Include Library -> Add .ZIP Library
3. Select downloaded ZIP
```

### SoftwareSerial Header file
Use the following code to read bluetooth commands (similar to Serial)
```
#include <SoftwareSerial.h>
SoftwareSerial *variableName* (RX, TX);   // use HM10 for name

char data = '';

void setup() {
  HM10.begin (9600);
}

void loop() {
  if (HM10.available()) {
    data = HM10.read();
  ...
  ...
}
```


### Spider-E Bluetooth Commands
| Command | Action |
|---|---|
| F / f | Step forward |
| B / b | Step backward |
| L / l | Turn left |
| R / r | Turn right |
| X / x | Stand up |
| Y / y | Sit down |
| W / w | Hand wave |
| V / v | Hand shake |
| U / u | Body dance |

## System Architecture
```
iPhone sends command (DSD TECH Bluetooth app)
    ↓ BLE 4.0
HM-10 Module → Arduino Nano (SoftwareSerial on A0/A1)
    ↓
loop() reads character command
    ↓
Movement function (step_forward, turn_left, etc.)
    ↓
set_site() — sets target foot coordinates in 3D space
           — calculates per-axis speed for straight-line motion
    ↓
FlexiTimer2 interrupt (every 20ms / 50Hz)
    ↓
servo_service()
    → increments site_now toward site_expect
    → cartesian_to_polar() — inverse kinematics
    → polar_to_servo() — writes angles to physical servos
```

## What we learn
### Coordinate system:
Each leg's foot tip is controlled in 3D space (x, y, z) in millimeters from the body-coax joint. Each leg has its own coordinates. After cartesian_to_servo() gives angle values, each joint angle has to be re-calculated. Leg 0 & 3 share same leg set-up and so do leg 1 & 2.

<img width="1464" height="778" alt="IMG_3694" src="https://github.com/user-attachments/assets/3755d6d7-0c4b-4568-aeee-69386d627df2" />

Fig. Leg in neutral position

### Inverse kinematics:
`cartesian_to_polar()` converts a 3D foot position into three joint angles using the law of cosines — femur angle (alpha), knee bend (beta), and coxa rotation (gamma). 
<img width="2456" height="1756" alt="IMG_3691" src="https://github.com/user-attachments/assets/6d8b01b6-56ca-4d6a-8d4e-53521d8c8119" />

Fig. Inverse Kinematics Calculation


<img width="1781" height="1029" alt="IMG_3692" src="https://github.com/user-attachments/assets/2102a931-7ace-460d-b205-23903f0704bf" />

Fig. Inverse Kinematics

<img width="2023" height="1426" alt="IMG_3693" src="https://github.com/user-attachments/assets/ac85f457-1dd9-433c-9835-b48584afe964" />

Fig. More Inverse Kinematics


### Smooth movement: 
`servo_service()` runs every 20ms via hardware timer interrupt, moving each foot tip incrementally toward its target and ensuring straight-line foot paths.

### Gait:
Walking uses an alternating diagonal pair of legs. 
1. The left or right front leg swings forward first.
2. The other front leg pulls the body, and the two back legs push backward on the ground to propel the body forward.
3. The diagonal leg from the first step swings forward.

<img width="1096" height="1836" alt="IMG_3689" src="https://github.com/user-attachments/assets/05797866-ded6-4afb-a143-f913740807a9" />

Fig. step_forward and step_backward() sequence

### Electronics:
Wires - has maxmium numbers for current load (AWG) -- the lower the number, the more it can carry
Battery - if the positive and negative node gets short circuit, it can heat up quick and catch fire!!!
Connection test - each soldered parts and every connection needs to be tested with a multimeter throughout the project
Types of switches - On/Off 2 pin, On/On 3 pin


## Meet Team Rice and Robots (R&R) 
Independent student robotics team - De Anza College
| Name | Role | Contact |
| --- | --- | ---|
| Nyi Htet | Electronics research, Software learning, Parts sourcing | nyilinhtet960@gmail.com |
| Van Hac Nguyen | CAD design, Bluetooth integration | vanhacnguyen1708@gmail.com |
| Phyo Han Kywe | CAD design, 3D printing | phyohankyawe2023@gmail.com |
| Ivan Htet | Software learning, FlexiTimer2 | nyish2204@gmail.com |

*All four members participated in wiring, soldering, assembly, and testing.*
---

## Credits

Original spider robot concept and code by:
**Arijit1080** — https://github.com/Arijit1080/Spidy-The-Spider-Robot





