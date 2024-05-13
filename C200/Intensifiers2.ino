void intensifier1OperationV2(){
  if(INTENSE1 != PREV1){
    timer[2] = 0;
    avgLow.clear();
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
      if(!timer[2]){ timer[2] = millis(); DO_HYD_XV460_DCV1_A = true; stateHistory1 = stateHistory1 + "+";}
      avgLow.addValue(AI_HYD_psig_PT467_HydraulicInlet1);
      if(millis() - timer[2] > 250 && timer[2]){
        if(AI_HYD_psig_PT467_HydraulicInlet1 > peakPsi1A){peakPsi1A = AI_HYD_psig_PT467_HydraulicInlet1;}
        if(AI_HYD_psig_PT467_HydraulicInlet1 > 1.8*avgLow.getStandardDeviation() + avgLow.getFastAverage()){ //maybe try exponential smoothing here?
          switchingPsi1A = avgLow.getValue(avgLow.getCount()-1);
          DO_HYD_XV460_DCV1_A = false;
        }
        else if(AI_HYD_psig_PT467_HydraulicInlet1 >= switchingPsi1A){ DO_HYD_XV460_DCV1_A = false; }
      }
      if((millis() - timer[2] > 30000 && timer[2]) || switchingPsi1A > deadHeadPsi1A-300){STATE = FAULT; faultString = "1A Timeout";}

      if(millis() - timer[2] > switchingTime1A-550 && timer[2] && !DO_HYD_XV460_DCV1_A){//check if minimum time has passed
        INTENSE1 = SIDE_B;
        lowCycleCnt++; //reached end of cycle time, switch sides 
      }
    break;

    case SIDE_B:
      if(!timer[2]){ timer[2] = millis(); DO_HYD_XV463_DCV1_B = true; stateHistory1 = stateHistory1 + "-";}
      avgLow.addValue(AI_HYD_psig_PT467_HydraulicInlet1);
      if(millis() - timer[2] > 250 && timer[2]){
        if(AI_HYD_psig_PT467_HydraulicInlet1 > peakPsi1B){peakPsi1B = AI_HYD_psig_PT467_HydraulicInlet1;}
        if(AI_HYD_psig_PT467_HydraulicInlet1 > 1.8*avgLow.getStandardDeviation() + avgLow.getFastAverage()){ //maybe try exponential smoothing here?
          switchingPsi1B = avgLow.getValue(avgLow.getCount()-1);
          DO_HYD_XV463_DCV1_B = false;
        }
        else if(AI_HYD_psig_PT467_HydraulicInlet1 >= switchingPsi1B){ DO_HYD_XV463_DCV1_B = false; }
      }
      if((millis() - timer[2] > 30000 && timer[2]) || switchingPsi1B > deadHeadPsi1B-300){STATE = FAULT; faultString = "1B Timeout";}

      if(millis() - timer[2] > switchingTime1B-550 && timer[2] && !DO_HYD_XV463_DCV1_B){//check if minimum time has passed
        INTENSE1 = SIDE_A;
        lowCycleCnt++; //reached end of cycle time, switch sides 
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
    break;

    default:
    break;
  }
}