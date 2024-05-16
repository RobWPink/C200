#include "C200.h"
void setup() {
  Serial.begin(9600);
  RPC.begin(); //boots M4
  pinModeSetup();
  Wire.begin();
  matrixSetup("C200v2_Longview", "V0.6.3");
  i2cSetup();
  Serial.println("OK");
  delay(3000);
  printMode = PACKET;
  DO_Comm_LSR_Local = true;
}

void loop() {
  DO_Comm_LSR_Local = true; //not used but needs to be true
  if(!loopTimer){loopTimer = millis();}

  SerialCLI();
  RPCtransceive();
  
//############# Buttons #############
  if(virtualAmberButton){ //button press for LSR reset
    if(virtualAmberButton == 1){virtualAmberButton = millis();DO_Comm_LSR_Reset = true;}
    if(millis() - virtualAmberButton > 1000){
      DO_Comm_LSR_Reset = false;
      virtualAmberButton = 0;
    }
  }

  if(virtualGreenButton){ //button press for green button
    if(virtualGreenButton == 1){virtualGreenButton = millis();}
    if(millis() - virtualGreenButton > 1000){
      virtualGreenButton = 0;
    }
  }

  if(virtualRedButton){ //button press for green button
    if(virtualRedButton == 1){virtualRedButton = millis();}
    if(millis() - virtualRedButton > 1000){
      virtualRedButton = 0;
    }
  }
  //#################################
  //rs485Transceive();
  i2cTransceive(250);
    
  if(!DI_Encl_ESTOP){STATE = ESTOP;}
  if((STATE == PRODUCTION || STATE == IDLE_ON) && !DI_CLT_FS000_CoolantFlowSwitch){STATE = FAULT; faultString = "Coolant Flow Error";}
  flashDriver();
  
  dataPrint(delayTime);
  daughterPrint(delayTime);

  //never go above 6.5/1 s1s/s2s
  PTdata[2].max = AI_H2_psig_PT911_Stage1_SuctionTank*Stage1_Compression_RATIO;
  PTdata[2].max = (PTdata[2].max > 1400)?1400:PTdata[2].max;
  PTdata[2].maxRecovery =  PTdata[2].max - 200;

  lowMax = mcpExist?max(AI_H2_C_TT917_Stage1_SuctionTank, max(AI_H2_C_TT701_Stage1_DischargePreTank, max(AI_H2_C_TT809_Stage1_Discharge1, AI_H2_C_TT810_Stage1_Discharge2))):0;
  lowMax = (lowMax > 800)?0:lowMax;

  highMax = mcpExist?max(AI_H2_C_TT715_Stage2_SuctionTank,max(AI_H2_C_TT520_Stage2_Discharge,max(AI_H2_C_TT521_Stage3_Suction,AI_H2_C_TT522_Stage3_Discharge))):0;
  highMax = (highMax > 800)?0:highMax;

  if(STATE != MANUAL_CONTROL){
    DO_Encl_PilotAmber = DI_Comm_LSR_Local;
    DO_CLT_PMP104_PMP204_CoolantPumps_Enable = !DI_Comm_LSR_Local;
    DO_CLT_FCU112_CoolantFan1_Enable = !DI_Comm_LSR_Local;
    DO_CLT_FCU212_CoolantFan2_Enable = !DI_Comm_LSR_Local;
    if(DI_Comm_LSR_Local && STATE == PRODUCTION){
      PREV_STATE = STATE;
      STATE = FAULT;
      faultString = "LSR Error";
    }
  }
  //########################STATE MACHINE################################
  if(STATE != CHANGED_STATE){ //reset state timer
    timer[0] = 0;
    timer[1] = 0;
    timer[2] = 0;
    timer[3] = 0;
    SUB_STATE1 = START;
    SUB_STATE2 = START;

    DO_Encl_PilotRed = false;
    DO_Encl_PilotGreen = false;
    DO_Encl_PilotAmber = false;

    flashGreen = 0;
    flashRed = 0;
    flashAmber = 0;
    manualPause = false;
    stateHistory1 = stateHistory1 + "(" + String(STATE) + ")";
    stateHistory2 = stateHistory2 + "(" + String(STATE) + ")";
    CHANGED_STATE = STATE;
  }

  switch(STATE){
    case IDLE_OFF:
      if(!timer[0]){ timer[0] = millis(); }

      smallMatrix[2].displayPlay(DI_Comm_LSR_Local);
      DO_Comm_LSR_Local = true;
      DO_Encl_PilotGreen = !DI_Comm_LSR_Local;
      DO_Encl_PilotAmber = DI_Comm_LSR_Local;
      
      if((DI_Encl_ButtonGreen || virtualGreenButton) && !DI_Comm_LSR_Local){ STATE = IDLE_ON; }
    break;

  //#####################################################################

    case IDLE_ON:
      if(!timer[0]){
        timer[0] = millis();
        DO_HYD_PMP458_HydraulicPump1_Enable = true; //turn on hydraulic pump 1
        DO_HYD_PMP552_HydraulicPump2_Enable = true; //turn on hydraulic pump 2
        DO_H2_XV907_SuctionPreTank = true;
        flashGreen = 500;
      }
      if(millis() - timer[0] > 5000 && timer[0]){ //after 5 seconds
        if(!DI_HYD_IS000_HydraulicFilterSwitch){ //check filter switch
          STATE = FAULT;
          faultString = "Filter Switch Error";
        }
        else{STATE = PRODUCTION;}
      }
    break;

  //#####################################################################
    case PRODUCTION:
      if(!timer[0]){
        timer[0] = millis();
        flashGreen = 1000;
        DO_HYD_XV460_DCV1_A = false;
        DO_HYD_XV463_DCV1_B = false;
        DO_HYD_XV554_DCV2_A = false;
        DO_HYD_XV557_DCV2_B = false;
      }

      // //if the red button is held for less than 5 seconds pause it otherwise safe shutdown
      // if(DI_Encl_ButtonRed && !prevR){
      //   prevR = true;
      //   if(!holdR){holdR = millis();}
      // }
      // if(!DI_Encl_ButtonRed && prevR){
      //   prevR = false;
      //   if(millis() - holdR < 5000 && holdR){
      //     SUB_STATE1 = PAUSE;
      //     SUB_STATE2 = PAUSE;
      //     manualPause = true;
      //   }
      //   else{STATE = SHUTDOWN;}
      //   holdR = 0;
      // }
      // if(manualPause && DI_Encl_ButtonGreen){
      //   manualPause = false;
      //   SUB_STATE1 = START;
      //   SUB_STATE2 = START;
      // }

      if(manualPause){ smallMatrix[2].displayPause(false); }
      else{ smallMatrix[2].displayQuadrants(DO_HYD_XV460_DCV1_A,DO_HYD_XV463_DCV1_B,DO_HYD_XV554_DCV2_A,DO_HYD_XV557_DCV2_B,(SUB_STATE1==PAUSE),(SUB_STATE2==PAUSE)); }
      

      //Main operation of compressing
      if(!tog[0]){intensifier1Operation();}
      if(millis() - timer[0] > 20000 && timer[0]){tog[1] = true;}
      if(tog[1]){if(!tog[2]){intensifier2Operation_OLD();}else{intensifier2Operation();}}

      

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
      }
      
      if(millis() - timer[0] > 30000 && timer[0]){
        for(int i = 0; i < DOsize;i++){
          if((DOdata[i].key.indexOf("XV") >= -1 || DOdata[i].key.indexOf("PMP") >= -1 || DOdata[i].key.indexOf("PL") >= -1) && DOdata[i].key.indexOf("CLT") == -1){
            *DOdata[i].value = false;
          }
        }
        timer[0] = 0;
        timer[1] = 0;
        timer[2] = 0;
        timer[3] = 0;
        DO_CLT_PMP104_PMP204_CoolantPumps_Enable = false;
        STATE = IDLE_OFF;
      }
      //if pressure lowers and temps lower shut down the rest
    break;

  //#####################################################################

    case MANUAL_CONTROL:
      if(!timer[0]){
        timer[0] = millis();
        smallMatrix[2].displayChar('M', false);
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
      }
      
      if(DI_Encl_ButtonRed){STATE = IDLE_OFF;}
    break;

  //#####################################################################

    case ESTOP:
      if(!timer[0]){
        timer[0] = millis();
        for(int i = 0; i < DOsize;i++){
          *DOdata[i].value = false;
        }
        DO_Encl_PilotRed = true;
        smallMatrix[2].displayStop(false);
      }
      for(int i = 0; i < DOsize;i++){
        if(DOdata[i].key.indexOf("PL") == -1){
          *DOdata[i].value = false;
        }
      }

      if(DI_Encl_ESTOP){STATE = IDLE_OFF;}
    break;

  //#####################################################################

    default:
      Serial.println("STATE ERROR!!!!!");
    break;
  }

  loopTime = millis() - loopTimer;
  loopTimer = 0;
}