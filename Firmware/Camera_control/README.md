# XIAO ESP32S3 Sense Camera Streaming with OV3660
## Features
- 📹 Live MJPEG streaming over WiFi
- 🎯 Optimized for OV3660 camera (not OV2640)
- 💾 8MB OCTAL PSRAM configuration
- 🌐 Web interface accessible from any browser
- ⚡ Up to 30fps at VGA, 20fps at HD
- 🔧 Configurable quality and resolution

## Hardware
- XIAO ESP32S3 Sense board
- OV3660 camera module (included with board)
- WiFi antenna (recommended)
- USB-C cable


## Configuration
### WiFi Configuration
```bash
# Copy template
cp sdkconfig.defaults.template sdkconfig.defaults
# Edit with your credentials
vim sdkconfig.defaults
```
Update these lines:
```ini
CONFIG_ESP_WIFI_SSID="YourWiFiName"
CONFIG_ESP_WIFI_PASSWORD="YourPassword"
```

### PSRAM Configuration (OCTAL Mode)
**File: `sdkconfig.defaults`**
```ini
# CRITICAL: XIAO ESP32S3 uses OCTAL PSRAM, not QUAD
CONFIG_SPIRAM_MODE_OCT=y
CONFIG_SPIRAM_TYPE_ESPPSRAM64=y
CONFIG_SPIRAM_SPEED_40M=y
# Memory allocation - reduce internal reservation
CONFIG_SPIRAM_MALLOC_ALWAYSINTERNAL=4096
```

### Resolution Options
Edit in `main/camera_stream.c`:
```c
.frame_size = FRAMESIZE_QVGA,  
```

### Quality Settings
```c
.jpeg_quality = 115,  // 0-63 (lower = better quality)
```
- **Fast:** 15-20 (smaller files, lower quality)


## Project Structure

```
ESPIDF-example/
├── boards/
│   └── xiao_esp32s3_sense/
│       ├── pins.h                    # GPIO pin definitions
│       ├── sdkconfig.board           # Hardware-specific config
│       └── board.cmake               # Board integration
│
├── main/
│   ├── camera_stream.c               # Main application
│   ├── Kconfig.projbuild             # WiFi credential definitions
│   └── CMakeLists.txt                # Build configuration
│
├── managed_components/
│   └── espressif__esp32-camera/      # Auto-installed
│
├── idf_component.yml                 # esp32-camera dependency
├── sdkconfig.defaults                # Your settings (gitignored)
├── sdkconfig.defaults.template       # Template for users
├── .gitignore
└── README.md
```