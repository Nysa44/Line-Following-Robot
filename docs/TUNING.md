# Calibration and tuning

## 1. Sensor calibration
The robot remains stationary for approximately 3 seconds after startup. During this time, move the sensor array by hand so each sensor sees both the line and the background. Calibration is only useful if the sensor readings change significantly.

Open Arduino IDE Serial Monitor at **115200 baud** to see status and line-position readings.

## 2. Check sensor polarity
The sketch defaults to a dark line on a light floor (`LINE_IS_DARK = true`). Different analog sensor boards can have different output polarity. If the robot does not recognize the line:
- Check that the sensor outputs are analog outputs, not only digital comparator outputs.
- Confirm each sensor's raw reading changes between line and floor.
- Adjust the polarity logic in `readSensors()` to match your module, then recalibrate.
- Keep the five sensors ordered left-to-right.

## 3. Tune speed and PID
Start with a low `BASE_SPEED` (around 90–125). Increase only after stable tracking.

- If it turns too weakly and drifts off the line, increase `KP` gradually.
- If it oscillates left and right, reduce `KP` or increase `KD` slightly.
- If it reacts too slowly to bends, increase `KD` carefully or reduce speed.
- Keep `KI` at zero initially. Add a very small value only if a persistent bias remains.

Change one value at a time and test on the same track section.

## 4. Common issues

| Symptom | Things to check |
|---|---|
| Robot drives backward | Reverse motor direction setting or swap that motor's leads |
| Robot spins in place | One motor polarity may be reversed; check wiring |
| Robot does not detect line | Sensor height, analog outputs, calibration movement, line/floor contrast, polarity |
| Robot veers off immediately | Sensor order, motor direction, sensor polarity, PID gains |
| Arduino resets when motors start | Separate motor supply, common ground, battery condition, wiring |
| L298N gets hot | Motor stall current, supply voltage, mechanical binding, driver limits |

## 5. Track assumptions
This baseline is intended for a continuous, high-contrast line on a relatively light, matte surface. Intersections, sharp 90-degree turns, gaps, and very wide lines may need additional logic and tuning.
