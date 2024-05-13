void intensifier1OperationV2(){
  if(INTENSE1 != PREV1){
    timer[2] = 0;
    DO_HYD_XV460_DCV1_A = false;
    DO_HYD_XV463_DCV1_B = false;
    peakPsi1A = 0;
    peakPsi1B = 0;
    stateHistory1 = stateHistory1 + String(INTENSE1);
    PREV1 = INTENSE1;
    return;
  }
  switch(INTENSE1){
    case OFF:
      DO_HYD_XV460_DCV1_A = false;
      DO_HYD_XV463_DCV1_B = false;
    break;

    case START:
      if(!timer[2]){timer[2] = millis();DO_HYD_XV463_DCV1_B = true;stateHistory1 = stateHistory1 + "-";}
      if(millis() - timer[2] > 5000 && timer[2]){
        DO_HYD_XV463_DCV1_B = false;
        INTENSE1 = DEADHEAD1;//(!deadHeadPsi1A || !deadHeadPsi1B) ? DEADHEAD1 : SIDE_A;
      }
      
    break;

    case DEADHEAD1:
      if(!timer[2]){timer[2] = millis();DO_HYD_XV460_DCV1_A = true;stateHistory1 = stateHistory1 + "+";}
      if(millis() - timer[2] > 500 && timer[2]){
        if(AI_HYD_psig_PT467_HydraulicInlet1 > peakPsi1A){peakPsi1A = AI_HYD_psig_PT467_HydraulicInlet1;}
      }
      if(millis() - timer[2] > 5000 && timer[2]){
        deadHeadPsi1A = peakPsi1A;
        DO_HYD_XV460_DCV1_A = false;
        INTENSE1 = DEADHEAD2;
      }
    break;

    case DEADHEAD2:
      if(!timer[2]){timer[2] = millis();DO_HYD_XV463_DCV1_B = true;stateHistory1 = stateHistory1 + "-";}
      if(millis() - timer[2] > 500 && timer[2]){
        if(AI_HYD_psig_PT467_HydraulicInlet1 > peakPsi1B){peakPsi1B = AI_HYD_psig_PT467_HydraulicInlet1;}
      }
      if(millis() - timer[2] > 5000 && timer[2]){
        deadHeadPsi1B = peakPsi1B;
        DO_HYD_XV463_DCV1_B = false;
        INTENSE1 = SIDE_A;
      }
    break;

    case SIDE_A:
      switch(SUBSTATE1){
        case STROKE:
          if(!timer[2]){ timer[2] = millis(); DO_HYD_XV460_DCV1_A = true; stateHistory1 = stateHistory1 + "+";}
          if(millis() - timer[2] > 500 && timer[2]){
            if(AI_HYD_psig_PT467_HydraulicInlet1 > peakPsi1A){peakPsi1A = AI_HYD_psig_PT467_HydraulicInlet1;}
            if(AI_HYD_psig_PT467_HydraulicInlet1 >= switchingPsi1A){
              if(millis() - timer[2] > 50 && timer[2]){
                DO_HYD_XV460_DCV1_A = false;
                SUBSTATE1 = warmUp1A?NORMAL:WARMUP;
              }
            }
          }
          if((millis() - timer[2] > 30000 && timer[2]) || switchingPsi1A > deadHeadPsi1A-300){STATE = FAULT; faultString = "1A Timeout";}
        break;

        case WARMUP: //needed to prevent changes in switchingPsi from interfering with nested checks
          if(peakPsi1A >= deadHeadPsi1A - 200){ //deadheaded
            switchingPsi1A = switchingPsi1A - 20; // fine tune decrement
            count1A = 1; //switch from incrementing to fine tune decrementing
          }
          else{
            if(!count1A){switchingPsi1A = switchingPsi1A + 100; } //increment if we arent finetuning 
            else{
              if(count1A > 3){warmUp1A = true;switchingTime1A = 1500; switchingRatio1A = switchingPsi1A/dynamicSwitchingPsi(false,AI_H2_psig_PT911_Stage1_SuctionTank,AI_H2_psig_PT716_Stage1_Discharge);} //make sure we dont deadhead 3 times in a row while decrement finetuning 
              else{count1A++;} //we didnt deadhead this time after finetuning 
            }
          }
          SUBSTATE1 = NORMAL;
        break;

        case NORMAL:
          if(warmUp1A){switchingPsi1A = dynamicSwitchingPsi(false,AI_H2_psig_PT911_Stage1_SuctionTank,AI_H2_psig_PT716_Stage1_Discharge) * switchingRatio1A;}
          if(millis() - timer[2] > switchingTime1A-550 && timer[2]){//check if minimum time has passed
            INTENSE1 = SIDE_B;
            SUBSTATE1 = STROKE;
            lowCycleCnt++; //reached end of cycle time, switch sides 
          }
        break;

        default:
        break;
      }
    break;

    case SIDE_B:
      switch(SUBSTATE1){
        case STROKE:
          if(!timer[2]){ timer[2] = millis(); DO_HYD_XV463_DCV1_B = true;stateHistory1 = stateHistory1 + "-";}
          if(millis() - timer[2] > 500 && timer[2]){
            if(AI_HYD_psig_PT467_HydraulicInlet1 > peakPsi1B){peakPsi1B = AI_HYD_psig_PT467_HydraulicInlet1;}
            if(AI_HYD_psig_PT467_HydraulicInlet1 >= switchingPsi1B){
              if(millis() - timer[2] > 50 && timer[2]){
                DO_HYD_XV463_DCV1_B = false;
                if(warmUp1A){ SUBSTATE1 = warmUp1B?NORMAL:WARMUP; }
                else{SUBSTATE1 = NORMAL;}
              }
            }
          }
          if((millis() - timer[2] > 30000 && timer[2]) || switchingPsi1B > deadHeadPsi1B-300){STATE = FAULT; faultString = "1B Timeout"; timer[2] = 0;}
        break;

        case WARMUP:
          if(peakPsi1B >= deadHeadPsi1B - 200){ //deadheaded
            switchingPsi1B = switchingPsi1B - 20; // fine tune decrement
            count1B = 1; //switch from incrementing to fine tune decrementing
          }
          else{
            if(!count1B){switchingPsi1B = switchingPsi1B + 100; }
            else{
              if(count1B > 3){warmUp1B = true;switchingTime1B = 1500;switchingRatio1B = switchingPsi1B/dynamicSwitchingPsi(false,AI_H2_psig_PT911_Stage1_SuctionTank,AI_H2_psig_PT716_Stage1_Discharge);}
              else{count1B++;}
            }
          }
          SUBSTATE1 = NORMAL;
        break;

        case NORMAL:
          if(warmUp1B){switchingPsi1B = dynamicSwitchingPsi(false,AI_H2_psig_PT911_Stage1_SuctionTank,AI_H2_psig_PT716_Stage1_Discharge)*switchingRatio1B;}
          if(millis() - timer[2] > switchingTime1B-550 && timer[2]){
            INTENSE1 = SIDE_A;
            SUBSTATE1 = STROKE;
            lowCycleCnt++;
          }
        break;

        default:
        break;
      }
    break;

    case PAUSE:
      DO_HYD_XV460_DCV1_A = false;
      DO_HYD_XV463_DCV1_B = false;
      if(manualPause){break;}
      for(int i = 0; i < PTsize;i++){
        if(PTdata[i].overPressure){ break; }
      }
      INTENSE1 = SIDE_A;
      SUBSTATE1 = STROKE;
    break;

    default:
    break;
  }
}