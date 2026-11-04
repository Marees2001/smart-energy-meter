## Configuration Setup

Before building or flashing the firmware, configure your local MQTT broker credentials in `define.h`:

1. Open `define.h` in your editor.
2. Locate the MQTT configuration macros:
   - `MQTT_BROKER_URL`: Set to your MQTT broker address/host (e.g., `"broker.emqx.io"` or your local IP/domain).
   - `MQTT_BROKER_USER`: Set your broker username.
   - `MQTT_BROKER_PASSWORD`: Set your broker password.
3. Save the file and compile the project.

> **Note:** Do not commit private or production credentials to public branches. Use a local config or keep sensitive tokens secure.
