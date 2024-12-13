#include <WiFi.h>
#include <WebServer.h>
#include "joystick_control.h"
//#include "autonomous_1.h"
#include "motor_control.h"
#include "wall_following.h"
#include "vive.h"

const char* ssid = "ESP32_Test";
const char* password = "12345678";

WebServer server(80);

// HTML webpage with joystick and buttons that change the state 
String webpage = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Jawn-E</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            background-color: #f0f0f0;
            text-align: center;
            padding: 20px;
            user-select: none;
            -webkit-user-select: none;
            -moz-user-select: none;
            -ms-user-select: none;
            touch-action: manipulation;
        }
        #robotCanvas { 
            border: 1px solid black; 
            margin: 20px auto;
        }
        #control-area {
            display: flex;
            flex-direction: column;
            align-items: center;
            margin: 20px 0;
        }
        #dpad {
            width: 300px;
            height: 300px;
            position: relative;
        }
        .dpad-button {
            position: absolute;
            width: 100px;
            height: 100px;
            background-color: #ddd;
            border: 1px solid #999;
            cursor: pointer;
            display: flex;
            justify-content: center;
            align-items: center;
            font-weight: bold;
        }
        .dpad-button.locked {
            background-color: #bbb; /* Darker shade */
        }
        #up { top: 0; left: 100px; }
        #right { top: 100px; right: 0; }
        #down { bottom: 0; left: 100px; }
        #left { top: 100px; left: 0; }
        #stop { 
            top: 100px; 
            left: 100px; 
            background-color: #ff4444; 
            color: white;
        }
        .control-buttons {
            margin-top: 20px;
        }
        button {
            margin: 10px;
            padding: 10px 20px;
            font-size: 16px;
            cursor: pointer;
        }
        #lock-container {
            margin-top: 20px;
            display: flex;
            align-items: center;
        }
        .switch {
            position: relative;
            display: inline-block;
            width: 60px;
            height: 34px;
            margin-right: 10px;
        }
        .switch input {
            opacity: 0;
            width: 0;
            height: 0;
        }
        .slider {
            position: absolute;
            cursor: pointer;
            top: 0;
            left: 0;
            right: 0;
            bottom: 0;
            background-color: #ccc;
            transition: .4s;
            border-radius: 34px;
        }
        .slider:before {
            position: absolute;
            content: "";
            height: 26px;
            width: 26px;
            left: 4px;
            bottom: 4px;
            background-color: white;
            transition: .4s;
            border-radius: 50%;
        }
        input:checked + .slider {
            background-color: #2196F3;
        }
        input:checked + .slider:before {
            transform: translateX(26px);
        }
        #lock-label {
            font-weight: bold;
        }
    </style>
</head>
<body>
    <h1>Jawn-E</h1>
    
    <h2>Manual Control</h2>
    <div id="control-area">
        <div id="dpad">
            <div id="up" class="dpad-button">Up</div>
            <div id="right" class="dpad-button">Right</div>
            <div id="down" class="dpad-button">Down</div>
            <div id="left" class="dpad-button">Left</div>
            <div id="stop" class="dpad-button">STOP</div>
        </div>
        <div id="lock-container">
            <label class="switch">
                <input type="checkbox" id="lock-switch">
                <span class="slider"></span>
            </label>
            <span id="lock-label">Lock</span>
        </div>
    </div>
    <div class="control-buttons">
        <button onclick="setMode('manual')">Manual</button>
        <button onclick="setMode('wall_following')">Wall Following</button>
    </div>

    <h2>Position Tracker</h2>
    <canvas id="robotCanvas" width="400" height="400"></canvas>
    <p>Current Position - X: <span id="xPos"></span>, Y: <span id="yPos"></span></p>
    <p>Distance: <span id="distance"></span>, Angle: <span id="angle"></span></p>

    <script>
    // D-Pad Control Script
    const dpad = document.getElementById("dpad");
    const buttons = dpad.getElementsByClassName("dpad-button");
    const lockSwitch = document.getElementById("lock-switch");
    let lockedDirection = null;

    for (let button of buttons) {
        button.addEventListener("pointerdown", startMove);
        button.addEventListener("pointerup", endMove);
        button.addEventListener("pointerleave", endMove);
    }

    function startMove(e) {
        const direction = e.target.id;
        if (lockSwitch.checked) {
            if (lockedDirection) {
                const previousButton = document.getElementById(lockedDirection);
                previousButton.classList.remove('locked');
                sendDpadData(0, 0);
            }
            lockedDirection = direction;
            e.target.classList.add('locked');
        }
        sendDirectionData(direction);
    }

    function endMove(e) {
        if (!lockSwitch.checked || e.target.id === "stop") {
            sendDpadData(0, 0);
            if (lockedDirection) {
                const lockedButton = document.getElementById(lockedDirection);
                lockedButton.classList.remove('locked');
                lockedDirection = null;
            }
        }
    }

    function sendDirectionData(direction) {
        switch (direction) {
            case "up":
                sendDpadData(0, 100);
                break;
            case "right":
                sendDpadData(100, 0);
                break;
            case "down":
                sendDpadData(0, -100);
                break;
            case "left":
                sendDpadData(-100, 0);
                break;
            case "stop":
                sendDpadData(0, 0);
                lockedDirection = null;
                break;
        }
    }

    function sendDpadData(x, y) {
        fetch(`/joystick?x=${x}&y=${y}`)
            .then(response => response.text())
            .then(data => console.log(data))
            .catch(error => console.error('Error:', error));
    }

    lockSwitch.addEventListener("change", function() {
        if (!this.checked && lockedDirection) {
            const lockedButton = document.getElementById(lockedDirection);
            lockedButton.classList.remove('locked');
            sendDpadData(0, 0);
            lockedDirection = null;
        }
    });

    function setMode(mode) {
        fetch(`/setMode?mode=${mode}`)
            .then(response => response.text())
            .then(data => console.log(`${mode} mode set.`))
            .catch(error => console.error('Error:', error));
    }

    // Robot Position Tracker Script
    const canvas = document.getElementById('robotCanvas');
    const ctx = canvas.getContext('2d');
    const xPosElement = document.getElementById('xPos');
    const yPosElement = document.getElementById('yPos');
    const distanceElement = document.getElementById('distance');
    const angleElement = document.getElementById('angle');

    function updatePosition() {
        fetch('/position')
            .then(response => response.json())
            .then(data => {
                if ('x' in data && 'y' in data) {
                    xPosElement.textContent = data.x;
                    yPosElement.textContent = data.y;
                }
                if ('distance' in data) {
                    distanceElement.textContent = data.distance.toFixed(2);
                }
                if ('angle' in data) {
                    angleElement.textContent = data.angle.toFixed(2);
                }
            })
            .catch(error => console.error('Error:', error));
    }

    setInterval(updatePosition, 100);
    </script>
</body>
</html>
)=====";

//sets webpage
void handleRoot() {
  server.send(200, "text/html", webpage);
}


String currentMode = "manual";
String prevMode = "manual"; 
void handleSetMode() {
  if (server.hasArg("mode")) {
    currentMode = server.arg("mode");
    server.send(200, "text/plain", "Mode set to " + currentMode);
    Serial.println("Mode changed to: " + currentMode);
  } else {
    server.send(400, "text/plain", "Invalid request");
  }
}

int x = 0;
int y = 0; 
void handleJoystick() {
  if (server.hasArg("x") && server.hasArg("y")) {
    x = server.arg("x").toInt();
    y = server.arg("y").toInt();
    server.send(200, "text/plain", "Joystick data received");
  } else {
    server.send(400, "text/plain", "Invalid request");
  }
  }

// void handlePosition() {
//     String json = vivehandlePosition(); 
//     server.send(200, "application/json", json);
// }

void setup() {
  //set up wifi 
  Serial.begin(115200);  //for printing
  
  //WIFI 
   // AP Wifi set up
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
 // yield(); 
  Serial.print("Access Point IP: ");
  Serial.println(IP);

  // Server set up (reference ChatGPT in doc)
  server.on("/", handleRoot);
  server.on("/joystick", handleJoystick);
  server.on("/setMode", handleSetMode);
 // server.on("/position", handlePosition); // had HTTP_GET, 
 // server.on("/setTarget", handleSetTarget);
  server.begin();
  // Serial.println("Web server started");

  setUpMotors(); 
  setup_wall_following();
  //vivesetup(); 
}

float mainStateArray[8] = {0.0};

// int cnx = 3950; 
// int cny = 4470; 
void loop(){

  // run whatever loop corresponds to the state chosen by clicking HTML buttons 
  server.handleClient(); // Handle incoming client requests
  if (prevMode != currentMode) {
      updateMotorControls(0, 0, 0, 0); 
      prevMode = currentMode; 
  }
  else if (currentMode == "manual") {
      // Serial.println("joystick"); 
     // delay(100); 
      runJoystick(x,y);
     // yield(); 

    } else if (currentMode == "wall_following") {
      // Serial.println("wall_following"); 
     // delay(100); 
      wall_following_loop(); 
    //  yield();
    
      } else if (currentMode == "go_forward") {
          updateMotorControls(1, 1, 0, 0); 
          mainStateArray[4] = 1;
          mainStateArray[5] = 1; 
          mainStateArray[6] = 0; 
          mainStateArray[7] = 0; 
          float* mainupdatedStateArray = motorLoop(mainStateArray); 
          for (int i = 0; i < 8; i++) {
             mainStateArray[i] = mainupdatedStateArray[i];
       }

     } 
        else if (currentMode == "go_backward") {
        updateMotorControls(1, 1, 1, 1); 
          mainStateArray[4] = 1;
          mainStateArray[5] = 1; 
          mainStateArray[6] = 1; 
          mainStateArray[7] = 1; 
          float* mainupdatedStateArray = motorLoop(mainStateArray); 
          for (int i = 0; i < 8; i++) {
             mainStateArray[i] = mainupdatedStateArray[i];
       }

    } 
        else if (currentMode == "go_right") {
      updateMotorControls(1, 1, 0, 1); 
          mainStateArray[4] = 1;
          mainStateArray[5] = 1; 
          mainStateArray[6] = 0; 
          mainStateArray[7] = 0; 
          float* mainupdatedStateArray = motorLoop(mainStateArray); 
          for (int i = 0; i < 8; i++) {
             mainStateArray[i] = mainupdatedStateArray[i];
       }

    } 
        else if (currentMode == "go_left") {
     updateMotorControls(1, 1, 1, 0); 
          mainStateArray[4] = 1;
          mainStateArray[5] = 1; 
          mainStateArray[6] = 0; 
          mainStateArray[7] = 0; 
          float* mainupdatedStateArray = motorLoop(mainStateArray); 
          for (int i = 0; i < 8; i++) {
             mainStateArray[i] = mainupdatedStateArray[i];
       }

    } 
    
    
    
    
    else if (currentMode == "center_nexus") {
        //viveSetTarget(cnx, cny); 
        //viveloop(); 
        //vivehandlePosition();
      // Serial.println("other"); 
     // delay(100); 
    }
}




