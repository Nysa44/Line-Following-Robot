# Wiring guide

This pin map is for an Arduino Uno/Nano + L298N + five analog sensor outputs.

## IR sensor array

| Sensor array output | Arduino |
|---|---|
| Leftmost analog output | A0 |
| Sensor 2 analog output | A1 |
| Center analog output | A2 |
| Sensor 4 analog output | A3 |
| Rightmost analog output | A4 |
| VCC | 5V (if supported by the sensor board) |
| GND | GND |

The sensor order must be left-to-right when viewed from behind the robot, looking in its direction of travel. If your board labels its outputs differently, map them accordingly.

## L298N motor driver

| L298N pin | Arduino |
|---|---|
| ENA | D5 (PWM) |
| IN1 | D7 |
| IN2 | D8 |
| ENB | D6 (PWM) |
| IN3 | D9 |
| IN4 | D10 |
| GND | Arduino GND |

- Remove the ENA and ENB enable jumpers if you want Arduino PWM speed control.
- Left motor connects to OUT1/OUT2.
- Right motor connects to OUT3/OUT4.
- Connect the motor battery to the L298N motor supply input, observing polarity.
- Connect Arduino GND, sensor GND, and L298N GND together (common ground).

## Power notes
- Do **not** connect the motor supply to Arduino 5V.
- Power the Arduino using USB or a suitable regulated supply.
- Check your specific L298N board's regulator/jumper arrangement and battery voltage before use.
- Confirm the motors' stall current is within the driver's safe limits. The L298N has significant voltage drop and may run hot.

## Direction check
Raise the wheels and briefly test. A positive speed command should make both wheels drive the robot forward. If one side runs backward, set its corresponding `MOTOR_*_REVERSED` value to `true` in the sketch.
