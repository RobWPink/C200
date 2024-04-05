#include "C100.h"
void setup() {
  Serial.begin(9600);
  //Serial1.begin(19200, SERIAL_8E1);
  //Serial4.begin(19200, SERIAL_8E1);
  pinModeSetup();
  Wire.begin();
  matrixSetup("C100", "V2.2.1");
  i2cSetup();
  // mbLocal.begin(OCI_MODBUS_ID, Serial4);
  // mbLocal.preTransmission(preTransmission);
  // mbLocal.postTransmission(postTransmission);

  dataPrintTimer = millis();
  daughterPrintTimer = millis();
  Serial.println("OK");
  delay(3000);
  printMode = PACKET;
}

void loop() {
  SerialCLI();
  if(lsrReset){
    if(lsrReset == 1){lsrReset = millis();DO_Comm_LSR_Reset = true;}
    if(millis() - lsrReset > 1000){
      DO_Comm_LSR_Reset = false;
      lsrReset = 0;
    }
  }
  //if(c50setup){ DO_HYD_XV554_DCV2_A = DO_H2_XV907_SuctionPreTank; }//added feature for paul
  //rs485Transceive();
  i2cTransceive();
/*
  if((!DI_HYD_LS000_HydraulicLevelSwitch || !DI_CLT_FS000_CoolantFlowSwitch || !DI_HYD_IS000_HydraulicFilterSwitch || !DI_Encl_ESTOP) && !DI_Comm_LSR_Local && lsrError == ""){
    if(!DI_HYD_LS000_HydraulicLevelSwitch){
      lsrError = lsrError + "DI_HYD_LS000_HydraulicLevelSwitch";
    }
    if(!DI_CLT_FS000_CoolantFlowSwitch){
      if(lsrError != ""){lsrError = lsrError + ",";}
      lsrError = lsrError + "DI_CLT_FS000_CoolantFlowSwitch";
    }
    if(!DI_HYD_IS000_HydraulicFilterSwitch){
      if(lsrError != ""){lsrError = lsrError + ",";}
      lsrError = lsrError + "DI_HYD_IS000_HydraulicFilterSwitch";
    }
    if(!DI_Encl_ESTOP){
      if(lsrError != ""){lsrError = lsrError + ",";}
      lsrError = lsrError + "DI_Encl_ESTOP";
    }
    delay(100);
  }
  else if(!DI_Comm_LSR_Local){lsrError = "";}
  else if(DI_Comm_LSR_Local && lsrError == ""){
    lsrError = "Unknown";
  }*/

    
  if(!DI_Encl_ESTOP){STATE = ESTOP;}
  flashDriver();
  
  dataPrint(delayTime);
  daughterPrint(delayTime);

  //variable max psi for discharge3 using suction2
  if(AI_H2_psig_PT712_Stage1_DischargeTank > 1600){PTdata[1].max = 1.9893*AI_H2_psig_PT712_Stage1_DischargeTank + 7521.3;}
  else{PTdata[1].max = 7.1429*AI_H2_psig_PT712_Stage1_DischargeTank - 657.14;}
  if(PTdata[1].max > 0){PTdata[1].maxRecovery = PTdata[1].max - 250;}

  if(STATE != MANUAL_CONTROL){
    DO_Encl_PilotAmber = DI_Comm_LSR_Local;
    DO_CLT_PMP104_PMP204_CoolantPumps_Enable = DO_Comm_LSR_Local;
    DO_CLT_FCU112_CoolantFan1_Enable = DO_Comm_LSR_Local;
    DO_CLT_FCU212_CoolantFan2_Enable = DO_Comm_LSR_Local;
    if(DI_Comm_LSR_Local && (STATE == PRODUCTION || STATE == PAUSE)){
      PREV_STATE = STATE;
      STATE = FAULT;
      faultString = "LSR Error";
    }
  }
  //########################STATE MACHINE################################
  String errDev[30] = {""};
  switch(STATE){
    case IDLE_OFF:
      if(!timer[0]){
        timer[0] = millis();
        smallMatrix[2].displayPlay(false);
        //lcd.setCursor(0, 0);
        //lcd.print("           ");//clear top line
        //lcd.setCursor(0, 0);
        //lcd.print("IDLE: Ready");
        DO_Encl_PilotGreen = true;
        DO_Encl_PilotRed = false;
        DO_Comm_LSR_Local = true;
        tog[0] = false;
        //lcd.setCursor(0, 1);
        //lcd.print("                ");//clear bottom line
        //lcd.setCursor(0, 1);
        //lcd.print("C:   H:   L:   ");
      }
      if(DI_Comm_LSR_Local && !tog[0]){
        tog[0] = true;
        smallMatrix[2].displayPlay(true);
        //lcd.setCursor(0, 0);
        //lcd.print("                ");//clear top line
        //lcd.setCursor(0, 0);
        //lcd.print("IDLE: LSR lock");
        DO_Encl_PilotGreen = false;
      }
      else if(!DI_Comm_LSR_Local && tog[0]){ timer[0] = 0; }
    
      if(DI_Encl_ButtonGreen){
        PREV_STATE = STATE;
        STATE = IDLE_ON;
      }
    break;

  //#####################################################################

    case IDLE_ON:
      if(!timer[0]){
        timer[0] = millis();
        DO_HYD_PMP458_HydraulicPump1_Enable = true; //turn on hydraulic pump 1
        DO_HYD_PMP552_HydraulicPump2_Enable = true; //turn on hydraulic pump 2 
        DO_H2_XV907_SuctionPreTank = true;
        flashGreen = 500;
        //lcd.setCursor(0, 1);
        //lcd.print("                ");//clear bottom line
        //lcd.setCursor(0, 1);
        //lcd.print("C:   H:   L:   ");
      }
      if(millis() - timer[0] > 5000 && timer[0]){ //after 5 seconds
        if(!DI_HYD_IS000_HydraulicFilterSwitch){ //check filter switch
          PREV_STATE = STATE;
          STATE = FAULT;
          faultString = "Filter Switch Error";
        }
        else{PREV_STATE = STATE; STATE = PRODUCTION;}
      }


    break;

  //#####################################################################

    case PRODUCTION:
      if(!timer[0]){
        timer[0] = millis();
        flashGreen = 2000;
        firstHighSide = true;
        DO_HYD_XV460_DCV1_A = false;
        DO_HYD_XV463_DCV1_B = false;
        timer[5] = millis();
        smallMatrix[2].displayPlay(false);
        //lcd.setCursor(0, 0);
        //lcd.print("                ");//clear top line
        //lcd.setCursor(0, 0);
        //lcd.print("Compressing...");
      }
      if(!timer[1]){timer[1] = millis();}
      if(millis() - timer[1] > 60000 && timer[1]){
        cycleCnt = highCycleCnt/2;
        highCycleCnt = 0;
        lowCycleCnt = 0;
        timer[1] = 0;
      }

      //if the red button is held for less than 5 seconds pause it otherwise safe shutdown
      if(DI_Encl_ButtonRed && !prevR){
        prevR = true;
        if(!holdR){holdR = millis();}
      }
      if(!DI_Encl_ButtonRed && prevR){
        prevR = false;
        if(millis() - holdR < 5000 && holdR){
          PREV_STATE = STATE;
          STATE = PAUSE;
          manualPause = true;
        }
        else{
          PREV_STATE = STATE;
          STATE = SHUTDOWN;
        }
        holdR = 0;
      }
      if(firstHighSide){
        DO_HYD_XV554_DCV2_A = true;
        DO_HYD_XV557_DCV2_B = false;
        firstHighSide = false;
        timer[4] = millis();
      }
      if(millis() - timer[4] > 500 && timer[4] && AI_HYD_psig_PT561_HydraulicInlet2 >= switchingPsi1){ //switching pressure
        DO_HYD_XV554_DCV2_A = !DO_HYD_XV554_DCV2_A;
        DO_HYD_XV557_DCV2_B = !DO_HYD_XV557_DCV2_B;
        timer[5] = millis();
        highCycleCnt++;
        timer[4] = millis();
      }
      if(millis() - timer[5] > 60000){
        PREV_STATE = STATE;
        STATE = IDLE_OFF;
        //lcd.setCursor(11, 0);
        //lcd.print("     ");//clear top line
        //lcd.setCursor(11, 0);
        //lcd.print("StkErr");
      }
    break;

  //#####################################################################

    case PAUSE:
      if(!timer[0]){
        timer[0] = millis();
        //flashAmber = 1000;
        for(int i = 0; i < DOsize;i++){
          if((DOdata[i].key.indexOf("XV") >= -1 || DOdata[i].key.indexOf("PMP") >= -1 || DOdata[i].key.indexOf("PL") >= -1) && DOdata[i].key.indexOf("LSR") == -1 && DOdata[i].key.indexOf("CLT") == -1){
            *DOdata[i].value = false;
          }
        }
        smallMatrix[2].displayPause(false);
        //lcd.setCursor(0, 0);
        //lcd.print("                ");//clear top line
        //lcd.setCursor(0, 0);
        //lcd.print(manualPause?"Manual Pause":"Pause");
      }
      j = 0;
      
      for(int i = 0; i < TTsize;i++){
        if(TTdata[i].overheat){
          errDev[j++] = TTdata[i].key + ":" + (int)*TTdata[i].value + " ";
          break;
        }
      }

      for(int i = 0; i < PTsize;i++){
        if(PTdata[i].overPressure){
          errDev[j++] = PTdata[i].key + ":" + (int)*PTdata[i].value + " ";
          break;
        }
      }

      k = 0;
      for(int i = 0; i < sizeof(errDev);i++){
        if(errDev[i] = ""){break;}
        if(errDev[i].length() + errMsg[k].length() - 1 < 16){ //-1 is for space at the end of each
          errMsg[k] = errMsg[k] + errDev[i];
        }
        else{
          errMsg[k][errMsg[k].length() - 1] = '\0'; //remove trailing space
          k++;
        }
      }

      if(!timer[2]){timer[2] = millis();}
      if(millis() - timer[2] > 1000 && timer[2]){
        if(errMsg[l] != ""){
          //lcd.setCursor(0, 1);
          //lcd.print("                ");//clear top line
          //lcd.setCursor(0, 1);
          //lcd.print(errMsg[l]);
          errMsg[l++] = "";
        }
        else{
          l = 0;
          //lcd.setCursor(0, 1);
          //lcd.print("                ");//clear top line
          //lcd.setCursor(0, 1);
          //lcd.print("No Errors");
        }
        
        timer[2] = millis();
      }
      if(manualPause){
        if(DI_Encl_ButtonGreen && errDev[0] == ""){
          STATE = PREV_STATE;
          PREV_STATE = PAUSE;
          timer[0] = 0;
        }
      }

    break;

  //#####################################################################

    case SHUTDOWN:
      if(!timer[0]){
        timer[0] = millis();
        for(int i = 0; i < DOsize;i++){
          if((DOdata[i].key.indexOf("XV") >= -1 || DOdata[i].key.indexOf("PMP") >= -1 || DOdata[i].key.indexOf("PL") >= -1) && DOdata[i].key.indexOf("LSR") == -1 && DOdata[i].key.indexOf("CLT") == -1){
            *DOdata[i].value = false;
          }
        }

        smallMatrix[2].displayStop(false);
        //lcd.setCursor(0, 0);
        //lcd.print("                ");//clear top line
        //lcd.setCursor(0, 0);
        //lcd.print("Shutting Down...");
      }
      
      if(millis() - timer[0] > 30000 && timer[0]){
        for(int i = 0; i < DOsize;i++){
          if((DOdata[i].key.indexOf("XV") >= -1 || DOdata[i].key.indexOf("PMP") >= -1 || DOdata[i].key.indexOf("PL") >= -1) && DOdata[i].key.indexOf("CLT") == -1){
            *DOdata[i].value = false;
          }
        }
        timer[0] = 0;
        timer[1] = 0;
        timer[3] = 0;
        timer[4] = 0;
        DO_CLT_PMP104_PMP204_CoolantPumps_Enable = false;
        PREV_STATE = STATE;
        STATE = IDLE_OFF;
      }
      //if pressure lowers and temps lower shut down the rest
    break;

  //#####################################################################

    case MANUAL_CONTROL:
      if(!timer[0]){
        timer[0] = millis();
        smallMatrix[2].displayChar('M', false);
        //lcd.setCursor(0, 0);
        //lcd.print("                ");//clear top line
        //lcd.setCursor(0, 0);
        //lcd.print("MANUAL MODE");
        Serial.println("manual");
      }
    break;

  //#####################################################################

    case FAULT:
      if(!timer[0]){
        timer[0] = millis();
        for(int i = 0; i < DOsize;i++){
          *DOdata[i].value = false;
        }
        flashRed = 1000;

        smallMatrix[2].displayStop(true);
        //lcd.setCursor(0, 0);
        //lcd.print("                ");//clear top line
        //lcd.setCursor(0, 0);
        //lcd.print("Faulted...");
        //lcd.setCursor(0, 1);
        //lcd.print("                ");//clear top line
        //lcd.setCursor(0, 1);
        //lcd.print(faultString);
      }
      
      if(DI_Encl_ButtonRed){PREV_STATE = STATE; STATE = IDLE_OFF;}
    break;

  //#####################################################################

    case ESTOP:
      if(!timer[0]){
        timer[0] = millis();
        for(int i = 0; i < DOsize;i++){
          *DOdata[i].value = false;
        }
        DO_Encl_PilotRed = true;
        //flashRed = 200;
        smallMatrix[2].displayStop(false);
        //lcd.setCursor(0, 0);
        //lcd.print("                ");//clear top line
        //lcd.setCursor(0, 0);
        //lcd.print("ESTOP!");
        //DO_Comm_LSR_Local = false;
      }
      for(int i = 0; i < DOsize;i++){
        if(DOdata[i].key.indexOf("PL") == -1){
          *DOdata[i].value = false;
        }
      }
      if(DI_Encl_ESTOP){DO_Comm_LSR_Local = true;STATE = IDLE_OFF;}
    break;

  //#####################################################################

    default:
      if(errorPrint){Serial.println("STATE ERROR!!!!!");}
      break;
  }

  if(STATE != CHANGED_STATE){ //reset state timer
    timer[0] = 0;
    timer[1] = 0;
    timer[3] = 0;
    timer[4] = 0;
    timer[5] = 0;

    DO_Encl_PilotRed = false;
    DO_Encl_PilotGreen = false;
    DO_Encl_PilotAmber = false;

    flashGreen = 0;
    flashRed = 0;
    flashAmber = 0;

    CHANGED_STATE = STATE;
  }
}