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
      if(!timer[2]){
        timer[2] = millis();
        DO_HYD_XV460_DCV1_A = true;
        stateHistory1 = stateHistory1 + "+";
        if(!CPMlowTimer){CPMlowTimer = millis();}
        else{ CPMlow = 60 / (millis() - CPMlowTimer);CPMlowTimer = millis(); }  
      }

      if(switchingPsi1A_Override){switchingPsi1A = switchingPsi1A_Override;}
      else{switchingPsi1A = getStage1SwitchingPsi_90_63();}

      if(millis() - timer[2] > 250 && timer[2] && AI_HYD_psig_PT467_HydraulicInlet1 >= switchingPsi1A){ DO_HYD_XV460_DCV1_A = false; temp = millis() - timer[2]; }

      if(millis() - timer[2] > 30000 && timer[2]){STATE = FAULT; faultString = faultString + "|1A Timeout|";}
      
      if(millis() - timer[2] > switchingTimeLow + temp && timer[2] && !DO_HYD_XV460_DCV1_A){//check if minimum time has passed

        SUB_STATE1 = SIDE_B;
      }
    break;

    case SIDE_B:
      if(!timer[2]){ timer[2] = millis(); DO_HYD_XV463_DCV1_B = true; stateHistory1 = stateHistory1 + "-";}

      if(switchingPsi1B_Override){switchingPsi1B = switchingPsi1B_Override;}
      else{switchingPsi1B = getStage1SwitchingPsi_90_63();}

      if(millis() - timer[2] > 250 && timer[2] && AI_HYD_psig_PT467_HydraulicInlet1 >= switchingPsi1B){ DO_HYD_XV463_DCV1_B = false; temp = millis() - timer[2];}
        
      if(millis() - timer[2] > 30000 && timer[2]){STATE = FAULT; faultString = faultString + "|1B Timeout|";}

      if(millis() - timer[2] > switchingTimeLow + temp && timer[2] && !DO_HYD_XV463_DCV1_B){//check if minimum time has passed
        
        //switchingTimeLow = ((lowMax>lowOverHeat)?((lowMax-lowOverHeat)/2):0)*time2CPMmult;

        //if(lowCPMCnt_ > lowCPM+0.99){switchingTimeLow = switchingTimeLow + (/*(lowCPMCnt_ - lowCPM+1) + */(lowMax>110)?((lowMax-110)/2):0)*time2CPMmult;}
        //else if(lowCPMCnt_ < lowCPM){switchingTimeLow = switchingTimeLow - (/*(lowCPM - lowCPMCnt_) + */(lowMax>110)?((lowMax-110)/2):0)*time2CPMmult;}

        SUB_STATE1 = SIDE_A;
      }
    break;

    case PAUSE:
      DO_HYD_XV460_DCV1_A = false;
      DO_HYD_XV463_DCV1_B = false;
      CPMlowTimer = 0;
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
      if(!timer[3]){
        timer[3] = millis();
        DO_HYD_XV554_DCV2_A = true;
        stateHistory2 = stateHistory2 + "+";
        if(!CPMhighTimer){CPMhighTimer = millis();}
        else{ CPMhigh = 60 / (millis() - CPMhighTimer);CPMhighTimer = millis(); }
      }

      if(switchingPsi2A_Override){switchingPsi2A = switchingPsi2A_Override;}
      else{switchingPsi2A = getStage2SwitchingPsi_150_90();}

      if(millis() - timer[3] > 250 && timer[3] && AI_HYD_psig_PT561_HydraulicInlet2 >= switchingPsi2A){ DO_HYD_XV554_DCV2_A = false; }

      if(millis() - timer[3] > 30000 && timer[3]){STATE = FAULT; faultString = faultString + "|2A Timeout|";}
      
      if(millis() - timer[3] > switchingTimeHigh && timer[3] && !DO_HYD_XV554_DCV2_A){//check if minimum time has passed
        SUB_STATE2 = SIDE_B;
      }
    break;

    case SIDE_B:
      if(!timer[3]){ timer[3] = millis(); DO_HYD_XV557_DCV2_B = true; stateHistory2 = stateHistory2 + "-";}

      if(switchingPsi2B_Override){switchingPsi2B = switchingPsi2B_Override;}
      else{switchingPsi2B = getStage3SwitchingPsi_150_90() - 200;}

      if(millis() - timer[3] > 250 && timer[3] && AI_HYD_psig_PT561_HydraulicInlet2 >= switchingPsi2B){ DO_HYD_XV557_DCV2_B = false; }
        
      if(millis() - timer[3] > 30000 && timer[3]){STATE = FAULT; faultString = faultString + "|2B Timeout|";}

      if(millis() - timer[3] > switchingTimeHigh && timer[3] && !DO_HYD_XV557_DCV2_B){//check if minimum time has passed
        //switchingTimeLow = ((highMax>highOverHeat)?((highMax-highOverHeat)/2):0)*time2CPMmult;

        // if(highCPMCnt_ > highCPM+0.99){switchingTimeHigh = switchingTimeHigh + ((highCPMCnt_ - highCPM+1) + (highMax>110)?((highMax-110)/2):0)*time2CPMmult;}
        // else if(highCPMCnt_ < highCPM){switchingTimeHigh = switchingTimeHigh - ((highCPM - highCPMCnt_)+ (highMax>110)?((highMax-110)/2):0)*time2CPMmult;}

        SUB_STATE2 = SIDE_A;
      }
    break;

    case PAUSE:
      DO_HYD_XV554_DCV2_A = false;
      DO_HYD_XV557_DCV2_B = false;
      CPMhighTimer = 0;
    break;

    default:
    break;
  }
}
