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
      // for(int i = 0; i < VARsize; i++){
      //   if(argStr.equalsIgnoreCase(VARdata[i].key)){
      //     String argStrVal = argBuf[++n];
      //     argVal = argStrVal.toDouble();
      //     *VARdata[i].value = argVal;
      //     digital = true;
      //     break;
      //   }
      // }
      if(digital){;}

      else if(argStr.equalsIgnoreCase("delay")) {
        String argStrVal = argBuf[++n];
        argVal = argStrVal.toInt();
        if(argVal > 0){delayTime = argVal;}
      }

      else if(argStr.equalsIgnoreCase("ratio")) {
        String argStrVal = argBuf[++n];
        Stage1_Compression_RATIO = argStrVal.toDouble();
      }

      else if(argStr.equalsIgnoreCase("swtmlow")) {
        String argStrVal = argBuf[++n];
        switchingTimeLow_Override = argStrVal.toDouble();
      }
      else if(argStr.equalsIgnoreCase("swtmhigh")) {
        String argStrVal = argBuf[++n];
        switchingTimeHigh_Override = argStrVal.toDouble();
      }

      else if(argStr.equalsIgnoreCase("swpsi1a")) {
        String argStrVal = argBuf[++n];
        switchingPsi1A_Override = argStrVal.toDouble();
      }
      else if(argStr.equalsIgnoreCase("swpsi1b")) {
        String argStrVal = argBuf[++n];
        switchingPsi1B_Override = argStrVal.toDouble();
      }
      else if(argStr.equalsIgnoreCase("swpsi2a")) {
        String argStrVal = argBuf[++n];
        switchingPsi2A_Override = argStrVal.toDouble();
      }
      else if(argStr.equalsIgnoreCase("swpsi2b")) {
        String argStrVal = argBuf[++n];
        switchingPsi2B_Override = argStrVal.toDouble();
      }
   
      else if(argStr.equalsIgnoreCase("help") || argStr.equalsIgnoreCase("h")){
        printHelp();
      }
      
      else if(argStr.equalsIgnoreCase("raw")){
        rawPrint = !rawPrint;
      }

      else if(argStr.equalsIgnoreCase("packet")){
        printMode = (printMode == PACKET)?NONE:PACKET;
      }

      else if(argStr.equalsIgnoreCase("di")){
        printMode = (printMode == DIGITAL_IN)?NONE:DIGITAL_IN;
      }

      else if(argStr.equalsIgnoreCase("do")){
        printMode = (printMode == DIGITAL_OUT)?NONE:DIGITAL_OUT;
      }

      else if(argStr.equalsIgnoreCase("tt")){
        printMode = (printMode == TT)?NONE:TT;
      }

      else if(argStr.equalsIgnoreCase("pt")){
        printMode = (printMode == PT)?NONE:PT;
      }

      else if(argStr.equalsIgnoreCase("var")){
        printMode = (printMode == VAR)?NONE:VAR;
      }

      else if(argStr.equalsIgnoreCase("fm")){
        printMode = (printMode == FM)?NONE:FM;
      }
      else if(argStr.equalsIgnoreCase("gbn")){
        virtualGreenButton = 1;
      }
      else if(argStr.equalsIgnoreCase("abn")){
        virtualAmberButton = 1;
      }
      else if(argStr.equalsIgnoreCase("rbn")){
        virtualRedButton = 1;
      }

      else if(argStr.equalsIgnoreCase("pauselow")){
        tog[2] = !tog[2];
      }
      else if(argStr.equalsIgnoreCase("pausehigh")){
        tog[3] = !tog[3];
      }

      else if(argStr.equalsIgnoreCase("estop")){
        STATE = ESTOP;
      }

      else if(argStr.equalsIgnoreCase("debug")){
        printMode = (printMode == DEBUG)?NONE:DEBUG;
      }

      else if(argStr.equalsIgnoreCase("plot")){
        plot = !plot;
      }

      else if(argStr.equalsIgnoreCase("err")){
        Serial.println(faultString);
        for(int h = 0; h < 29; h++){
          Serial.print(errMsg[h]);
          if(errMsg[h+1] != ""){Serial.print(",");}
        }
        Serial.println();
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

      else if(argStr.equalsIgnoreCase("stop") || argStr.equalsIgnoreCase("quiet") || argStr.equalsIgnoreCase("q")){
        printMode = NONE;
      }

      else{
        Serial.println("Invalid Entry, type \"help\" or \"h\" for a list of commands.");
      }
    }
  }
}

void printHelp(){
  Serial.println("################## HELP MENU ##################");
  Serial.println("packet             -> Print data in standardized packet format");
  Serial.println("di                 -> Print all gpio digital inputs");
  Serial.println("do                 -> Print all gpio digital outputs");
  Serial.println("pt                 -> Print all PT values");
  Serial.println("tt                 -> Print all TT values");
  Serial.println("fm                 -> Print all Flow meter values");
  Serial.println("var                -> Print all user adjustable variables");
  Serial.println("gbn                -> Simulate Green Button Push");
  Serial.println("abn                -> Simulate Amber Button Push");
  Serial.println("rbn                -> Simulate Red Button Push");
  Serial.println("estop              -> Simulate ESTOP Button Push");
  Serial.println("pretty             -> Toggle print mode labeled lists <--> csv list");
  Serial.println("raw                -> Toggle to print only raw values");
  Serial.println("plot               -> Toggle hydraulic PSI plotting for Arduino Serial Plotter");
  Serial.println("stop/quiet/q       -> Silence all printouts");
  for(int i = 0; i < DOsize; i++){
    String cc = DOdata[i].key;
    for(int b = 1;b < (19 - cc.length());b++){
      cc = cc + " ";
    }
    cc = cc + "-> Toggle " + DOdata[i].name;
    Serial.println(cc);
  }
  for(int i = 0; i < VARsize; i++){
    String cc = VARdata[i].key;
    for(int b = 1;b < (19 - cc.length());b++){
      cc = cc + " ";
    }
    cc = cc + "-> Change " + VARdata[i].name + " (Default: " + VARdata[i].defaultValue + ")";
    Serial.println(cc);
  }
  Serial.println("manual             -> Control all output manually");
  Serial.println("help/h             -> This help menu");
  Serial.println("###############################################");

}

void dataPrint(unsigned long dly){
  if(!dataPrintTimer){dataPrintTimer = millis();}
  
  if(plot){
  //   String msgA = "";
  //   String msgB = "";
  //   String msgC = "";
  //   String msgD = "";
  //   String msgE = "";
  //   String msgF = "";
  //   String msgG = "\"STATE\":" + String(STATE) + "," + "\"INT_STATE1\":" + String(SUB_STATE1) + "," + "\"INT_STATE2\":" + String(SUB_STATE2) + "," + "\"SUB_STATE1\":" + String(SUBSTATE1) + "," + "\"SUB_STATE1\":" + String(SUBSTATE2);
  //   for(int i = 0; i < TTsize;i++){
  //     msgA = msgA + "\"" + TTdata[i].key + "\":" + *TTdata[i].value + ",";
  //   }
  //   for(int i = 0; i < PTsize;i++){
  //     msgB = msgB + PTdata[i].key + ":" + *PTdata[i].value + ",";
  //   }
  //   for(int i = 0; i < 2;i++){
  //     for(int j = 0; j < 4;j++){
  //       msgC = msgC + "\"" + flowMeters[i].flowData[j].key + "\"" + ":" + *flowMeters[i].flowData[j].value + ",";
  //     }
  //   }
  //   for(int i = 0; i < VARsize;i++){
  //     msgD = msgD + "\"" + VARdata[i].key + "\"" + ":" + *VARdata[i].value + ",";
  //   }
  //   for(int i = 0; i < DOsize;i++){
  //     msgE = msgE + "\"" + DOdata[i].key + "\"" + ":" + *DOdata[i].value + ",";
  //   }
  //   for(int i = 0; i < DIsize;i++){
  //     msgF = msgF + "\"" + DIdata[i].key + "\"" + ":" + *DIdata[i].value + ",";
  //   }
  //   msgF.remove(msgF.length()-1,1);//get rid of extra comma

  //   Serial.print(msgA);
  //   Serial.print(msgB);
  //   Serial.print(msgC);
  //   Serial.print(msgD);
  //   Serial.print(msgE);
  //   Serial.print(msgF);
  //   Serial.println(msgG);
  //   Serial.print("DCV1A:");
  // Serial.print(DO_HYD_XV460_DCV1_A*switchingPsi1A);
  // Serial.print(",");
  // Serial.print("DCV1B:");
  // Serial.print(DO_HYD_XV463_DCV1_B*switchingPsi1B);
  // Serial.print(",");
  // Serial.print("DCV2A:");
  // Serial.print(DO_HYD_XV554_DCV2_A*switchingPsi2A);
  // Serial.print(",");
  // Serial.print("DCV2B:");
  // Serial.print(DO_HYD_XV557_DCV2_B*switchingPsi2B);
  // Serial.print(",");
  Serial.print("accelLow:");
  Serial.print(accelLow);
  Serial.print(",");
  Serial.print("accelHigh:");
  Serial.print(accelHigh);
  Serial.print(",");
  Serial.print("low:");
  Serial.print(AI_HYD_psig_PT467_HydraulicInlet1);
  Serial.print(",");
  Serial.print("high:");
  Serial.println(AI_HYD_psig_PT561_HydraulicInlet2);
  }
  else if(millis() - dataPrintTimer > dly && dataPrintTimer){
    if(!printMode){return;}
    String msg = "{";
    switch(printMode){
      case PACKET:
       // msg = msg + "\"STATE\":" + STATE + "," + "\"INT_STATE1\":" + SUB_STATE1 + "," + "\"INT_STATE2\":" + SUB_STATE2 + "," + "\"SUB_STATE1\":" + SUBSTATE1 + "," + "\"SUB_STATE1\":" + SUBSTATE2;
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
        
        for(int i = 0; i < 2; i++){
          for(int j = 0; j < 4; j++){
            if(*flowMeters[i].flowData[j].value != flowMeters[i].flowData[j].prev){
              if(msg != "{"){msg = msg + ",";}
              msg = msg + "\"" + flowMeters[i].flowData[j].key + "\":" + *flowMeters[i].flowData[j].value;
              flowMeters[i].flowData[j].prev = *flowMeters[i].flowData[j].value;
            }
          }
        }

        for(int i = 0; i < VARsize;i++){
          if(*VARdata[i].value != VARdata[i].prev){
            if(msg != "{"){msg = msg + ",";}
            msg = msg + "\"" + VARdata[i].key + "\":" + *VARdata[i].value;
            VARdata[i].prev = *VARdata[i].value;
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

      case VAR:
        for(int i = 0; i < VARsize; i++){
          if(prettyPrint){
            Serial.print("\"");
            Serial.print(VARdata[i].name);
            Serial.print("\": ");
          }
          Serial.print(*VARdata[i].value);
          if(prettyPrint){ Serial.println(); }
          else{ Serial.print(", "); }
        }
        Serial.println();
      break;

      case FM:
        for(int i = 0; i < 2; i++){
          for(int j = 0; j < 4; j++){
            if(prettyPrint){
              Serial.print("\"");
              Serial.print(flowMeters[i].flowData[j].name);
              Serial.print("\": ");
            }
            Serial.print(*flowMeters[i].flowData[j].value);
            if(prettyPrint){ Serial.println(); }
            else{ Serial.print(", "); }
          }
        }
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
    
    case DEBUG:
      Serial.print("LoopTime: ");
      Serial.println(loopTime);
      // Serial.print("FlowMeter:");
      // for(int i = 0; i < 2; i++){
      //   for(int j = 0; j < 4; j++){
      //     Serial.print(*flowMeters[i].flowData[j].value);
      //     Serial.print(",");
      //   }
      //   if(i < 1){Serial.print("|||");}
      // }
      Serial.println();
      Serial.print("STATE: ");
      Serial.println(STATE);

      Serial.print("stateHistory1: ");
      if(stateHistory1.length() > 30){
        stateHistory1.remove(0,3);
      }
      Serial.println(stateHistory1);

      Serial.print("stateHistory2: ");
      if(stateHistory2.length() > 30){
        stateHistory2.remove(0,3);
      }
      Serial.println(stateHistory2);

      Serial.print("SUB_STATE timers: ");
      Serial.print(millis() - timer[2]);Serial.print(", ");
      Serial.println(millis() - timer[3]);

      Serial.print("DCV:1A,1B,2A,2B:");
      Serial.print(DO_HYD_XV460_DCV1_A);
      Serial.print(DO_HYD_XV463_DCV1_B);
      Serial.print(DO_HYD_XV554_DCV2_A);
      Serial.println(DO_HYD_XV557_DCV2_B);

      Serial.print("S1_ratio: ");
      Serial.println(Stage1_Compression_RATIO);

      Serial.print("switchingPSI:1A/1B/2A/2B: ");
      Serial.print(switchingPsi1A);Serial.print(", ");
      Serial.print(switchingPsi1B);Serial.print(", ");
      Serial.print(switchingPsi2A);Serial.print(", ");
      Serial.println(switchingPsi2B);

      Serial.print("switchingTime:Low/High: ");
      Serial.print(switchingTimeLow);Serial.print(", ");
      Serial.println(switchingTimeHigh);

      Serial.print("UnderPressure:");
      for(int i = 0; i < PTsize; i++){
        Serial.print(PTdata[i].underPressure?1:0);
      }
      Serial.println();

      Serial.print("OverPressure:");
      for(int i = 0; i < PTsize; i++){
        Serial.print(PTdata[i].overPressure?1:0);
      }
      Serial.println();

      Serial.print("CPM: ");
      Serial.print(CPMlow);Serial.print(", ");
      Serial.println(CPMhigh);

      Serial.print("TTmax: ");
      Serial.print(lowMax);Serial.print(", ");
      Serial.println(highMax);

      Serial.print("TTDelay: ");
      Serial.print(CPMlowTemp);Serial.print(", ");
      Serial.println(CPMhighTemp);

      Serial.print("Stage1PT Max: ");
      Serial.println(PTdata[2].max);

      Serial.print("inlets: ");
      Serial.print(AI_HYD_psig_PT467_HydraulicInlet1);Serial.print(", ");
      Serial.println(AI_HYD_psig_PT561_HydraulicInlet2);

      Serial.print("suctions: ");
      Serial.print(AI_H2_psig_PT911_Stage1_SuctionTank);Serial.print(", ");
      Serial.println(AI_H2_psig_PT712_Stage1_DischargeTank);

      Serial.print("discharges: ");
      Serial.print(AI_H2_psig_PT716_Stage1_Discharge);Serial.print(", ");
      Serial.print(AI_H2_psig_PT519_Stage2_Discharge);Serial.print(", ");
      Serial.print(AI_H2_psig_PT407_Stage3_Discharge);Serial.print(", ");
      Serial.println(AI_H2_psig_PT410_Stage3_DischargeTank);
      
    break;

    default:
    break;
    }
    dataPrintTimer = millis();
  }
}
 