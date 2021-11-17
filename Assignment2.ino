#include <RL_LCD.h>    //Include LCD 
#include "RL_Switch.h" // Include Switch library 
#include <RL_RFID.h>    //Include RFID header
#include <SPI.h>        //Include SPI header
#include <Ethernet.h>   //Include Ethernet Header
#include "RL_PWM.h" // include RGBLED module library

Switch pushSwitch;   //Switch object
RFID rfid;           //RFID object
PWMController pwmc; // create an RGBLED object

/*Global Variables*/

/* Display Message for Items */
String displayMessage = "Choose Item!";
String displayItemsOption = "1,2,3";
String displayItem1 = "1. Coke";
String displayItem2 = "2. Dew";
String displayItem3 = "3. Ice";

int isItemEntered = 0;          //Flag to check if user has entered item
boolean isCardScanned = 0;      //By default cardisnotscanned.
boolean isCardReadable = 0;     //By default cardisnotreadable.
boolean isCardStored = 0;       //Check if the card value is stored.
LCD lcd(0x27,16,2);             //if this does not work then change to (0x3F,16,2);
String cardValue = "";          //To Store the Card Value
int downloadFile = 0;           //Flag to check if file is downloaed
int cardCodeEntered = 0;        //Flag to check if the 4 digit code is entered
long lastDebounceTime = 0;      //the last time the output pin was toggled
long debounceDelay = 3000;      //the debounce time; increase if the output flickers
int input_secretCode[4];        //Array to store pin code entered by user 
int buttonPushed = 0;           //Flag to count the number of times the code is entered
int isBalanceDisplayed = 0;     //Flag to check if the balance is displayed

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

// Set the static IP address
IPAddress ip(192, 168, 0, 111);

// Initialize the Ethernet client object
EthernetClient client;

char server[16] = "192.168.0.1";
char url[2000];


// last time you connected to the server, in milliseconds
unsigned long lastConnectionTime = 0;           

// delay between updates, in milliseconds
const unsigned long postingInterval = 10000UL;

char tempString[100]; //Temp String to store value returned by server
int counter = 0;      //Counter
int balance = 0.0;    //Store the balance
int isPinValid = 0;   //Flag to check if the valid pin is entered
int validatePin = 0;  //Flag to check if pin is validated
int isPurchaseMade =0;        //Flag to check if purchase is made  
int isPurchaseProcessed = 0;  //Flag to check if purchase is processed
long interval = 50000;        //Interval for timeout
long current = 0;             //Store the current time
long previous = 0;            //Store the elapsed time

void setup() {
  lcd.init();         //Initialize the lcd
  lcd.noBacklight();  //Light off too bright
  pushSwitch.begin(); //Initialise Switch module 
  rfid.PCD_Init();    //Initialise MFRC522
  Serial.begin(57600);  //Initialise Serial Monitor
  Ethernet.init(32);    // Ethernet.init(pin) to configure the CS pin
  Ethernet.begin(mac, ip);
  pwmc.begin(); // initialise the RGBLED module
  delay(1000);   //Delay

}

/* Function to reset the variable to restart the flow */
void reset(){
  isItemEntered = 0;                            
  isCardScanned = 0;                         
  isCardReadable = 0;                         
  isCardStored = 0;                           
  cardValue = "";
  downloadFile = 0;
  cardCodeEntered = 0;         
  buttonPushed = 0;           
  isBalanceDisplayed = 0;
  balance = 0.0; 
  isPinValid = 0;
  validatePin = 0;
  isPurchaseMade =0;
  isPurchaseProcessed = 0;
}

/* Function to display the items on LCD */
void showItems(){
  lcd.setCursor(2,0);
  lcd.print(displayMessage);
  lcd.setCursor(5,3);
  lcd.print(displayItemsOption);
  delay(2000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(displayItem1);
  lcd.setCursor(9,0);
  lcd.print(displayItem2);
  lcd.setCursor(5,3);
  lcd.print(displayItem3);
  delay(2000);
  lcd.clear();
  
}

/*Function to check if RFID is scanned*/
int isRFIDScanned(){
    //If RFID is not present, give feedback message and return
    if(rfid.isNewCardPresent()) {
      return 1; 
    }else{
      return 0;
    }
}

/*Function to check if Card is Readable*/
int readSerial(){
    //If RFID cannot be read return
    if(rfid.readCardSerial()) { 
      return 1; 
    }else{
      return 0;
    }
}

/*Send HTTP request to download the balance for the given card*/
void httpRequest_downloadBalance(String cardValue){
  // close any connection before send a new request
  client.stop();

  // if there's a successful connection
  if (client.connect(server, 80)) {
    Serial.println("Connected to server");
    // Make a HTTP request

    memset(url, '\0', 2000);
    
    if(cardValue != NULL) {
      sprintf(url, "GET /alacritas_direct.php?acc=sgiri4&call=kit117/Assignment2/%s-balance.txt HTTP/1.1", cardValue.c_str());
    }
               
    Serial.println(url);
    
    delay(10);
    
    client.println(url);
    client.println("Host: sss-blockchain.cis.utas.edu.au"); 
    client.println("Connection: close");
    client.println();

    lastConnectionTime = millis();
    downloadFile = 1;

  }
  else {
    // if you couldn't make a connection
    Serial.println("Connection failed");
  }


}

/*Function to send HTTP request to purchase the item*/
void httpRequest_purchase(int chooseItem, String cardValue){
    // close any connection before send a new request
  client.stop();

  // if there's a successful connection
  if (client.connect(server, 80)) {
    Serial.println("Connected to server");
    // Make a HTTP request

    memset(url, '\0', 2000);
    int quantity = random(1, 5);
    if(cardValue != NULL) {
      sprintf(url, "GET /alacritas_direct.php?acc=sgiri4&call=kit117/Assignment2/purchase.php&chooseItem=%d&card=%s&quantity=%d HTTP/1.1",chooseItem, cardValue.c_str(),quantity);
    }
               
    Serial.println(url);
    
    delay(10);
    
    client.println(url);
    client.println("Host: sss-blockchain.cis.utas.edu.au"); 
    client.println("Connection: close");
    client.println();

    lastConnectionTime = millis();
    isPurchaseMade = 1;

  }
  else {
    // if you couldn't make a connection
    Serial.println("Connection failed");
  }


}

/*Function to Validate the Card Pin */
void httpRequest_validatePin(String cardValue, String userPin){
     // close any connection before send a new request
  client.stop();

  // if there's a successful connection
  if (client.connect(server, 80)) {
    Serial.println("Connected to server");
    // Make a HTTP request

    memset(url, '\0', 2000);
    
    if(cardValue != NULL) {
      sprintf(url, "GET /alacritas_direct.php?acc=sgiri4&call=kit117/Assignment2/validatepin.php&card=%s&pin=%s HTTP/1.1", cardValue.c_str(), userPin.c_str());
    }
               
    Serial.println(url);
    
    delay(10);
    
    client.println(url);
    client.println("Host: sss-blockchain.cis.utas.edu.au"); 
    client.println("Connection: close");
    client.println();

    lastConnectionTime = millis();
    validatePin = 1;

  }
  else {
    // if you couldn't make a connection
    Serial.println("Connection failed");
  }
}


void loop() {
  if(isItemEntered == 0){
    showItems();
  }

  uint8_t chooseItem = pushSwitch.getPushKey();  
  if(chooseItem >= 0 && chooseItem!=255){
    if(chooseItem == 1 || chooseItem == 2 || chooseItem == 3){  //Make sure item selected is valid.
      isItemEntered = 1;
    }
  }

  if(isItemEntered==1){     //Item is Entered! Ask to Scan the RFID
    while(isCardScanned!=1){
      isCardScanned = isRFIDScanned();
      lcd.setCursor(3,0);
      lcd.print("Scan RFID");
      delay(2000);
      lcd.clear();   
    }

    while(isCardReadable!=1){
      isCardReadable = readSerial();
    }
    
  }
         
//Store the card into a array
  if(isCardStored == 0){
    if(isCardScanned && isCardReadable){
          for (int i = 0; i < 4; i++) {
            String uid_part = String(rfid.uid.uidByte[i], HEX); 
            cardValue += uid_part;           
          }
     isCardStored = 1;
    }
  }

  if(isCardStored == 1){
 

    if (downloadFile == 0 && millis() - lastConnectionTime > 3000) { // wait for 3 secs
      httpRequest_downloadBalance(cardValue);
    }

    if(downloadFile == 1){
      delay(1000);
      if(client.find("\r\n\r\n"))  {      
            
        memset(tempString, '\0', 100);    // reset tempString to empty
        counter = 0;                      // reset counter to 0
        
        while (client.available()) {     // if any character is avaiable then             
          char c = client.read();        // read each character
          tempString[counter] = c;       // collect each charatcer in tempString
          counter = counter + 1;         // increment for taking the next character
        }
    
       // Serial.println(tempString);       // show the string in tempString
    
        balance = atoi(tempString);  // convert the character array or string 
                                          
    
        //if card is valid print the balance
        lcd.setCursor(0,0);
        lcd.print("Balance: " + String(balance));
        delay(5000);
        lcd.clear();
        downloadFile = 2;
        isBalanceDisplayed = 1;
      
      }
   }
}

 if(isBalanceDisplayed == 1){
    delay(1000);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("4 digit pin: ");
  
    while(cardCodeEntered != 1){  
           
    //Loop until 4 digit code is entered
      uint8_t pin = pushSwitch.getPushKey();  
      current = millis();
        if(current - previous > interval){
           long previous = current;
           reset();
           lcd.clear();
           lcd.setCursor(0,0);
           lcd.print("No Action!");
           delay(2000);
           lcd.clear();
           break;
        }
       //Check if pin is entered.
      if(pin >= 0 && pin!=255){
        //Filter out debounce by setting time buffer
        if ((millis() - lastDebounceTime) > debounceDelay) { 
          input_secretCode[buttonPushed] = pin;
          lcd.setCursor(0,4);
          lcd.print(input_secretCode[0]);
          lcd.setCursor(4,4);
          lcd.print(input_secretCode[1]);
          lcd.setCursor(8,4);
          lcd.print(input_secretCode[2]);
          lcd.setCursor(12,4);
          lcd.print(input_secretCode[3]);
          
          buttonPushed++;
          pin = 255; //Reset pin to 255 
          lastDebounceTime = millis(); //set the current time
         }
      }
      //If all the 4 digit is entered set the flag to true
        if(buttonPushed == 4){
          delay(1000);
          lcd.clear();
          cardCodeEntered = 1; 
        }
    }

 }

 if(cardCodeEntered == 1){ //Validate the pin
    String userPin = "";
    for(int i=0; i<4; i++){
      userPin = userPin + input_secretCode[i];
    }   
    if (validatePin == 0 && millis() - lastConnectionTime > 3000) { // wait for 3 secs
      httpRequest_validatePin(cardValue, userPin);
     }
    // Serial.println("Your balance is: " + String(balance));
    //Serial.println("Your card is: " + cardValue);   
    //Serial.println("Your card is: " + userPin); 
    delay(1000);
 }

 if(validatePin == 1){
  delay(1000);
   //Send HTTP request to download the card file if the card matches the card stored in the server
  // if there are incoming bytes available
  // from the server, read them and print them
  
    if(client.find("\r\n\r\n"))  {      
      while (client.available()) {     // if any character is avaiable then             
        char c = client.read();        // read each character
        Serial.println(c);
        if(c == '0'){
          lcd.setCursor(0,0);
          lcd.print("Incorrect Pin");
          delay(4000);
          lcd.clear();
          reset();
        }
        else if(c == '1'){
          isPinValid = 1;   
        }
      }
   }
 
  }
  if(isPinValid == 1){
        //Send Request for purchase
        //Serial.println("Purchase processing!");
        lcd.setCursor(0,0);
        lcd.print("Processing Order");
        delay(4000);
        lcd.clear();
        //int quantity =2;
     if(isPurchaseProcessed == 0){
        httpRequest_purchase(chooseItem,cardValue);
        delay(1000);
          //Send HTTP request to download the card file if the card matches the card stored in the server
          // if there are incoming bytes available
          // from the server, read them and print them
        
        if(client.find("\r\n\r\n"))  {      
              
          counter = 0;                      // reset counter to 0
          
          while (client.available()) {     // if any character is avaiable then             
            char d = client.read();        // read each character
            if(d =='1'){
              //Serial.println(d);
              isPurchaseMade =1; //Successfull
  
            }
    
            if(d == '0'){
              isPurchaseMade = 2; //No Balance
             // Serial.println(d);
  
            }
            isPurchaseProcessed = 1;
            }
        } 
   
      //Call Function to make purchase with the item_number, card_id,  a random quantity is sent to the server. The balance value is updated on the server based on the quantity x price. 
      //Then the message displayed is “Successful”, and the RGB LED goes GREEN. If the balance is not available or the pin is incorrect, 
      //RGB LED goes RED and display the message "Not enough Balance" or “Incorrect Pin”. Decide what values you want to return from the PHP, e.g., 0 – Not Enough Balance, 1 – Incorrect Pin, 2 – Successful.
     }
  
  }

  if(isPurchaseMade == 1){
     
     pwmc.setAll (0, 255,0); // Set the RGBLED so Red=0 Green=255 Blue=0
     delay(1000);
     pwmc.setAll (0, 0,0); // Set the RGBLED so Red=0 Green=0 Blue=0
     
     lcd.setCursor(0,0);
     lcd.print("Purchased!");
     delay(5000);
     lcd.clear();
    
     lcd.setCursor(0,0);
     lcd.print("Buy Again!");
     delay(5000);

     lcd.clear();
       reset();
  
  }

  if(isPurchaseMade == 2){
    lcd.setCursor(0,0);
    lcd.print("Not Enough Balance");
    lcd.clear();

     pwmc.setAll (255,0,0); // Set the RGBLED so Red=255 Green=0 Blue=0
     delay(1000);
     pwmc.setAll (0, 0,0); // Set the RGBLED so Red=0 Green=0 Blue=0
     reset();
  }
  
}


   
 

  //Lab 3 Card 1: 651cffd1
  //Lab2 Card 1: 35197d2
  //Lab 4 Card 1: 85e0fed1
 
  
