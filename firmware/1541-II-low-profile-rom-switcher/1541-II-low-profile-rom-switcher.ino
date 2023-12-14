#include <EEPROM.h>

#define SKS64_MAGIC     0xde
#define MAX_BANK        3

#define A14             PIN_PA3
#define A15             PIN_PA6

void setup() {
  // Communication from sks64 in the c64 is one way to us over the drive's RESET
  // line. This leaves the UART's TX line available for us to use for serial
  // debug output from the ATtiny

  // This switches the UART to use it's alternate pins on the ATtiny, which are
  // more convenient for trace routing
  // RX = PA2 (pin 5) connected to the RESET port
  // TX = PA1 (pin 4) connected to the DEBUG port
  Serial.swap();
  Serial.begin(19200, SERIAL_8N1);

  pinMode(A14, OUTPUT);
  pinMode(A15, OUTPUT);
  
  setBank(loadBank());
}

uint8_t loadBank(void) {
  uint8_t bank = EEPROM.read(0);

  Serial.write('L');
  if(bank > MAX_BANK) {
    Serial.write('E');
    bank = 0;
  }
  Serial.write('0' + bank);
  
  return bank;
}

void setBank(uint8_t bank) {
  digitalWrite(A14, bank & 0x01);
  digitalWrite(A15, bank & 0x02);

  Serial.write('S');
  Serial.write('0' + bank);
}


void saveBank(uint8_t bank) {
  EEPROM.write(0, bank);
  Serial.write('W');
  Serial.write('0' + bank);
  Serial.println();
}

void loop() {
  
  if(!Serial.available()) {
    return;
  }

  // When sks64 in the c64 changes the rom bank in the c64 it will send 2 bytes to
  // the disk drive over the RESET line
  // 0xde = magic byte
  // 0x0n = bank it switched to (0 to 3 or 7 depending on the sks64's configuration)
  uint8_t magic = Serial.read();
  if(magic != SKS64_MAGIC) {
    return;
  }
  Serial.write('M');

  delay(1);
  
  if(!Serial.available()) {
    return;
  }

  uint8_t bank = Serial.read();
  Serial.write('R');
  Serial.print(bank, HEX);

  // Should we take some other action if the user is using a 8 kernel configuration
  // with the sks64 in the c64? Below will force kernels 5-8 to always get our 4th
  // kernel+dos bank
  if(bank > MAX_BANK) {
    bank = MAX_BANK;
  }
  
  setBank(bank);
  saveBank(bank);
}
