# Power_Note__Automated_Load_Allocation_&_Acknowledgement_System

🔹 Overview
Power_Note is an IoT-based system designed to automate the process of power load (MW) allocation from a central control unit to multiple substations. It eliminates manual phone communication by using wireless data transmission and confirmation feedback.

🔹 System Architecture
Transmitter: ESP32 (Wi-Fi AP mode) with 16×2 I2C LCD
Receivers: ESP8266 (Station_1, Station_2) with 16×2 I2C LCD, buzzer, and push button
Communication: Local Wi-Fi (ESP32 as Access Point)

🔹 How It Works
ESP32 creates a Wi-Fi network (SSID: Power Note).
User opens a web interface and sets MW values for Station_1 and Station_2.
Receivers automatically fetch their assigned load and display it on the LCD.
On receiving new data, the buzzer alerts the station operator.
A long-press on the button stops the buzzer and sends an acknowledgment back to the transmitter.
The transmitter displays the “Received” status on the web interface and LCD.

🔹 Key Features
Automatic MW distribution
No manual calling or IP management
Station-wise identification
Visual (LCD) and audio (buzzer) alerts
Acknowledgement feedback system
Mobile-friendly web interface

🔹 Applications
Smart grid demonstration
Power distribution monitoring
Academic IoT and automation projects

🔹 Conclusion
Power_Note demonstrates how automation and IoT can improve reliability, efficiency, and transparency in power load management systems, making it suitable for both educational and real-world smart grid simulations.

📩 Contact, 
For help, suggestions, or collaboration, feel free to reach out. 
📧 Email: riyadhasan24a@gmail.com 
📱 WhatsApp: +88 01730 288553
