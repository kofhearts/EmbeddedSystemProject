# EmbeddedSystemProject

This project is about building simple EFTPOS like Embedded System that connects to web server to retreive the user's balance and make successfuly purchase based on the item's price. All the interaction to the system is made through buttons, sensors and display in the embedded system.

# Usage
The LCD viewer displays the items like in a vending machine. User choose between item 1 to item 3. System prompts user to scan the RFID which then displays the user balance. If there is enough balance user gets to enter 4 digit pin for the RFID card to make the purchase. Upon entering valid pin, system processes purchase and makes update on the user's balance. Timeout helps the system restart the process and start from the beginning if no user input is recorded in 15 seconds.

# Sensors and Devices Used

- LCD
- Switch
- RFID
- Ethernet
- RGB LED
- Button Array
