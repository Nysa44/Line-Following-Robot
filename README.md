# Line-Following Robot

An Arduino-based autonomous line-following robot for a black line on a light surface.
The project uses a 5-channel analog IR reflectance sensor array and an L298N dual H-bridge motor driver.

> **Hardware assumed:** Arduino Uno/Nano (ATmega328P), 5 analog IR sensors, L298N, two DC geared motors, and a suitable battery. If your actual board or driver differs, update the pin definitions and wiring before powering it.

## Features
- Five-sensor weighted line-position estimate
- PID steering control
- Startup sensor calibration
- Recovery behavior when the line is temporarily lost
- Adjustable speed and PID constants

## Repository structure

```text
line-following-robot/
├── src/
│   └── LineFollowingRobot.ino
├── docs/
│   ├── WIRING.md
│   └── TUNING.md
├── LICENSE
└── README.md
```

## Required components
- Arduino Uno or Nano (5 V logic)
- 5-channel **analog** IR reflectance sensor array (outputs `S1`–`S5` or `OUT1`–`OUT5`)
- L298N motor driver module
- 2 DC geared motors and wheels
- Robot chassis and caster wheel
- Battery suitable for the motors and driver
- Jumper wires and a power switch

## Quick start
1. Read [`docs/WIRING.md`](docs/WIRING.md) and connect everything with power disconnected.
2. Keep the sensor array close to the floor (typically 5–10 mm; follow the sensor-board guidance).
3. Open `src/LineFollowingRobot.ino` in Arduino IDE.
4. Select **Arduino Uno** (or your compatible board) and the correct port, then upload.
5. Put the robot on a light surface with the black line under the sensor array.
6. On startup, the robot stops for a 3-second calibration period. During this period, slowly move the sensor array across both the line and the surrounding surface so every sensor sees both.
7. Place the robot on the track. If it steers away from the line, see the polarity setting and tuning guide.

## Important configuration
At the top of the sketch:
- `LINE_IS_DARK`: set `true` for a dark line on a light floor; set `false` for a light line on a dark floor.
- `BASE_SPEED`: starting motor speed (0–255).
- `KP`, `KI`, `KD`: steering controller tuning values.
- `MOTOR_LEFT_REVERSED` / `MOTOR_RIGHT_REVERSED`: set if a motor spins opposite to the expected direction.

## Safety
- Test with the wheels raised first.
- Keep fingers, hair, and loose wires away from wheels and gears.
- Do not power motors from the Arduino 5 V pin.
- Use a common ground between Arduino and L298N.
- Verify your battery voltage and motor current are within the ratings of your driver and motors.
- Disconnect power before changing wiring.

## Notes
This is a practical baseline for the specified hardware, not a claim that it was tested on your exact robot. Motor polarity, sensor output polarity, sensor height, track width, and battery voltage vary between builds. Tune those for your hardware.
