#include <SPI.h>

#include <Ethernet.h>
#include <EthernetClient.h>
#include <EthernetServer.h>
#include <EthernetUdp.h>
#include <util.h>



/*
**
** WARNING: Ethernet Sheild uses analog pins 0 and 1 for the SD card
**          Even if you're not using the SD card, there are 10k pull-ups
**          So readings will be inaccurate
**
*/


/*
**
** X values range from 80-750
** Lower value as low as 50 at extreme edge
** Upper value slightly dependant on Y coord, but much more affected by pressure
**
** Y values range from 150-720
** Lower value 180 with finger
** Upper value slightly dependant on X coord (range from 680-720)
*/

/*******************************/

// Touch
const int xLow = 17;
const int xHigh = 19; // 15
const int yLow = 16;
const int yHigh = 18; // 14
int touchX;
int touchY;
boolean ignore_touch = false;
const int TOUCHES_TO_IGNORE = 2; // ignore first 2 touches (filters bad inputs)
int ignored_touches = 0;

// Ethernet
uint8_t eth_mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };  
uint8_t eth_ip[] = { 192,168,1,3 };
uint8_t eth_theserver[] = { 192,168,1,100 };
int eth_port = 12345;
boolean eth_connected = false;
EthernetClient client;

// Relay
const int relay_pin = 9;
boolean relay_on = false;


/*******************************/

void set_relay(boolean state = false) {
  if (state) {
    digitalWrite(relay_pin, HIGH);
    relay_on = true;
  } else {
    digitalWrite(relay_pin, LOW);
    relay_on = false;
  }
}

void toggle_relay() {
  if (relay_on)
    set_relay(false);
  else
    set_relay(true);
}

void getTouchValues(int delay_before_read = 10) {
  pinMode(xLow,OUTPUT);
  pinMode(xHigh,OUTPUT);
  digitalWrite(xLow,LOW);
  digitalWrite(xHigh,HIGH);
  digitalWrite(yLow,LOW);
  digitalWrite(yHigh,LOW);
  pinMode(yLow,INPUT);
  pinMode(yHigh,INPUT);
  delay(delay_before_read);
 
  //analog pins are 0-5
  touchX = analogRead(yLow -14);
 
  pinMode(yLow,OUTPUT);
  pinMode(yHigh,OUTPUT);
  digitalWrite(yLow,LOW);
  digitalWrite(yHigh,HIGH);
  digitalWrite(xLow,LOW);
  digitalWrite(xHigh,LOW);
  pinMode(xLow,INPUT);
  pinMode(xHigh,INPUT);
  delay(delay_before_read);
 
  //analog pins are 0-5
  touchY = analogRead(xLow - 14);
}

void music_play_pause() {
  Serial.println("Play/Pause");
  client.print('1');
}

void music_stop() {
  Serial.println("Stop");
  client.print('4');
}

void music_next() {
  Serial.println("Next");
  client.print('2');
}

void music_prev() {
  Serial.println("Previous");
  client.print('3');
}

void music_playlist_local() {
  Serial.println("Switching to Local Libray");
  client.print('a');
}

void music_playlist_dm() {
  Serial.println("Switching to DJ Mixes");
  client.print('c');
}

void music_playlist_eh() {
  Serial.println("Switching to Electro House");
  client.print('b');
}

boolean process_touch() {
  if ((touchX == 0) && (touchY == 0)) {
    ignore_touch = false;
    ignored_touches = 0;
    return false;
  } else if ((touchX == 0) || (touchY == 0)) {
    // Ignore partial touch inputs
    return false;
  } else if (ignore_touch) {
    return false;
  } else {
    if (ignored_touches < TOUCHES_TO_IGNORE) {
      ignored_touches++;
      return false;
    }
  }
  ignore_touch = true;
  
  // Process the touch
  if (touchX <= 200) { // Next or Prev
    if (touchY <= 400)
      music_prev();
    else
      music_next();
  } 
  else if ((touchX >= 250) && (touchX <= 400)) {
    if ((touchY >= 280) && (touchY <= 520)) {
      music_stop();
    }
  } 
  else if ((touchX >= 440) && (touchX <= 670)) {
    if ((touchY >= 200) && (touchY <= 660)) {
      music_play_pause();
    }
  } 
  else if ((touchX >= 700)) { // Playlists
    if ((touchY < 250)) {
      music_playlist_eh();
    } 
    else if ((touchY >= 350) && (touchY <= 510)) {
      music_playlist_dm();
    }
    else {
      music_playlist_local();
    }
  }
  
  return true;
}

/*******************************/
void setup(){
  Serial.begin(9600);
  Ethernet.begin(eth_mac, eth_ip);
  
  pinMode(relay_pin, OUTPUT);
  
  Serial.println("connecting...");
  if (client.connect(eth_theserver, eth_port)) {
    Serial.println("connected");
  } else {
    Serial.println("connection failed");
  }
}

/*******************************/
void loop(){
  
  // TODO: remove for final build
  if (Serial.available() > 0) {
    byte incomingByte = Serial.read();
    Serial.println("Toggling relay");
    toggle_relay();
  }
  
  getTouchValues();
//  Serial.print(touchX,DEC);   
//  Serial.print(",");     
//  Serial.println(touchY,DEC); 
  
  process_touch();

  //delay(30);
  
  while (client.available()) {
    char c = client.read();
    Serial.print(c);
  }
//  if (!client.connected()) {
//    Serial.println("disconnecting.");
//    client.stop();
//  }
}








