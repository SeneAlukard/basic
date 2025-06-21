// src/drone.cpp

#include "RF24.h"
#include "mpu6050.hpp"
#include <chrono>
#include <cstdint>
#include <iostream>
#include <thread>

// GPIO pin tanımları
constexpr uint8_t TX_CE_PIN = 27;
constexpr uint8_t TX_CSN_PIN = 00;
constexpr uint8_t RX_CE_PIN = 22;
constexpr uint8_t RX_CSN_PIN = 10;

// 40-bit RF adresleri
static const uint64_t ADDR_DRONE_TO_GBS = 0xF0F0F0F0D2ULL;
static const uint64_t ADDR_GBS_TO_DRONE = 0xF0F0F0F0E1ULL;

// Drone → GBS için gyro paketi
struct GyroPacket {
  int16_t gyroX, gyroY, gyroZ;
  uint32_t seq;
};

// GBS → Drone için basit komut paketi
struct CommandPacket {
  uint8_t cmdId;
  int16_t param;
  uint32_t seq;
};

int main() {
  // 1) İki ayrı RF24 nesnesi: biri TX, biri RX
  RF24 radioTx(TX_CE_PIN, TX_CSN_PIN);
  RF24 radioRx(RX_CE_PIN, RX_CSN_PIN);

  // 2) Modülleri başlat
  if (!radioTx.begin() || !radioRx.begin()) {
    std::cerr << "RF24 modülleri başlatılamadı!\n";
    return 1;
  }

  // --- TX ayarları ---
  radioTx.openWritingPipe(ADDR_DRONE_TO_GBS);
  radioTx.setPALevel(RF24_PA_LOW);
  radioTx.setDataRate(RF24_1MBPS);
  radioTx.stopListening(); // yalnızca gönderim modu

  // --- RX ayarları ---
  radioRx.openReadingPipe(1, ADDR_GBS_TO_DRONE);
  radioRx.setPALevel(RF24_PA_LOW);
  radioRx.setDataRate(RF24_1MBPS);
  radioRx.startListening(); // yalnızca alım modu

  // 3) MPU6050 başlat
  Mpu6050 sensor;
  if (!sensor.init("/dev/i2c-1", 0x68)) {
    std::cerr << "MPU6050 başlatılamadı!\n";
    return 2;
  }

  // 4) Döngü: her 50 ms’de gyro gönder, gelen komutları kontrol et
  const auto interval = std::chrono::milliseconds(50);
  uint32_t seq = 0;

  while (true) {
    // --- Gönderim ---
    int16_t gx, gy, gz;
    if (sensor.readGyro(gx, gy, gz)) {
      GyroPacket gp{gx, gy, gz, seq++};
      bool ok = radioTx.write(&gp, sizeof(gp));
      if (!ok) {
        std::cerr << "[TX ERROR] seq=" << gp.seq << "\n";
      } else {
        std::cout << "[TX] gyro=(" << gp.gyroX << "," << gp.gyroY << ","
                  << gp.gyroZ << ") seq=" << gp.seq << "\n";
      }
    } else {
      std::cerr << "[SENSOR ERROR] Gyro okunamadı\n";
    }

    // --- Alım (komutlar) ---
    if (radioRx.available()) {
      CommandPacket cp;
      radioRx.read(&cp, sizeof(cp));
      std::cout << "[RX] cmdId=" << int(cp.cmdId) << " param=" << cp.param
                << " seq=" << cp.seq << "\n";
      // Burada cp.cmdId ve cp.param’a göre drone’unuzu kontrol edebilirsiniz
    }

    std::this_thread::sleep_for(interval);
  }

  return 0;
}
