#include "C200.h"
void setup() {
  Serial.begin(9600);
  //Serial1.begin(19200, SERIAL_8E1);
  //Serial4.begin(19200, SERIAL_8E1);
  pinModeSetup();
  Wire.begin();
  matrixSetup("C200", "V2.3.0");
  i2cSetup();
  // mbLocal.begin(OCI_MODBUS_ID, Serial4);
  // mbLocal.preTransmission(preTransmission);
  // mbLocal.postTransmission(postTransmission);

  Serial.println("OK");
  delay(3000);
  printMode = PACKET;
  DO_Comm_LSR_Local = true;
}

void loop() {
  DO_Comm_LSR_Local = true; //not used but needs to be true
  if(!loopTimer){loopTimer = millis();}

  SerialCLI();

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
    DO_CLT_PMP104_PMP204_CoolantPumps_Enable = DI_Comm_LSR_Local;
    DO_CLT_FCU112_CoolantFan1_Enable = DI_Comm_LSR_Local;
    DO_CLT_FCU212_CoolantFan2_Enable = DI_Comm_LSR_Local;
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
    timer[5] = 0;
    timer[5] = 0;

    DO_Encl_PilotRed = false;
    DO_Encl_PilotGreen = false;
    DO_Encl_PilotAmber = false;

    flashGreen = 0;
    flashRed = 0;
    flashAmber = 0;

    CHANGED_STATE = STATE;
  }

  switch(STATE){
    case IDLE_OFF:
      if(!timer[0]){ timer[0] = millis(); }

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
//warm up phase:
//start
//set all switchingPSI to 300
//stroke side a 
//wait until 1500ms from start of stroke
//continually check inlet for switchingPSI
//once switchingPSI reached wait 100ms
//check if inlet at or greater than 2000
//if not, increment switchingPSI by 100
//if so, wait until next a stroke and decrement switchingPSI by 10
  //once switchingPSI reached wait 100ms
  //check if inlet at or greater than 2000
  //if so, go back to decrement by switchingPSI 10
  //if not, check this stroke 3 more times, before moving to normal mode
//stroke side b, go back to "wait until 1500ms"

      switch(INTENSE1){
        case OFF:
          DO_HYD_XV460_DCV1_A = false;
          DO_HYD_XV463_DCV1_B = false;
        break;

        case START:
          side1 = false;
          INTENSE1 = ON;
          switchingTime1A = 1500;
          switchingTime1B = 1500;
          //prevDischarge1 = AI_H2_psig_PT716_Stage1_Discharge;
          //prevSuction1 = AI_H2_psig_PT911_Stage1_SuctionTank;
          timer[3] = 0;
        break;

        case ON:
          switchingPsi1 = side1?switchingPsi1B:switchingPsi1A;
          switchingTime1 = side1?switchingTime1B:switchingTime1A;
          spiked1 = side1?spiked1B:spiked1A;
          if(!timer[3]){timer[3] = millis();side1?DO_HYD_XV463_DCV1_B:DO_HYD_XV460_DCV1_A = true;}
          if(AI_HYD_psig_PT467_HydraulicInlet1 >= switchingPsi1){ //check if we reached switching pressure
            side1?DO_HYD_XV463_DCV1_B:DO_HYD_XV460_DCV1_A = false; //turn off solenoid
            if(millis() - timer[3] > switchingTime1 && timer[3]){ //before even thinking about switching sides, check if minimum time has passed
              if(warmpUp1){
                if(millis() - timer[3] > switchingTime1 + 100){ //wait to see if the pressure spikes
                  if(AI_HYD_psig_PT467_HydraulicInlet1 >= 2000){ //deadheaded
                    switchingPsi1 = switchingPsi1 - 10;
                    spiked1 = 1; //switch from incrementing to fine tune decrementing
                  }
                  else{ //didnt deadhead
                    if(!spiked1){ switchingPsi1 = switchingPsi1 + 100; } //increment if we arent finetuning 
                    else{
                      if(spiked1 > 3){warmpUp1 = false;} //make sure we dont deadhead 3 times in a row while decrement finetuning 
                      else{spiked1++;} //we didnt deadhead this time after finetuning 
                    }
                  }
                  side1 = !side1;
                  lowCycleCnt++;
                  timer[3] = 0;
                }
              }
              else{ //if not during warmup sequence
              /*
                if(!timer[4]){timer[4] = millis();}
                if(millis() - timer[4] > 3000){
                  suctionDelta1 = AI_H2_psig_PT911_Stage1_SuctionTank - prevSuction1;
                  dischargeDelta1 = AI_H2_psig_PT716_Stage1_Discharge - prevDischarge1;
                  switchingPsi1 = switchingPsi1 - suctionDelta1*XXX; //NEEDS INCREMENT MULTIPLIER!!!!!
                  switchingPsi1 = switchingPsi1 + dischargeDelta1*XXX; //NEEDS INCREMENT MULTIPLIER!!!!!
                  
                  if(AI_H2_psig_PT716_Stage1_Discharge >= 1400){
                    if(AI_H2_psig_PT716_Stage1_Discharge - prevDischarge1 > 5){
                      switchingTime1 = switchingTime1 + 250;
                    }
                    else if(prevDischarge1 - AI_H2_psig_PT716_Stage1_Discharge > 5){
                      switchingTime1 = switchingTime1 - (switchingTime1 >= 1500+250)?250:0;
                    }
                  }
                  else{switchingTime1 = 1500;}
                  prevDischarge1 = AI_H2_psig_PT716_Stage1_Discharge;
                  timer[4] = 0;
                }*/
                side1 = !side1;
                lowCycleCnt++;
                timer[3] = 0;
              }
            }
          }
          //if(millis() - timer[3] > 60000){ STATE = IDLE_OFF; }
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
          side2 = false;
          if(AI_H2_psig_PT712_Stage1_DischargeTank >= 1350){INTENSE2 = ON;} //make sure not to even start compressing until stage 1 discharge tank (stage 2 suction) is at minimum 1350psi
          switchingTime2A = 1500; //reset switching time delays
          switchingTime2B = 1500;
          //prevDischarge2 = AI_H2_psig_PT407_Stage3_Discharge;
          //prevSuction2 = AI_H2_psig_PT712_Stage1_DischargeTank;
          timer[5] = 0;
        break;

        case ON:
          switchingPsi2 = side2?switchingPsi2B:switchingPsi2A; //choose side A or side B
          switchingTime2 = side2?switchingTime2B:switchingTime2A; //choose side A or side B
          spiked2 = side2?spiked2B:spiked2A; //choose side A or side B
          if(!timer[5]){timer[5] = millis();side2?DO_HYD_XV557_DCV2_B:DO_HYD_XV554_DCV2_A = true;} //open solenoid and start timer
          if(AI_HYD_psig_PT467_HydraulicInlet1 >= switchingPsi2){ //check if we reached switching pressure
            side2?DO_HYD_XV557_DCV2_B:DO_HYD_XV554_DCV2_A = false; //turn off solenoid
            if(millis() - timer[5] > switchingTime2 && timer[5]){ //before even thinking about switching sides, check if minimum time has passed
              if(warmUp2){ //are we in the warmp up phase?
                if(millis() - timer[5] > switchingTime2 + 100){ //wait to see if the pressure spikes
                  if(AI_HYD_psig_PT467_HydraulicInlet1 >= 2000){ //deadheaded
                    switchingPsi2 = switchingPsi2 - 10;
                    spiked2 = 1; //switch from incrementing to fine tune decrementing
                  }
                  else{ //didnt deadhead
                    if(!spiked2){ switchingPsi2 = switchingPsi2 + 100; } //increment if we arent finetuning 
                    else{
                      if(spiked2 > 3){warmUp2 = false;} //make sure we dont deadhead 3 times in a row while decrement finetuning 
                      else{spiked2++;} //we didnt deadhead this time after finetuning 
                    }
                  }
                  side2 = !side2;
                  highCycleCnt++;
                  timer[5] = 0;
                }
              }
              else{ //if not during warmup sequence
              /*
                if(!timer[6]){timer[6] = millis();}
                if(millis() - timer[6] > 3000){
                  suctionDelta2 = AI_H2_psig_PT911_Stage1_SuctionTank - prevSuction2;
                  dischargeDelta2 = AI_H2_psig_PT716_Stage1_Discharge - prevDischarge2;
                  switchingPsi2 = switchingPsi2 - suctionDelta2*XXX; //NEEDS INCREMENT MULTIPLIER!!!!!
                  switchingPsi2 = switchingPsi2 + dischargeDelta2*XXX; //NEEDS INCREMENT MULTIPLIER!!!!!
                  timer[6] = 0;
                }*/
                side2 = !side2;
                highCycleCnt++;
                timer[5] = 0;
              }
            }
          }
          //if(millis() - timer[5] > 60000){ STATE = IDLE_OFF; }
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
        timer[5] = 0;
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