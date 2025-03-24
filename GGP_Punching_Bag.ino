/*
  A punching bag for Gundam G Generation Pocket using ESP32.
  Author : joushiikuta
  Date   : 2025/03/24
  Rev    : 1.0.0
  link   : https://www.youtube.com/@joushiikuta
*/

#define Pin_LED GPIO_NUM_2
#define Pin_Signal GPIO_NUM_15

#define START_INTERVAL_0 8050
#define START_INTERVAL_1 8150
#define BIT_INTERVAL 7800
#define END_INTERVAL 7450

bool timeout_error = false;
bool receive_error = false;

unsigned long PacketReceive[4];
unsigned long PacketSend[4] = {  /* Versus Core Fighter[NOR] S[0] */
  /* We attack first */
  0b10011111101101111010, 0b11110111101110011010, 0b11110101101111100110, 0b01010111101101111110,
  /* Enemy attack first */
  //  0b11011111101101111010, 0b11110111101110011010, 0b11110101101111100110, 0b11011111101001111110,
};

unsigned long GGP_pluseReceive(int timeout) {
  while (digitalRead(Pin_Signal) == LOW) {
    delayMicroseconds(1000);
    if (timeout == 0) {
      timeout_error = true;
      return 0;
    }
    else {
      timeout = timeout - 1;
    }
  }
  delayMicroseconds(BIT_INTERVAL / 2);
  unsigned long Receive = 0;
  for (int i = 1; i < 20; i++) {  // Receive's lsb most be 0
    if (digitalRead(Pin_Signal)) {
      Receive = Receive | ((unsigned long)1 << i);
    }
    delayMicroseconds(BIT_INTERVAL);
  }
  // Print Receive
  Serial.print("Receive: ");
  for (int i = 19; i >= 0; i--) {
    Serial.print((Receive & ((unsigned long)1 << i)) ? 1 : 0 );
  }
  Serial.println();
  delayMicroseconds(5000);
  if (Receive == 0) {
    receive_error = true;
  }
  return Receive;
}

void GGP_pluseSend(unsigned long Send) {
  pinMode(Pin_Signal, OUTPUT);
  // Print Send
  Serial.print("Send   : ");
  for (int i = 19; i >= 0; i--) {
    Serial.print((Send & ((unsigned long)1 << i)) ? 1 : 0 );
  }
  Serial.println();
  /* Packet 20bits */
  digitalWrite(Pin_Signal, (Send & ((unsigned long)1 << 0)) ? HIGH : LOW);
  delayMicroseconds(START_INTERVAL_0);
  digitalWrite(Pin_Signal, (Send & ((unsigned long)1 << 1)) ? HIGH : LOW);
  delayMicroseconds(START_INTERVAL_1);
  for (int i = 2; i < 19; i++) {
    digitalWrite(Pin_Signal, (Send & ((unsigned long)1 << i)) ? HIGH : LOW);
    delayMicroseconds(BIT_INTERVAL);
  }
  digitalWrite(Pin_Signal, (Send & ((unsigned long)1 << 19)) ? HIGH : LOW);
  delayMicroseconds(END_INTERVAL);
  digitalWrite(Pin_Signal, LOW);
  delayMicroseconds(1200);
  pinMode(Pin_Signal, INPUT_PULLDOWN);
}

void TryBattle() {
  Serial.println("------- Try GGP Battle -------");
  for (int i = 0; i < 4; i++) {
    GGP_pluseSend(PacketSend[i]);
    PacketReceive[i] = GGP_pluseReceive(100);
    if (timeout_error || receive_error) {
      break;
    }
  }
  if (timeout_error) {
    Serial.println("Timeout Error.");
  }
  else if (receive_error) {
    Serial.println("Receive Error.");
  }
  Serial.println("------------------------------");
}

void Waiting() {
  int count = 0;
  timeout_error = false;
  receive_error = false;
  while (digitalRead(Pin_Signal) == LOW) {
    delayMicroseconds(1000);
    count++;
    if (count >= 5) {
      pinMode(Pin_Signal, INPUT_PULLDOWN);
      digitalWrite(Pin_LED, HIGH);
      delay(500);
      TryBattle();
      pinMode(Pin_Signal, INPUT_PULLUP);
      digitalWrite(Pin_LED, LOW);
      delay(100);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(Pin_Signal, INPUT_PULLUP);
  pinMode(Pin_LED, OUTPUT);
  digitalWrite(Pin_LED, LOW);
  Serial.println("Gundam G Generation Pocket Punching Bag Start!");
  Serial.println("Press A button to start a battle.");
}

void loop() {
  Waiting();
}
