#include <FastLED.h>
#include <EEPROM.h>
#define PINK CRGB(255,20,147)
#define BLUE CRGB::Blue
#define BLACK CRGB::Black
#define WHITE CRGB::White
#define YELLOW CRGB::Yellow
#define RED CRGB::Red
#define ORANGE 0xFF4500
#define GREEN CRGB::Green
#define PURPLE CRGB::Purple

// How many leds are in the strip?
#define NUM_LEDS 1000
#define BRIGHTNESS 255
#define TEMP UncorrectedTemperature
#define DATA_PIN 3

uint16_t gHue = 0; // rotating "base color" used by many of the patterns
boolean verb = true; //verbosity level 
String cMode = "blue"; // mode currently running

CRGB leds[NUM_LEDS];
CRGBPalette16 tranny2 = CRGBPalette16(BLACK, BLACK, BLUE, PINK, WHITE, PINK, BLUE, BLACK,
                                      BLACK, BLACK, BLUE, PINK, WHITE, PINK, BLUE, BLACK);
CRGB tFlag[6] = {BLUE, PINK, WHITE, PINK, BLUE, BLACK};
CRGB pFlag[4] = {PINK, YELLOW, BLUE, BLACK};
CRGB gFlag[7] = {RED, ORANGE, YELLOW, GREEN, BLUE, PURPLE, BLACK};
CRGB nFlag[5] = {YELLOW, WHITE, PURPLE, BLACK, BLACK};
CRGB allFlag[22] = {PINK, YELLOW, BLUE, BLACK, RED, ORANGE, YELLOW, GREEN, BLUE, PURPLE, BLACK, YELLOW, WHITE, PURPLE, BLACK, BLACK, BLUE, PINK, WHITE, PINK, BLACK, BLACK};
CRGB mColor = CRGB::Blue;

//print help documentation to serial port
boolean help() {
  Serial.print(
    "Light Strip Controller - Juno Presken " + mDate + "\n"
    "Preface each command with > 100 spaces\n\n"
    "tranny - moving trans flags\n"
    "gay - moving LGBTQ+ flags\n"
    "beesly - moving pan flags\n"
    "enby - moving nonbinary flags\n"
    "flags - parade of all of those flags\n"
    "rainbow - FastLED's builtin rainbow function\n"
    "red - turn the whole strip red\n"
    "blue - turn the whole strip blue\n"
    "green - turn the whole strip green\n"
    "white - turn the whole strip white\n"
    "purple - turn the whole strip purple\n"
    "seizure - seizure mode - this sucks\n"
    "color (red) (green) (blue) - set whole strip to the specified color, numbers can be from 0 to 255\n"
    "smeizure - seizure mode but with individual leds instead of the whole strip - this sucks less\n"
    "chasers - blue chasers\n"
    "off - turn the whole strip off\n"
    "on - turn the whole strip white and set brightness to maximum\n"
    "brightness (percent) - sets brightness to (percent)% of full\n"
    "help - prints this document and turns on rainbow mode\n"
    "verbose - toggles verbosity (currently set to " + verb + ")\n");
    return false;
}

//display moving flag
boolean flags(int indices, CRGB flagarray[], int len, int offset) {
  len = len / indices;
  offset = offset % (len * indices);
  for (int i = 0; i < NUM_LEDS; i += len) {
    for (int j = 0; j < len; j++) {
      leds[(i + j + offset) % NUM_LEDS] = flagarray[i / len % indices];
    }
  }
  return true;
}

//entire strip random color
boolean seizure() {
  oneColor(CRGB(random8(), random8(), random8()));
  return true;
}

boolean rainbow()
{
  // FastLED's built-in rainbow generator
  fill_rainbow(leds, NUM_LEDS, gHue * 5, 7);
  return true;
}

//dumb wrapper, too lazy to refactor
boolean oneColor(CRGB clr) {
  fill_solid(leds, NUM_LEDS, clr);
  return true;
}

//pulse entire strip to beat
boolean pulse(int bpm){
  //bpm = bpm * 0.95;   //beats function seems to be about 5% slow, mostly corect for it :P
  
  int brightness = beats(bpm, 100, 255);
  for(int i=0; i < NUM_LEDS; i++){
    FastLED.setBrightness(brightness);
  }
  return true;
}

//shamelessly ripped from a fastled example program
boolean bpm2(uint8_t BeatsPerMinute, CRGBPalette16 palette)
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for ( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, i * 2, beat - gHue + (i * 10), NOBLEND);
  }
  return true;
}


//returns a number that goes from minimum to maximum bpm times per minute
// good luck debugging, i just wrote this and i'm not sure i understand it fully
int beats(int bpm, int minimum, int maximum){
  bpm = bpm * 1.15;
  return int(minimum + ((maximum - minimum) * ((sin((bpm * millis()) / (3600 * 3.14159))/2)+0.5)));
}


//moving pattern of small gradients to black. currently hardcoded to be blue
boolean chasers(CRGB clr, int offset) {
  for (int i = 0; i < NUM_LEDS; i++) {
    int bness = 10 * (i - offset) % 255;
    leds[i] = CRGB(0, 0, bness);
  }
  return true;
}

// print messasge to serial. If important is false, only print if in verbose mode
void say(String message, boolean important = false) {
  if (verb || important){
    Serial.println(message);
  }
}

//random color on all leds, no idea why theres an m in the name
boolean smeizure() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB(random8(), random8(), random8());
  }
  return true;
}

// toggle verbosity level
boolean verbToggle(){
  verb = !verb;
  if(verb){
    say("verbosity set to true");
  } else {
    say("verbosity set to false");
  }
  return false;  
}

// turn whole strip to full brightness, change cMode to "white"
boolean stripOn(){
  FastLED.setBrightness(255);
  cMode = "white";
  say("recieved on command, turning brightness to full and changing mode to \"white\"");
  return false;
}

// set strip's brightness to (level) percent
boolean bness(int level){
  FastLED.setBrightness((int)(level * 2.55));
  say("recieved bness command. Changing brightness without changing mode");
  return false;
}

//turns whole strip the specified color
//cString is a string of the format "R G B", with R, G, and B each being a number from one to 255
//this code hurt my brain to write XP
boolean color(String cString){
  int red = cString.substring(0,cString.indexOf(" ")).toInt();
  int green = cString.substring(cString.indexOf(" ")+1,cString.indexOf(" ",cString.indexOf(" ")+1)).toInt();
  int blue = cString.substring(cString.indexOf(" ",cString.indexOf(" ")+1)).toInt();
  fill_solid(leds, NUM_LEDS, CRGB(red,green,blue));
  return true;
}

// Parse command. return true if it's a new mode, false if it's an invalid command or a modifier
boolean stringButton(String command) {
  // each function will return true only if it is a "true" mode (i.e. not a transient command like brightness)
  // this way, the function as a whole will return true only if the command it is given is a "true" mode
  if (command.equals("tranny")) return flags(6, tFlag, 50, gHue);
  else if (command.equals("gay")) return flags(7, gFlag, 50, gHue);
  else if (command.equals("beesly")) return flags(4, pFlag, 50, gHue);
  else if (command.equals("enby")) return flags(5, nFlag, 50, gHue);
  else if (command.equals("flags")) return flags(22, allFlag, 150, gHue);
  else if (command.equals("rainbow")) return rainbow();
  else if (command.equals("red")) return oneColor(RED);
  else if (command.equals("blue")) return oneColor(BLUE);
  else if (command.equals("green")) return oneColor(GREEN);
  else if (command.equals("white")) return oneColor(WHITE);
  else if (command.equals("purple")) return oneColor(PURPLE);
  else if (command.equals("chasers")) return chasers(mColor, gHue);
  else if (command.equals("seizure")) return seizure();
  else if (command.equals("smeizure")) return smeizure();
  else if (command.equals("off")) return oneColor(BLACK);
  else if (command.equals("verbose")) return verbToggle();
  else if (command.equals("help")) return help();
  else if (command.equals("on")) return stripOn();
  else if (command.indexOf("brightness") != -1) return bness(command.substring(11).toInt());
  else if (command.indexOf("color") != -1) return color(command.substring(6));
  else {
    say(command);
    return false;
  }
  return true;
}

//check serial port, parse if there's stuff there
void checkSerial() {
  if (Serial.available() > 0) { //if there's serial data
    String serialInput = Serial.readString(); //read serail data into serialInput
    serialInput.trim(); //remove whitespace
    if (stringButton(serialInput)){
      say("recieved valid MODE command, applying and writing to EEPRO- uh, reminding juno to fix the eeprom code...");
      cMode = serialInput;
    }
  }
}

void setup() {
  delay(2000);
  Serial.begin(9600);
  say("Running Juno's LED control interface", true);
  say("because of a bug, commands must be prefaced with > 100 spaces", true);
  say("send the command \"help\" to get documentation", true);
  say("starting in mode \"" + cMode + "\"", true);
  FastLED.addLeds<WS2811, DATA_PIN, GRB>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.setTemperature(TEMP);
}

void loop() {
  checkSerial();
  stringButton(cMode);
  FastLED.show();
  EVERY_N_MILLISECONDS( 20 ) {
    gHue++;
  }
}
