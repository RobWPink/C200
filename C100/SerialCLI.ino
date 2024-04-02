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
      else if(argStr.equalsIgnoreCase("delay")) {
        String argStrVal = argBuf[++n];
        argVal = argStrVal.toInt();
        if(argVal > 0){delayTime = argVal;}
      }
   
      else if(argStr.equalsIgnoreCase("help") || argStr.equalsIgnoreCase("h")){
        printHelp();
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
      else if(argStr.equalsIgnoreCase("plot")){
        plot = !plot;
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
  Serial.println("pretty             -> Toggle print mode labeled lists <--> csv list");
  //Serial.println("keys               -> Toggle print mode labeled keys <--> csv list");
  Serial.println("raw                -> Toggle to print only raw values");
  Serial.println("stop               -> Silence all printouts");
  Serial.println("quiet              -> Silence all printouts");
  Serial.println("manual             -> Control all output manually");
  Serial.println("help/h             -> This help menu");
  Serial.println("###############################################");

}

void dataPrint(unsigned long dly){
  if(plot){
    Serial.print(AI_HYD_psig_PT561_HydraulicInlet2);
    Serial.print(" ");
    Serial.println(DO_HYD_XV554_DCV2_A*2000);
  }
  else if(millis() - dataPrintTimer > dly && dataPrintTimer){
    if(!printMode){return;}
    String msg = "{";
    switch(printMode){
      case PACKET:
        for(int i = 0; i < TTsize;i++){
          if(*TTdata[i].value != prevTT[i]){
            if(msg != "{"){msg = msg + ",";}
            msg = msg + "\"" + TTdata[i].key + "\":" + *TTdata[i].value;
            prevTT[i] = *TTdata[i].value;
          }
        }

        for(int i = 0; i < PTsize;i++){
          if(*PTdata[i].value != prevPT[i]){
            if(msg != "{"){msg = msg + ",";}
            msg = msg + "\"" + PTdata[i].key + "\":" + *PTdata[i].value;
            prevPT[i] = *PTdata[i].value;
          }
        }

        for(int i = 0; i < DOsize;i++){
          if(*DOdata[i].value != prevDO[i]){
            if(msg != "{"){msg = msg + ",";}
            msg = msg + "\"" + DOdata[i].key + "\":" + *DOdata[i].value;
            prevDO[i] = *DOdata[i].value;
          }
        }

        for(int i = 0; i < DIsize;i++){
          if(*DIdata[i].value != prevDI[i]){
            if(msg != "{"){msg = msg + ",";}
            msg = msg + "\"" + DIdata[i].key + "\":" + *DIdata[i].value;
            prevDI[i] = *DIdata[i].value;
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
    
    default:
    break;
    }
    dataPrintTimer = millis();
  }
}
 