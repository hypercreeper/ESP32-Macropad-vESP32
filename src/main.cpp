#include <Arduino.h>
#include <TFT_eSPI.h>
#include <esp_now.h>
#include <SPI.h>
#include <XPT2046_Touchscreen.h>
#include "images.h"
#include <PNGdec.h>
#include <WiFi.h>
#include <Preferences.h>
#include <map>

#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33

TFT_eSPI tft = TFT_eSPI(); // Invoke custom library
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
PNG png;
Preferences pref;

#define MAX_IMAGE_WIDTH 50
// 68:b6:b3:21:8a:a8
uint8_t receiverAddress[] = {0x68, 0xB6, 0xB3, 0x21, 0x8A, 0xA8};
esp_now_peer_info_t peerInfo;
typedef struct button_press
{
  String buttonLabel;
} button_press;

typedef struct buttonss_send
{
  String buttons[12];
  int part = 0;
  int arrayLength;
} buttonss_send;
typedef struct buttons_images_send
{
  String btnImages[12];
  int part = 1;
  int arrayLength;
} buttons_images_send;
typedef struct buttons_borders_send
{
  String borderColors[12];
  int arrayLength;
  int part = 2;
} buttons_borders_send;

typedef struct buttons_colors_send
{
  String btnColors[12];
  int part = 3;
  int arrayLength;
} buttons_colors_send;
buttonss_send btnsend;
buttons_colors_send btnclrsend;
buttons_images_send btnimgsend;
buttons_borders_send btnbordsend;

typedef struct buttons_send
{
  String buttons[12];
  String btnColors[12];
  String borderColors[12];
  String btnImages[12];
  int arrayLength;
  int payloadpart;
} buttons_send;
struct Box
{
  int x, y, w, h;
  String text;
  uint16_t btnColor;
  uint16_t borderColor;
};
button_press recvbtnPrsd;
button_press btnPrsd;

buttons_send buttonsArray;
Box buttons[12];

int xPos[] = {0, (tft.width() / 3) * 1, (tft.width() / 3) * 2, 0, (tft.width() / 3) * 1, (tft.width() / 3) * 2, 0, (tft.width() / 3) * 1, (tft.width() / 3) * 2, 0, (tft.width() / 3) * 1, (tft.width() / 3) * 2};
int yPos[] = {0, 0, 0, (tft.height() / 4) * 1, (tft.height() / 4) * 1, (tft.height() / 4) * 1, (tft.height() / 4) * 2, (tft.height() / 4) * 2, (tft.height() / 4) * 2, (tft.height() / 4) * 3, (tft.height() / 4) * 3, (tft.height() / 4) * 3};
int xpos = 0, ypos = 0;
String listofbtns = "";

std::map<String, char *> images;

uint16_t getRGBFromString(String rgbstr)
{
  int arraySize = 3;             // Number of elements in the array
  String stringArray[arraySize]; // Declare the String array with a fixed size
  int index = 0;                 // Index for the String array

  // Split the inputString with the delimiter ","
  while (rgbstr.length() > 0 && index < arraySize)
  {
    int commaIndex = rgbstr.indexOf(",");
    if (commaIndex >= 0)
    {
      stringArray[index] = rgbstr.substring(0, commaIndex);
      rgbstr = rgbstr.substring(commaIndex + 1);
    }
    else
    {
      stringArray[index] = rgbstr;
      rgbstr = "";
    }
    index++;
  }
  return tft.color565(stringArray[0].toInt(), stringArray[1].toInt(), stringArray[2].toInt());
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void pngDraw(PNGDRAW *pDraw)
{
  uint16_t lineBuffer[MAX_IMAGE_WIDTH];        // Line buffer for rendering
  uint8_t maskBuffer[1 + MAX_IMAGE_WIDTH / 8]; // Mask buffer

  png.getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_BIG_ENDIAN, 0xffffffff);

  if (png.getAlphaMask(pDraw, maskBuffer, 255))
  {
    // Note: pushMaskedImage is for pushing to the TFT and will not work pushing into a sprite
    tft.pushMaskedImage(xpos, ypos + pDraw->y, pDraw->iWidth, 1, lineBuffer, maskBuffer);
  }
}
int getIndexFromName(String key) {
  for(int i = 0; i < 28; i++) {
    if(key == imageLabels[i]) {
      return i;
    }
  }
}
void redrawIcons(int i)
{
  Serial.println("Getting image from flash");
  Serial.println(buttonsArray.btnImages[i]);
  Serial.println(getIndexFromName(buttonsArray.btnImages[i]));
  // int16_t rc = png.openFLASH((uint8_t *)imageData(buttonsArray.btnImages[i]), sizeof(imageData(buttonsArray.btnImages[i])), pngDraw);
  // if (rc == PNG_SUCCESS)
  // {
  //   Serial.println("Successfully opened png file");
  //   xpos = xPos[i] + 2 + (tft.width() / 3) / 2 - png.getWidth() / 2;
  //   ypos = yPos[i] + 2 + (tft.height() / 4) / 2 - png.getHeight() / 2;
  //   tft.startWrite();
  //   uint32_t dt = millis();
  //   rc = png.decode(NULL, 0);
  //   tft.endWrite();
  //   Serial.print(millis() - dt);
  //   Serial.println("ms");
  //   tft.endWrite();
  // }
  // else {
  //   Serial.print("Error: ");
  //   Serial.println(rc);
  // }

  if (buttonsArray.btnImages[i] == "Copy")
  {
    int16_t rc = png.openFLASH((uint8_t *)copy, sizeof(copy), pngDraw);
    if (rc == PNG_SUCCESS)
    {
      Serial.println("Successfully opened png file");
      xpos = xPos[i] + 2 + (tft.width() / 3) / 2 - png.getWidth() / 2;
      ypos = yPos[i] + 2 + (tft.height() / 4) / 2 - png.getHeight() / 2;
      tft.startWrite();
      uint32_t dt = millis();
      rc = png.decode(NULL, 0);
      tft.endWrite();
      Serial.print(millis() - dt);
      Serial.println("ms");
      tft.endWrite();
    }
  }
  else if (buttonsArray.btnImages[i] == "Cut")
  {
    int16_t rc = png.openFLASH((uint8_t *)cut, sizeof(cut), pngDraw);

    if (rc == PNG_SUCCESS)
    {
      Serial.println("Successfully opened png file");
      xpos = xPos[i] + 2 + (tft.width() / 3) / 2 - png.getWidth() / 2;
      ypos = yPos[i] + 2 + (tft.height() / 4) / 2 - png.getHeight() / 2;
      tft.startWrite();
      uint32_t dt = millis();
      rc = png.decode(NULL, 0);
      tft.endWrite();
      Serial.print(millis() - dt);
      Serial.println("ms");
      tft.endWrite();
    }
  }
  else if (buttonsArray.btnImages[i] == "Paste")
  {
    int16_t rc = png.openFLASH((uint8_t *)paste, sizeof(paste), pngDraw);

    if (rc == PNG_SUCCESS)
    {
      Serial.println("Successfully opened png file");
      xpos = xPos[i] + 2 + (tft.width() / 3) / 2 - png.getWidth() / 2;
      ypos = yPos[i] + 2 + (tft.height() / 4) / 2 - png.getHeight() / 2;
      tft.startWrite();
      uint32_t dt = millis();
      rc = png.decode(NULL, 0);
      tft.endWrite();
      Serial.print(millis() - dt);
      Serial.println("ms");
      tft.endWrite();
    }
  }
  else if (buttonsArray.btnImages[i] == "Next")
  {
    int16_t rc = png.openFLASH((uint8_t *)next, sizeof(next), pngDraw);

    if (rc == PNG_SUCCESS)
    {
      Serial.println("Successfully opened png file");
      xpos = xPos[i] + 2 + (tft.width() / 3) / 2 - png.getWidth() / 2;
      ypos = yPos[i] + 2 + (tft.height() / 4) / 2 - png.getHeight() / 2;
      tft.startWrite();
      uint32_t dt = millis();
      rc = png.decode(NULL, 0);
      tft.endWrite();
      Serial.print(millis() - dt);
      Serial.println("ms");
      tft.endWrite();
    }
  }
  else if (buttonsArray.btnImages[i] == "Play/Pause")
  {
    int16_t rc = png.openFLASH((uint8_t *)playpause, sizeof(playpause), pngDraw);

    if (rc == PNG_SUCCESS)
    {
      Serial.println("Successfully opened png file");
      xpos = xPos[i] + 2 + (tft.width() / 3) / 2 - png.getWidth() / 2;
      ypos = yPos[i] + 2 + (tft.height() / 4) / 2 - png.getHeight() / 2;
      tft.startWrite();
      uint32_t dt = millis();
      rc = png.decode(NULL, 0);
      tft.endWrite();
      Serial.print(millis() - dt);
      Serial.println("ms");
      tft.endWrite();
    }
  }
  else if (buttonsArray.btnImages[i] == "Previous")
  {
    int16_t rc = png.openFLASH((uint8_t *)previous, sizeof(previous), pngDraw);

    if (rc == PNG_SUCCESS)
    {
      Serial.println("Successfully opened png file");
      xpos = xPos[i] + 2 + (tft.width() / 3) / 2 - png.getWidth() / 2;
      ypos = yPos[i] + 2 + (tft.height() / 4) / 2 - png.getHeight() / 2;
      tft.startWrite();
      uint32_t dt = millis();
      rc = png.decode(NULL, 0);
      tft.endWrite();
      Serial.print(millis() - dt);
      Serial.println("ms");
      tft.endWrite();
    }
  }
  else if (buttonsArray.btnImages[i] == "Up")
  {
    int16_t rc = png.openFLASH((uint8_t *)up, sizeof(up), pngDraw);

    if (rc == PNG_SUCCESS)
    {
      Serial.println("Successfully opened png file");
      xpos = xPos[i] + 2 + (tft.width() / 3) / 2 - png.getWidth() / 2;
      ypos = yPos[i] + 2 + (tft.height() / 4) / 2 - png.getHeight() / 2;
      tft.startWrite();
      uint32_t dt = millis();
      rc = png.decode(NULL, 0);
      tft.endWrite();
      Serial.print(millis() - dt);
      Serial.println("ms");
      tft.endWrite();
    }
  }
  else if (buttonsArray.btnImages[i] == "Down")
  {
    int16_t rc = png.openFLASH((uint8_t *)down, sizeof(down), pngDraw);

    if (rc == PNG_SUCCESS)
    {
      Serial.println("Successfully opened png file");
      xpos = xPos[i] + 2 + (tft.width() / 3) / 2 - png.getWidth() / 2;
      ypos = yPos[i] + 2 + (tft.height() / 4) / 2 - png.getHeight() / 2;
      tft.startWrite();
      uint32_t dt = millis();
      rc = png.decode(NULL, 0);
      tft.endWrite();
      Serial.print(millis() - dt);
      Serial.println("ms");
      tft.endWrite();
    }
  }
  else if (buttonsArray.btnImages[i] == "Left")
  {
    int16_t rc = png.openFLASH((uint8_t *)left, sizeof(left), pngDraw);

    if (rc == PNG_SUCCESS)
    {
      Serial.println("Successfully opened png file");
      xpos = xPos[i] + 2 + (tft.width() / 3) / 2 - png.getWidth() / 2;
      ypos = yPos[i] + 2 + (tft.height() / 4) / 2 - png.getHeight() / 2;
      tft.startWrite();
      uint32_t dt = millis();
      rc = png.decode(NULL, 0);
      tft.endWrite();
      Serial.print(millis() - dt);
      Serial.println("ms");
      tft.endWrite();
    }
  }
  else if (buttonsArray.btnImages[i] == "Right")
  {
    int16_t rc = png.openFLASH((uint8_t *)right, sizeof(right), pngDraw);

    if (rc == PNG_SUCCESS)
    {
      Serial.println("Successfully opened png file");
      xpos = xPos[i] + 2 + (tft.width() / 3) / 2 - png.getWidth() / 2;
      ypos = yPos[i] + 2 + (tft.height() / 4) / 2 - png.getHeight() / 2;
      tft.startWrite();
      uint32_t dt = millis();
      rc = png.decode(NULL, 0);
      tft.endWrite();
      Serial.print(millis() - dt);
      Serial.println("ms");
      tft.endWrite();
    }
  }
  else if (buttonsArray.btnImages[i] == "Escape")
  {
    int16_t rc = png.openFLASH((uint8_t *)escape, sizeof(escape), pngDraw);

    if (rc == PNG_SUCCESS)
    {
      Serial.println("Successfully opened png file");
      xpos = xPos[i] + 2 + (tft.width() / 3) / 2 - png.getWidth() / 2;
      ypos = yPos[i] + 2 + (tft.height() / 4) / 2 - png.getHeight() / 2;
      tft.startWrite();
      uint32_t dt = millis();
      rc = png.decode(NULL, 0);
      tft.endWrite();
      Serial.print(millis() - dt);
      Serial.println("ms");
      tft.endWrite();
    }
  }
  else if (buttonsArray.btnImages[i] == "Enter")
  {
    int16_t rc = png.openFLASH((uint8_t *)enter, sizeof(enter), pngDraw);

    if (rc == PNG_SUCCESS)
    {
      Serial.println("Successfully opened png file");
      xpos = xPos[i] + 2 + (tft.width() / 3) / 2 - png.getWidth() / 2;
      ypos = yPos[i] + 2 + (tft.height() / 4) / 2 - png.getHeight() / 2;
      tft.startWrite();
      uint32_t dt = millis();
      rc = png.decode(NULL, 0);
      tft.endWrite();
      Serial.print(millis() - dt);
      Serial.println("ms");
      tft.endWrite();
    }
  }
  else if (buttonsArray.btnImages[i] == "Volume Up")
  {
    int16_t rc = png.openFLASH((uint8_t *)volup, sizeof(volup), pngDraw);

    if (rc == PNG_SUCCESS)
    {
      Serial.println("Successfully opened png file");
      xpos = xPos[i] + 2 + (tft.width() / 3) / 2 - png.getWidth() / 2;
      ypos = yPos[i] + 2 + (tft.height() / 4) / 2 - png.getHeight() / 2;
      tft.startWrite();
      uint32_t dt = millis();
      rc = png.decode(NULL, 0);
      tft.endWrite();
      Serial.print(millis() - dt);
      Serial.println("ms");
      tft.endWrite();
    }
  }
  else if (buttonsArray.btnImages[i] == "Volume Down")
  {
    int16_t rc = png.openFLASH((uint8_t *)voldown, sizeof(voldown), pngDraw);

    if (rc == PNG_SUCCESS)
    {
      Serial.println("Successfully opened png file");
      xpos = xPos[i] + 2 + (tft.width() / 3) / 2 - png.getWidth() / 2;
      ypos = yPos[i] + 2 + (tft.height() / 4) / 2 - png.getHeight() / 2;
      tft.startWrite();
      uint32_t dt = millis();
      rc = png.decode(NULL, 0);
      tft.endWrite();
      Serial.print(millis() - dt);
      Serial.println("ms");
      tft.endWrite();
    }
  }
  else if (buttonsArray.btnImages[i] == "Mute")
  {
    int16_t rc = png.openFLASH((uint8_t *)mute, sizeof(mute), pngDraw);

    if (rc == PNG_SUCCESS)
    {
      Serial.println("Successfully opened png file");
      xpos = xPos[i] + 2 + (tft.width() / 3) / 2 - png.getWidth() / 2;
      ypos = yPos[i] + 2 + (tft.height() / 4) / 2 - png.getHeight() / 2;
      tft.startWrite();
      uint32_t dt = millis();
      rc = png.decode(NULL, 0);
      tft.endWrite();
      Serial.print(millis() - dt);
      Serial.println("ms");
      tft.endWrite();
    }
  }
  else if (buttonsArray.btnImages[i] == "Brush")
  {
    int16_t rc = png.openFLASH((uint8_t *)brush, sizeof(brush), pngDraw);

    if (rc == PNG_SUCCESS)
    {
      Serial.println("Successfully opened png file");
      xpos = xPos[i] + 2 + (tft.width() / 3) / 2 - png.getWidth() / 2;
      ypos = yPos[i] + 2 + (tft.height() / 4) / 2 - png.getHeight() / 2;
      tft.startWrite();
      uint32_t dt = millis();
      rc = png.decode(NULL, 0);
      tft.endWrite();
      Serial.print(millis() - dt);
      Serial.println("ms");
      tft.endWrite();
    }
  }
  else if (buttonsArray.btnImages[i] == "Cursor")
  {
    int16_t rc = png.openFLASH((uint8_t *)cursor, sizeof(cursor), pngDraw);

    if (rc == PNG_SUCCESS)
    {
      Serial.println("Successfully opened png file");
      xpos = xPos[i] + 2 + (tft.width() / 3) / 2 - png.getWidth() / 2;
      ypos = yPos[i] + 2 + (tft.height() / 4) / 2 - png.getHeight() / 2;
      tft.startWrite();
      uint32_t dt = millis();
      rc = png.decode(NULL, 0);
      tft.endWrite();
      Serial.print(millis() - dt);
      Serial.println("ms");
      tft.endWrite();
    }
  }
  else if (buttonsArray.btnImages[i] == "Crop")
  {
    int16_t rc = png.openFLASH((uint8_t *)crop, sizeof(crop), pngDraw);

    if (rc == PNG_SUCCESS)
    {
      Serial.println("Successfully opened png file");
      xpos = xPos[i] + 2 + (tft.width() / 3) / 2 - png.getWidth() / 2;
      ypos = yPos[i] + 2 + (tft.height() / 4) / 2 - png.getHeight() / 2;
      tft.startWrite();
      uint32_t dt = millis();
      rc = png.decode(NULL, 0);
      tft.endWrite();
      Serial.print(millis() - dt);
      Serial.println("ms");
      tft.endWrite();
    }
  }
  else if (buttonsArray.btnImages[i] == "Move")
  {
    int16_t rc = png.openFLASH((uint8_t *)move, sizeof(move), pngDraw);

    if (rc == PNG_SUCCESS)
    {
      Serial.println("Successfully opened png file");
      xpos = xPos[i] + 2 + (tft.width() / 3) / 2 - png.getWidth() / 2;
      ypos = yPos[i] + 2 + (tft.height() / 4) / 2 - png.getHeight() / 2;
      tft.startWrite();
      uint32_t dt = millis();
      rc = png.decode(NULL, 0);
      tft.endWrite();
      Serial.print(millis() - dt);
      Serial.println("ms");
      tft.endWrite();
    }
  }
  else if (buttonsArray.btnImages[i] == "Paintbucket")
  {
    int16_t rc = png.openFLASH((uint8_t *)paintbucket, sizeof(paintbucket), pngDraw);

    if (rc == PNG_SUCCESS)
    {
      Serial.println("Successfully opened png file");
      xpos = xPos[i] + 2 + (tft.width() / 3) / 2 - png.getWidth() / 2;
      ypos = yPos[i] + 2 + (tft.height() / 4) / 2 - png.getHeight() / 2;
      tft.startWrite();
      uint32_t dt = millis();
      rc = png.decode(NULL, 0);
      tft.endWrite();
      Serial.print(millis() - dt);
      Serial.println("ms");
      tft.endWrite();
    }
  }
  else if (buttonsArray.btnImages[i] == "Pan")
  {
    int16_t rc = png.openFLASH((uint8_t *)pan, sizeof(pan), pngDraw);

    if (rc == PNG_SUCCESS)
    {
      Serial.println("Successfully opened png file");
      xpos = xPos[i] + 2 + (tft.width() / 3) / 2 - png.getWidth() / 2;
      ypos = yPos[i] + 2 + (tft.height() / 4) / 2 - png.getHeight() / 2;
      tft.startWrite();
      uint32_t dt = millis();
      rc = png.decode(NULL, 0);
      tft.endWrite();
      Serial.print(millis() - dt);
      Serial.println("ms");
      tft.endWrite();
    }
  }
  else if (buttonsArray.btnImages[i] == "Shapes")
  {
    int16_t rc = png.openFLASH((uint8_t *)shapes, sizeof(shapes), pngDraw);

    if (rc == PNG_SUCCESS)
    {
      Serial.println("Successfully opened png file");
      xpos = xPos[i] + 2 + (tft.width() / 3) / 2 - png.getWidth() / 2;
      ypos = yPos[i] + 2 + (tft.height() / 4) / 2 - png.getHeight() / 2;
      tft.startWrite();
      uint32_t dt = millis();
      rc = png.decode(NULL, 0);
      tft.endWrite();
      Serial.print(millis() - dt);
      Serial.println("ms");
      tft.endWrite();
    }
  }
  else if (buttonsArray.btnImages[i] == "Text")
  {
    int16_t rc = png.openFLASH((uint8_t *)text, sizeof(text), pngDraw);

    if (rc == PNG_SUCCESS)
    {
      Serial.println("Successfully opened png file");
      xpos = xPos[i] + 2 + (tft.width() / 3) / 2 - png.getWidth() / 2;
      ypos = yPos[i] + 2 + (tft.height() / 4) / 2 - png.getHeight() / 2;
      tft.startWrite();
      uint32_t dt = millis();
      rc = png.decode(NULL, 0);
      tft.endWrite();
      Serial.print(millis() - dt);
      Serial.println("ms");
      tft.endWrite();
    }
  }
  else if (buttonsArray.btnImages[i] == "Comment")
  {
    int16_t rc = png.openFLASH((uint8_t *)comment, sizeof(comment), pngDraw);

    if (rc == PNG_SUCCESS)
    {
      Serial.println("Successfully opened png file");
      xpos = xPos[i] + 2 + (tft.width() / 3) / 2 - png.getWidth() / 2;
      ypos = yPos[i] + 2 + (tft.height() / 4) / 2 - png.getHeight() / 2;
      tft.startWrite();
      uint32_t dt = millis();
      rc = png.decode(NULL, 0);
      tft.endWrite();
      Serial.print(millis() - dt);
      Serial.println("ms");
      tft.endWrite();
    }
  }
  else if (buttonsArray.btnImages[i] == "Run")
  {
    int16_t rc = png.openFLASH((uint8_t *)run, sizeof(run), pngDraw);

    if (rc == PNG_SUCCESS)
    {
      Serial.println("Successfully opened png file");
      xpos = xPos[i] + 2 + (tft.width() / 3) / 2 - png.getWidth() / 2;
      ypos = yPos[i] + 2 + (tft.height() / 4) / 2 - png.getHeight() / 2;
      tft.startWrite();
      uint32_t dt = millis();
      rc = png.decode(NULL, 0);
      tft.endWrite();
      Serial.print(millis() - dt);
      Serial.println("ms");
      tft.endWrite();
    }
    else {
      Serial.print("Error: ");
      Serial.println(rc);
    }
  }
  else if (buttonsArray.btnImages[i] == "Stop")
  {
    int16_t rc = png.openFLASH((uint8_t *)stop, sizeof(stop), pngDraw);

    if (rc == PNG_SUCCESS)
    {
      Serial.println("Successfully opened png file");
      xpos = xPos[i] + 2 + (tft.width() / 3) / 2 - png.getWidth() / 2;
      ypos = yPos[i] + 2 + (tft.height() / 4) / 2 - png.getHeight() / 2;
      tft.startWrite();
      uint32_t dt = millis();
      rc = png.decode(NULL, 0);
      tft.endWrite();
      Serial.print(millis() - dt);
      Serial.println("ms");
      tft.endWrite();
    }
    else
    {
      Serial.print("Error: ");
      Serial.println(rc);
    }
  }
  else if (buttonsArray.btnImages[i] == "Keyboard")
  {
    int16_t rc = png.openFLASH((uint8_t *)keyboardshortcut, sizeof(keyboardshortcut), pngDraw);

    if (rc == PNG_SUCCESS)
    {
      Serial.println("Successfully opened png file");
      xpos = xPos[i] + 2 + (tft.width() / 3) / 2 - png.getWidth() / 2;
      ypos = yPos[i] + 2 + (tft.height() / 4) / 2 - png.getHeight() / 2;
      tft.startWrite();
      uint32_t dt = millis();
      rc = png.decode(NULL, 0);
      tft.endWrite();
      Serial.print(millis() - dt);
      Serial.println("ms");
      tft.endWrite();
    }
    else
    {
      Serial.print("Error: ");
      Serial.println(rc);
    }
  }
  else if (buttonsArray.btnImages[i] == "Back")
  {
    int16_t rc = png.openFLASH((uint8_t *)back, sizeof(back), pngDraw);

    if (rc == PNG_SUCCESS)
    {
      Serial.println("Successfully opened png file");
      xpos = xPos[i] + 2 + (tft.width() / 3) / 2 - png.getWidth() / 2;
      ypos = yPos[i] + 2 + (tft.height() / 4) / 2 - png.getHeight() / 2;
      tft.startWrite();
      uint32_t dt = millis();
      rc = png.decode(NULL, 0);
      tft.endWrite();
      Serial.print(millis() - dt);
      Serial.println("ms");
      tft.endWrite();
    }
    else
    {
      Serial.print("Error: ");
      Serial.println(rc);
    }
  }
  else if (buttonsArray.btnImages[i] == "Forward")
  {
    int16_t rc = png.openFLASH((uint8_t *)forward, sizeof(forward), pngDraw);

    if (rc == PNG_SUCCESS)
    {
      Serial.println("Successfully opened png file");
      xpos = xPos[i] + 2 + (tft.width() / 3) / 2 - png.getWidth() / 2;
      ypos = yPos[i] + 2 + (tft.height() / 4) / 2 - png.getHeight() / 2;
      tft.startWrite();
      uint32_t dt = millis();
      rc = png.decode(NULL, 0);
      tft.endWrite();
      Serial.print(millis() - dt);
      Serial.println("ms");
      tft.endWrite();
    }
    else
    {
      Serial.print("Error: ");
      Serial.println(rc);
    }
  }
  else if (buttonsArray.btnImages[i] == "Reload")
  {
    int16_t rc = png.openFLASH((uint8_t *)reload, sizeof(reload), pngDraw);

    if (rc == PNG_SUCCESS)
    {
      Serial.println("Successfully opened png file");
      xpos = xPos[i] + 2 + (tft.width() / 3) / 2 - png.getWidth() / 2;
      ypos = yPos[i] + 2 + (tft.height() / 4) / 2 - png.getHeight() / 2;
      tft.startWrite();
      uint32_t dt = millis();
      rc = png.decode(NULL, 0);
      tft.endWrite();
      Serial.print(millis() - dt);
      Serial.println("ms");
      tft.endWrite();
    }
    else
    {
      Serial.print("Error: ");
      Serial.println(rc);
    }
  }
  else if (buttonsArray.btnImages[i] == "Zoom In")
  {
    int16_t rc = png.openFLASH((uint8_t *)zoomin, sizeof(zoomin), pngDraw);

    if (rc == PNG_SUCCESS)
    {
      Serial.println("Successfully opened png file");
      xpos = xPos[i] + 2 + (tft.width() / 3) / 2 - png.getWidth() / 2;
      ypos = yPos[i] + 2 + (tft.height() / 4) / 2 - png.getHeight() / 2;
      tft.startWrite();
      uint32_t dt = millis();
      rc = png.decode(NULL, 0);
      tft.endWrite();
      Serial.print(millis() - dt);
      Serial.println("ms");
      tft.endWrite();
    }
    else
    {
      Serial.print("Error: ");
      Serial.println(rc);
    }
  }
  else if (buttonsArray.btnImages[i] == "Zoom Out")
  {
    int16_t rc = png.openFLASH((uint8_t *)zoomout, sizeof(zoomout), pngDraw);

    if (rc == PNG_SUCCESS)
    {
      Serial.println("Successfully opened png file");
      xpos = xPos[i] + 2 + (tft.width() / 3) / 2 - png.getWidth() / 2;
      ypos = yPos[i] + 2 + (tft.height() / 4) / 2 - png.getHeight() / 2;
      tft.startWrite();
      uint32_t dt = millis();
      rc = png.decode(NULL, 0);
      tft.endWrite();
      Serial.print(millis() - dt);
      Serial.println("ms");
      tft.endWrite();
    }
    else
    {
      Serial.print("Error: ");
      Serial.println(rc);
    }
  }
  else if (buttonsArray.btnImages[i] == "Undo")
  {
    int16_t rc = png.openFLASH((uint8_t *)undo, sizeof(undo), pngDraw);

    if (rc == PNG_SUCCESS)
    {
      Serial.println("Successfully opened png file");
      xpos = xPos[i] + 2 + (tft.width() / 3) / 2 - png.getWidth() / 2;
      ypos = yPos[i] + 2 + (tft.height() / 4) / 2 - png.getHeight() / 2;
      tft.startWrite();
      uint32_t dt = millis();
      rc = png.decode(NULL, 0);
      tft.endWrite();
      Serial.print(millis() - dt);
      Serial.println("ms");
      tft.endWrite();
    }
    else
    {
      Serial.print("Error: ");
      Serial.println(rc);
    }
  }
  else if (buttonsArray.btnImages[i] == "Redo")
  {
    int16_t rc = png.openFLASH((uint8_t *)redo, sizeof(redo), pngDraw);

    if (rc == PNG_SUCCESS)
    {
      Serial.println("Successfully opened png file");
      xpos = xPos[i] + 2 + (tft.width() / 3) / 2 - png.getWidth() / 2;
      ypos = yPos[i] + 2 + (tft.height() / 4) / 2 - png.getHeight() / 2;
      tft.startWrite();
      uint32_t dt = millis();
      rc = png.decode(NULL, 0);
      tft.endWrite();
      Serial.print(millis() - dt);
      Serial.println("ms");
      tft.endWrite();
    }
    else
    {
      Serial.print("Error: ");
      Serial.println(rc);
    }
  }
  else if (buttonsArray.btnImages[i] == "Volume Mixer")
  {
    int16_t rc = png.openFLASH((uint8_t *)volumemixer, sizeof(volumemixer), pngDraw);

    if (rc == PNG_SUCCESS)
    {
      Serial.println("Successfully opened png file");
      xpos = xPos[i] + 2 + (tft.width() / 3) / 2 - png.getWidth() / 2;
      ypos = yPos[i] + 2 + (tft.height() / 4) / 2 - png.getHeight() / 2;
      tft.startWrite();
      uint32_t dt = millis();
      rc = png.decode(NULL, 0);
      tft.endWrite();
      Serial.print(millis() - dt);
      Serial.println("ms");
      tft.endWrite();
    }
    else
    {
      Serial.print("Error: ");
      Serial.println(rc);
    }
  }
  else if (buttonsArray.btnImages[i] == "Search")
  {
    int16_t rc = png.openFLASH((uint8_t *)search, sizeof(search), pngDraw);

    if (rc == PNG_SUCCESS)
    {
      Serial.println("Successfully opened png file");
      xpos = xPos[i] + 2 + (tft.width() / 3) / 2 - png.getWidth() / 2;
      ypos = yPos[i] + 2 + (tft.height() / 4) / 2 - png.getHeight() / 2;
      tft.startWrite();
      uint32_t dt = millis();
      rc = png.decode(NULL, 0);
      tft.endWrite();
      Serial.print(millis() - dt);
      Serial.println("ms");
      tft.endWrite();
    }
    else
    {
      Serial.print("Error: ");
      Serial.println(rc);
    }
  }
  else if (buttonsArray.btnImages[i] == "Home")
  {
    int16_t rc = png.openFLASH((uint8_t *)home, sizeof(home), pngDraw);

    if (rc == PNG_SUCCESS)
    {
      Serial.println("Successfully opened png file");
      xpos = xPos[i] + 2 + (tft.width() / 3) / 2 - png.getWidth() / 2;
      ypos = yPos[i] + 2 + (tft.height() / 4) / 2 - png.getHeight() / 2;
      tft.startWrite();
      uint32_t dt = millis();
      rc = png.decode(NULL, 0);
      tft.endWrite();
      Serial.print(millis() - dt);
      Serial.println("ms");
      tft.endWrite();
    }
    else
    {
      Serial.print("Error: ");
      Serial.println(rc);
    }
  }
  else if (buttonsArray.btnImages[i] == "Right Click")
  {
    int16_t rc = png.openFLASH((uint8_t *)rightclick, sizeof(rightclick), pngDraw);

    if (rc == PNG_SUCCESS)
    {
      Serial.println("Successfully opened png file");
      xpos = xPos[i] + 2 + (tft.width() / 3) / 2 - png.getWidth() / 2;
      ypos = yPos[i] + 2 + (tft.height() / 4) / 2 - png.getHeight() / 2;
      tft.startWrite();
      uint32_t dt = millis();
      rc = png.decode(NULL, 0);
      tft.endWrite();
      Serial.print(millis() - dt);
      Serial.println("ms");
      tft.endWrite();
    }
    else
    {
      Serial.print("Error: ");
      Serial.println(rc);
    }
  }
  
  else
  {
    tft.setTextColor(TFT_BLACK);
    tft.drawCentreString(buttonsArray.buttons[i], xPos[i] + (tft.width() / 3) / 2, yPos[i] + (tft.height() / 4) / 2, 1);
  }
}
void redrawMacroPad()
{
  tft.fillScreen(TFT_BLACK);
  Serial.print("buttonsarray.length: ");
  Serial.println(buttonsArray.arrayLength);
  for (int i = 0; i < buttonsArray.arrayLength; i++)
  {
    Serial.println("Creating iteration: " + String(i));
    Serial.println("Filling");
    tft.fillSmoothRoundRect(xPos[i] + 2, yPos[i] + 2, (tft.width() / 3) - 2, (tft.height() / 4) - 2, 10, getRGBFromString(buttonsArray.btnColors[i]), TFT_TRANSPARENT);
    Serial.println("Drawing border");
    tft.drawSmoothRoundRect(xPos[i] + 2, yPos[i] + 2, 10, 9, (tft.width() / 3) - 2, (tft.height() / 4) - 2, getRGBFromString(buttonsArray.borderColors[i]), TFT_TRANSPARENT);
    tft.setTextSize(1);
    Serial.println("Drawing icons");
    redrawIcons(i);
    Serial.println(buttonsArray.buttons[i]);
    Serial.println("Setting details");
    buttons[i].x = xPos[i];
    buttons[i].y = yPos[i];
    buttons[i].w = (tft.width() / 3);
    buttons[i].h = (tft.height() / 4);
    buttons[i].text = buttonsArray.buttons[i];
    Serial.println("Done");
  }
}
int part = 0;
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
  Serial.print("Bytes received: ");
  Serial.println(len);
  if (part == 0)
  {
    memcpy(&btnsend, incomingData, sizeof(btnsend));
    for (int i = 0; i < 12; i++)
    {
      Serial.print(btnsend.buttons[i]);
      if (i == 0)
      {
        listofbtns += btnsend.buttons[i];
      }
      else
      {
        listofbtns += "," + btnsend.buttons[i];
      }
    }
    Serial.println("^^^^^");
    pref.putString("btns", listofbtns);
  }
  else if (part == 1)
  {
    memcpy(&btnimgsend, incomingData, sizeof(btnimgsend));
    for (int i = 0; i < 12; i++)
    {
      pref.putString((String("btnimg") + String(i)).c_str(), btnimgsend.btnImages[i]);
    }
  }
  else if (part == 2)
  {
    memcpy(&btnclrsend, incomingData, sizeof(btnclrsend));
    for (int i = 0; i < 12; i++)
    {
      pref.putString((String("col") + String(i)).c_str(), btnclrsend.btnColors[i]);
      pref.putString((String("bord") + String(i)).c_str(), btnbordsend.borderColors[i]);
    }
  }
  else if (part == 3)
  {
    memcpy(&btnbordsend, incomingData, sizeof(btnbordsend));
    for (int i = 0; i < 12; i++)
    {
      pref.putString((String("bord") + String(i)).c_str(), btnbordsend.borderColors[i]);
    }
    for(int i = 0; i < 12; i++) {
      buttonsArray.arrayLength = 12;
      buttonsArray.borderColors[i] = btnbordsend.borderColors[i];
      buttonsArray.btnColors[i] = btnclrsend.btnColors[i];
      buttonsArray.btnImages[i] = btnimgsend.btnImages[i];
      buttonsArray.buttons[i] = btnsend.buttons[i];
    }
    redrawMacroPad();
  }
  part++;
}

void setup()
{
  Serial.begin(115200);
  tft.begin();
  tft.setRotation(0);
  ts.setRotation(0);
  SPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  if (!ts.begin())
  {
    Serial.println("Error starting touchscreen");
    return;
  }
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.println("Starting...");
  WiFi.mode(WIFI_STA);
  tft.println(WiFi.macAddress());
  Serial.println("Loading Images");
  for (int i = 0; i < 28; i++)
  {
    Serial.print("Begin loading " + String(i) + ": " + imageLabels[i] + "...");
    images[imageLabels[i]] = (char *)imageDatas[i];
    Serial.println("done");
  }
  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_send_cb(OnDataSent);
  memcpy(peerInfo.peer_addr, receiverAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = true;

  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("Failed to add peer");
    return;
  }
  esp_now_register_recv_cb(OnDataRecv);
  Serial.println("ESP-NOW Initialized Successfully");
  pref.begin("btns");
  String inputString = pref.getString("btns", ",btn1,btn1,btn1,btn1,btn1,btn1,btn1,btn1,btn1,btn1,btn1,btn1");
  Serial.println(inputString);
  int arraySize = 12;            // Number of elements in the array
  String stringArray[arraySize]; // Declare the String array with a fixed size
  int index = 0;                 // Index for the String array

  // Split the inputString with the delimiter ","
  while (inputString.length() > 0 && index < arraySize)
  {
    int commaIndex = inputString.indexOf(",");
    if (commaIndex >= 0)
    {
      stringArray[index] = inputString.substring(0, commaIndex);
      inputString = inputString.substring(commaIndex + 1);
    }
    else
    {
      stringArray[index] = inputString;
      inputString = "";
    }
    index++;
  }

  // Print the String array to the Serial Monitor
  for (int i = 0; i < arraySize; i++)
  {
    Serial.println(stringArray[i]);
    buttonsArray.buttons[i] = stringArray[i];
    buttonsArray.btnColors[i] = pref.getString((String("col") + String(i)).c_str(), "255,0,0");
    buttonsArray.borderColors[i] = pref.getString((String("bord") + String(i)).c_str(), "255,0,0");
    buttonsArray.btnImages[i] = pref.getString((String("btnimg") + String(i)).c_str(), "Copy");
  }
  buttonsArray.arrayLength = arraySize;
  redrawMacroPad();
}
bool skipDelay = false;
void loop()
{
  TS_Point touchPoint = ts.getPoint();

  if (ts.touched())
  {
    // The touch panel was touched
    int x = touchPoint.x;
    int y = touchPoint.y;
    int tftX = map(x, 280, 3860, 0, tft.width());
    int tftY = map(y, 340, 3860, 0, tft.height());
    // tft.setCursor(0, 0);
    // tft.setTextColor(TFT_BLACK, TFT_WHITE);
    // tft.setTextSize(2);
    // tft.print(pin);
    Serial.print("x: ");
    Serial.print(x);
    Serial.print(" | ");
    Serial.print(tftX);
    Serial.print(", y: ");
    Serial.print(y);
    Serial.print(" | ");
    Serial.println(tftY);
    for (int i = 0; i < 12; i++)
    {
      if (tftX > buttons[i].x && tftX < buttons[i].x + buttons[i].w && tftY > buttons[i].y && tftY < buttons[i].y + buttons[i].h)
      {
        btnPrsd.buttonLabel = buttons[i].text;

        tft.fillSmoothRoundRect(xPos[i] + 2, yPos[i] + 2, (tft.width() / 3) - 2, (tft.height() / 4) - 2, 10, getRGBFromString(buttonsArray.borderColors[i]), TFT_TRANSPARENT);
        tft.drawSmoothRoundRect(xPos[i] + 2, yPos[i] + 2, 10, 9, (tft.width() / 3) - 2, (tft.height() / 4) - 2, getRGBFromString(buttonsArray.btnColors[i]), TFT_TRANSPARENT);
        tft.setTextSize(1);
        redrawIcons(i);
        Serial.println(buttonsArray.buttons[i]);
        buttons[i].x = xPos[i];
        buttons[i].y = yPos[i];
        buttons[i].w = (tft.width() / 3);
        buttons[i].h = (tft.height() / 4);
        buttons[i].text = buttonsArray.buttons[i];

        esp_err_t result = esp_now_send(receiverAddress, (uint8_t *)&btnPrsd, sizeof(btnPrsd));

        if (result == ESP_OK)
        {
          Serial.println("Sent with success");
        }
        else
        {
          Serial.println("Error sending the data");
        }
      }
    }
    if (true == false)
    {
      skipDelay = true;
    }
    else
    {
      skipDelay = true;
      delay(100);
      for (int i = 0; i < 12; i++)
      {
        if (tftX > buttons[i].x && tftX < buttons[i].x + buttons[i].w && tftY > buttons[i].y && tftY < buttons[i].y + buttons[i].h)
        {
          btnPrsd.buttonLabel = buttons[i].text;

          tft.fillSmoothRoundRect(xPos[i] + 2, yPos[i] + 2, (tft.width() / 3) - 2, (tft.height() / 4) - 2, 10, getRGBFromString(buttonsArray.btnColors[i]), TFT_TRANSPARENT);
          tft.drawSmoothRoundRect(xPos[i] + 2, yPos[i] + 2, 10, 9, (tft.width() / 3) - 2, (tft.height() / 4) - 2, getRGBFromString(buttonsArray.borderColors[i]), TFT_TRANSPARENT);
          tft.setTextSize(1);
          redrawIcons(i);
        }
      }
    }
  }
}