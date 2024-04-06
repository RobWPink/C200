#define SPTR_SIZE   20
void SerialCLI() {
  String str;
  char *argBuf[20] = { 0 };
  double argVal = 0;
  while (Serial.available()) { str = Serial.readStringUntil('\n'); }

  if (str.length() > 0) {
    
    //Parse string function wasn't working on GIGA
    char s[100] = { 0 };
    int numArgs = 0;
    int size = sizeof(argBuf);
    strcpy(s, str.c_str());
    argBuf[0] = strtok(s, " ");
    //Serial.print(argBuf[0]);
    for (numArgs = 1; NULL != (argBuf[numArgs] = strtok(NULL, " ")); numArgs++) {
      //Serial.println(argBuf[numArgs] );
      if (size == numArgs) { break; }
    }
    
    for (int n = 0; n < numArgs; n++) {
      String argStr = argBuf[n];
      int digital = false;
      for(int i = 0; i < DOsize; i++){
        if(argStr.equalsIgnoreCase(DOdata[i].key)){
          String argStrVal = argBuf[++n];
          if(argStrVal.equalsIgnoreCase("false")){
            *DOdata[i].value = false;
          }
          else if(argStrVal.equalsIgnoreCase("true")){
            *DOdata[i].value = true;
          }
          else{ *DOdata[i].value = !*DOdata[i].value; }
          digital = true;
          break;
        }
      }
      if(digital){;}

       else if(argStr.equalsIgnoreCase("sw1")) {
        String argStrVal = argBuf[++n];
        argVal = argStrVal.toInt();
        if(argVal > 0){switchingPsi1 = argVal;}
      }

      else if(argStr.equalsIgnoreCase("sw2")) {
        String argStrVal = argBuf[++n];
        argVal = argStrVal.toInt();
        if(argVal > 0){switchingPsi1 = argVal;}
      }

      else if(argStr.equalsIgnoreCase("delay")) {
        String argStrVal = argBuf[++n];
        argVal = argStrVal.toInt();
        if(argVal > 0){delayTime = argVal;}
      }
   
      else if(argStr.equalsIgnoreCase("help") || argStr.equalsIgnoreCase("h")){
        printHelp();
      }

      else if(argStr.equalsIgnoreCase("lsr")){
        lsrReset = 1;
      }
      
      else if(argStr.equalsIgnoreCase("c50setup")){
        c50setup = !c50setup;
      }
      else if(argStr.equalsIgnoreCase("raw")){
        rawPrint = !rawPrint;
      }

      else if(argStr.equalsIgnoreCase("packet")){
        printMode = (printMode == PACKET)?OFF:PACKET;
      }

      else if(argStr.equalsIgnoreCase("alldata")){
        printMode = (printMode == ALL)?OFF:ALL;
      }

      else if(argStr.equalsIgnoreCase("di")){
        printMode = (printMode == DIGITAL_IN)?OFF:DIGITAL_IN;
      }

      else if(argStr.equalsIgnoreCase("do")){
        printMode = (printMode == DIGITAL_OUT)?OFF:DIGITAL_OUT;
      }
      else if(argStr.equalsIgnoreCase("tt")){
        printMode = (printMode == TT)?OFF:TT;
      }

      else if(argStr.equalsIgnoreCase("pt")){
        printMode = (printMode == PT)?OFF:PT;
      }

      else if(argStr.equalsIgnoreCase("debug")){
        printMode = (printMode == debug)?OFF:debug;
      }

      else if(argStr.equalsIgnoreCase("plot")){
        plot = !plot;
      }
      else if(argStr.equalsIgnoreCase("err")){
        if(errMsg[0] == ""){Serial.println("No PAUSE Errors");}
        else{
          for(int h = 0; h < 30; h++){
            if(errMsg[h] != ""){Serial.print(errMsg[h]);}
            else{break;}
          }
        }
      }
        
      else if(argStr.equalsIgnoreCase("pretty")){
        prettyPrint = !prettyPrint;
      }
      else if(argStr.equalsIgnoreCase("manual")){
        manualMode = !manualMode;
        if(manualMode){
          PREV_STATE = STATE;
          STATE = MANUAL_CONTROL;
        }
        else{
          STATE = PREV_STATE;
          PREV_STATE = IDLE_OFF;
        }
      }

      // else if(argStr.equalsIgnoreCase("keys")){
      //   keyPrint = !keyPrint;
      // }

      else if(argStr.equalsIgnoreCase("stop") || argStr.equalsIgnoreCase("quiet")){
        printMode = OFF;
      }

      else{
        Serial.println("Invalid Entry");
      }
    }
  }
}

void printHelp(){
  Serial.println("################## HELP MENU ##################");
  for(int i = 0; i < DOsize; i++){
    String cc = DOdata[i].key;
    for(int b = 1;b < (19 - cc.length());b++){
      cc = cc + " ";
    }
    cc = cc + "-> Toggle " + DOdata[i].name;
    Serial.println(cc);
  }
  Serial.println("off                -> Turn off ALL devices");
  Serial.println("packet             -> Print data in standardized packet format");
  Serial.println("di                 -> Print all gpio digital inputs");
  Serial.println("do                 -> Print all gpio digital outputs");
  Serial.println("pt                 -> Print all PT values");
  Serial.println("tt                 -> Print all TT values");
  Serial.println("printall           -> Print all data");
  Serial.println("delay              -> Designate print delay time in ms (default: 1000)");
  Serial.println("sw1                -> Change switching pressure 1");
  Serial.println("sw2                -> Change switching pressure 2");
  Serial.println("lsr                -> Virtually press lsr reset button for 500ms");
  Serial.println("c50setup           -> paul's feature");
  Serial.println("pretty             -> Toggle print mode labeled lists <--> csv list");
  //Serial.println("keys               -> Toggle print mode labeled keys <--> csv list");
  Serial.println("raw                -> Toggle to print only raw values");
  Serial.println("plot               -> Toggle Plotting of all values using Arduino Serial Plotter");
  Serial.println("stop/quiet         -> Silence all printouts");
  Serial.println("manual             -> Control all output manually");
  Serial.println("help/h             -> This help menu");
  Serial.println("###############################################");

}

void dataPrint(unsigned long dly){
  if(!dataPrintTimer){dataPrintTimer = millis();}
  if(plot){
    Serial.print("AI_HYD_psig_PT561_HydraulicInlet2:");
    Serial.print(AI_HYD_psig_PT561_HydraulicInlet2);
    Serial.print(" DO_HYD_XV554_DCV2_A:");
    Serial.print(DO_HYD_XV554_DCV2_A*2000);
    Serial.print(" DO_HYD_XV557_DCV2_B:");
    Serial.print(DO_HYD_XV557_DCV2_B*2000);
    Serial.print(" SwitchingPressure:");
    Serial.print(switchingPsi1);
    Serial.print(" AI_H2_psig_PT712_Stage1_DischargeTank:");
    Serial.print(AI_H2_psig_PT712_Stage1_DischargeTank);
    Serial.print(" AI_H2_psig_PT407_Stage3_Discharge:");
    Serial.print(AI_H2_psig_PT407_Stage3_Discharge);
    Serial.print(" AI_H2_psig_PT410_Output:");
    Serial.print(AI_H2_psig_PT410_Output);
    Serial.print(" AI_HYD_C_TT454_HydraulicTank:");
    Serial.print(AI_HYD_C_TT454_HydraulicTank);
    Serial.print(" AI_CLT_C_TT207_CoolantSupply2:");
    Serial.print(AI_CLT_C_TT207_CoolantSupply2);
    Serial.print(" AI_H2_C_TT715_Stage2SuctionTank:");
    Serial.print(AI_H2_C_TT715_Stage2SuctionTank);
    Serial.print(" AI_H2_C_TT520_Stage2Discharge:");
    Serial.print(AI_H2_C_TT520_Stage2Discharge);
    Serial.print(" AI_H2_C_TT521_Stage3Suction:");
    Serial.println(AI_H2_C_TT521_Stage3Suction);
    Serial.print(" AI_H2_C_TT522_Stage3Discharge:");
    Serial.println(AI_H2_C_TT522_Stage3Discharge);
  }
  else if(millis() - dataPrintTimer > dly && dataPrintTimer){
    if(!printMode){return;}
    String msg = "{";
    switch(printMode){
      case PACKET:
        for(int i = 0; i < TTsize;i++){
          if(*TTdata[i].value != TTdata[i].prev){
            if(msg != "{"){msg = msg + ",";}
            msg = msg + "\"" + TTdata[i].key + "\":" + *TTdata[i].value;
            TTdata[i].prev = *TTdata[i].value;
          }
        }

        for(int i = 0; i < PTsize;i++){
          if(*PTdata[i].value != PTdata[i].prev){
            if(msg != "{"){msg = msg + ",";}
            msg = msg + "\"" + PTdata[i].key + "\":" + *PTdata[i].value;
            PTdata[i].prev = *PTdata[i].value;
          }
        }

        for(int i = 0; i < DOsize;i++){
          if(*DOdata[i].value != DOdata[i].prev){
            if(msg != "{"){msg = msg + ",";}
            msg = msg + "\"" + DOdata[i].key + "\":" + *DOdata[i].value;
            DOdata[i].prev = *DOdata[i].value;
          }
        }

        for(int i = 0; i < DIsize;i++){
          if(*DIdata[i].value != DIdata[i].prev){
            if(msg != "{"){msg = msg + ",";}
            msg = msg + "\"" + DIdata[i].key + "\":" + *DIdata[i].value;
            DIdata[i].prev = *DIdata[i].value;
          }
        }
        for(int i = 0; i < varSize;i++){
          if(*varData[i].value != varData[i].prev){
            if(msg != "{"){msg = msg + ",";}
            msg = msg + "\"" + varData[i].key + "\":" + *varData[i].value;
            varData[i].prev = *varData[i].value;
          }
        }
        if(msg != "{"){
          msg = msg + "}";
          Serial.println(msg);
        }
      break;

      case TT:
        for(int i = 0; i < TTsize; i++){
          if(prettyPrint){
            Serial.print("\"");
            Serial.print(TTdata[i].name);
            Serial.print("\": ");
          }
          Serial.print(rawPrint?TTdata[i].raw:*TTdata[i].value);
          if(prettyPrint){ Serial.println(); }
          else{ Serial.print(", "); }
        }
        Serial.println();
      break;
      
      case PT:
        for(int i = 0; i < PTsize; i++){
          if(prettyPrint){
            Serial.print("\"");
            Serial.print(PTdata[i].name);
            Serial.print("\": ");
          }
          Serial.print(rawPrint?PTdata[i].raw:*PTdata[i].value);
          if(prettyPrint){ Serial.println(); }
          else{ Serial.print(", "); }
        }
        Serial.println();
      break;

      case DIGITAL_IN:
        for(int i = 0; i < DIsize; i++){
          if(prettyPrint){
            Serial.print("\"");
            Serial.print(DIdata[i].name);
            Serial.print("\": ");
          }
          Serial.print(*DIdata[i].value);
          if(prettyPrint){ Serial.println(); }
          else{ Serial.print(", "); }
        }
        Serial.println();
      break;

      case DIGITAL_OUT:
        for(int i = 0; i < DOsize; i++){
          if(prettyPrint){
            Serial.print("\"");
            Serial.print(DOdata[i].name);
            Serial.print("\": ");
          }
          Serial.print(*DOdata[i].value);
          if(prettyPrint){ Serial.println(); }
          else{ Serial.print(", "); }
        }
        Serial.println();
      break;
    
    case debug:
      Serial.print("LoopTime: ");
      Serial.println(loopTime);
      
      Serial.print("firstHighSide: ");
      Serial.println(firstHighSide);

      Serial.print("timer4: ");
      Serial.println(timer[4]);

      Serial.print("DO_HYD_XV554_DCV2_A: ");
      Serial.println(DO_HYD_XV554_DCV2_A);

      Serial.print("DO_HYD_XV557_DCV2_B: ");
      Serial.println(DO_HYD_XV557_DCV2_B);

      Serial.print("hydraulic/switching: ");
      Serial.print(AI_HYD_psig_PT561_HydraulicInlet2);
      Serial.print(" / ");
      Serial.println(switchingPsi1);

      Serial.print("STATE: ");
      Serial.println(STATE);

      Serial.print("suction: ");
      Serial.println(AI_H2_psig_PT712_Stage1_DischargeTank);
      Serial.print("max discharge3: ");
      Serial.println(PTdata[1].max);
    break;

    default:
    break;
    }
    dataPrintTimer = millis();
  }
}
 