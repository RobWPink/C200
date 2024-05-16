void intensifier1Operation(){
  if(SUB_STATE1 != PREV1){
    timer[2] = 0;
    DO_HYD_XV460_DCV1_A = false;
    DO_HYD_XV463_DCV1_B = false;
    stateHistory1 = stateHistory1 + String(SUB_STATE1);
    PREV1 = SUB_STATE1;
    return;
  }
  switch(SUB_STATE1){
    case OFF:
      DO_HYD_XV460_DCV1_A = false;
      DO_HYD_XV463_DCV1_B = false;
    break;

    case START:
      DO_HYD_XV460_DCV1_A = false;
      DO_HYD_XV463_DCV1_B = false;
      SUB_STATE1 = SIDE_A;//(!deadHeadPsi1A || !deadHeadPsi1B) ? DEADHEAD1 : SIDE_A;
      
    break;

    case SIDE_A:
      if(!timer[2]){ timer[2] = millis(); DO_HYD_XV460_DCV1_A = true; stateHistory1 = stateHistory1 + "+";}

      switchingPsi1A = getStage1SwitchingPsi();

      if(millis() - timer[2] > 500 && timer[2] && AI_HYD_psig_PT467_HydraulicInlet1 >= switchingPsi1A){ DO_HYD_XV460_DCV1_A = false; temp = millis() - timer[2]; }

      if(millis() - timer[2] > 30000 && timer[2]){STATE = FAULT; faultString = faultString + "|1A Timeout|";}
      
      if(millis() - timer[2] > switchingTimeLow + temp && timer[2] && !DO_HYD_XV460_DCV1_A){//check if minimum time has passed
        lowCPMCnt = millis() - timer[2];

        SUB_STATE1 = SIDE_B;
      }
    break;

    case SIDE_B:
      if(!timer[2]){ timer[2] = millis(); DO_HYD_XV463_DCV1_B = true; stateHistory1 = stateHistory1 + "-";}

      switchingPsi1B = getStage1SwitchingPsi();

      if(millis() - timer[2] > 500 && timer[2] && AI_HYD_psig_PT467_HydraulicInlet1 >= switchingPsi1B){ DO_HYD_XV463_DCV1_B = false; temp = millis() - timer[2];}
        
      if(millis() - timer[2] > 30000 && timer[2]){STATE = FAULT; faultString = faultString + "|1B Timeout|";}

      if(millis() - timer[2] > switchingTimeLow + temp && timer[2] && !DO_HYD_XV463_DCV1_B){//check if minimum time has passed
        lowCPMCnt = lowCPMCnt + millis() - timer[2];
        lowCPMCnt_ = 60 / lowCPMCnt;
        switchingTimeLow = ((lowMax>60)?((lowMax-60)/2):0)*time2CPMmult;
        //if(lowCPMCnt_ > lowCPM+0.99){switchingTimeLow = switchingTimeLow + (/*(lowCPMCnt_ - lowCPM+1) + */(lowMax>110)?((lowMax-110)/2):0)*time2CPMmult;}
        //else if(lowCPMCnt_ < lowCPM){switchingTimeLow = switchingTimeLow - (/*(lowCPM - lowCPMCnt_) + */(lowMax>110)?((lowMax-110)/2):0)*time2CPMmult;}

        SUB_STATE1 = SIDE_A;
      }
    break;

    case PAUSE:
      if(!timer[2]){ timer[2] = millis(); DO_HYD_XV460_DCV1_A = false; DO_HYD_XV463_DCV1_B = false;}
      
      if(manualPause){break;}
      if(millis() - timer[2] > 5000 && timer[2]){ 
        for(int i = 0; i < PTsize;i++){
          if(PTdata[i].overPressure){ break; }
          if(i == PTsize - 1){SUB_STATE1 = SIDE_A;}
        }
      }
      
      
    break;

    default:
    break;
  }
}

void intensifier2Operation(){
  if(SUB_STATE2 != PREV2){
    timer[3] = 0;
    DO_HYD_XV554_DCV2_A = false;
    DO_HYD_XV557_DCV2_B = false;
    stateHistory2 = stateHistory2 + String(SUB_STATE2);
    PREV2 = SUB_STATE2;
    return;
  }
  switch(SUB_STATE2){
    case OFF:
      DO_HYD_XV554_DCV2_A = false;
      DO_HYD_XV557_DCV2_B = false;
    break;

    case START:
      DO_HYD_XV554_DCV2_A = false;
      DO_HYD_XV557_DCV2_B = false;
      SUB_STATE2 = SIDE_A;
      
    break;

    case SIDE_A:
      if(!timer[2]){ timer[3] = millis(); DO_HYD_XV554_DCV2_A = true; stateHistory2 = stateHistory2 + "+";}

      switchingPsi2A = getStage2SwitchingPsi();

      if(millis() - timer[3] > 500 && timer[3] && AI_HYD_psig_PT561_HydraulicInlet2 >= switchingPsi2A){ DO_HYD_XV554_DCV2_A = false; }

      if(millis() - timer[3] > 30000 && timer[3]){STATE = FAULT; faultString = faultString + "|2A Timeout|";}
      
      if(millis() - timer[3] > switchingTimeHigh && timer[3] && !DO_HYD_XV554_DCV2_A){//check if minimum time has passed
        highCPMCnt = millis() - timer[3];

        SUB_STATE2 = SIDE_B;
      }
    break;

    case SIDE_B:
      if(!timer[2]){ timer[3] = millis(); DO_HYD_XV557_DCV2_B = true; stateHistory2 = stateHistory2 + "-";}

      switchingPsi2B = getStage3SwitchingPsi();

      if(millis() - timer[3] > 500 && timer[3] && AI_HYD_psig_PT561_HydraulicInlet2 >= switchingPsi2B){ DO_HYD_XV557_DCV2_B = false; }
        
      if(millis() - timer[3] > 30000 && timer[3]){STATE = FAULT; faultString = faultString + "|2B Timeout|";}

      if(millis() - timer[3] > switchingTimeHigh && timer[3] && !DO_HYD_XV557_DCV2_B){//check if minimum time has passed
        highCPMCnt = highCPMCnt + millis() - timer[3];
        highCPMCnt_ = 60 / highCPMCnt;

        if(highCPMCnt_ > highCPM+0.99){switchingTimeHigh = switchingTimeHigh + ((highCPMCnt_ - highCPM+1) + (highMax>110)?((highMax-110)/2):0)*time2CPMmult;}
        else if(highCPMCnt_ < highCPM){switchingTimeHigh = switchingTimeHigh - ((highCPM - highCPMCnt_)+ (highMax>110)?((highMax-110)/2):0)*time2CPMmult;}

        SUB_STATE2 = SIDE_A;
      }
    break;

    case PAUSE:
      if(!timer[2]){ timer[3] = millis(); DO_HYD_XV554_DCV2_A = false; DO_HYD_XV557_DCV2_B = false;}
      
      if(manualPause){break;}
      if(millis() - timer[3] > 5000 && timer[3]){ 
        for(int i = 0; i < PTsize;i++){
          if(PTdata[i].overPressure){ break; }
          if(i == PTsize - 1){SUB_STATE2 = SIDE_A; }
        }
      }
      
    break;

    default:
    break;
  }
}

void intensifier2Operation_OLD(){
  if(SUB_STATE2 != PREV2){
    timer[3] = 0;
    DO_HYD_XV554_DCV2_A = false;
    DO_HYD_XV557_DCV2_B = false;
    peakPsi2A = 0;
    peakPsi2B = 0;
    stateHistory2 = stateHistory2 + String(SUB_STATE2);
    PREV2 = SUB_STATE2;
    return;
  }
  switch(SUB_STATE2){
    case OFF:
      DO_HYD_XV554_DCV2_A = false;
      DO_HYD_XV557_DCV2_B = false;
    break;

    case START:
      if(!timer[3]){timer[3] = millis();DO_HYD_XV557_DCV2_B = true;stateHistory2 = stateHistory2 + "-";}
      if(millis() - timer[3] > 3000 && timer[3]){
        DO_HYD_XV557_DCV2_B = false;
        SUB_STATE2 = DEADHEAD1;//(!deadHeadPsi2A || !deadHeadPsi2B) ? DEADHEAD1 : SIDE_A;
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
        SUB_STATE2 = DEADHEAD2;
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
        SUB_STATE2 = SIDE_A;
      }
    break;

    case SIDE_A:
      if(!timer[3]){ timer[3] = millis(); DO_HYD_XV554_DCV2_A = true; stateHistory2 = stateHistory2 + "+";}
      if(millis() - timer[3] > 500 && timer[3]){
        if(AI_HYD_psig_PT561_HydraulicInlet2 > deadHeadPsi2A-400){ DO_HYD_XV554_DCV2_A = false; }
      }
      if(millis() - timer[2] > switchingTimeHigh-500 && timer[2] && !DO_HYD_XV554_DCV2_A){//check if minimum time has passed
        SUB_STATE2 = SIDE_B;
        ///highCycleCnt++; //reached end of cycle time, switch sides 
      }
    break;

    case SIDE_B:
      if(!timer[3]){ timer[3] = millis(); DO_HYD_XV557_DCV2_B = true; stateHistory2 = stateHistory2 + "-";}
      if(millis() - timer[3] > 500 && timer[3]){
        if(AI_HYD_psig_PT561_HydraulicInlet2 > deadHeadPsi2B-400){ DO_HYD_XV557_DCV2_B = false; }
      }
      if(millis() - timer[2] > switchingTimeHigh-500 && timer[2] && !DO_HYD_XV557_DCV2_B){//check if minimum time has passed
        SUB_STATE2 = SIDE_A;
        //highCycleCnt++; //reached end of cycle time, switch sides 
      }
    break;

    case PAUSE:
      if(!timer[3]){ timer[3] = millis(); DO_HYD_XV554_DCV2_A = false; DO_HYD_XV557_DCV2_B = false;}
      
      if(manualPause){break;}
      if(millis() - timer[3] > 5000 && timer[3]){ 
        for(int i = 0; i < PTsize;i++){
          if(PTdata[i].overPressure){ break; }
          if(i == PTsize - 1){SUB_STATE2 = SIDE_A; }
        }
      }
      
    break;

    default:
    break;
  }
}