#include "C200.h"
void setup() {
  Serial.begin(9600);
  RPC.begin(); //boots M4
  pinModeSetup();
  Wire.begin();
  matrixSetup("C200_Longview", "V0.2.1");
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
  
  if(lsrReset){ //button press for LSR reset
    if(lsrReset == 1){lsrReset = millis();DO_Comm_LSR_Reset = true;}
    if(millis() - lsrReset > 1000){
      DO_Comm_LSR_Reset = false;
      lsrReset = 0;
    }
  }

  //rs485Transceive();
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
    timer[2] = 0;
    timer[3] = 0;


    DO_Encl_PilotRed = false;
    DO_Encl_PilotGreen = false;
    DO_Encl_PilotAmber = false;

    flashGreen = 0;
    flashRed = 0;
    flashAmber = 0;
    manualPause = false;
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
        INTENSE1 = START;
        //INTENSE2 = START;
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
      if(manualPause && DI_Encl_ButtonGreen){
        manualPause = false;
        INTENSE1 = START;
        INTENSE2 = START;
      }

      if(manualPause){ smallMatrix[2].displayPause(false); }
      else{ smallMatrix[2].displayQuadrants(DO_HYD_XV460_DCV1_A,DO_HYD_XV463_DCV1_B,DO_HYD_XV554_DCV2_A,DO_HYD_XV557_DCV2_B); }
      
      if(!timer[1]){timer[1] = millis();}
      if(millis() - timer[1] > 60000 && timer[1]){
        highCycleCnt_ = highCycleCnt/2;
        lowCycleCnt_ = lowCycleCnt/2;
        highCycleCnt = 0;
        lowCycleCnt = 0;
        timer[1] = 0;
      }

      //#########################################################
      //####################### LOW SIDE ########################
      //#########################################################

      if(INTENSE1 != PREV1){
        timer[2] = 0;
        DO_HYD_XV460_DCV1_A = false;
        DO_HYD_XV463_DCV1_B = false;
        PREV1 = INTENSE1;
      }
      switch(INTENSE1){
        case OFF:
          DO_HYD_XV460_DCV1_A = false;
          DO_HYD_XV463_DCV1_B = false;
        break;

        case START:
        if(!deadHeadPsi1A || !deadHeadPsi1B){INTENSE1 = DEADHEAD1;}
        else{INTENSE1 = SIDE_A;}
        break;

        case DEADHEAD1:
          if(!timer[2]){timer[2] = millis();DO_HYD_XV460_DCV1_A = true;}
          if(millis() - timer[2] > 5000 && timer[2]){
            deadHeadPsi1A = AI_HYD_psig_PT467_HydraulicInlet1;
            DO_HYD_XV460_DCV1_A = false;
            INTENSE1 = DEADHEAD2;
          }
        break;

        case DEADHEAD2:
          if(!timer[2]){timer[2] = millis();DO_HYD_XV463_DCV1_B = true;}
          if(millis() - timer[2] > 5000 && timer[2]){
            deadHeadPsi1B = AI_HYD_psig_PT467_HydraulicInlet1;
            DO_HYD_XV463_DCV1_B = false;
            INTENSE1 = SIDE_A;
          }
        break;

        case SIDE_A:
          if(!timer[2]){ timer[2] = millis();DO_HYD_XV460_DCV1_A = true;DO_HYD_XV463_DCV1_B = false;} 
          if(AI_HYD_psig_PT467_HydraulicInlet1 >= switchingPsi1A){ //check if we reached switching pressure
            DO_HYD_XV460_DCV1_A = false; //turn off solenoid
            if(millis() - timer[2] > switchingTime1A && timer[2]){ //before even thinking about switching sides, check if minimum time has passed
              if(!warmUp1A){
                if(AI_HYD_psig_PT467_HydraulicInlet1 >= (deadHeadPsi1A-switchingPsi1A)/3 + switchingPsi1A){ //deadheaded
                  switchingPsi1A = switchingPsi1A - 10; // fine tune decrement
                  spiked1A = 1; //switch from incrementing to fine tune decrementing
                }
                else{
                  if(!spiked1A){switchingPsi1A = switchingPsi1A + 100; } //increment if we arent finetuning 
                  else{
                    if(spiked1A > 3){warmUp1A = true;} //make sure we dont deadhead 3 times in a row while decrement finetuning 
                    else{spiked1A++;} //we didnt deadhead this time after finetuning 
                  }
                }
              }
              lowCycleCnt++; //reached end of cycle time, switch sides 
              INTENSE1 = SIDE_B;
            }
          }
        break;

        case SIDE_B:
          if(!timer[2]){ timer[2] = millis();DO_HYD_XV460_DCV1_A = false;DO_HYD_XV463_DCV1_B = true;} 
          if(AI_HYD_psig_PT467_HydraulicInlet1 >= switchingPsi1B){ //check if we reached switching pressure
            DO_HYD_XV463_DCV1_B = false; //turn off solenoid
            if(millis() - timer[2] > switchingTime1B && timer[2]){ //before even thinking about switching sides, check if minimum time has passed
              if(!warmUp1B){
                if(AI_HYD_psig_PT467_HydraulicInlet1 >= (deadHeadPsi1B-switchingPsi1B)/3 + switchingPsi1B){ //deadheaded
                  switchingPsi1B = switchingPsi1B - 10; // fine tune decrement
                  spiked1B = 1; //switch from incrementing to fine tune decrementing
                }
                else{
                  if(!spiked1B){switchingPsi1B = switchingPsi1B + 100; } //increment if we arent finetuning 
                  else{
                    if(spiked1B > 3){warmUp1B = true;} //make sure we dont deadhead 3 times in a row while decrement finetuning 
                    else{spiked1B++;} //we didnt deadhead this time after finetuning 
                  }
                }
              }
              lowCycleCnt++; //reached end of cycle time, switch sides 
              INTENSE1 = SIDE_A;
            }
          }
        break;
       
        case PAUSE:
          DO_HYD_XV460_DCV1_A = false;
          DO_HYD_XV463_DCV1_B = false;
          for(int i = 0; i < PTsize;i++){
            if(!manualPause && PTdata[i].overPressure && (PTdata[i].pause == 1 || !PTdata[i].pause)){
              break;
            }
          }
          INTENSE1 = SIDE_A;
        break;

        default:
        break;
      }

      //#########################################################
      //####################### HIGH SIDE #######################
      //#########################################################

      if(INTENSE2 != PREV2){
        timer[3] = 0;
        DO_HYD_XV554_DCV2_A = false;
        DO_HYD_XV557_DCV2_B = false;
        PREV2 = INTENSE2;
      }
      switch(INTENSE2){
        case OFF:
          DO_HYD_XV554_DCV2_A = false;
          DO_HYD_XV557_DCV2_B = false;
        break;

        case START:
        if(!deadHeadPsi2A || !deadHeadPsi2B){INTENSE2 = DEADHEAD1;}
        else{INTENSE2 = SIDE_A;}
        break;

        case DEADHEAD1:
          if(!timer[3]){timer[3] = millis();DO_HYD_XV554_DCV2_A = true;}
          if(millis() - timer[3] > 5000 && timer[3]){
            deadHeadPsi2A = AI_HYD_psig_PT561_HydraulicInlet2;
            DO_HYD_XV554_DCV2_A = false;
            INTENSE2 = DEADHEAD2;
          }
        break;

        case DEADHEAD2:
          if(!timer[3]){timer[3] = millis();DO_HYD_XV557_DCV2_B = true;}
          if(millis() - timer[3] > 5000 && timer[3]){
            deadHeadPsi2B = AI_HYD_psig_PT561_HydraulicInlet2;
            DO_HYD_XV557_DCV2_B = false;
            INTENSE2 = SIDE_A;
          }
        break;

        case SIDE_A:
          if(!timer[3]){ timer[3] = millis();DO_HYD_XV554_DCV2_A = true;DO_HYD_XV557_DCV2_B = false;} 
          if(AI_HYD_psig_PT561_HydraulicInlet2 >= switchingPsi2A){ //check if we reached switching pressure
            DO_HYD_XV554_DCV2_A = false; //turn off solenoid
            if(millis() - timer[3] > switchingTime2A && timer[3]){ //before even thinking about switching sides, check if minimum time has passed
              if(!warmUp2A){
                if(AI_HYD_psig_PT561_HydraulicInlet2 >= (deadHeadPsi2A-switchingPsi2A)/3 + switchingPsi2A){ //deadheaded
                  switchingPsi2A = switchingPsi2A - 10; // fine tune decrement
                  spiked2A = 1; //switch from incrementing to fine tune decrementing
                }
                else{
                  if(!spiked2A){switchingPsi2A = switchingPsi2A + 100; } //increment if we arent finetuning 
                  else{
                    if(spiked2A > 3){warmUp2A = true;} //make sure we dont deadhead 3 times in a row while decrement finetuning 
                    else{spiked2A++;} //we didnt deadhead this time after finetuning 
                  }
                }
              }
              lowCycleCnt++; //reached end of cycle time, switch sides 
              INTENSE2 = SIDE_B;
            }
          }
        break;

        case SIDE_B:
          if(!timer[3]){ timer[3] = millis();DO_HYD_XV554_DCV2_A = false;DO_HYD_XV557_DCV2_B = true;} 
          if(AI_HYD_psig_PT561_HydraulicInlet2 >= switchingPsi2B){ //check if we reached switching pressure
            DO_HYD_XV557_DCV2_B = false; //turn off solenoid
            if(millis() - timer[3] > switchingTime2B && timer[3]){ //before even thinking about switching sides, check if minimum time has passed
              if(!warmUp2B){
                if(AI_HYD_psig_PT561_HydraulicInlet2 >= (deadHeadPsi2B-switchingPsi2B)/3 + switchingPsi2B){ //deadheaded
                  switchingPsi2B = switchingPsi2B - 10; // fine tune decrement
                  spiked2B = 1; //switch from incrementing to fine tune decrementing
                }
                else{
                  if(!spiked2B){switchingPsi2B = switchingPsi2B + 100; } //increment if we arent finetuning 
                  else{
                    if(spiked2B > 3){warmUp2B = true;} //make sure we dont deadhead 3 times in a row while decrement finetuning 
                    else{spiked2B++;} //we didnt deadhead this time after finetuning 
                  }
                }
              }
              lowCycleCnt++; //reached end of cycle time, switch sides 
              INTENSE2 = SIDE_A;
            }
          }
        break;
       
        case PAUSE:
          DO_HYD_XV554_DCV2_A = false;
          DO_HYD_XV557_DCV2_B = false;
          for(int i = 0; i < PTsize;i++){
            if(!manualPause && PTdata[i].overPressure && (PTdata[i].pause == 1 || !PTdata[i].pause)){
              break;
            }
          }
          INTENSE2 = SIDE_A;
        break;

        default:
        break;
      }


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
      if(errorPrint){Serial.println("STATE ERROR!!!!!");}
      break;
  }

  loopTime = millis() - loopTimer;
  loopTimer = 0;
  Serial.println();
}