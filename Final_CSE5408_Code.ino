#define PLAYER_WAIT_TIME 1000000 

byte sequence[100];           // Storage for sequence
byte curLen = 0;              
byte inputCount = 0;          
byte lastInput = 0;           // Last input from player
byte expRd = 0;               // expected LED from player
bool btnDwn = false;          // check if a button is pressed
bool wait = false;            // Is the program waiting for the user to press a button
bool resetFlag = false;       // Used to indicate to the program that once the player lost

int gameSelected = 0;         // value for selected game
int score;                    // value for score
int hiscore;                  // value for session hiscore
int offline;                  // value for if playing game offline from app

byte noPins = 4;              // Number of buttons/LEDs
                             
byte pins[] = {2, 8, 10, 13}; // Button input pins and LED ouput pins
                              
                              
long inputTime = 0;           // Timer variable for the delay between user inputs

void setup() {                
  Serial.begin(9600);         
  Reset();
}


void setPinDirection(byte dir){
  for(byte i = 0; i < noPins; i++){
    pinMode(pins[i], dir); 
  }
}

//send the same value to all the LED pins
void writeAllPins(byte val){
  for(byte i = 0; i < noPins; i++){
    digitalWrite(pins[i], val); 
  }
}


void flash(short freq){       // flashes all LEDs
  setPinDirection(OUTPUT);    
  for(int i = 0; i < 5; i++){
    writeAllPins(HIGH);
    delay(freq);
    writeAllPins(LOW);
    delay(freq);
  }
}

// This function resets all the game variables to their default values
void Reset(){
  flash(250);
  curLen = 0;
  inputCount = 0;
  lastInput = 0;
  expRd = 0;
  btnDwn = false;
  wait = false;
  resetFlag = false;
  gameSelected = 0;
  score = 0;
  offline = 0;
}


void Lose(){
  flash(50);
}


void playSequence(){
  // Loop through the stored sequence to show the player the sequence they must remember or inputted correctly
  for(int i = 0; i < curLen; i++){
      digitalWrite(sequence[i], HIGH);
      delay(500);
      digitalWrite(sequence[i], LOW);
      delay(250);
    } 
}


void DoLoseProcess(){ // Lost process
  Lose();             
  delay(1000);
  playSequence();     
  delay(1000);
  Reset();            // Reset for a new game
  Serial.print(score);
  Serial.print(",");
  Serial.print("l");
  Serial.print(",");
  Serial.print(hiscore);
}


void memoryGame(){
  if(!wait){ //Arduino Turn
    
    setPinDirection(OUTPUT);                      
    
    randomSeed(analogRead(A0));                   // choosing pin at random
    sequence[curLen] = pins[random(0,noPins)];    
    curLen++;                                     // update length
    
    playSequence();                               // Show sequence
    
    wait = true;                                  
    inputTime = millis();                         
  
  }else{ //Player turn
    
    //set initial counter
    setPinDirection(INPUT);                       

    if(millis() - inputTime > PLAYER_WAIT_TIME){  // If the player takes more than the allowed time,
      DoLoseProcess();                           
      return;
    }      
        
    if(!btnDwn){                                   
      expRd = sequence[inputCount];               // value expected from the player
      
      for(int i = 0; i < noPins; i++){           // Loop through all the pins
        if(pins[i]==expRd)                        
          continue;                               // Ignore the correct pin
        if(digitalRead(pins[i]) == HIGH){         // Is the button pressed
          lastInput = pins[i];
          resetFlag = true;                       // Set the resetFlag - this means you lost
          btnDwn = true;                          // prevent program from doing same thing over and over
        }
      }      
    }

    if(digitalRead(expRd) == 1 && !btnDwn)        // player pressed right button
    {
      inputTime = millis();                       
      lastInput = expRd;
      inputCount++;                               // user pressed correct button again
      btnDwn = true;                              // Tprevent program from doing same thing over and over
      //reset counter after each button press
    }else{
      if(btnDwn && digitalRead(lastInput) == LOW){  // Check if player released button
        btnDwn = false;
        delay(20);
        if(resetFlag){                              // If this was set to true up above, you lost
          DoLoseProcess();                          // So we do the losing sequence of events
        }
        else{
          if(inputCount == curLen){                 // if player finished repeating the sequence
            wait = false;                           // next turn is the program's turn
            inputCount = 0;                         // Reset the number of times that the player has pressed a button
            score = score + 1;
            if(hiscore < score){                    // check if score is higher than current hiscore
              hiscore = score;                      // if so, set hi-score equal to current score
              }
            Serial.print(score);                    // print out values of score, playing status and hi-score
            Serial.print(",");
            Serial.print("p");
            Serial.print(",");
            Serial.print(hiscore);
            delay(1500);
          }
        }
      }
    }    
  }
}

///
/// Where the magic happens
///
void loop() { 
  if(Serial.available()>0){
    gameSelected = Serial.read();
    } 
  if ((digitalRead(pins[2]) == HIGH) && (digitalRead(pins[8]) == HIGH) && (digitalRead(pins[10]) == HIGH) && (digitalRead(pins[13]) == HIGH)){
      flash(250);
      offline = 1;      // set game to play offline
    }
  while(offline == 1){  // when set to offline start game
      memoryGame();
    }
  while(gameSelected == '1'){  // when start button pressed start game
      memoryGame();
    }
  if(gameSelected == '2'){     // when going to home reset session hi-score as bluetooth disconnects
      hiscore = 0;
    }
}
