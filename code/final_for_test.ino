#include <camera_VC0706.h>
#include <SD.h>
#include <SoftwareSerial.h>
#include <Ultrasonic.h>

//Set commections for camera module
#define chipSelect 10
#if ARDUINO >= 100
SoftwareSerial cameraconnection = SoftwareSerial(2, 3); // set the tx and rx pin
#else
NewSoftSerial cameraconnection = NewSoftSerial(2, 3);
#endif
camera_VC0706 cam = camera_VC0706(&cameraconnection);

//Set connections for ultrasonic sensor
int trigpin = 7;//appoint trigger pin
int echopin = 6;//appoint echo pin
Ultrasonic ultrasonic(trigpin,echopin);

void setup() {
  Serial.begin(9600);
  
//Set the pins for SD card module
#if !defined(SOFTWARE_SPI)
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  if(chipSelect != 53) pinMode(53, OUTPUT); // SS on Mega
#else
  if(chipSelect != 10) pinMode(10, OUTPUT); // SS on Uno, etc.
#endif
#endif

  //Testing on devices:
  Serial.println("Initializing devices:");
  if (!init_camera()) {return;}
  if (!init_SDcard()) {return;}
  // 选择合适的图片尺寸 640x480, 320x240 or 160x120
  // 图片越大，传输速度越慢
  cam.setImageSize(VC0706_640x480);
  //cam.setImageSize(VC0706_320x240);
  //cam.setImageSize(VC0706_160x120);
  Serial.println("Initializing success and finished!");
}


boolean toke_photo = true;
void loop() {
  float distance = getDistance();
  Serial.print("get distance in centimeter: ");
  Serial.println(distance);
  if (distance < 10 && getDistance() < 10) {
    if (!toke_photo) {
      toke_photo = true;
      Serial.println("----Taking picture-----");

      if (!cam.takePicture()) {Serial.println("Failed to snap");}
      char filename[11];
      strcpy(filename, "IMG000.JPG");
      for (int i = 0; i < 1000; i++) {
        filename[3] = '0' + i/100;
        filename[4] = '0' + (i/10)%10;
        filename[5] = '0' + i%10;
        //Skip if the name is exist, not opning existing file.
        if (!SD.exists(filename)) {break;}
      }
      File imgFile = SD.open(filename, FILE_WRITE);
      uint16_t jpglen = cam.frameLength();
      //Writing image
      while (jpglen > 0) {
        uint8_t *buffer;
        uint8_t bytesToRead = min(16, jpglen);//Set the bytes to read at once. from 16 - 64 bytes
        buffer = cam.readPicture(bytesToRead);
        imgFile.write(buffer, bytesToRead);
        jpglen -= bytesToRead;
      }
      imgFile.close();
      
      Serial.print("finished: ");
      Serial.println(filename);
      cam.resumeVideo();
    }
  } else {toke_photo = false;}
}

float getDistance() {
  //Acquiring distance sensor input in centimeter
  long microsec = ultrasonic.timing();
  return ultrasonic.CalcDistance(microsec,Ultrasonic::CM);//this result unit is centimeter
}

boolean init_camera() {
  //Test on Camera:
  Serial.println("Initializing camera:");
  if (cam.begin()){
    Serial.println("Camera found:");
  }else{
    Serial.println("No camera found!\n Initializing failed");
    return false;
  }
  char *reply = cam.getVersion();
  if (reply == 0) {
    Serial.print("Failed to get version");
    return false;
  } else {
    Serial.print(reply);
  }
  return true;
}

boolean init_SDcard() {
  //Test on SD card module:
  Serial.println("Initializing SD card Module:");
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    return false;
  }
  return true;
}

