// #include <WiFi.h>
// #include "vive510.h"
// #include <math.h>

// #define SIGNALPIN1 38 // pin receiving signal from Vive circuit 1
// #define SIGNALPIN2 37 // pin receiving signal from Vive circuit 2
// Vive510 vive1(SIGNALPIN1);
// Vive510 vive2(SIGNALPIN2);

// #define FREQ 1 // in Hz

// // Print interval in milliseconds
// const unsigned long PRINT_INTERVAL = 100; // Print every 1/10 second
// unsigned long lastPrintTime = 0;

// uint16_t xvive = 0, yvive = 0;
// uint16_t x2 = 0, y2 = 0;
// uint16_t target_x = 0, target_y = 0;
// bool target_set = false;


// void viveSetTarget(int xin, int yin) {
//   target_x = xin; 
//   target_y = yin; 
//   target_set = true; 
// }


// float calculate_angle_and_path(uint16_t current_x, uint16_t current_y, uint16_t target_x, uint16_t target_y, float &distance) {
//     float delta_x = target_x - current_x;
//     float delta_y = target_y - current_y;
//     distance = sqrt(delta_x*delta_x + delta_y*delta_y);
//     float angle_rad = atan2(delta_y, delta_x);
//     float angle_deg = angle_rad * 180.0 / M_PI;
//     return angle_deg;
// }

// void vivesetup() {
//     pinMode(LED_BUILTIN, OUTPUT);
//     vive1.begin();
//     vive2.begin();
//     Serial.println("Vive trackers started");
// }

// uint32_t med3filt(uint32_t a, uint32_t b, uint32_t c) {
//     uint32_t middle;
//     if ((a <= b) && (a <= c))
//         middle = (b <= c) ? b : c;
//     else if ((b <= a) && (b <= c))
//         middle = (a <= c) ? a : c;
//     else
//         middle = (a <= b) ? a : b;
//     return middle;
// }

// void viveloop() {
//     if (vive1.status() == VIVE_RECEIVING) {
//         static uint16_t x0, y0, oldx1, oldx2, oldy1, oldy2;
//         oldx2 = oldx1;
//         oldy2 = oldy1;
//         oldx1 = x0;
//         oldy1 = y0;
//         x0 = vive1.xCoord();
//         y0 = vive1.yCoord();
//         xvive = med3filt(x0, oldx1, oldx2);
//         yvive = med3filt(y0, oldy1, oldy2);
//         if (xvive > 8000 || yvive > 8000 || xvive < 1000 || yvive < 1000) {
//             xvive = 0;
//             yvive = 0;
//             digitalWrite(LED_BUILTIN, LOW);
//         } else {
//             digitalWrite(LED_BUILTIN, HIGH);
//         }
//     } else {
//         digitalWrite(LED_BUILTIN, LOW);
//         xvive = 0;
//         yvive = 0;
//         vive1.sync(15);
//     }

//     // Read data from the second Vive sensor
//     if (vive2.status() == VIVE_RECEIVING) {
//         x2 = vive2.xCoord();
//         y2 = vive2.yCoord();
//         if (x2 > 8000 || y2 > 8000 || x2 < 1000 || y2 < 1000) {
//             x2 = 0;
//             y2 = 0;
//         }
//     } else {
//         x2 = 0;
//         y2 = 0;
//         vive2.sync(15);
//     }

//     // Print coordinates at the specified interval
//     unsigned long currentTime = millis();
//     if (currentTime - lastPrintTime >= PRINT_INTERVAL) {
//         Serial.print("Vive 1:");
//         Serial.print(xvive);
//         Serial.print(",");
//         Serial.print(yvive);
//         Serial.print(" ");
//         Serial.print("Vive 2:");
//         Serial.print(x2);
//         Serial.print(",");
//         Serial.println(y2);
//         lastPrintTime = currentTime;
//     }
// }

// String vivehandlePosition() {
//     float distance = 0;
//     float angle = 0;
//     String json = "{";
//     bool hasData = false;
//     if (xvive != 0 && yvive != 0) {
//         json += "\"x\":" + String(xvive) + ",\"y\":" + String(yvive);
//         hasData = true;
//         if (target_set) {
//             angle = calculate_angle_and_path(xvive, yvive, target_x, target_y, distance);
//             json += ",\"distance\":" + String(distance) + ",\"angle\":" + String(angle);
//         }
//     }
//     if (x2 != 0 && y2 != 0) {
//         if (hasData) json += ",";
//         json += "\"x2\":" + String(x2) + ",\"y2\":" + String(y2);
//         hasData = true;
//     }
//     if (target_set) {
//         if (hasData) json += ",";
//         json += "\"target_x\":" + String(target_x) + ",\"target_y\":" + String(target_y);
//     }
//     json += "}";
//     return json; 
// }

