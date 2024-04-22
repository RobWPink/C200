#include "C200.h"
void setup() {
  Serial.begin(9600);
  RPC.begin(); //boots M4
  pinModeSetup();
  Wire.begin();
  matrixSetup("C200", "V2.2.5");
  i2cSetup();

  Serial.println("OK");
  delay(3000);
  printMode = PACKET;
  
}

void loop() {
  if(!loopTimer){loopTimer = millis();}

  SerialCLI();
  RPCtransceive();
  if(lsrReset){
    if(lsrReset == 1){lsrReset = millis();DO_Comm_LSR_Reset = true;}
    if(millis() - lsrReset > 1000){
      DO_Comm_LSR_Reset = false;
      lsrReset = 0;
    }
  }
  i2cTransceive(250);
    
  if(!DI_Encl_ESTOP){STATE = ESTOP;}
  flashDriver();
  
  dataPrint(delayTime);
  daughterPrint(delayTime);

  if(STATE != MANUAL_CONTROL){
    DO_Encl_PilotAmber = DI_Comm_LSR_Local;
    DO_CLT_PMP104_PMP204_CoolantPumps_Enable = !DI_Comm_LSR_Local;
    DO_CLT_FCU112_CoolantFan1_Enable = !DI_Comm_LSR_Local;
    DO_CLT_FCU212_CoolantFan2_Enable = !DI_Comm_LSR_Local;
    if(DI_Comm_LSR_Local && (STATE == PRODUCTION || STATE == PAUSE)){
      PREV_STATE = STATE;
      STATE = FAULT;
      faultString = "LSR Error";
    }
  }
  //########################STATE MACHINE################################
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

  String errDev[30] = {""};
  switch(STATE){
    case IDLE_OFF:
      if(!timer[0]){ timer[0] = millis(); }
      DO_Comm_LSR_Local = true;
      smallMatrix[2].displayPlay(DI_Comm_LSR_Local);
      DO_Encl_PilotGreen = !DI_Comm_LSR_Local;
      DO_Encl_PilotAmber = DI_Comm_LSR_Local;
    
      if(DI_Encl_ButtonGreen || virtualGreenButton){ STATE = IDLE_ON; }
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
        flashGreen = 2000;

        DO_HYD_XV460_DCV1_A = false;
        DO_HYD_XV463_DCV1_B = false;

        DO_HYD_XV554_DCV2_A = false;
        DO_HYD_XV557_DCV2_B = false;
        
        INTENSE1 = START;
        INTENSE2 = START;
        smallMatrix[2].displayPlay(false);
      }
      
      //if the red button is held for less than 5 seconds pause it otherwise safe shutdown
      if(DI_Encl_ButtonRed && !prevR){
        prevR = true;
        if(!holdR){holdR = millis();}
      }
      if(!DI_Encl_ButtonRed && prevR){
        prevR = false;
        if(millis() - holdR < 5000 && holdR){
          INTENSE1 = PAUSE;
          INTENSE2 = PAUSE;
          manualPause = true;
        }
        else{STATE = SHUTDOWN;}
        holdR = 0;
      }
      
      if(!timer[1]){timer[1] = millis();}
      if(millis() - timer[1] > 60000 && timer[1]){
        highCycleCnt_ = highCycleCnt/2;
        lowCycleCnt_ = lowCycleCnt/2;
        highCycleCnt = 0;
        lowCycleCnt = 0;
        timer[1] = 0;
      }

      switch(INTENSE1){
        case OFF:
          DO_HYD_XV460_DCV1_A = false;
          DO_HYD_XV463_DCV1_B = false;
        break;

        case START:
          DO_HYD_XV460_DCV1_A = true;
          DO_HYD_XV463_DCV1_B = false;
          INTENSE1 = ON;
          timer[3] = millis();
        break;

        case ON:
          if(millis() - timer[3] > 1500 && timer[3] && AI_HYD_psig_PT467_HydraulicInlet1 >= switchingPsi1){ //switching pressure
            DO_HYD_XV460_DCV1_A = !DO_HYD_XV460_DCV1_A;
            DO_HYD_XV463_DCV1_B = !DO_HYD_XV463_DCV1_B;
            timer[3] = millis();
            lowCycleCnt++;
          }
          if(millis() - timer[3] > 60000){ STATE = IDLE_OFF; }
        break;

        case PAUSE:
          DO_HYD_XV460_DCV1_A = false;
          DO_HYD_XV463_DCV1_B = false;
          for(int i = 0; i < PTsize;i++){
            if(!manualPause && PTdata[i].overPressure && (PTdata[i].pause == 1 || !PTdata[i].pause)){
              break;
            }
          }
          INTENSE1 = START;
        break;

        default:
        break;
      }

      switch(INTENSE2){
        case OFF:
          DO_HYD_XV554_DCV2_A = false;
          DO_HYD_XV557_DCV2_B = false;
        break;
        
        case START:
          DO_HYD_XV554_DCV2_A = true;
          DO_HYD_XV557_DCV2_B = false;
          timer[4] = millis();
          INTENSE2 = ON;
        break;

        case ON:
          if(millis() - timer[4] > 1500 && timer[4] && AI_HYD_psig_PT561_HydraulicInlet2 >= switchingPsi2){ //switching pressure
            DO_HYD_XV554_DCV2_A = !DO_HYD_XV554_DCV2_A;
            DO_HYD_XV557_DCV2_B = !DO_HYD_XV557_DCV2_B;
            timer[4] = millis();
            highCycleCnt++;
          }
          if(millis() - timer[4] > 60000){ STATE = IDLE_OFF; }
        break;

        case PAUSE:
          DO_HYD_XV554_DCV2_A = false;
          DO_HYD_XV557_DCV2_B = false;
          for(int i = 0; i < PTsize;i++){
            if(!manualPause && PTdata[i].overPressure && (PTdata[i].pause == 2 || !PTdata[i].pause)){
              break;
            }
          }
          INTENSE2 = START;
        break;

        default:
        break;
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
      if(errorPrint){Serial.println("STATE ERROR!!!!!");}
      break;
  }

  loopTime = millis() - loopTimer;
  loopTimer = 0;
}