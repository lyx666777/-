#include "BluetoothSerial.h"

BluetoothSerial SerialBT;
//电机驱动引脚，根据实际情况进行修改
int motor1Pin1 = 18; // Motor 1 control pin 1
int motor1Pin2 = 19; // Motor 1 control pin 2
int motor2Pin1 = 26; // Motor 2 control pin 1
int motor2Pin2 = 27; // Motor 2 control pin 2
//电机调速引脚，根据实际修改
int motorspeedPin1 = 2;//Motor 1 speed pin
int motorspeedPin2 = 4;//Motor 2 speed pin

//电机初始速度，0最小，255最大
int motorSpeed1 = 140; // Motor speed (0-255)
int motorSpeed2 = 155; // Motor speed (0-255)
  
//这个必须测试，否则小车无法移动到指定点
int waitTimes1 = 510; //延时毫秒数，延时时长需要测试，为小车移动一格距离的所需时间，单位毫秒
int waitTimes2 = 370;

bool isover = true;
int currentX = 0;
int currentY = 0;
struct Coordinate {
    int x;
    int y;
    float distance; // Distance from origin
};

int four[][2] = { {3,0},{3,2},{1,2},{1,5},{2,5},{2,3},{3,3},{3,5 }, { 4, 5 }, { 4, 3 }, { 5, 3 }, { 5, 2 }, { 4, 2 }, { 4, 0 }, { 0, 0 } };
int three[][2] = { {1,0},{1,1},{3,1},{3,2},{1,2},{1,3},{3,3},{3,4},{1,4},{1,5},{4,5},{4,0},{0,0} };
int wang[][2] = { {0,1},{2,1},{2,2},{1,2},{1,3},{2,3},{2,4},{0,4},{0,5},{5,5},{5,4},{3,4},{3,3},{4,3},{4,2},{3,2},{3,1},{5,1},{5,0},{0,0} };
const int MAX_COORDINATES = 10; // Maximum number of coordinates
Coordinate coordinates[MAX_COORDINATES];
int numCoordinates = 0;

void setup() {
    Serial.begin(9600);
    SerialBT.begin("ESP32BT");
    pinMode(motor1Pin1, OUTPUT);
    pinMode(motor1Pin2, OUTPUT);
    pinMode(motor2Pin1, OUTPUT);
    pinMode(motor2Pin2, OUTPUT);
    pinMode(motorspeedPin1, OUTPUT);
    pinMode(motorspeedPin2, OUTPUT);
    analogWrite(motorspeedPin1, motorSpeed1);
    analogWrite(motorspeedPin2, motorSpeed2);
    stopMotors();
}

void loop() {
    if (SerialBT.available() > 0) {
        isover = false;
        numCoordinates = 0;
        String input = SerialBT.readStringUntil(':');

        if (!isover)
        {
            if (input == "4")//画4
            {
                int les = sizeof(four) / sizeof(four[0]);
                drawfont(four, les);

            }
            else if (input == "3")//画3
            {
                int les = sizeof(three) / sizeof(three[0]);
                drawfont(three, les);
            }
            else if (input == "5")//画王
            {
                int les = sizeof(wang) / sizeof(wang[0]);
                drawfont(wang, les);
            }
            else {

                while (input.length() > 0) {
                    processCoordinate(input);
                    input = SerialBT.readStringUntil(':');
                }
                if (numCoordinates > 1)//多个坐标
                {
                    sortCoordinates();
                    targetMove();
                    CarMove(0, 0, true);//返回原点
                    isover = true;
                    Serial.println("stop");
                }
                else {  //单个坐标
                    for (int i = 0; i < 3; i++)
                    {
                        CarMove(coordinates[0].x, coordinates[0].y, true);
                        Car_back_Move(0, 0, true);
                    }
                    isover = true;
                    Serial.println("stop");
                }
            }


        }

    }
}


void drawfont(int(*a)[2], int lengths) {

    // int lengths = sizeof(a) / sizeof(a[0]);
    // Serial.println(lengths);
    for (int i = 0; i < lengths; i++)
    {
        CarMove(a[i][0], a[i][1], false);
    }
    isover = true;
    Serial.println("stop");
}
void processCoordinate(String coordinate) {
    int commaIndex = coordinate.indexOf(',');
    if (commaIndex != -1 && numCoordinates < MAX_COORDINATES) {
        String xStr = coordinate.substring(0, commaIndex);
        String yStr = coordinate.substring(commaIndex + 1);

        int x = xStr.toInt();
        int y = yStr.toInt();

        float distance = sqrt(x * x + y * y);

        coordinates[numCoordinates].x = x;
        coordinates[numCoordinates].y = y;
        coordinates[numCoordinates].distance = distance;

        numCoordinates++;
    }
}

void sortCoordinates() {
    // Bubble sort (you can replace this with a more efficient sorting algorithm)
    for (int i = 0; i < numCoordinates - 1; i++) {
        for (int j = 0; j < numCoordinates - i - 1; j++) {
            if (coordinates[j].distance > coordinates[j + 1].distance) {
                // Swap coordinates
                Coordinate temp = coordinates[j];
                coordinates[j] = coordinates[j + 1];
                coordinates[j + 1] = temp;
            }
        }
    }

    // // Print sorted coordinates
    // Serial.println("Sorted Coordinates:");
    // for (int i = 0; i < numCoordinates; i++) {
    //     Serial.print("(");
    //     Serial.print(coordinates[i].x);
    //     Serial.print(", ");
    //     Serial.print(coordinates[i].y);
    //     Serial.print(") - Distance: ");
    //     Serial.println(coordinates[i].distance);
    // }
}
void Car_back_Move(int x, int y, bool stop) {

    int delaytimes_x = abs(currentX - x);
    int delaytimes_y = abs(currentY - y);

    if (currentY - y < 0)
    {
        moveForward();
        Serial.print("前*");
        Serial.println(delaytimes_y);
        delay(delaytimes_y * waitTimes1);
        stopMotors();
        delay(1000);
    }
    else {
        moveBackward();
        Serial.print("后*");
        Serial.println(delaytimes_y);
        delay(delaytimes_y * waitTimes1);
        stopMotors();
        delay(1000);
    }

    if (currentX - x < 0)
    {
        moveRight();
        Serial.print("右*");
        Serial.println(delaytimes_x);
        delay(delaytimes_x * waitTimes2);
        stopMotors();
        delay(1000);
    }
    else {
        moveLeft();
        Serial.print("左*");
        Serial.println(delaytimes_x);
        delay(delaytimes_x * waitTimes2);
        stopMotors();
        delay(1000);
    }
    currentX = x;
    currentY = y;
    if (stop)
    {
        stopMotors();
        delay(2000);
    }

}


void CarMove(int x, int y, bool stop) {
    int delaytimes_x = abs(currentX - x);
    int delaytimes_y = abs(currentY - y);
    if (currentX - x < 0)
    {
        moveRight();
        Serial.print("右*");
        Serial.println(delaytimes_x);
        delay(delaytimes_x * waitTimes2);
        stopMotors();
        delay(1000);
    }
    else {
        moveLeft();
        Serial.print("左*");
        Serial.println(delaytimes_x);
        delay(delaytimes_x * waitTimes2);
        stopMotors();
        delay(1000);
    }

    if (currentY - y < 0)
    {
        moveForward();
        // Serial.print("前");
        Serial.print("前*");
        Serial.println(delaytimes_y);
        delay(delaytimes_y * waitTimes1);
        stopMotors();
        delay(1000);
    }
    else {
        moveBackward();
        Serial.print("后*");
        Serial.println(delaytimes_y);
        delay(delaytimes_y * waitTimes1);
        stopMotors();
        delay(1000);
    }
    currentX = x;
    currentY = y;
    if (stop)
    {
          stopMotors();
        delay(2000);

    }

}


void targetMove() {
    for (int i = 0; i < numCoordinates; i++) {
        Serial.print("(");
        Serial.print(coordinates[i].x);
        Serial.print(", ");
        Serial.print(coordinates[i].y);
        Serial.print(") - Distance: ");
        Serial.println(coordinates[i].distance);
        CarMove(coordinates[i].x, coordinates[i].y, true);
    }
}
void moveForward() {
    digitalWrite(motor1Pin1, HIGH);
    digitalWrite(motor1Pin2, LOW);
    digitalWrite(motor2Pin1, HIGH);
    digitalWrite(motor2Pin2, LOW);
}

void moveBackward() {
    digitalWrite(motor1Pin1, LOW);
    digitalWrite(motor1Pin2, HIGH);
    digitalWrite(motor2Pin1, LOW);
    digitalWrite(motor2Pin2, HIGH);
}

void moveLeft() {
    digitalWrite(motor1Pin1, LOW);
    digitalWrite(motor1Pin2, HIGH);
    digitalWrite(motor2Pin1, HIGH);
    digitalWrite(motor2Pin2, LOW);
}

void moveRight() {
    digitalWrite(motor1Pin1, HIGH);
    digitalWrite(motor1Pin2, LOW);
    digitalWrite(motor2Pin1, LOW);
    digitalWrite(motor2Pin2, HIGH);
}

void stopMotors() {
    digitalWrite(motor1Pin1, LOW);
    digitalWrite(motor1Pin2, LOW);
    digitalWrite(motor2Pin1, LOW);
    digitalWrite(motor2Pin2, LOW);
}
