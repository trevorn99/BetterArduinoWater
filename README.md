# Automatic Plant Watering System

A 4-zone automatic plant watering system using Arduino, moisture sensors, OLED display, and RTC. The system monitors soil moisture levels and automatically waters plants when they fall below the target threshold.

## Features

- **4 Independent Moisture Sensors** - Monitor up to 4 different plant zones
- **Automatic Watering** - Waters plants when moisture drops below 40%
- **OLED Display** - Shows real-time moisture levels, time, and date
- **Real-Time Clock** - Accurate timekeeping with DS1307 RTC
- **Button Interface** - Toggle between time display and moisture readings
- **Sensor Validation** - Automatically detects and disables faulty sensors
- **Serial Monitoring** - Debug information via Serial Monitor
- **Hysteresis Control** - Prevents rapid on/off cycling (40% start, 55% stop)

## Hardware Requirements

### Microcontroller
- Arduino Uno/Nano or compatible board

### Sensors & Displays
- **4x Soil Moisture Sensors** (connected to A0-A3)
- **SSD1306 OLED Display** (128x64, I2C)
- **DS1307 RTC Module** (I2C)

### Watering Components
- **Water Pump** (connected to pin 4)
- **4x Solenoid Valves/Relays** (connected to pins 6, 8, 9, 10)

### User Interface
- **Push Button** (connected to pin 12)

### Power
- **5V Power Supply** for Arduino
- **12V Power Supply** for pump (separate from Arduino)
- **CR2032 Battery** for RTC backup

## Wiring Diagram

```
Arduino Pins    →    Components
----------------------------------------
A0              →    Moisture Sensor 1
A1              →    Moisture Sensor 2  
A2              →    Moisture Sensor 3
A3              →    Moisture Sensor 4

Pin 4           →    Water Pump (via relay/transistor)
Pin 6           →    Valve/Relay 1
Pin 8           →    Valve/Relay 2
Pin 9           →    Valve/Relay 3
Pin 10          →    Valve/Relay 4

Pin 12          →    Push Button (with pull-up resistor)

I2C (A4, A5)    →    OLED Display (SDA→A4, SCL→A5)
I2C (A4, A5)    →    DS1307 RTC (SDA→A4, SCL→A5)
```

## Installation

### 1. Install Required Libraries

Install these libraries via Arduino IDE Library Manager:
- **U8glib** (for OLED display)
- **RTClib** (for DS1307 RTC)

### 2. Hardware Setup

1. Connect all components according to the wiring diagram
2. Ensure proper power supplies for pump (12V) and Arduino (5V)
3. Insert CR2032 battery into RTC module
4. Connect moisture sensors to soil (probes should be fully inserted)

### 3. Software Setup

1. Open Arduino IDE
2. Copy the provided `.ino` code
3. Select your board and port
4. Upload the code to Arduino

### 4. Initial Configuration

1. Open Serial Monitor (9600 baud) to verify sensor readings
2. Check that all sensors show valid readings (360-600 range)
3. Press the button to toggle between time and moisture displays
4. Set RTC time if needed (code auto-adjusts on first run)

## Configuration

### Watering Thresholds

The system waters plants when moisture drops below **40%** and stops when it reaches **55%**.

To adjust thresholds, modify the `water_flower()` function:

```cpp
// Water when below this value (currently 40%)
if (sensor_active[0] && moisture1_value < 40) {
    // Start watering
}

// Stop watering when above this value (currently 55%)
} else if (sensor_active[0] && moisture1_value > 55) {
    // Stop watering
}
```

### Sensor Calibration

The moisture mapping is calibrated for readings between **360-600** ADC values:

```cpp
moisture1_value = map(average1, 600, 360, 0, 100); // Dry=600, Wet=360
```

To recalibrate:
1. Test dry soil reading (should be ~600)
2. Test wet soil reading (should be ~360)  
3. Adjust the `map()` values accordingly

### Active Sensors

Disable unused sensors by modifying the sensor flags:

```cpp
bool sensor_active[4] = {true, true, false, false}; // Only first 2 sensors active
```

## Operation

### Display Modes

**Time Mode** (default):
- Shows current date and time
- Displays Elecrow logo

**Moisture Mode** (button press):
- Shows moisture percentages for all 4 sensors
- Displays flower icons (good/bad based on moisture levels)

### Watering Logic

1. **Sensor Reading**: Takes 20 samples per sensor, averages them
2. **Validation**: Checks if readings are within valid range (360-600)
3. **Threshold Check**: 
   - If < 40%: Open valve and start pump
   - If > 55%: Close valve
4. **Pump Control**: Pump runs only when at least one valve is open

### Sensor Status

- **Active**: Green flower icon, shows percentage
- **Inactive**: "N/A" display, no watering
- **Faulty**: Automatically disabled if readings are out of range

## Serial Monitor Output

Open Serial Monitor at 9600 baud to see:
```
A0 Avg: 425.3 Range: 8 Active: true
A1 Avg: 512.7 Range: 5 Active: true
A2 Avg: 389.2 Range: 12 Active: true
A3 Avg: 1023.0 Range: 45 Active: false
```

## Troubleshooting

### Sensor Issues

| Problem | Cause | Solution |
|---------|-------|----------|
| Shows "N/A" | Sensor disconnected | Check wiring to A0-A3 |
| Always 100% | Out of valid range | Calibrate sensor or check soil contact |
| Erratic readings | Loose connection | Secure sensor wires, check for corrosion |
| Range > 10 | Electrical noise | Add 0.1µF capacitor across sensor |

### Display Issues

| Problem | Cause | Solution |
|---------|-------|----------|
| Blank screen | I2C connection | Check SDA/SCL wiring (A4/A5) |
| Garbage text | Wrong library | Use U8glib, not U8g2 |
| No time | RTC not running | Check RTC battery and wiring |

### Watering Issues

| Problem | Cause | Solution |
|---------|-------|----------|
| Won't water | Moisture > 40% | Wait for soil to dry or lower threshold |
| Constant watering | Sensor fault | Check Serial Monitor for sensor status |
| Pump won't start | Relay wiring | Verify pin 4 connection and power supply |

## Customization

### Adding More Sensors

To add sensors 5-6:
1. Define new analog pins: `int moisture5 = A6; int moisture6 = A7;`
2. Add new moisture variables and flags
3. Extend `read_value()` function
4. Add new relay pins and logic in `water_flower()`

### Display Customization

Modify `drawTH()` and `drawflower()` functions to:
- Change font sizes (`u8g_font_*`)
- Adjust display positions
- Add custom graphics or icons

### Watering Schedule

Add time-based watering by modifying `water_flower()`:

```cpp
DateTime now = RTC.now();
if (now.hour() >= 6 && now.hour() <= 20) { // Water only 6AM-8PM
    // Existing watering logic
}
```

## Safety Notes

⚠️ **Important Safety Warnings:**

1. **Water & Electricity**: Keep Arduino and wiring away from water sources
2. **Pump Power**: Use separate 12V supply for pump, never connect to Arduino 5V
3. **Overwatering**: Monitor system initially to prevent root rot
4. **Sensor Placement**: Ensure sensors don't touch metal containers
5. **RTC Battery**: Replace CR2032 battery every 2-3 years

## License

This project is open source and available under the MIT License. Feel free to modify and distribute.

## Support

- **Issues**: Check troubleshooting section or open GitHub issue
- **Forum**: Arduino forums or Elecrow community
- **Contact**: www.elecrow.com support

---

*Built with ❤️ by the Elecrow Community*  
*Last Updated: September 2025*
