/* Bluetooth系 */
#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;

/*M5Stack系*/
#include <M5Stack.h>
#include "utility/MPU9250.h"

MPU9250 IMU;


void setup() {
  M5.begin();
  Wire.begin();
  delay(1000);

  Serial.begin(115200);
  SerialBT.begin("gengen_m5stack"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");

  IMU.calibrateMPU9250(IMU.gyroBias, IMU.accelBias);
  IMU.initMPU9250();
  IMU.initAK8963(IMU.magCalibration);
}

void loop() {
  // If intPin goes high, all data registers have new data
  // On interrupt, check if data ready interrupt
  if (IMU.readByte(MPU9250_ADDRESS, INT_STATUS) & 0x01)
  {
    IMU.readAccelData(IMU.accelCount);
    IMU.getAres();

    IMU.ax = (float)IMU.accelCount[0] * IMU.aRes; // - accelBias[0];
    IMU.ay = (float)IMU.accelCount[1] * IMU.aRes; // - accelBias[1];
    IMU.az = (float)IMU.accelCount[2] * IMU.aRes; // - accelBias[2];

    IMU.readGyroData(IMU.gyroCount);  // Read the x/y/z adc values
    IMU.getGres();

    // Calculate the gyro value into actual degrees per second
    // This depends on scale being set
    IMU.gx = (float)IMU.gyroCount[0] * IMU.gRes;
    IMU.gy = (float)IMU.gyroCount[1] * IMU.gRes;
    IMU.gz = (float)IMU.gyroCount[2] * IMU.gRes;

    IMU.readMagData(IMU.magCount);  // Read the x/y/z adc values
    IMU.getMres();
    // User environmental x-axis correction in milliGauss, should be
    // automatically calculated
    //IMU.magbias[0] = +470.;
    // User environmental x-axis correction in milliGauss TODO axis??
    //IMU.magbias[1] = +120.;
    // User environmental x-axis correction in milliGauss
    //IMU.magbias[2] = +125.;

    // Calculate the magnetometer values in milliGauss
    // Include factory calibration per data sheet and user environmental
    // corrections
    // Get actual magnetometer value, this depends on scale being set
    IMU.mx = (float)IMU.magCount[0] * IMU.mRes * IMU.magCalibration[0] -
             IMU.magbias[0];
    IMU.my = (float)IMU.magCount[1] * IMU.mRes * IMU.magCalibration[1] -
             IMU.magbias[1];
    IMU.mz = (float)IMU.magCount[2] * IMU.mRes * IMU.magCalibration[2] -
             IMU.magbias[2];

    IMU.tempCount = IMU.readTempData();  // Read the adc values
    // Temperature in degrees Centigrade
    IMU.temperature = ((float) IMU.tempCount) / 333.87 + 21.0;

    int x=64+10;
    int y=128+20;
    int z=192+30;

    // M5StackのLCDへの表示
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(GREEN , BLACK);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(0, 0); M5.Lcd.print("MPU9250/AK8963");
    M5.Lcd.setCursor(0, 32); M5.Lcd.print("x");
    M5.Lcd.setCursor(x, 32); M5.Lcd.print("y");
    M5.Lcd.setCursor(y, 32); M5.Lcd.print("z");

    M5.Lcd.setTextColor(YELLOW , BLACK);
    M5.Lcd.setCursor(0, 48 * 2); M5.Lcd.print((int)(1000 * IMU.ax));
    M5.Lcd.setCursor(x, 48 * 2); M5.Lcd.print((int)(1000 * IMU.ay));
    M5.Lcd.setCursor(y, 48 * 2); M5.Lcd.print((int)(1000 * IMU.az));
    M5.Lcd.setCursor(z, 48 * 2); M5.Lcd.print("mg");

    M5.Lcd.setCursor(0, 64 * 2); M5.Lcd.print((int)(IMU.gx));
    M5.Lcd.setCursor(x, 64 * 2); M5.Lcd.print((int)(IMU.gy));
    M5.Lcd.setCursor(y, 64 * 2); M5.Lcd.print((int)(IMU.gz));
    M5.Lcd.setCursor(z, 64 * 2); M5.Lcd.print("o/s");

    M5.Lcd.setCursor(0, 80 * 2); M5.Lcd.print((int)(IMU.mx));
    M5.Lcd.setCursor(x, 80 * 2); M5.Lcd.print((int)(IMU.my));
    M5.Lcd.setCursor(y, 80 * 2); M5.Lcd.print((int)(IMU.mz));
    M5.Lcd.setCursor(z, 80 * 2); M5.Lcd.print("mG");

    M5.Lcd.setCursor(0,  96 * 2); M5.Lcd.print("Gyro Temperature ");
    M5.Lcd.setCursor(z,  96 * 2); M5.Lcd.print(IMU.temperature, 1);
    M5.Lcd.print(" C");

    // 各軸の加速度
    int ax = (int)(1000 * IMU.ax);
    int ay = (int)(1000 * IMU.ay);
    int az = (int)(1000 * IMU.az);

    // シリアル通信
    Serial.print(ax);
    Serial.print(' ');
    Serial.print(ay);
    Serial.print(' ');
    Serial.print(az);
    Serial.print('\n');

    // Bluetooth通信
    SerialBT.print(ax);
    SerialBT.print(' ');
    SerialBT.print(ay);
    SerialBT.print(' ');
    SerialBT.print(az);
    SerialBT.print('\n');
    delay(100);
  }
}
