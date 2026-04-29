# 📟 Smart Warehouse Firmware (ESP32)

<div align="center">
  <img src="https://img.shields.io/badge/Hardware-ESP32-E7352C?style=for-the-badge" alt="ESP32" />
  <img src="https://img.shields.io/badge/Language-C/C++-00599C?style=for-the-badge" alt="C" />
  <img src="https://img.shields.io/badge/Framework-ESP--IDF-E7352C?style=for-the-badge" alt="ESP-IDF" />
</div>

## 📖 Introduction
Đây là mã nguồn nhúng (Firmware) chạy trên Vi điều khiển trung tâm **ESP32**. Nhiệm vụ của Firmware là giao tiếp trực tiếp với các cảm biến vật lý (Nhiệt độ, Độ ẩm, Khí Gas), quản lý đầu đọc thẻ từ RFID RC522 để thực hiện các thao tác kiểm kho, và kết nối với Server Backend qua giao thức MQTT.

## 📂 Folder Structure
Cấu trúc thư mục được tuân theo chuẩn của ESP-IDF (Espressif IoT Development Framework).

```text
firmware/
├── CMakeLists.txt           # File cấu hình build project bằng CMake
├── sdkconfig                # File cấu hình hệ thống (WiFi, MQTT, Clock...) của ESP-IDF
└── main/                    # Thư mục chứa mã nguồn chính
    ├── main.c               # Chạy chương trình chính            
└── components/              # thư mục quản lý thành phần thư viện khác    
    ├── iot_network.h./.c    # Xử lý kết nối WiFi và giao thức MQTT
    ├── dht11.h/.c           # Code giao tiếp với cảm biến DHT11 (Nhiệt độ/Độ ẩm)
    ├── rc522.h/.c           # Giao tiếp SPI với module RFID RC522
    └── mq2_sensor.h/.c      # Xử lý tín hiệu Analog từ cảm biến khí Gas MQ2
```

## 🚀 Setup & Run Instructions

Để biên dịch và nạp đoạn mã này vào ESP32, bạn cần môi trường **ESP-IDF**.

### Bước 1: Chuẩn bị môi trường
1. Cài đặt **ESP-IDF** (phiên bản v5.x) theo hướng dẫn của Espressif.
2. Hoặc sử dụng Extension **ESP-IDF** trên Visual Studio Code để thao tác dễ dàng hơn.


### Bước 2: Cấu hình hệ thống
Trước khi nạp code, bạn cần thiết lập cấu hình mạng WiFi và MQTT Broker:
1. Mở file `main/main.c` (hoặc cấu hình qua Menuconfig).
2. Thay đổi thông tin mạng cục bộ của bạn:
   ```c
   #define WIFI_SSID "Tên_WiFi_Của_Bạn"
   #define WIFI_PASS "Mật_Khẩu_WiFi"
   ```
3. Đảm bảo MQTT Broker (HiveMQ) đang được cấu hình đúng endpoint.

### Bước 3: Build & Flash
Sử dụng Terminal của ESP-IDF, chạy các lệnh sau (thay `COM3` bằng cổng USB kết nối với ESP32 của bạn):

```bash
# 1. Đặt target là esp32
idf.py set-target esp32

# 2. Biên dịch mã nguồn
idf.py build

# 3. Nạp code và xem Monitor Log 
idf.py -p COM3 flash monitor 
```

**Lưu ý:** Nếu khi Flash báo lỗi, hãy thử nhấn giữ nút `BOOT` trên board mạch ESP32 khi màn hình terminal hiển thị dòng chữ `Connecting...`.
