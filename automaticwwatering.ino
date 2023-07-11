


#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#define SOIL_MOIST_1_PIN A1
#define DHTTYPE DHT11 
#define PUMP 5
#define BUTTON A3
#define DHTTYPE DHT11
#define DHTPIN 2 
LiquidCrystal_I2C lcd(0x3F,16,2); 

DHT dht(DHTPIN, DHTTYPE);

int humDHT;
int tempDHT;
int soilMoist;

// Biến lưu trạng thái bơm
boolean pumpWaterStatus = 0;
int timePumpOn = 20; // Thời gian bật bơm nước
// Biến cho timer
long sampleTimingSeconds = 50; // ==> Thời gian đọc cảm biến (s)
long startTiming = 0;
long elapsedTime = 0;

void setup() 
{
  pinMode(PUMP, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP); 
   aplyCmd();
  // Khởi tạo cổng serial baud 115200
  Serial.begin(115200);
  lcd.begin(16, 2);

  // Bắt đầu đọc dữ liệu
  readSensors(); // Khởi tạo đọc cảm biến
  startTiming = millis(); // Bắt đầu đếm thời gian
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("Vui long cho ...");
}

void loop() {
  // Khởi tạo timer
  elapsedTime = millis() - startTiming;
  readLocalCmd();
  
  if (elapsedTime > (sampleTimingSeconds * 1000))
  {
    readSensors();
    printData();
    showDataLCD();
    autoControlPlantation();
    startTiming = millis();
  }
}

int getSoilMoist()
{
  int i = 0;
  int anaValue = 0;
  for (i = 0; i < 10; i++)  //
  {
    anaValue += analogRead(SOIL_MOIST_1_PIN); //Đọc giá trị cảm biến độ ẩm đất
    delay(50);   // Đợi đọc giá trị ADC
  }
  anaValue = anaValue / (i);
  anaValue = map(anaValue, 1023, 0, 0, 100); //Ít nước:0%  ==> Nhiều nước 100%
  return anaValue;
}
void readSensors(void)
{
  tempDHT = dht.readTemperature();   //Đọc nhiệt độ DHT22
  humDHT = dht.readHumidity();       //Đọc độ ẩm DHT22
  soilMoist = getSoilMoist();        //Đọc cảm biến độ ẩm đất
}
void showDataLCD(void)
{
  lcd.setCursor(0, 1);
  lcd.print(" DO.AM% = ");
  lcd.print(humDHT);
  lcd.println("  % " );

  lcd.setCursor(1, 0);
  lcd.print(" NH.DO = ");
  lcd.print(tempDHT);
  lcd.println(" *C ");
  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print(" AM.DAT% = ");
  lcd.print(soilMoist);
  lcd.println("  %   " );


  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("  BOM.NC = ");
  lcd.print(pumpWaterStatus);
  lcd.println("      " );

  delay(1000);
}
void printData(void)
{
  // IN thông tin ra màn hình
  Serial.print("Do am: ");
  Serial.print(humDHT);
  Serial.print(" %\t");
  Serial.print("Nhiet do: ");
  Serial.print(tempDHT);
  Serial.print(" *C\t");
  Serial.print("Do am dat: ");
  Serial.print(soilMoist);
  Serial.println(" %");
}
/***************************************************
  Hàm bật bơm nước
****************************************************/
void turnPumpOn()
{
  digitalWrite(PUMP, HIGH);
  pumpWaterStatus = 1;
  showDataLCD();
  delay (timePumpOn * 1000);
  digitalWrite(PUMP, LOW);
  pumpWaterStatus = 0;
  showDataLCD();
}
/****************************************************************
  Hàm đọc trạng thái bơm và kiểm tra nút nhấn
  (Nút nhấn mặc định là mức "CAO"):
****************************************************************/
void readLocalCmd()
{
  int digiValue = debounce(BUTTON);
  if (!digiValue)
  {
    pumpWaterStatus = !pumpWaterStatus;
    showDataLCD();
    aplyCmd();
  }
}
/***************************************************
  Hàm kiểm tra trạng thái phím bấm
****************************************************/
boolean debounce(int pin)
{
  boolean state;
  boolean previousState;
  const int debounceDelay = 60;

  previousState = digitalRead(pin);
  for (int counter = 0; counter < debounceDelay; counter++)
  {
    delay(1);
    state = digitalRead(pin);
    if (state != previousState)
    {
      counter = 0;
      previousState = state;
    }
  }
  return state;
}
/***************************************************
   Chức năng tự động tưới tiêu
****************************************************/
void autoControlPlantation()
{
  //-------- PHUN NƯỚC ------//
  if (soilMoist < 40 || tempDHT > 35)
  {
    turnPumpOn();
  }
}
void aplyCmd()
{
  if (pumpWaterStatus == 1) digitalWrite(PUMP, HIGH);
  if (pumpWaterStatus == 0) digitalWrite(PUMP, LOW);
}
