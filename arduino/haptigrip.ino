#include <Arduino.h>
#include <Servo.h>
#include <SoftwareSerial.h>
#include <string.h>

// ============================================
// BLUETOOTH PINS
// HC-05: TX → Pin10, RX → Pin11
// ============================================
#define BT_RX  0
#define BT_TX  1
SoftwareSerial BT(BT_RX, BT_TX);

// ============================================
// SERVO PINS
// ============================================
#define PIN_THUMB   3
#define PIN_INDEX   5
#define PIN_MIDDLE  6
#define PIN_RING    9
#define PIN_PINKY   10

// ============================================
// FSR SENSOR PIN
// ============================================
#define FSR_PIN    A0

// ============================================
// FSR ENABLE FLAG
// ============================================
#define FSR_ENABLED false

// ============================================
// FSR THRESHOLDS (only used if FSR_ENABLED)
// ============================================
#define FSR_LIGHT   300
#define FSR_MEDIUM  550
#define FSR_HARD    800

// ============================================
// VIBRATION MOTOR PIN
// ============================================
#define MOTOR_PIN   2
#define MOTOR_MODE  0  // 0 = source mode, 1 = sink mode

// ============================================
// THUMB DIRECTION FIX
// ============================================
#define THUMB_FLIP true

#define OPEN_ANGLE   0
#define CLOSE_ANGLE  180
#define RESET_DELAY  600

// ============================================
// SERVO OBJECTS
// ============================================
Servo servos[5];
const char* fingerNames[5] = {"thumb","index","middle","ring","pinky"};
const int   fingerPins[5]  = {PIN_THUMB, PIN_INDEX, PIN_MIDDLE, PIN_RING, PIN_PINKY};

// ============================================
// STATE TRACKING
// ============================================
int  currentAngles[5] = {30, 10, 10, 10, 10};
char currentLabel[32] = "idle";

// ============================================
// EMG SERIAL OUTPUT
// Format: EMG,timestamp,thumb,index,middle,ring,pinky,fsr,label
// ============================================
void sendEMG(const char* label) {
    int fsr = FSR_ENABLED ? analogRead(FSR_PIN) : 0;
    Serial.print("EMG,");
    Serial.print(millis());
    Serial.print(",");
    for (int i = 0; i < 5; i++) {
        Serial.print(currentAngles[i]);
        Serial.print(",");
    }
    Serial.print(fsr);
    Serial.print(",");
    Serial.println(label);
}

// ============================================
// VIBRATION MOTOR
// ============================================
void buzz(int ms) {
    if (MOTOR_MODE == 0) {
        digitalWrite(MOTOR_PIN, HIGH); delay(ms); digitalWrite(MOTOR_PIN, LOW);
    } else {
        digitalWrite(MOTOR_PIN, LOW);  delay(ms); digitalWrite(MOTOR_PIN, HIGH);
    }
}

void buzzOnce()   { buzz(150); }
void buzzTwice()  { buzz(120); delay(100); buzz(120); }
void buzzThrice() { buzz(100); delay(80);  buzz(100); delay(80); buzz(100); }

// ============================================
// THUMB ANGLE HELPER
// ============================================
int thumbAngle(int a) {
    return THUMB_FLIP ? (180 - a) : a;
}

// ============================================
// FINGER HELPERS
// ============================================
void moveFinger(int idx, int angle) {
    if (idx < 0 || idx > 4) return;
    angle = constrain(angle, 0, 180);
    currentAngles[idx] = angle;
    servos[idx].write((idx == 0) ? thumbAngle(angle) : angle);
}

int getFingerIndex(const char* name) {
    for (int i = 0; i < 5; i++)
        if (strcmp(name, fingerNames[i]) == 0) return i;
    return -1;
}

void moveFingerByName(const char* name, int angle) {
    int idx = getFingerIndex(name);
    if (idx == -1) { BT.println("ERR: Unknown finger"); return; }
    moveFinger(idx, angle);
    BT.print("OK: "); BT.print(name);
    BT.print(" -> "); BT.println(angle);
}

// ============================================
// RESET
// ============================================
void resetToNormal() {
    moveFinger(0, 30);
    moveFinger(1, 10);
    moveFinger(2, 10);
    moveFinger(3, 10);
    moveFinger(4, 10);
    strncpy(currentLabel, "idle", sizeof(currentLabel));
    delay(500);
    BT.println(">> Ready for next command");
}

// ============================================
// POSES
// ============================================
void openHand() {
    BT.println(">> Opening hand...");
    strncpy(currentLabel, "open", sizeof(currentLabel));
    moveFinger(0, OPEN_ANGLE); moveFinger(1, OPEN_ANGLE);
    moveFinger(2, OPEN_ANGLE); moveFinger(3, OPEN_ANGLE);
    moveFinger(4, OPEN_ANGLE);
    sendEMG("open");
    buzzOnce(); delay(800); resetToNormal();
}

void closeHand() {
    BT.println(">> Closing hand (fist)...");
    strncpy(currentLabel, "close", sizeof(currentLabel));
    moveFinger(0, CLOSE_ANGLE); moveFinger(1, CLOSE_ANGLE);
    moveFinger(2, CLOSE_ANGLE); moveFinger(3, CLOSE_ANGLE);
    moveFinger(4, CLOSE_ANGLE);
    sendEMG("close");
    buzzOnce(); delay(800); resetToNormal();
}

void grip() {
    BT.println(">> Grip...");
    strncpy(currentLabel, "grip", sizeof(currentLabel));
    moveFinger(0, 60);
    moveFinger(1, CLOSE_ANGLE); moveFinger(2, CLOSE_ANGLE);
    moveFinger(3, CLOSE_ANGLE); moveFinger(4, CLOSE_ANGLE);
    sendEMG("grip");
    buzzTwice(); delay(800); resetToNormal();
}

void peaceSign() {
    BT.println(">> Peace sign...");
    strncpy(currentLabel, "peace", sizeof(currentLabel));
    moveFinger(0, CLOSE_ANGLE);
    moveFinger(1, OPEN_ANGLE); moveFinger(2, OPEN_ANGLE);
    moveFinger(3, CLOSE_ANGLE); moveFinger(4, CLOSE_ANGLE);
    sendEMG("peace");
    buzzOnce(); delay(800); resetToNormal();
}

void okSign() {
    BT.println(">> OK sign...");
    strncpy(currentLabel, "ok", sizeof(currentLabel));
    moveFinger(0, CLOSE_ANGLE); moveFinger(1, CLOSE_ANGLE);
    moveFinger(2, OPEN_ANGLE);
    moveFinger(3, OPEN_ANGLE); moveFinger(4, OPEN_ANGLE);
    sendEMG("ok");
    buzzOnce(); delay(800); resetToNormal();
}

void pointFinger() {
    BT.println(">> Pointing...");
    strncpy(currentLabel, "point", sizeof(currentLabel));
    moveFinger(0, CLOSE_ANGLE);
    moveFinger(1, OPEN_ANGLE);
    moveFinger(2, CLOSE_ANGLE); moveFinger(3, CLOSE_ANGLE);
    moveFinger(4, CLOSE_ANGLE);
    sendEMG("point");
    buzzOnce(); delay(800); resetToNormal();
}

void thumbsUp() {
    BT.println(">> Thumbs up...");
    strncpy(currentLabel, "thumbsup", sizeof(currentLabel));
    moveFinger(0, OPEN_ANGLE);
    moveFinger(1, CLOSE_ANGLE); moveFinger(2, CLOSE_ANGLE);
    moveFinger(3, CLOSE_ANGLE); moveFinger(4, CLOSE_ANGLE);
    sendEMG("thumbsup");
    buzzOnce(); delay(800); resetToNormal();
}

void waveHand() {
    BT.println(">> Waving...");
    for (int w = 0; w < 3; w++) {
        strncpy(currentLabel, "wave_open", sizeof(currentLabel));
        moveFinger(0, OPEN_ANGLE); moveFinger(1, OPEN_ANGLE);
        moveFinger(2, OPEN_ANGLE); moveFinger(3, OPEN_ANGLE);
        moveFinger(4, OPEN_ANGLE);
        sendEMG("wave_open");
        delay(350);

        strncpy(currentLabel, "wave_close", sizeof(currentLabel));
        moveFinger(1, CLOSE_ANGLE); moveFinger(2, CLOSE_ANGLE);
        moveFinger(3, CLOSE_ANGLE); moveFinger(4, CLOSE_ANGLE);
        sendEMG("wave_close");
        delay(350);
    }
    buzzTwice(); resetToNormal();
}

// ============================================
// FSR AUTO-GRIP
// ============================================
void checkFSR() {
    if (!FSR_ENABLED) return;

    int val = analogRead(FSR_PIN);
    Serial.print("FSR: "); Serial.println(val);

    if (val > FSR_HARD) {
        BT.print("FSR HARD: "); BT.println(val);
        strncpy(currentLabel, "fsr_hard", sizeof(currentLabel));
        moveFinger(0, CLOSE_ANGLE); moveFinger(1, CLOSE_ANGLE);
        moveFinger(2, CLOSE_ANGLE); moveFinger(3, CLOSE_ANGLE);
        moveFinger(4, CLOSE_ANGLE);
        sendEMG("fsr_hard");
        buzz(400); delay(800); resetToNormal();

    } else if (val > FSR_MEDIUM) {
        BT.print("FSR MEDIUM: "); BT.println(val);
        strncpy(currentLabel, "fsr_medium", sizeof(currentLabel));
        moveFinger(0, 60);
        moveFinger(1, CLOSE_ANGLE); moveFinger(2, CLOSE_ANGLE);
        moveFinger(3, CLOSE_ANGLE); moveFinger(4, CLOSE_ANGLE);
        sendEMG("fsr_medium");
        buzzTwice(); delay(800); resetToNormal();

    } else if (val > FSR_LIGHT) {
        BT.print("FSR LIGHT: "); BT.println(val);
        strncpy(currentLabel, "fsr_light", sizeof(currentLabel));
        moveFinger(1, 90); moveFinger(2, 90);
        moveFinger(3, 90); moveFinger(4, 90);
        sendEMG("fsr_light");
        buzzOnce(); delay(600); resetToNormal();
    }
}

// ============================================
// COMMAND PARSER
// ============================================
void parseCommand(char* cmd) {
    while (*cmd == '#' || *cmd == ' ') cmd++;
    for (int i = 0; cmd[i]; i++)
        if (cmd[i] >= 'A' && cmd[i] <= 'Z') cmd[i] += 32;
    if (strlen(cmd) == 0) return;

    char* token = strtok(cmd, " ");
    if (!token) return;

    if (strcmp(token, "open") == 0) {
        char* next = strtok(NULL, " ");
        if (next) {
            moveFingerByName(next, OPEN_ANGLE);
            sendEMG("open"); delay(600); resetToNormal();
        } else { openHand(); }

    } else if (strcmp(token, "close") == 0) {
        char* next = strtok(NULL, " ");
        if (next) {
            moveFingerByName(next, CLOSE_ANGLE);
            sendEMG("close"); delay(600); resetToNormal();
        } else { closeHand(); }

    } else if (strcmp(token, "grip")     == 0) { grip();
    } else if (strcmp(token, "peace")    == 0) { peaceSign();
    } else if (strcmp(token, "ok")       == 0) { okSign();
    } else if (strcmp(token, "point")    == 0) { pointFinger();
    } else if (strcmp(token, "wave")     == 0) { waveHand();
    } else if (strcmp(token, "thumbsup") == 0) { thumbsUp();
    } else if (strcmp(token, "reset")    == 0) {
        resetToNormal(); sendEMG("idle"); buzzOnce();

    } else if (strcmp(token, "move") == 0 || strcmp(token, "set") == 0) {
        char* fTok = strtok(NULL, " ");
        char* aTok = strtok(NULL, " ");
        if (!fTok || !aTok) { BT.println("ERR: Usage: move <finger> <angle>"); return; }
        if (strcmp(aTok, "to") == 0) aTok = strtok(NULL, " ");
        if (!aTok) { BT.println("ERR: Missing angle"); return; }
        moveFingerByName(fTok, atoi(aTok));
        sendEMG("move"); buzzOnce(); delay(600); resetToNormal();

    } else {
        BT.println("ERR: Unknown command.");
        BT.println("Commands: open, close, grip, peace,");
        BT.println("  ok, point, wave, thumbsup, reset");
        BT.println("  open/close <finger>");
        BT.println("  move <finger> <angle>");
    }

    delay(RESET_DELAY);
}

// ============================================
// SETUP
// ============================================
void setup() {
    Serial.begin(9600);
    BT.begin(9600);

    pinMode(MOTOR_PIN, OUTPUT);
    digitalWrite(MOTOR_PIN, (MOTOR_MODE == 0) ? LOW : HIGH);

    for (int i = 0; i < 5; i++)
        servos[i].attach(fingerPins[i]);

    resetToNormal();
    buzzThrice();

    BT.println("==============================");
    BT.println(" Prosthetic Hand Ready!");
    BT.println(" FSR: DISABLED (command-only)");
    BT.println("------------------------------");
    BT.println(" Commands:");
    BT.println("  open        - open all");
    BT.println("  close       - make fist");
    BT.println("  grip        - power grip");
    BT.println("  peace       - peace sign");
    BT.println("  ok          - ok sign");
    BT.println("  point       - point finger");
    BT.println("  wave        - wave hand");
    BT.println("  thumbsup    - thumbs up");
    BT.println("  reset       - rest position");
    BT.println("  open thumb  - open one finger");
    BT.println("  close index - close one finger");
    BT.println("  move ring 90 - set angle");
    BT.println("==============================");
}

// ============================================
// LOOP
// ============================================
void loop() {
    static char          buf[64];
    static int           pos = 0;
    static unsigned long lastStream = 0;

    checkFSR();

    if (millis() - lastStream >= 50) {
        sendEMG(currentLabel);
        lastStream = millis();
    }

    while (BT.available()) {
        char c = (char)BT.read();
        if (c == '\n' || c == '\r') {
            if (pos > 0) {
                buf[pos] = '\0';
                BT.print("CMD received: "); BT.println(buf);
                parseCommand(buf);
                pos = 0;
            }
        } else if (pos < 63) {
            buf[pos++] = c;
        }
    }

    delay(50);
}
