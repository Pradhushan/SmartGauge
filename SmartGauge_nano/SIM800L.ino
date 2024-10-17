bool connectToNetwork(){
  int attempt = 0;

  while (attempt < 10){ // Retry up to 15 times
    sim800l.println("AT+CREG?");
    delay(1000);

    if (sim800l.available()){
      String response = sim800l.readString();
      if (response.indexOf("+CREG: 0,1") != -1 || response.indexOf("+CREG: 0,5") != -1){
        return true; // Connected to the network
      }
    }

    attempt++;
    delay(2000); // Wait for 2 seconds before the next attempt
  }

  return false; // Failed to connect to the network
}



// Configure SMS mode
void configureSMSMode(){
  sim800l.println("AT+CMGF=1");
  delay(1000);
  sim800l.println("AT+CNMI=2,2,0,0,0");
  delay(1000);
}


// Extract Sender's number
String parseSenderNumber(String sms){
  int startPos = sms.indexOf("\"", sms.indexOf("+"));
  int endPos = sms.indexOf("\"", startPos + 1);
  
  if (startPos != -1 && endPos != -1){
    return sms.substring(startPos + 1, endPos);
  }

  return "";
}


// Restart Command
void restart(){
  asm volatile ("  jmp 0");
}