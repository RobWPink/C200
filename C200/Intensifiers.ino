void intensifier1Operation(){
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
      if(millis() - timer[2] > 3000 && timer[2]){
        DO_HYD_XV463_DCV1_B = false;
        INTENSE1 = DEADHEAD1;//(!deadHeadPsi1A || !deadHeadPsi1B) ? DEADHEAD1 : SIDE_A;
      }
      
    break;

    case DEADHEAD1:
      if(!timer[2]){timer[2] = millis();DO_HYD_XV460_DCV1_A = true;stateHistory1 = stateHistory1 + "+";}
      if(millis() - timer[2] > 500 && timer[2]){
        if(AI_HYD_psig_PT467_HydraulicInlet1 > peakPsi1A){peakPsi1A = AI_HYD_psig_PT467_HydraulicInlet1;}
      }
      if(millis() - timer[2] > 3000 && timer[2]){
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
      if(millis() - timer[2] > 3000 && timer[2]){
        deadHeadPsi1B = peakPsi1B;
        DO_HYD_XV463_DCV1_B = false;
        INTENSE1 = SIDE_A;
      }
    break;

    case SIDE_A:
      if(!timer[2]){ timer[2] = millis(); DO_HYD_XV460_DCV1_A = true; stateHistory1 = stateHistory1 + "+";}

      if(AI_HYD_psig_PT467_HydraulicInlet1 > peakPsi1A){peakPsi1A = AI_HYD_psig_PT467_HydraulicInlet1; }

      if(AI_HYD_psig_PT467_HydraulicInlet1 > 200){ avgLow.addValue(AI_HYD_psig_PT467_HydraulicInlet1); }

      if(avgLow.getCount() > 4 && AI_HYD_psig_PT467_HydraulicInlet1 - avgLow.getValue(avgLow.getCount()-2) > sdmLow + avgLow.getStandardDeviation() || peakPsi1A > deadHeadPsi1A - difLow){
        DO_HYD_XV460_DCV1_A = false;
      }

      if((millis() - timer[2] > 30000 && timer[2])){STATE = FAULT; faultString = faultString + "|1A Timeout|";}
      
      if(millis() - timer[2] > switchingTime1A-550 && timer[2] && !DO_HYD_XV460_DCV1_A){//check if minimum time has passed
        INTENSE1 = SIDE_B;
        lowCycleCnt++; //reached end of cycle time, switch sides 
      }
    break;

    case SIDE_B:
      if(!timer[2]){ timer[2] = millis(); DO_HYD_XV463_DCV1_B = true; stateHistory1 = stateHistory1 + "+";}

      if(AI_HYD_psig_PT467_HydraulicInlet1 > peakPsi1B){peakPsi1B = AI_HYD_psig_PT467_HydraulicInlet1; }

      if(AI_HYD_psig_PT467_HydraulicInlet1 > 200){ avgLow.addValue(AI_HYD_psig_PT467_HydraulicInlet1); }

      if(avgLow.getCount() > 4 && AI_HYD_psig_PT467_HydraulicInlet1 - avgLow.getValue(avgLow.getCount()-2) > sdmLow + avgLow.getStandardDeviation() || peakPsi1B > deadHeadPsi1B - difLow){
        DO_HYD_XV463_DCV1_B = false;
      }

      if((millis() - timer[2] > 30000 && timer[2])){STATE = FAULT; faultString = faultString + "|1B Timeout|";}

      if(millis() - timer[2] > switchingTime1B-550 && timer[2] && !DO_HYD_XV463_DCV1_B){//check if minimum time has passed
        INTENSE1 = SIDE_A;
        lowCycleCnt++; //reached end of cycle time, switch sides 
      }
    break;

    case PAUSE:
      if(!timer[2]){ timer[2] = millis(); DO_HYD_XV460_DCV1_A = false; DO_HYD_XV463_DCV1_B = false;}
      
      if(manualPause){break;}
      for(int i = 0; i < PTsize;i++){
        if(PTdata[i].overPressure){ break; }
      }
      if(millis() - timer[2] > 5000 && timer[2]){ INTENSE1 = SIDE_A; }
      
    break;

    default:
    break;
  }
}





void intensifier2Operation(){
  if(INTENSE2 != PREV2){
    timer[3] = 0;
    avgHigh.clear();
    DO_HYD_XV554_DCV2_A = false;
    DO_HYD_XV557_DCV2_B = false;
    peakPsi2A = 0;
    peakPsi2B = 0;
    stateHistory2 = stateHistory2 + String(INTENSE2);
    PREV2 = INTENSE2;
    return;
  }
  switch(INTENSE2){
    case OFF:
      DO_HYD_XV554_DCV2_A = false;
      DO_HYD_XV557_DCV2_B = false;
    break;

    case START:
      if(!timer[3]){timer[3] = millis();DO_HYD_XV557_DCV2_B = true;stateHistory2 = stateHistory2 + "-";}
      if(millis() - timer[3] > 3000 && timer[3]){
        DO_HYD_XV557_DCV2_B = false;
        INTENSE2 = DEADHEAD1;//(!deadHeadPsi2A || !deadHeadPsi2B) ? DEADHEAD1 : SIDE_A;
      }
      
    break;

    case DEADHEAD1:
      if(!timer[3]){timer[3] = millis();DO_HYD_XV554_DCV2_A = true;stateHistory2 = stateHistory2 + "+";}
      if(millis() - timer[3] > 500 && timer[3]){
        if(AI_HYD_psig_PT561_HydraulicInlet2 > peakPsi2A){peakPsi2A = AI_HYD_psig_PT561_HydraulicInlet2;}
      }
      if(millis() - timer[3] > 3000 && timer[3]){
        deadHeadPsi2A = peakPsi2A;
        DO_HYD_XV554_DCV2_A = false;
        INTENSE2 = DEADHEAD2;
      }
    break;

    case DEADHEAD2:
      if(!timer[3]){timer[3] = millis();DO_HYD_XV557_DCV2_B = true;stateHistory2 = stateHistory2 + "-";}
      if(millis() - timer[3] > 500 && timer[3]){
        if(AI_HYD_psig_PT561_HydraulicInlet2 > peakPsi2B){peakPsi2B = AI_HYD_psig_PT561_HydraulicInlet2;}
      }
      if(millis() - timer[3] > 3000 && timer[3]){
        deadHeadPsi2B = peakPsi2B;
        DO_HYD_XV557_DCV2_B = false;
        INTENSE2 = SIDE_A;
      }
    break;

    case SIDE_A:
      if(!timer[3]){ timer[3] = millis(); DO_HYD_XV554_DCV2_A = true; stateHistory2 = stateHistory2 + "+";}

      if(AI_HYD_psig_PT561_HydraulicInlet2 > peakPsi2A){peakPsi2A = AI_HYD_psig_PT561_HydraulicInlet2; }

      if(AI_HYD_psig_PT561_HydraulicInlet2 > 200){ avgHigh.addValue(AI_HYD_psig_PT561_HydraulicInlet2); }

      if(avgHigh.getCount() > 4 && AI_HYD_psig_PT561_HydraulicInlet2 - avgHigh.getValue(avgHigh.getCount()-2) > sdmHigh + avgHigh.getStandardDeviation() || peakPsi2A > deadHeadPsi2A - difHigh){
        DO_HYD_XV554_DCV2_A = false;
      }

      if((millis() - timer[3] > 30000 && timer[3])){STATE = FAULT; faultString = faultString + "|2A Timeout|";}
      
      if(millis() - timer[3] > switchingTime2A-550 && timer[3] && !DO_HYD_XV554_DCV2_A){//check if minimum time has passed
        INTENSE2 = SIDE_B;
        highCycleCnt++; //reached end of cycle time, switch sides 
      }
    break;

    case SIDE_B:
      if(!timer[3]){ timer[3] = millis(); DO_HYD_XV557_DCV2_B = true; stateHistory2 = stateHistory2 + "+";}

      if(AI_HYD_psig_PT561_HydraulicInlet2 > peakPsi2B){peakPsi2B = AI_HYD_psig_PT561_HydraulicInlet2; }

      if(AI_HYD_psig_PT561_HydraulicInlet2 > 200){ avgHigh.addValue(AI_HYD_psig_PT561_HydraulicInlet2); }

      if(avgHigh.getCount() > 4 && AI_HYD_psig_PT561_HydraulicInlet2 - avgHigh.getValue(avgHigh.getCount()-2) > sdmHigh + avgHigh.getStandardDeviation() || peakPsi2B > deadHeadPsi2B - difHigh){
        DO_HYD_XV557_DCV2_B = false;
      }

      if((millis() - timer[3] > 30000 && timer[3])){STATE = FAULT; faultString = faultString + "|2B Timeout|";}
      
      if(millis() - timer[3] > switchingTime2B-550 && timer[3] && !DO_HYD_XV557_DCV2_B){//check if minimum time has passed
        INTENSE2 = SIDE_A;
        highCycleCnt++; //reached end of cycle time, switch sides 
      }
    break;

    case PAUSE:
      if(!timer[3]){ timer[3] = millis(); DO_HYD_XV554_DCV2_A = false; DO_HYD_XV557_DCV2_B = false;}
      
      if(manualPause){break;}
      for(int i = 0; i < PTsize;i++){
        if(PTdata[i].overPressure){ break; }
      }
      if(millis() - timer[3] > 5000 && timer[3]){ INTENSE2 = SIDE_A; }
      
    break;

    default:
    break;
  }
}















void intensifier2Operation_OLD(){
  if(INTENSE2 != PREV2){
    timer[3] = 0;
    avgHigh.clear();
    DO_HYD_XV554_DCV2_A = false;
    DO_HYD_XV557_DCV2_B = false;
    peakPsi2A = 0;
    peakPsi2B = 0;
    stateHistory2 = stateHistory2 + String(INTENSE2);
    PREV2 = INTENSE2;
    return;
  }
  switch(INTENSE2){
    case OFF:
      DO_HYD_XV554_DCV2_A = false;
      DO_HYD_XV557_DCV2_B = false;
    break;

    case START:
      if(!timer[3]){timer[3] = millis();DO_HYD_XV557_DCV2_B = true;stateHistory2 = stateHistory2 + "-";}
      if(millis() - timer[3] > 3000 && timer[3]){
        DO_HYD_XV557_DCV2_B = false;
        INTENSE2 = DEADHEAD1;//(!deadHeadPsi2A || !deadHeadPsi2B) ? DEADHEAD1 : SIDE_A;
      }
      
    break;

    case DEADHEAD1:
      if(!timer[3]){timer[3] = millis();DO_HYD_XV554_DCV2_A = true;stateHistory2 = stateHistory2 + "+";}
      if(millis() - timer[3] > 500 && timer[3]){
        if(AI_HYD_psig_PT561_HydraulicInlet2 > peakPsi2A){peakPsi2A = AI_HYD_psig_PT561_HydraulicInlet2;}
      }
      if(millis() - timer[3] > 3000 && timer[3]){
        deadHeadPsi2A = peakPsi2A;
        DO_HYD_XV554_DCV2_A = false;
        INTENSE2 = DEADHEAD2;
      }
    break;

    case DEADHEAD2:
      if(!timer[3]){timer[3] = millis();DO_HYD_XV557_DCV2_B = true;stateHistory2 = stateHistory2 + "-";}
      if(millis() - timer[3] > 500 && timer[3]){
        if(AI_HYD_psig_PT561_HydraulicInlet2 > peakPsi2B){peakPsi2B = AI_HYD_psig_PT561_HydraulicInlet2;}
      }
      if(millis() - timer[3] > 3000 && timer[3]){
        deadHeadPsi2B = peakPsi2B;
        DO_HYD_XV557_DCV2_B = false;
        INTENSE2 = SIDE_A;
      }
    break;

    case SIDE_A:
      if(!timer[3]){ timer[3] = millis(); DO_HYD_XV554_DCV2_A = true; stateHistory2 = stateHistory2 + "+";}
      if(millis() - timer[3] > 500 && timer[3]){
        if(AI_HYD_psig_PT561_HydraulicInlet2 > deadHeadPsi2A-400){ DO_HYD_XV554_DCV2_A = false; }
      }
      if(millis() - timer[2] > switchingTime2A-550 && timer[2] && !DO_HYD_XV554_DCV2_A){//check if minimum time has passed
        INTENSE2 = SIDE_B;
        ///highCycleCnt++; //reached end of cycle time, switch sides 
      }
    break;

    case SIDE_B:
      if(!timer[3]){ timer[3] = millis(); DO_HYD_XV557_DCV2_B = true; stateHistory2 = stateHistory2 + "-";}
      if(millis() - timer[3] > 500 && timer[3]){
        if(AI_HYD_psig_PT561_HydraulicInlet2 > deadHeadPsi2B-400){ DO_HYD_XV557_DCV2_B = false; }
      }
      if(millis() - timer[2] > switchingTime2B-550 && timer[2] && !DO_HYD_XV557_DCV2_B){//check if minimum time has passed
        INTENSE2 = SIDE_A;
        //highCycleCnt++; //reached end of cycle time, switch sides 
      }
    break;

    case PAUSE:
      if(!timer[3]){ timer[3] = millis(); DO_HYD_XV554_DCV2_A = false; DO_HYD_XV557_DCV2_B = false;}
      
      if(manualPause){break;}
      for(int i = 0; i < PTsize;i++){
        if(PTdata[i].overPressure){ break; }
      }
      if(millis() - timer[3] > 5000 && timer[3]){ INTENSE2 = SIDE_A; }
      
    break;

    default:
    break;
  }
}