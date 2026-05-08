# Spider-E
## Description
A four-legged robot designed and built by a team of four students. 
Controlled wirelessly from an iPhone via Bluetooth, capable of walking, turning, waving, and dancing.


## Project Overview
We built this quadruped robot as a learning project of a complete engineering pipeline-- from CAD design to 3D printing to electronics integration to software. Starting from Arijit1080's open-source spider robot as a reference, we re-CADed the frame entirely, partially in OnShape and SolidWorks, went through 3-4 iterations for the leg prototype, and modified electronics to support iOS bluetooth control using HM-10 BLE module instead of the original HC-)5. The result is a fully functional walking robot that we understand at every level - mechanical, electrical, and software.

## Demo 


## Modification
| Area | Original | Our version |
| ---- | ---- | ---- |
| CAD | Used author's existing STL files | Re-designed from scratch in OnShape and SolidWorks |
| Frame Size | Original dimensions | Scaled up 15% |
| Leg design | Orignial geometry | 3-4 iterations, optimized fit, structural strength and servo clearance |
| Bluetooth | HC-05 (Android compatible) | HM-10 BLE (iOS compatible) |
| Wiring | Direct Serial Connection | Voltage divider on HM-10 BLE on RX line with 1k + 2k ohm resistors |
| Battery | Origninal set up with breakout board | 2S LiPo 7.4V 3000mAh with Dean connector |
| Power regulation | Original set-up | LM2596 buck converter calibrated to 5V |
| Software pins | Hardware serial | SoftwareSerial on A0 / A1 |







Four-Legged "Pretty COOL" robot

This is the code repo for our Quadraped bot. We follow the open source project on youtube (https://www.youtube.com/watch?v=fnMmnd9k6q8)
