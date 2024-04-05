void daughterPrint(unsigned long dly){
  sprintf(ln1,"C.%02d.CT.%02d.HT.%02d   ", cycleCnt, (int)AI_CLT_C_TT207_CoolantSupply2, (int)AI_HYD_C_TT454_HydraulicTank);
  sprintf(ln2,"S2S.%03d.S2D.%03d   ", (int)AI_H2_C_TT715_Stage2SuctionTank, (int)AI_H2_C_TT520_Stage2Discharge);
  sprintf(ln3,"S3S.%03d.S3D.%03d   ", (int)AI_H2_C_TT521_Stage3Suction, (int)AI_H2_C_TT522_Stage3Discharge);

  if(millis() - daughterPrintTimer > dly && daughterPrintTimer){
    daughterPrintTimer = millis();
    bigMatrix[0].writeInt(AI_HYD_psig_PT561_HydraulicInlet2);  // Write local pressure values to 7-seg screen
    if(DO_HYD_XV554_DCV2_A){ smallMatrix[0].displayChar('2', false); }
    else if(DO_HYD_XV557_DCV2_B){ smallMatrix[0].displayChar('1', false); }
    else{ smallMatrix[0].halfMatrix(false,false); }
    bigMatrix[1].writeInt(daughterTog?AI_H2_psig_PT410_Output:AI_H2_psig_PT407_Stage3_Discharge);
    smallMatrix[1].displayChar(daughterTog?'T':'D',false);
    bigMatrix[2].writeInt(AI_H2_psig_PT712_Stage1_DischargeTank);

  }
  if(!daughterPrintTimer2){daughterPrintTimer2 = millis();}
  if(millis() - daughterPrintTimer2 > 2000 && daughterPrintTimer2){
    daughterTog = !daughterTog;
    daughterPrintTimer2 = millis();
    if(scrollCnt < 1){scrollCnt++;}
    else{scrollCnt = 0;}
  }
  
  lcd.setCursor(0, 0);
  if(DI_Comm_LSR_Local || !DI_Encl_ESTOP || STATE == FAULT){
    switch(STATE){
      case IDLE_OFF:
        lcd.print("IDLE: ");
        if(DI_Comm_LSR_Local){lcd.print("LSR Error     ");}
      break;
      case PAUSE:
        lcd.print("PAUSED: ");
        if(DI_Comm_LSR_Local){lcd.print("LSR Error     ");}
      break;
      case FAULT:
        lcd.print("FAULT: ");
        lcd.print(faultString);
        lcd.print("        ");
      break;
      case ESTOP:
        lcd.print("ESTOP! ");
        if(DI_Comm_LSR_Local){lcd.print("LSR Error      ");}
      break;
      default:
      break;
    }
  }
  //else if(!scrollCnt){lcd.print(ln1);}
  else if(scrollCnt == 0){lcd.print(ln2);}
  else if(scrollCnt == 1){lcd.print(ln3);}
  lcd.setCursor(0, 1);
  lcd.print(ln1);
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

//void rs485Transceive(){;}

void i2cTransceive(){
  for(int i = 0; i < TTsize;i++){
    if(TTdata[i].channel == -1){;
     // TTdata[i].mcp.setFilterCoefficient(TTdata[i].coef);
      if(TTdata[i].mcp == 1){
        TTdata[i].rawTemp = mcp1.readThermocouple();
      }
      else if(TTdata[i].mcp == 2){
        TTdata[i].rawTemp = mcp2.readThermocouple();
      }
      else if(TTdata[i].mcp == 3){
        TTdata[i].rawTemp = mcp3.readThermocouple();
      }
      else if(TTdata[i].mcp == 4){
        TTdata[i].rawTemp = mcp4.readThermocouple();
      }
      else if(TTdata[i].mcp == 5){
        TTdata[i].rawTemp = mcp5.readThermocouple();
      }
      else if(TTdata[i].mcp == 6){
        TTdata[i].rawTemp = mcp6.readThermocouple();
      }
      else{;}
      if(0 < TTdata[i].mcp <= 6){
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
      
    if(TTdata[i].max != -1){
      if(*TTdata[i].value >= TTdata[i].max && !TTdata[i].overheat){
        TTdata[i].overheat = true;
        if(STATE == PRODUCTION){STATE = PAUSE;}
      }
      else if(*TTdata[i].value <= TTdata[i].maxRecovery && TTdata[i].overheat){
        TTdata[i].overheat = false;
          if(STATE == PAUSE){STATE = IDLE_ON;}
      }
    }

      if(TTdata[i].min != -1){
        if(*TTdata[i].value <= TTdata[i].min && !TTdata[i].overheat){
          TTdata[i].overheat = true;
          if(STATE == PRODUCTION){STATE = PAUSE;}
        }
        else if(*TTdata[i].value <= TTdata[i].minRecovery && TTdata[i].overheat){
          TTdata[i].overheat = false;
          if(STATE == PAUSE){STATE = IDLE_ON;}
        }
      }
  }
  for(int i = 0; i < PTsize;i++){
    if(PTdata[i].channel != -1){
      PTdata[i].raw = PTdata[i].adc.read(PTdata[i].channel,SD);
      if(PTdata[i].mapA != -1 && PTdata[i].mapB != -1 && PTdata[i].mapC != -1 && PTdata[i].mapD != -1){
        PTdata[i].mapped = map((int)PTdata[i].raw, PTdata[i].mapA, PTdata[i].mapB, PTdata[i].mapC, PTdata[i].mapD) + PTdata[i].offset;
        PTdata[i].avg.addValue(PTdata[i].mapped);
        *PTdata[i].value = PTdata[i].avg.getAverage();
      }
    }
    if(PTdata[i].max != -1){
      if(*PTdata[i].value >= PTdata[i].max && !PTdata[i].overPressure){
        PTdata[i].overPressure = true;
        if(STATE == PRODUCTION){STATE = PAUSE;}
      }
      else if(*PTdata[i].value <= PTdata[i].maxRecovery && PTdata[i].overPressure){
        PTdata[i].overPressure = false;
        if(STATE == PAUSE){STATE = IDLE_ON;}
      }
    }

    if(PTdata[i].min != -1){
      if(*PTdata[i].value < PTdata[i].min && !PTdata[i].overPressure){
        PTdata[i].overPressure = true;
        if(STATE == PRODUCTION){STATE = PAUSE;}
      }
      else if(*PTdata[i].value <= PTdata[i].minRecovery && PTdata[i].overPressure){
        PTdata[i].overPressure = false;
        if(STATE == PAUSE){STATE = IDLE_ON;}
      }
    }
  }

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
  }
  if(!mcp2.begin(0x61)){
    Serial.println("WARNING!! Couldnt detect mcp2(0x61)");
  }
  if(!mcp3.begin(0x62)){
    Serial.println("WARNING!! Couldnt detect mcp3(0x62)");
  }
  if(!mcp4.begin(0x63)){
    Serial.println("WARNING!! Couldnt detect mcp4(0x63)");
  }
  if(!mcp5.begin(0x64)){
    Serial.println("WARNING!! Couldnt detect mcp5(0x64)");
  }
  if(!mcp6.begin(0x65)){
    Serial.println("WARNING!! Couldnt detect mcp6(0x65)");
  }

  mcp1.setADCresolution(MCP9600_ADCRESOLUTION_12);
  mcp2.setADCresolution(MCP9600_ADCRESOLUTION_12);
  mcp3.setADCresolution(MCP9600_ADCRESOLUTION_12);
  mcp4.setADCresolution(MCP9600_ADCRESOLUTION_12);
  mcp5.setADCresolution(MCP9600_ADCRESOLUTION_12);
  mcp6.setADCresolution(MCP9600_ADCRESOLUTION_12);

  mcp1.setThermocoupleType(MCP9600_TYPE_K);
  mcp2.setThermocoupleType(MCP9600_TYPE_K);
  mcp3.setThermocoupleType(MCP9600_TYPE_K);
  mcp4.setThermocoupleType(MCP9600_TYPE_K);
  mcp5.setThermocoupleType(MCP9600_TYPE_K);
  mcp6.setThermocoupleType(MCP9600_TYPE_K);

  mcp1.enable(true);
  mcp2.enable(true);
  mcp3.enable(true);
  mcp4.enable(true);
  mcp5.enable(true);
  mcp6.enable(true);


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
