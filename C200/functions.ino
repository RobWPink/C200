
void daughterPrint(unsigned long dly){
  char ln1[16] = {' '};
  char ln0[5][16] = {{' '},{' '},{' '},{' '},{' '}};
  if(!twoTimer){twoTimer = millis();}
  if(millis() - twoTimer > 2000 && twoTimer){
    daughterTog = !daughterTog;
    twoTimer = millis();
    scrollCnt++;
    sprintf(ln0[0],"S2S.%03d.S2D.%03d ", (int)AI_H2_C_TT715_Stage2_SuctionTank, (int)AI_H2_C_TT520_Stage2_Discharge);
    sprintf(ln0[1],"S3S.%03d.S3D.%03d ", (int)AI_H2_C_TT521_Stage3_Suction, (int)AI_H2_C_TT522_Stage3_Discharge);
    switch(STATE){
      case FAULT:
        sprintf(ln0[2],"FAULT: %s",faultString);
      break;
      case ESTOP:
        sprintf(ln0[2],"ESTOP!          ");
      break;
      default:
      break;
    }
    if(manualPause){
      if(errMsg[0] != ""){ sprintf(ln0[2],"PAUSE: %s",errMsg[0]); }
      else{ sprintf(ln0[2],"PAUSE: No Errors"); }
    }
    lcd.setCursor(0, 0);
    if(ln0[scrollCnt][0] == ' '){ scrollCnt = 0; }//reset scrollCnt
    lcd.print(ln0[scrollCnt]);

    lcd.setCursor(0, 1);
    sprintf(ln1,"C%02dCT%02dHT%02dMX%03d", highCycleCnt, (int)AI_CLT_C_TT207_CoolantSupply2, (int)AI_HYD_C_TT454_HydraulicTank, (int)PTdata[1].max/100);
    lcd.print(ln1);
  }
  
  if(!daughterPrintTimer){daughterPrintTimer = millis();}
  if(millis() - daughterPrintTimer > dly && daughterPrintTimer){
    daughterPrintTimer = 0;
    bigMatrix[0].writeInt(daughterTog?AI_HYD_psig_PT467_HydraulicInlet1:AI_HYD_psig_PT561_HydraulicInlet2);
    smallMatrix[0].displayChar(daughterTog?'L':'H',false);
    bigMatrix[1].writeInt(daughterTog?AI_H2_psig_PT410_Stage3_DischargeTank:AI_H2_psig_PT407_Stage3_Discharge);
    smallMatrix[1].displayChar(daughterTog?'T':'D',false);
    bigMatrix[2].writeInt(AI_H2_psig_PT911_Stage1_SuctionTank);
  }
}

double potToTemp(double potReading, double BCOEFFICIENT_VALUE) {
  double potResistance = 4096 / potReading - 1;
  potResistance = SERIESRESISTOR / potResistance;  //Convert Pot Reading to resistance
  //----------Apply steinhart equation to reading-----------//
  double steinhart = log(potResistance / THERMISTORNOMINAL);                                                                            // ln(R/Ro)
  steinhart /= BCOEFFICIENT_VALUE;                                                                                                     // 1/B * ln(R/Ro)
  steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15);                                                                                    // + (1/To)
  steinhart = 1.0 / steinhart;                                                                                                         // Invert
  double potTemperature = steinhart + .0000007318 * pow(steinhart, 3) - .001011 * pow(steinhart, 2) + .4318 * steinhart - 58 - 273.15;  // magic and convert absolute temp to C
  return potTemperature;
}

void flashDriver(){
  if (!flashTimer[0] && flashGreen) { flashTimer[0] = millis(); }
  if (millis() - flashTimer[0] > flashGreen && flashGreen) {
    flashTog[0] = !flashTog[0];
    DO_Encl_PilotGreen = flashTog[0];
    flashTimer[0] = millis();
  }
  else if (!flashGreen && flashTimer[0]) {
    DO_Encl_PilotGreen = false;
    flashTimer[0] = 0;
  }

  if (!flashTimer[1] && flashAmber) { flashTimer[1] = millis(); }
  if (millis() - flashTimer[1] > flashAmber && flashAmber) {
    flashTog[1] = !flashTog[1];
    DO_Encl_PilotAmber = flashTog[1];
    flashTimer[1] = millis();
  }
  else if (!flashAmber && flashTimer[1]) {
    DO_Encl_PilotAmber = false;
    flashTimer[1] = 0;
  }

  if (!flashTimer[2] && flashRed) { flashTimer[2] = millis(); }
  if (millis() - flashTimer[2] > flashRed && flashRed) {
    flashTog[2] = !flashTog[2];
    DO_Encl_PilotRed = flashTog[2];
    flashTimer[2] = millis();
  }
  else if (!flashRed && flashTimer[2]) {
    DO_Encl_PilotRed = false;
    flashTimer[2] = 0;
  }
}


String parseString(String data, char separator, int index){
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "\0";
}

void RPCtransceive(){
  String rhe28msg = "";
  String redmsg = "";
  String msg = ""; 
  if(millis() - twoTimer > 2000 && twoTimer){
    while(RPC.available()){ msg = RPC.readStringUntil('\n'); }
    rhe28msg = parseString(msg,'$',0);
    redmsg = parseString(msg,'$',1);

    if(rhe28msg.indexOf("RHE28:") == 0){
      rhe28msg.remove(rhe28msg.indexOf("RHE28:"),6);
      if(rhe28msg.indexOf("ERROR:") >= 0){
        rhe28msg.remove(rhe28msg.indexOf("ERROR:"),6);
        Serial.print("RHE28 Revieving Error: ");Serial.println(rhe28msg);
      }
      else{
        AI_H2_KGPM_RHE28_Flow = parseString(rhe28msg,',',0).toFloat();
        AI_H2_C_RHE28_Temp = parseString(rhe28msg,',',1).toFloat();
        AI_H2_psig_RHE28_Pressure = parseString(rhe28msg,',',2).toFloat();
        AI_H2_KGPD_RHE28_Total = parseString(rhe28msg,',',3).toFloat();
      }
    }
    if(redmsg.indexOf("RED:") == 0){
      redmsg.remove(redmsg.indexOf("RED:"),4);
      if(redmsg.indexOf("ERROR:") >= 0){
        redmsg.remove(redmsg.indexOf("ERROR:"),6);
        Serial.print("RED Revieving Error: ");Serial.println(redmsg);
      }
      else{
        AI_H2_KGPM_RED_Flow = parseString(redmsg,',',0).toFloat();
        AI_H2_C_RED_Temp = parseString(redmsg,',',1).toFloat();
        AI_H2_psig_RED_Pressure = parseString(redmsg,',',2).toFloat();
        AI_H2_KGPD_RED_Total = parseString(redmsg,',',3).toFloat();
      }
    }
  }
}

double dynamicSwitching100Psi(bool highlow, double suction, double discharge){
  if(!highlow){
    if(70 <= suction < 80){ return 1.1524*discharge + 196.83;}
    else if(80 <= suction < 90){ return 1.1641*discharge + 181.37;}
    else if(90 <= suction < 100){ return 1.1735*discharge + 165.88;}
    else if(100 <= suction < 110){ return 1.1625*discharge + 152.75;}
    else if(110 <= suction < 120){ return 1.1705*discharge + 137.25;}
    else if(120 <= suction < 130){ return 1.1773*discharge + 121.73;}
    else if(130 <= suction < 140){ return 1.1831*discharge + 106.2;}
    else if(140 <= suction < 150){ return 1.1882*discharge + 90.658;}
    else{return -1;}
  }
  else{
    return -1;
  }
}

void i2cTransceive(int ptInterval){

  for(int i = 0; i < TTsize;i++){
    if(TTdata[i].channel == -1){;
     // TTdata[i].mcp.setFilterCoefficient(TTdata[i].coef);
      if(TTdata[i].mcp == 1 && !bitRead(mcpExist,0)){
        TTdata[i].rawTemp = mcp1.readThermocouple();
      }
      else if(TTdata[i].mcp == 2 && !bitRead(mcpExist,1)){
        TTdata[i].rawTemp = mcp2.readThermocouple();
      }
      else if(TTdata[i].mcp == 3 && !bitRead(mcpExist,2)){
        TTdata[i].rawTemp = mcp3.readThermocouple();
      }
      else if(TTdata[i].mcp == 4 && !bitRead(mcpExist,3)){
        TTdata[i].rawTemp = mcp4.readThermocouple();
      }
      else if(TTdata[i].mcp == 5 && !bitRead(mcpExist,4)){
        TTdata[i].rawTemp = mcp5.readThermocouple();
      }
      else if(TTdata[i].mcp == 6 && !bitRead(mcpExist,5)){
        TTdata[i].rawTemp = mcp6.readThermocouple();
      }
      else{;}
      if(0 < TTdata[i].mcp <= 6 && !bitRead(mcpExist,TTdata[i].mcp)){
        TTdata[i].avg.addValue(TTdata[i].rawTemp);
        *TTdata[i].value = TTdata[i].avg.getAverage();
      }
    }
    else if(TTdata[i].channel != -1){
      TTdata[i].raw = TTdata[i].adc.read(TTdata[i].channel,SD);
      TTdata[i].rawTemp = potToTemp(TTdata[i].raw,TTdata[i].coef);
      TTdata[i].avg.addValue(TTdata[i].rawTemp);
      *TTdata[i].value = TTdata[i].avg.getAverage();    
    }
/*
    if(TTdata[i].max != -1){
      if(*TTdata[i].value >= TTdata[i].max && !TTdata[i].overheat){
        TTdata[i].overheat = true;
        if(STATE == PRODUCTION){STATE = PAUSE;}
      }
      else if(*TTdata[i].value <= TTdata[i].maxRecovery && TTdata[i].overheat){
        TTdata[i].overheat = false;
          
      }
    }

    if(TTdata[i].min != -1){
      if(*TTdata[i].value <= TTdata[i].min && !TTdata[i].overheat){
        TTdata[i].overheat = true;
        if(STATE == PRODUCTION){STATE = PAUSE;}
      }
      else if(*TTdata[i].value >= TTdata[i].minRecovery && TTdata[i].overheat){
        TTdata[i].overheat = false;
        
      }
    }
    */
  }

  if(!dataTimer){dataTimer = millis();}
  if(millis() - dataTimer > ptInterval){
    for(int i = 0; i < PTsize;i++){
      if(PTdata[i].channel != -1 || PTdata[i].key != "PT467" || PTdata[i].key != "PT561"){
        PTdata[i].raw = PTdata[i].adc.read(PTdata[i].channel,SD);
        if(PTdata[i].mapA != -1 && PTdata[i].mapB != -1 && PTdata[i].mapC != -1 && PTdata[i].mapD != -1){
          PTdata[i].mapped = map((int)PTdata[i].raw, PTdata[i].mapA, PTdata[i].mapB, PTdata[i].mapC, PTdata[i].mapD) + PTdata[i].offset; //read slowly
          PTdata[i].avg.addValue(PTdata[i].mapped);
          *PTdata[i].value = PTdata[i].avg.getAverage();
        }
      }
      /*
      if(PTdata[i].max != -1 && PTdata[i].pause != -1){
        if(*PTdata[i].value >= PTdata[i].max && !PTdata[i].overPressure){
          PTdata[i].overPressure = true;
          if(STATE == PRODUCTION){
            if(!PTdata[i].pause){ INTENSE1 = PAUSE; INTENSE2 = PAUSE; }
            else{
              if(PTdata[i].pause == 1){INTENSE1 = PAUSE;}
              else if(PTdata[i].pause == 2){INTENSE2 = PAUSE;}
              else{Serial.println("pause state error");}
            }
          }
        }
        else if(*PTdata[i].value <= PTdata[i].maxRecovery && PTdata[i].overPressure){
          PTdata[i].overPressure = false;
        }
      }

      if(PTdata[i].min != -1){
        if(*PTdata[i].value < PTdata[i].min && !PTdata[i].overPressure){
          PTdata[i].overPressure = true;
          if(STATE == PRODUCTION){
            if(!PTdata[i].pause){ INTENSE1 = PAUSE; INTENSE2 = PAUSE; }
            else{
              if(PTdata[i].pause == 1){INTENSE1 = PAUSE;}
              else if(PTdata[i].pause == 2){INTENSE2 = PAUSE;}
              else{Serial.println("pause state error");}
            }
          }
        }
        else if(*PTdata[i].value >= PTdata[i].minRecovery && PTdata[i].overPressure){
          PTdata[i].overPressure = false;
        }
      }
      */
    }
  }
  PTdata[6].mapped = map((int)PTdata[6].raw, PTdata[6].mapA, PTdata[6].mapB, PTdata[6].mapC, PTdata[6].mapD) + PTdata[6].offset; //read hydraulics quickly
  PTdata[6].avg.addValue(PTdata[6].mapped);
  *PTdata[6].value = PTdata[6].avg.getAverage();

  PTdata[7].mapped = map((int)PTdata[7].raw, PTdata[7].mapA, PTdata[7].mapB, PTdata[7].mapC, PTdata[7].mapD) + PTdata[7].offset; //read hydraulics quickly
  PTdata[7].avg.addValue(PTdata[7].mapped);
  *PTdata[7].value = PTdata[7].avg.getAverage();

  for(int i = 0; i < DIsize;i++){
    if(DIdata[i].addr != -1){
      DIdata[i].gpio.digitalRead(DIdata[i].addr,DIdata[i].value);
    }
  }

  for(int i = 0; i < DOsize;i++){
    if(DOdata[i].addr != -1){
      DOdata[i].gpio.digitalWrite(DOdata[i].addr,*DOdata[i].value);
    }
  }
  
}



void i2cSetup(){
  adc1.init();
  adc2.init();
  adc3.init();

  if(!mcp1.begin(0x60)){
    Serial.println("WARNING!! Couldnt detect mcp1(0x60)");
    bitSet(mcpExist,0);
  }
  else{
    mcp1.setADCresolution(MCP9600_ADCRESOLUTION_12);
    mcp1.setThermocoupleType(MCP9600_TYPE_K);
    mcp1.enable(true);
  }

  if(!mcp2.begin(0x61)){
    Serial.println("WARNING!! Couldnt detect mcp2(0x61)");
    bitSet(mcpExist,1);
  }
  else{
    mcp2.setADCresolution(MCP9600_ADCRESOLUTION_12);
    mcp2.setThermocoupleType(MCP9600_TYPE_K);
    mcp2.enable(true);
  }

  if(!mcp3.begin(0x62)){
    Serial.println("WARNING!! Couldnt detect mcp3(0x62)");
    bitSet(mcpExist,2);
  }
  else{
    mcp3.setADCresolution(MCP9600_ADCRESOLUTION_12);
    mcp3.setThermocoupleType(MCP9600_TYPE_K);
    mcp3.enable(true);
  }

  if(!mcp4.begin(0x63)){
    Serial.println("WARNING!! Couldnt detect mcp4(0x63)");
    bitSet(mcpExist,3);
  }
  else{
    mcp4.setADCresolution(MCP9600_ADCRESOLUTION_12);
    mcp4.setThermocoupleType(MCP9600_TYPE_K);
    mcp4.enable(true);
  }

  if(!mcp5.begin(0x64)){
    Serial.println("WARNING!! Couldnt detect mcp5(0x64)");
    bitSet(mcpExist,4);
  }
  else{
    mcp5.setADCresolution(MCP9600_ADCRESOLUTION_12);
    mcp5.setThermocoupleType(MCP9600_TYPE_K);
    mcp5.enable(true);
  }
  
  if(!mcp6.begin(0x65)){
    Serial.println("WARNING!! Couldnt detect mcp6(0x65)");
    bitSet(mcpExist,5);
  }
  else{
    mcp6.setADCresolution(MCP9600_ADCRESOLUTION_12);
    mcp6.setThermocoupleType(MCP9600_TYPE_K);
    mcp6.enable(true);
  }

  gpio1.begin();
  gpio1.pinMode(P0_1, INPUT_PULLUP, true);
  gpio1.pinMode(P0_2, INPUT_PULLUP, true);
  gpio1.pinMode(P0_3, INPUT_PULLUP, true);
  gpio1.pinMode(P0_4, INPUT_PULLUP, true);
  gpio1.pinMode(P0_5, INPUT_PULLUP, true);
  gpio1.pinMode(P0_6, INPUT_PULLUP, true);
  gpio1.pinMode(P0_7, INPUT_PULLUP, true);
  gpio1.pinMode(P1_0, INPUT_PULLUP, true);
  gpio1.pinMode(P1_1, INPUT_PULLUP, true);
  gpio1.pinMode(P1_2, INPUT_PULLUP, true);
  gpio1.pinMode(P1_5, INPUT_PULLUP, true);
  gpio1.pinMode(P1_6, INPUT_PULLUP, true);
  gpio1.pinMode(P1_7, INPUT_PULLUP, true);
  gpio1.pinMode(P2_0, INPUT_PULLUP, true);
  gpio1.pinMode(P2_1, INPUT_PULLUP, true);
  gpio1.pinMode(P2_2, INPUT_PULLUP, true);
  gpio1.pinMode(P2_3, INPUT_PULLUP, true);
  gpio1.pinMode(P2_4, INPUT_PULLUP, true);
  gpio1.pinMode(P2_5, INPUT_PULLUP, true);
  gpio1.pinMode(P2_6, INPUT_PULLUP, true);
  gpio1.pinMode(P2_7, INPUT_PULLUP, true);
  gpio1.pinMode(P3_0, INPUT_PULLUP, true);
  gpio1.pinMode(P3_1, INPUT_PULLUP, true);
  gpio1.pinMode(P3_2, INPUT_PULLUP, true);
  gpio1.pinMode(P3_3, OUTPUT);
  gpio1.pinMode(P3_4, OUTPUT);
  gpio1.pinMode(P3_5, OUTPUT);
  gpio1.pinMode(P3_6, OUTPUT);
  gpio1.pinMode(P3_7, OUTPUT);
  gpio1.pinMode(P4_0, OUTPUT);
  gpio1.pinMode(P4_1, OUTPUT);
  gpio1.digitalWrite(P3_3, HIGH);
  gpio1.digitalWrite(P3_4, LOW);
  gpio1.digitalWrite(P3_5, LOW);
  gpio1.digitalWrite(P3_6, LOW);
  gpio1.digitalWrite(P3_7, LOW);
  gpio1.digitalWrite(P4_0, LOW);
  gpio1.digitalWrite(P4_1, LOW);

  gpio2.begin();
  gpio2.pinMode(P0_0, OUTPUT);
  gpio2.pinMode(P0_1, OUTPUT);
  gpio2.pinMode(P0_2, OUTPUT);
  gpio2.pinMode(P0_3, OUTPUT);
  gpio2.pinMode(P0_4, OUTPUT);
  gpio2.pinMode(P0_5, OUTPUT);
  gpio2.pinMode(P0_6, OUTPUT);
  gpio2.pinMode(P0_7, OUTPUT);
  gpio2.pinMode(P1_0, INPUT_PULLUP, true);
  gpio2.pinMode(P1_1, INPUT_PULLUP, true);
  gpio2.pinMode(P1_2, INPUT_PULLUP, true);
  gpio2.pinMode(P1_5, INPUT_PULLUP, true);
  gpio2.pinMode(P1_6, INPUT_PULLUP, true);
  gpio2.pinMode(P1_7, INPUT_PULLUP, true);
  gpio2.pinMode(P2_0, INPUT_PULLUP, true);
  gpio2.pinMode(P2_1, INPUT_PULLUP, true);
  gpio2.pinMode(P2_2, INPUT_PULLUP, true);
  gpio2.pinMode(P2_3, INPUT_PULLUP, true);
  gpio2.pinMode(P2_4, INPUT_PULLUP, true);
  gpio2.pinMode(P2_5, INPUT_PULLUP, true);
  gpio2.pinMode(P2_6, INPUT_PULLUP, true);

  gpio2.pinMode(P2_7, INPUT_PULLUP, true);
  gpio2.pinMode(P3_0, INPUT_PULLUP, true);
  gpio2.pinMode(P3_1, INPUT_PULLUP, true);
  gpio2.pinMode(P3_2, INPUT_PULLUP, true);
  gpio2.pinMode(P3_3, INPUT_PULLUP, true);
  gpio2.pinMode(P3_4, INPUT_PULLUP, true);
  gpio2.pinMode(P3_5, INPUT_PULLUP, true);
  gpio2.pinMode(P3_6, INPUT_PULLUP, true);
  gpio2.pinMode(P3_7, INPUT_PULLUP, true);
  gpio2.pinMode(P4_0, INPUT_PULLUP, true);
  gpio2.pinMode(P4_1, INPUT_PULLUP, true);
  gpio2.digitalWrite(P0_0, LOW);
  gpio2.digitalWrite(P0_1, LOW);
  gpio2.digitalWrite(P0_2, LOW);
  gpio2.digitalWrite(P0_3, LOW);
  gpio2.digitalWrite(P0_4, LOW);
  gpio2.digitalWrite(P0_5, LOW);
  gpio2.digitalWrite(P0_6, LOW);
  gpio2.digitalWrite(P0_7, HIGH);
}


void pinModeSetup(){
  pinMode(ESTOP_BREAK, OUTPUT);
  digitalWrite(ESTOP_BREAK, HIGH);  // HIGH to keep Estop loop intact
  pinMode(LED_PWR, OUTPUT);
  digitalWrite(LED_PWR, HIGH);  // HIGH to provide power to Daughterboard
  pinMode(TRACO_24VDC, OUTPUT);
  digitalWrite(TRACO_24VDC, HIGH);  // HIGH to enable 24V Power Supply
}


void matrixSetup(String s1, String s2) {
  //begin matrices
  for (int i = 0; i < 3; i++) {
    smallMatrix[i].begin();
    bigMatrix[i].begin();
  }

  //begin LCD
  lcd.begin(16, 2);
  lcd.clear();
  lcd.print(s1);
  lcd.setCursor(0, 1);
  lcd.print(s2);
  lcd.setBacklight(HIGH);
}

void preTransmission() {
  digitalWrite(RE_DE1, 1);
}

void postTransmission() {
  digitalWrite(RE_DE1, 0);
}
