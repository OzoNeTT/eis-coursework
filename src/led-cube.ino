#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>

#define INVERT_Y 1    // инвертировать по вертикали (если дождь идёт вверх)
#define INVERT_X 0    // инвертировать по горизонтали (если текст не читается)
#define XAXIS 0
#define YAXIS 1
#define ZAXIS 2
#define POS_X 0
#define NEG_X 1
#define POS_Z 2
#define NEG_Z 3
#define POS_Y 4
#define NEG_Y 5
#define Up_buttonPin A3
#define Down_buttonPin A2
#define RED_LED A1
#define GREEN_LED A0
//control buttons -----------
int buttonPushCounter = 0;   // counter for the number of button presses
int up_buttonState = 0;         // current state of the up button
int up_lastButtonState = 0;     // previous state of the up button
int down_buttonState = 0;         // current state of the up button
int down_lastButtonState = 0;     // previous state of the up button
bool bPress = false;
// --------------------------
// effects duration ----------
#define RAIN_TIME 260
#define PLANE_BOING_TIME 220
#define SEND_VOXELS_TIME 140
#define WOOP_WOOP_TIME 350
#define CUBE_JUMP_TIME 200
#define CLOCK_TIME 500
#define GLOW_TIME 8
#define WALKING_TIME 100
// ---------------------------
int currentEffect; // for glowing
uint8_t cube[8][8];
uint32_t millTimer;
uint16_t timer;
uint16_t modeTimer;
bool loading;
int8_t pos;
int8_t vector[3];
int16_t coord[3];
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
void setup()
{
    Serial.begin(9600);
    lcd.begin(16, 2);
    lcd.setCursor(0, 0);
    lcd.print("Select mode:");
    SPI.begin();
    SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
    randomSeed(analogRead(0));
    pinMode(Up_buttonPin,INPUT_PULLUP);
    pinMode(Down_buttonPin,INPUT_PULLUP);
    pinMode(RED_LED, OUTPUT);
    pinMode(GREEN_LED, OUTPUT);
    lcd.setCursor(0,1);
    lcd.print("<     GLOW     >");

    loading = true;
    currentEffect = 0;
    changeMode();

}
void loop()
{
    checkUp();
    checkDown();
    if( bPress){
        bPress = false;
        currentEffect = abs(buttonPushCounter) % 10;
        changeMode();
    }
    switch (currentEffect) {
        case 1: rain(); break;
        case 2: planeBoing(); break;
        case 3: sendVoxels(); break;
        case 4: woopWoop(); break;
        case 5: cubeJump(); break;
        case 6: lit(); break;
        case 7: sinusFill(); break;
        case 8: sinusThin(); break;
        case 9: walkingCube(); break;
        case 0: glow(); break;
    }
    renderCube();
}

byte func1(byte x){
    return ((x&1) | ((x >> 2) << 1 )) & 3;
}

byte func2(byte x){
    return (((x & 128) | ((x << 2) >> 1)) & (128 + 64)) >> 4;
}

byte func_produce(int a, int i, int j) {
	return func2(cube[a][i] & (~31)) | func1(cube[a][i] & 31)  |  (func2(cube[a][j] & (~31)) | func1(cube[a][j] & 31)) << 4
}

void renderCube() {
    digitalWrite(SS, LOW);
    SPI.transfer(0x01 << 0);
	byte num11 = func_produce(0, 2, 0);
    SPI.transfer(num11);
    byte num12 = func_produce(0, 7, 5);
    SPI.transfer(num12);
    digitalWrite (SS, HIGH);
    delayMicroseconds(50);

    digitalWrite (SS, LOW);
    SPI.transfer(0x01 << 1);
	byte num21 = func_produce(2, 2, 0);
    SPI.transfer(num21);
    byte num22 = func_produce(2, 7, 5);
    SPI.transfer(num22);
    digitalWrite (SS, HIGH);
    delayMicroseconds(50);

    digitalWrite (SS, LOW);
    SPI.transfer(0x01 << 2);
    byte num31 = func_produce(5, 2, 0);
    SPI.transfer(num31);
    byte num32 = func_produce(5, 7, 5);
    SPI.transfer(num32);
    digitalWrite (SS, HIGH);
    delayMicroseconds(50);

    digitalWrite (SS, LOW);
    SPI.transfer(0x01 << 3);
    byte num41 = func_produce(7, 2, 0);
    SPI.transfer(num41);
    byte num42 = func_produce(7, 7, 5);
    SPI.transfer(num42);
    digitalWrite (SS, HIGH);
    delayMicroseconds(50);
}

void changeMode() {
    clearCube();
    loading = true;
    timer = 0;
    randomSeed(millis());
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);
    delay(500);
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, HIGH);

    switch (currentEffect) {
        case 1:
            lcd.setCursor(0,1);
            lcd.print("<     RAIN     >");
            modeTimer = RAIN_TIME;
            break;
        case 2:
            lcd.setCursor(0,1);
            lcd.print("<    PLANE     >");
            modeTimer = PLANE_BOING_TIME;
            break;
        case 3:
            lcd.setCursor(0,1);
            lcd.print("<    VOXELS    >");
            modeTimer = SEND_VOXELS_TIME;
            break;
        case 4:
            lcd.setCursor(0,1);
            lcd.print("<     WOOP     >");
            modeTimer = WOOP_WOOP_TIME;
            break;
        case 5:
            lcd.setCursor(0,1);
            lcd.print("<     JUMP     >");
            modeTimer = CUBE_JUMP_TIME;
            break;
        case 6:
            lcd.setCursor(0,1);
            lcd.print("<      LIT     >");
            modeTimer = CLOCK_TIME;
            break;
        case 7:
            lcd.setCursor(0,1);
            lcd.print("<     SIN 1    >");
            modeTimer = RAIN_TIME;
            break;
        case 8:
            lcd.setCursor(0,1);
            lcd.print("<     SIN 2    >");
            modeTimer = RAIN_TIME;
            break;
        case 9:
            lcd.setCursor(0,1);
            lcd.print("<    WALKING   >");
            modeTimer = WALKING_TIME;
            break;
        case 0:
            lcd.setCursor(0,1);
            lcd.print("<     GLOW     >");
            modeTimer = GLOW_TIME;
            break;

    }
}

void checkUp()
{
    up_buttonState = digitalRead(Up_buttonPin);
    if (up_buttonState != up_lastButtonState) {
        if (up_buttonState == LOW) {
            bPress = true;
            buttonPushCounter++;
        } 
        delay(50);
    }
    up_lastButtonState = up_buttonState;
}

void checkDown()
{
    down_buttonState = digitalRead(Down_buttonPin);
    if (down_buttonState != down_lastButtonState) {
        if (down_buttonState == LOW) {
            bPress = true;
            if (buttonPushCounter - 1 < 0){
                buttonPushCounter = 9;
            } else {
                buttonPushCounter--;
            }
        } else {

        }
        delay(50);
    }
    down_lastButtonState = down_buttonState;

}

void walkingCube() {
    if (loading) {
        clearCube();
        loading = false;
        for (byte i = 0; i < 3; i++) {
            coord[i] = 300;
            vector[i] = random(3, 8) * 15;
        }
    }
    timer++;
    if (timer > modeTimer) {
        timer = 0;
        clearCube();
        for (byte i = 0; i < 3; i++) {
            coord[i] += vector[i];
            if (coord[i] < 1) {
                coord[i] = 1;
                vector[i] = -vector[i];
                vector[i] += random(0, 6) - 3;
            }
            if (coord[i] > 700 - 100) {
                coord[i] = 700 - 100;
                vector[i] = -vector[i];
                vector[i] += random(0, 6) - 3;
            }
        }
        int8_t thisX = coord[0] / 100;
        int8_t thisY = coord[1] / 100;
        int8_t thisZ = coord[2] / 100;

        setVoxel(thisX, thisY, thisZ);
        setVoxel(thisX + 1, thisY, thisZ);
        setVoxel(thisX, thisY + 1, thisZ);
        setVoxel(thisX, thisY, thisZ + 1);
        setVoxel(thisX + 1, thisY + 1, thisZ);
        setVoxel(thisX, thisY + 1, thisZ + 1);
        setVoxel(thisX + 1, thisY, thisZ + 1);
        setVoxel(thisX + 1, thisY + 1, thisZ + 1);
    }
}

void sinusFill() {
    if (loading) {
        clearCube();
        loading = false;
    }
    timer++;
    if (timer > modeTimer) {
        timer = 0;
        clearCube();
        if (++pos > 10) pos = 0;
        for (uint8_t i = 0; i < 8; i++) {
            for (uint8_t j = 0; j < 8; j++) {
                int8_t sinZ = 4 + ((float)sin((float)(i + pos) / 2) * 2);
                for (uint8_t y = 0; y < sinZ; y++) {
                    setVoxel(i, y, j);
                }
            }
        }
    }
}

void sinusThin() {
    if (loading) {
        clearCube();
        loading = false;
    }
    timer++;
    if (timer > modeTimer) {
        timer = 0;
        clearCube();
        if (++pos > 10) pos = 0;
        for (uint8_t i = 0; i < 8; i++) {
            for (uint8_t j = 0; j < 8; j++) {
                int8_t sinZ = 4 + ((float)sin((float)(i + pos) / 2) * 2);
                setVoxel(i, sinZ, j);
            }
        }
    }
}

void rain() {
    if (loading) {
        clearCube();
        loading = false;
    }
    timer++;
    if (timer > modeTimer) {
        timer = 0;
        shift(NEG_Y);
        uint8_t numDrops = random(0, 5);
        for (uint8_t i = 0; i < numDrops; i++) {
            setVoxel(random(0, 8), 7, random(0, 8));
        }
    }
}

uint8_t planePosition = 0;
uint8_t planeDirection = 0;
bool looped = false;

void planeBoing() {
    if (loading) {
        clearCube();
        uint8_t axis = random(0, 3);
        planePosition = random(0, 2) * 7;
        setPlane(axis, planePosition);
        if (axis == XAXIS) {
            if (planePosition == 0) {
                planeDirection = POS_X;
            } else {
                planeDirection = NEG_X;
            }
        } else if (axis == YAXIS) {
            if (planePosition == 0) {
                planeDirection = POS_Y;
            } else {
                planeDirection = NEG_Y;
            }
        } else if (axis == ZAXIS) {
            if (planePosition == 0) {
                planeDirection = POS_Z;
            } else {
                planeDirection = NEG_Z;
            }
        }
        timer = 0;
        looped = false;
        loading = false;
    }

    timer++;
    if (timer > modeTimer) {
        timer = 0;
        shift(planeDirection);
        if (planeDirection % 2 == 0) {
            planePosition++;
            if (planePosition == 7) {
                if (looped) {
                    loading = true;
                } else {
                    planeDirection++;
                    looped = true;
                }
            }
        } else {
            planePosition--;
            if (planePosition == 0) {
                if (looped) {
                    loading = true;
                } else {
                    planeDirection--;
                    looped = true;
                }
            }
        }
    }
}

uint8_t selX = 0;
uint8_t selY = 0;
uint8_t selZ = 0;
uint8_t sendDirection = 0;
bool sending = false;

void sendVoxels() {
    if (loading) {
        clearCube();
        for (uint8_t x = 0; x < 8; x++) {
            for (uint8_t z = 0; z < 8; z++) {
                setVoxel(x, random(0, 2) * 7, z);
            }
        }
        loading = false;
    }

    timer++;
    if (timer > modeTimer) {
        timer = 0;
        if (!sending) {
            selX = random(0, 8);
            selZ = random(0, 8);
            if (getVoxel(selX, 0, selZ)) {
                selY = 0;
                sendDirection = POS_Y;
            } else if (getVoxel(selX, 7, selZ)) {
                selY = 7;
                sendDirection = NEG_Y;
            }
            sending = true;
        } else {
            if (sendDirection == POS_Y) {
                selY++;
                setVoxel(selX, selY, selZ);
                clearVoxel(selX, selY - 1, selZ);
                if (selY == 7) {
                    sending = false;
                }
            } else {
                selY--;
                setVoxel(selX, selY, selZ);
                clearVoxel(selX, selY + 1, selZ);
                if (selY == 0) {
                    sending = false;
                }
            }
        }
    }
}

uint8_t cubeSize = 0;
bool cubeExpanding = true;

void woopWoop() {
    if (loading) {
        clearCube();
        cubeSize = 2;
        cubeExpanding = true;
        loading = false;
    }

    timer++;
    if (timer > modeTimer) {
        timer = 0;
        if (cubeExpanding) {
            cubeSize += 2;
            if (cubeSize == 8) {
                cubeExpanding = false;
            }
        } else {
            cubeSize -= 2;
            if (cubeSize == 2) {
                cubeExpanding = true;
            }
        }
        clearCube();
        drawCube(4 - cubeSize / 2, 4 - cubeSize / 2, 4 - cubeSize / 2, cubeSize);
    }
}

uint8_t xPos;
uint8_t yPos;
uint8_t zPos;

void cubeJump() {
    if (loading) {
        clearCube();
        xPos = random(0, 2) * 7;
        yPos = random(0, 2) * 7;
        zPos = random(0, 2) * 7;
        cubeSize = 8;
        cubeExpanding = false;
        loading = false;
    }

    timer++;
    if (timer > modeTimer) {
        timer = 0;
        clearCube();
        if (xPos == 0 && yPos == 0 && zPos == 0) {
            drawCube(xPos, yPos, zPos, cubeSize);
        } else if (xPos == 7 && yPos == 7 && zPos == 7) {
            drawCube(xPos + 1 - cubeSize, yPos + 1 - cubeSize, zPos + 1 - cubeSize, cubeSize);
        } else if (xPos == 7 && yPos == 0 && zPos == 0) {
            drawCube(xPos + 1 - cubeSize, yPos, zPos, cubeSize);
        } else if (xPos == 0 && yPos == 7 && zPos == 0) {
            drawCube(xPos, yPos + 1 - cubeSize, zPos, cubeSize);
        } else if (xPos == 0 && yPos == 0 && zPos == 7) {
            drawCube(xPos, yPos, zPos + 1 - cubeSize, cubeSize);
        } else if (xPos == 7 && yPos == 7 && zPos == 0) {
            drawCube(xPos + 1 - cubeSize, yPos + 1 - cubeSize, zPos, cubeSize);
        } else if (xPos == 0 && yPos == 7 && zPos == 7) {
            drawCube(xPos, yPos + 1 - cubeSize, zPos + 1 - cubeSize, cubeSize);
        } else if (xPos == 7 && yPos == 0 && zPos == 7) {
            drawCube(xPos + 1 - cubeSize, yPos, zPos + 1 - cubeSize, cubeSize);
        }
        if (cubeExpanding) {
            cubeSize++;
            if (cubeSize == 8) {
                cubeExpanding = false;
                xPos = random(0, 2) * 7;
                yPos = random(0, 2) * 7;
                zPos = random(0, 2) * 7;
            }
        } else {
            cubeSize--;
            if (cubeSize == 1) {
                cubeExpanding = true;
            }
        }
    }
}

bool glowing;
uint16_t glowCount = 0;

void glow() {
    if (loading) {
        clearCube();
        glowCount = 0;
        glowing = true;
        loading = false;
    }

    timer++;
    if (timer > modeTimer) {
        timer = 0;
        if (glowing) {
            if (glowCount < 448) {
                do {
                    selX = random(0, 8);
                    selY = random(0, 8);
                    selZ = random(0, 8);
                } while (getVoxel(selX, selY, selZ));
                setVoxel(selX, selY, selZ);
                glowCount++;
            } else if (glowCount < 512) {
                lightCube();
                glowCount++;
            } else {
                glowing = false;
                glowCount = 0;
            }
        } else {
            if (glowCount < 448) {
                do {
                    selX = random(0, 8);
                    selY = random(0, 8);
                    selZ = random(0, 8);
                } while (!getVoxel(selX, selY, selZ));
                clearVoxel(selX, selY, selZ);
                glowCount++;
            } else {
                clearCube();
                glowing = true;
                glowCount = 0;
            }
        }
    }
}


void lit() {
    if (loading) {
        clearCube();
        for (uint8_t i = 0; i < 8; i++) {
            for (uint8_t j = 0; j < 8; j++) {
                cube[i][j] = 0xFF;
            }
        }
        loading = false;
    }
}

void setVoxel(uint8_t x, uint8_t y, uint8_t z) {
    cube[7 - y][7 - z] |= (0x01 << x);
}

void clearVoxel(uint8_t x, uint8_t y, uint8_t z) {
    cube[7 - y][7 - z] ^= (0x01 << x);
}

bool getVoxel(uint8_t x, uint8_t y, uint8_t z) {
    return (cube[7 - y][7 - z] & (0x01 << x)) == (0x01 << x);
}

void setPlane(uint8_t axis, uint8_t i) {
    for (uint8_t j = 0; j < 8; j++) {
        for (uint8_t k = 0; k < 8; k++) {
            if (axis == XAXIS) {
                setVoxel(i, j, k);
            } else if (axis == YAXIS) {
                setVoxel(j, i, k);
            } else if (axis == ZAXIS) {
                setVoxel(j, k, i);
            }
        }
    }
}

void shift(uint8_t dir) {

    if (dir == POS_X) {
        for (uint8_t y = 0; y < 8; y++) {
            for (uint8_t z = 0; z < 8; z++) {
                cube[y][z] = cube[y][z] << 1;
            }
        }
    } else if (dir == NEG_X) {
        for (uint8_t y = 0; y < 8; y++) {
            for (uint8_t z = 0; z < 8; z++) {
                cube[y][z] = cube[y][z] >> 1;
            }
        }
    } else if (dir == POS_Y) {
        for (uint8_t y = 1; y < 8; y++) {
            for (uint8_t z = 0; z < 8; z++) {
                cube[y - 1][z] = cube[y][z];
            }
        }
        for (uint8_t i = 0; i < 8; i++) {
            cube[7][i] = 0;
        }
    } else if (dir == NEG_Y) {
        for (uint8_t y = 7; y > 0; y--) {
            for (uint8_t z = 0; z < 8; z++) {
                cube[y][z] = cube[y - 1][z];
            }
        }
        for (uint8_t i = 0; i < 8; i++) {
            cube[0][i] = 0;
        }
    } else if (dir == POS_Z) {
        for (uint8_t y = 0; y < 8; y++) {
            for (uint8_t z = 1; z < 8; z++) {
                cube[y][z - 1] = cube[y][z];
            }
        }
        for (uint8_t i = 0; i < 8; i++) {
            cube[i][7] = 0;
        }
    } else if (dir == NEG_Z) {
        for (uint8_t y = 0; y < 8; y++) {
            for (uint8_t z = 7; z > 0; z--) {
                cube[y][z] = cube[y][z - 1];
            }
        }
        for (uint8_t i = 0; i < 8; i++) {
            cube[i][0] = 0;
        }
    }
}

void drawCube(uint8_t x, uint8_t y, uint8_t z, uint8_t s) {
    for (uint8_t i = 0; i < s; i++) {
        setVoxel(x, y + i, z);
        setVoxel(x + i, y, z);
        setVoxel(x, y, z + i);
        setVoxel(x + s - 1, y + i, z + s - 1);
        setVoxel(x + i, y + s - 1, z + s - 1);
        setVoxel(x + s - 1, y + s - 1, z + i);
        setVoxel(x + s - 1, y + i, z);
        setVoxel(x, y + i, z + s - 1);
        setVoxel(x + i, y + s - 1, z);
        setVoxel(x + i, y, z + s - 1);
        setVoxel(x + s - 1, y, z + i);
        setVoxel(x, y + s - 1, z + i);
    }
}

void lightCube() {
    for (uint8_t i = 0; i < 8; i++) {
        for (uint8_t j = 0; j < 8; j++) {
            cube[i][j] = 0xFF;
        }
    }
}

void clearCube() {
    for (uint8_t i = 0; i < 8; i++) {
        for (uint8_t j = 0; j < 8; j++) {
            cube[i][j] = 0;
        }
    }
}
