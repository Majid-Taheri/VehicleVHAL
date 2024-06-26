# Vehicle Speed Emulator

## Simulation Description

This C++ simulation models manual driving and cruise control scenarios, designed with specific assumptions to balance realism and computational simplicity.

### Assumptions

#### Manual Mode
- **Velocity Achievement:** The vehicle instantly reaches the randomly generated and optionally smoothed velocity without considering acceleration limits or vehicle dynamics.
- **Smoothing:** Velocity smoothing is implemented using a moving average filter with a sliding window. A window size of 1 deactivates smoothing, while larger sizes increase the smoothing effect.

#### Cruise Control Mode
- **Velocity Setting:** Similar to manual mode, the vehicle instantly achieves the set velocity without acceleration or dynamic constraints.
- **Smoothing:** No smoothing is applied to the cruise control speed setting, assuming a constant velocity once set.

### Simulation Environment
The simulation operates in real-time to better mimic realistic conditions, utilizing timestamps from the system clock for accurate timing.

### Considerations
- **Trade-offs:** The assumptions prioritize behavioral modeling over exact vehicle dynamics, balancing simulation complexity with computational feasibility.
- **Implementation:** Ensure the simulation accurately reflects intended manual driving and cruise control behaviors based on the specified assumptions.

## Project Structure

- **CMakeLists.txt:** Defines the project build configuration, including required libraries and source files.
- **IVehicle.h:** Defines the base class interface for vehicle operations.
- **VehicleClient.cpp:** Contains the CLI implementation for interacting with the VHAL.
- **VehicleImpl.cpp:** Implements the vehicle's speed control logic.
- **VehicleImpl.h:** Declares the functions and variables used in `VehicleImpl.cpp`.

## Classes and Their Responsibilities

### `IVehicle.h`
Defines the interface for vehicle operations:
- `virtual void setSpeed(int speed) = 0;`
- `virtual void unsetSpeed() = 0;`
- `virtual int getSpeed() const = 0;`
- `virtual int getHistoricCCSpeed(int index) const = 0;`
- `virtual void subscribeSpeed() = 0;`
- `virtual void unsubscribeSpeed() = 0;`

### `VehicleImpl.h` and `VehicleImpl.cpp`
Implements the vehicle's speed control logic:
- **Manual Mode:** Generates random speeds.
- **Cruise Control Mode:** Sets and maintains a constant speed, logs historical speeds.
- **Historical Data Management:** Reads from and writes to a JSON file for historical cruise control speeds.

### `VehicleClient.cpp`
Implements the command-line interface for user interaction:
- **Commands:** `get speed`, `set speed <value>`, `unset speed`, `get historic_cc_speed <index>`, `subscribe speed`, `unsubscribe speed`.


### CLI Commands

- **`get speed`**
    Returns the current speed. In Default Mode, the speed varies randomly. In Cruise Control, the speed remains constant.

- **`set speed <value>`**
    Sets the speed to the provided value for Cruise Control.

- **`unset speed`**
    Disables Cruise Control and returns to Default Mode with randomly generated speeds.

- **`get historic_cc_speed <index>`**
    Returns the historical Cruise Control speed at the provided index. If `index` is `min`, it returns the oldest recorded speed. If `index` is `max`, it returns the most recent recorded speed.

- **`subscribe speed`**
    Continuously prints the current speed in the command line.

- **`unsubscribe speed`**
    Ends the continuous printing of the current speed.

## Logging

The emulator also saves logs in a JSON file for post-processing. Each log entry contains the current state of the cruise control, the current speed, and a timestamp. 
