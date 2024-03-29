#include "C200.h"
void setup() {
  Serial.begin(2000000);
  Serial1.begin(19200, SERIAL_8E1);
  //pinModeSetup();
  Wire.begin();
  // mbLocal.begin(OCI_MODBUS_ID, Serial4);
  // mbLocal.preTransmission(preTransmission);
  // mbLocal.postTransmission(postTransmission);

  memset(timer,millis(),sizeof(timer));
}

void loop() {
  //SerialCLI();
  //rs485Transceive();
  i2cTransceive();
  flashDriver();
  if(millis() - timer[2] > delayTime && timer[2]){
    dataPrint();
    timer[2] = millis();
  }

  //########################STATE MACHINE################################

  switch(STATE){
    case IDLE:
      if(!timer[0]){timer[0] = millis();}
      DO_Encl_PilotAmber = DI_Comm_LSR_Local; //APL = LSR
      DO_Encl_PilotGreen = true; //GPL
      DO_Encl_PilotRed = false; //RPL
      if(DI_Encl_ButtonGreen){ // GBN
        DO_CLT_PMP104_PMP204_CoolantPumps_Enable = true;
        DO_CLT_FCU112_CoolantFan1_Enable = true;
        DO_CLT_FCU212_CoolantFan2_Enable = true;
        flashGreen = 500;
        PREV_STATE = STATE; //not used yet
        STATE = HYDRAULIC_FILTER;
      }
    break;
  
  //#####################################################################

    case HYDRAULIC_FILTER:
      if(!timer[1]){
        timer[1] = millis();
        DO_HYD_PMP458_HydraulicPump1_Enable = true; //turn on hydraulic pump 1
        DO_HYD_PMP552_HydraulicPump2_Enable = true; //turn on hydraulic pump 2  
      }
      if(millis() - timer[1] > 5000 && timer[1]){ //Check after 5 seconds
        if(DI_HYD_IS000_HydraulicFilterSwitch){ //check filter switch
          PREV_STATE = STATE; //not used yet
          STATE = PRODUCTION;
          flashRed = 0;
        }
        else{
          if(errorPrint){Serial.println("Filter Switch Error!");}
          flashRed = 250;
        }
      }

    break;

  //#####################################################################

    case PRODUCTION:
      if(!timer[0]){
        timer[0] = millis();
        flashGreen = 2000;
        DO_H2_XV907_SuctionPreTank = true; //turn on suction solenoid valve
        firstLowSide = true;
        firstHighSide = true;
        DO_HYD_XV460_DCV1_A = false;
        DO_HYD_XV463_DCV1_B = false;
        DO_HYD_XV554_DCV2_A = false;
        DO_HYD_XV557_DCV2_B = false;
      }

      //if the red button is held for less than 5 seconds pause it otherwise safe shutdown
      if((DI_Encl_ButtonRed || virtualRedButton) && !prevR){
        prevR = true;
        if(!holdR){holdR = millis();}
      }
      if((!DI_Encl_ButtonRed && !virtualRedButton) && prevR){
        prevR = false;
        if(100 < (millis() - holdR) < 5000 && holdR){
          PREV_STATE = STATE;
          STATE = PAUSE;
        }
        else if((millis() - holdR) > 5000 && holdR){
          PREV_STATE = STATE;
          STATE = SHUTDOWN;
        }
        holdR = 0;
      }

      if(
        90 - 10.0 < AI_H2_psig_PT911_SuctionTank < 250 || 
        AI_H2_psig_PT712_Stage1_DischargeTank > (AI_H2_psig_PT911_SuctionTank*RATIO)
      ){
        strokeLowSide = false;
      }
      else if(
        AI_H2_psig_PT911_SuctionTank > 90 && 
        AI_H2_psig_PT712_Stage1_DischargeTank < (AI_H2_psig_PT911_SuctionTank*RATIO)
      ){
        strokeLowSide = true;
      }

      if(AI_H2_psig_PT712_Stage1_DischargeTank < 450 || AI_H2_psig_PT410_Output > 8000){
        strokeHighSide = false;
      }

      else if(AI_H2_psig_PT712_Stage1_DischargeTank > 1100 && AI_H2_psig_PT410_Output < 8000){
        strokeHighSide = true;
      }
      if(strokeLowSide){
        if(firstLowSide){
          DO_HYD_XV460_DCV1_A  = true;
          DO_HYD_XV463_DCV1_B = false;
          firstLowSide = false;
          timer[3] = millis();
        }
        if(millis() - timer[3] > 500 && timer[3] && AI_HYD_psig_PT467_HydraulicInlet1 >= 1400){
          DO_HYD_XV460_DCV1_A  = !DO_HYD_XV460_DCV1_A ;
          DO_HYD_XV463_DCV1_B = !DO_HYD_XV463_DCV1_B;
          timer[3] = millis();
        }
      }
      if(strokeHighSide){
        if(firstHighSide){
          DO_HYD_XV554_DCV2_A = true;
          DO_HYD_XV557_DCV2_B = false;
          firstHighSide = false;
          timer[4] = millis();
        }
        if(millis() - timer[4] > 500 && timer[4] && AI_HYD_psig_PT561_HydraulicInlet2 >= 1400){
          DO_HYD_XV554_DCV2_A = !DO_HYD_XV554_DCV2_A;
          DO_HYD_XV557_DCV2_B = !DO_HYD_XV557_DCV2_B;
          timer[4] = millis();
        }
      }

    break;

  //#####################################################################

    case PAUSE:
      if(!timer[0]){
        timer[0] = millis();
        flashAmber = 1000;
        for(int i = 0; i < sizeof(DOdata);i++){
          if((DOdata[i].key.indexOf("XV") >= -1 || DOdata[i].key.indexOf("PMP") >= -1 || DOdata[i].key.indexOf("PL") >= -1) && DOdata[i].key.indexOf("CLT") == -1){
            *DOdata[i].value = false;
          }
        }
        DO_CLT_PMP104_PMP204_CoolantPumps_Enable = true;
      }
      //String msg = "Over limits: ";
      for(int i = 0; i < sizeof(TTdata);i++){
        if(TTdata[i].overheat){
          break;
          //msg = msg + TTdata[i].key + ", ";
        }
      }

      for(int i = 0; i < sizeof(PTdata);i++){
        if(PTdata[i].overPressure){
          break;
          //msg = msg + PTdata[i].key + ", ";
        }
      }

      if(DI_Comm_LSR_Local){
        break;
        //msg = msg + "LSR";
      }
      // if(msg != "Over limit: "){
      //   if(errorPrint){Serial.println(msg);}
      // }

      STATE = PREV_STATE;
      PREV_STATE = PAUSE;
      timer[0] = 0;

    break;

  //#####################################################################

    case SHUTDOWN:
      if(!timer[0]){
        timer[0] = millis();
        for(int i = 0; i < sizeof(DOdata);i++){
          if((DOdata[i].key.indexOf("XV") >= -1 || DOdata[i].key.indexOf("PMP") >= -1 || DOdata[i].key.indexOf("PL") >= -1) && DOdata[i].key.indexOf("CLT") == -1){
            *DOdata[i].value = false;
          }
        }
      }
      
      if(millis() - timer[0] > 30000 && timer[0]){
        for(int i = 0; i < sizeof(DOdata);i++){
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
        STATE = IDLE;
      }
      //if pressure lowers and temps lower shut down the rest
    break;

  //#####################################################################

    case MANUAL_CONTROL:
      if(!timer[0]){timer[0] = millis();}
    break;

  //#####################################################################

    case FAULT:
      if(!timer[0]){
        timer[0] = millis();
        flashRed = 1000;
      }
      
      //ADD EXIT!!!!!!!!!!!!!!!!!!!!!!!!!
    break;

  //#####################################################################

    case ESTOP:
      if(!timer[0]){timer[0] = millis();}
      for(int i = 0; i < sizeof(DOdata);i++){
        *DOdata[i].value = false;
      }
      flashRed = 200;
      STATE = IDLE;
    break;

  //#####################################################################

    default:
      if(errorPrint){Serial.println("STATE ERROR!!!!!");}
      break;
  }

  if(STATE != CHANGED_STATE){ //reset state timer
    timer[0] = 0;
    CHANGED_STATE = STATE;
  }
}
