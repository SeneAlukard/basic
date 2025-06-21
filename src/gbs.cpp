// src/server.cpp

#include <RF24.h>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <thread>

// GPIO pin tanımları (drone ile simetrik kullanın)
constexpr uint8_t RX_CE_PIN = 27; // drone → server gyro paketi
constexpr uint8_t RX_CSN_PIN = 0;
constexpr uint8_t TX_CE_PIN = 22; // server → drone komut paketi
constexpr uint8_t TX_CSN_PIN = 10;

// 40-bit RF adresleri (drone.cpp ile aynı)
static const uint64_t ADDR_DRONE_TO_GBS = 0xF0F0F0F0D2ULL;
static const uint64_t ADDR_GBS_TO_DRONE = 0xF0F0F0F0E1ULL;

// drone’dan gelen paket
struct GyroPacket {
  int16_t gyroX, gyroY, gyroZ;
  uint32_t seq;
};

// drone’a gidecek ACK/komut paketi
struct CommandPacket {
  uint8_t cmdId;
  int16_t param;
  uint32_t seq;
};

int main() {
  // 1. İki RF24 nesnesi: biri receive, biri transmit
  RF24 radioRx(RX_CE_PIN, RX_CSN_PIN);
  RF24 radioTx(TX_CE_PIN, TX_CSN_PIN);

  // 2. Başlat ve yapılandır
  if (!radioRx.begin() || !radioTx.begin()) {
    std::cerr << "RF24 modülleri başlatılamadı!\n";
    return 1;
  }

  // RX ayarları (drone → GBS)
  radioRx.openReadingPipe(1, ADDR_DRONE_TO_GBS);
  radioRx.setPALevel(RF24_PA_LOW);
  radioRx.setDataRate(RF24_1MBPS);
  radioRx.startListening();

  // TX ayarları (GBS → drone)
  radioTx.openWritingPipe(ADDR_GBS_TO_DRONE);
  radioTx.setPALevel(RF24_PA_LOW);
  radioTx.setDataRate(RF24_1MBPS);
  radioTx.stopListening();

  std::cout << "Server: Listening for gyro packets...\n";

  uint32_t ackSeq = 0;
  const auto interval = std::chrono::milliseconds(10);

  while (true) {
    // --- Gelen gyro paketlerini oku ---
    if (radioRx.available()) {
      GyroPacket gp;
      radioRx.read(&gp, sizeof(gp));
      std::cout << "[RX] gyro=(" << gp.gyroX << "," << gp.gyroY << ","
                << gp.gyroZ << ") seq=" << gp.seq << "\n";

      // --- Basit bir ACK / komut paketi gönder ---
      CommandPacket cp;
      cp.cmdId = 0x01;                         // örnek komut ID’si
      cp.param = static_cast<int16_t>(gp.seq); // geri seq’i param olarak dön
      cp.seq = ackSeq++;

      bool ok = radioTx.write(&cp, sizeof(cp));
      if (ok) {
        std::cout << "[TX] ACK cmdId=" << int(cp.cmdId) << " param=" << cp.param
                  << " seq=" << cp.seq << "\n";
      } else {
        std::cerr << "[TX ERROR] ACK gönderilemedi\n";
      }
    }

    std::this_thread::sleep_for(interval);
  }

  return 0;
}
