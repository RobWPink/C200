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
        CPMlowTimer = millis();
      }

      switchingPsi1A = getStage1SwitchingPsi_90_63();

      if(millis() - timer[2] > 250 && timer[2] && AI_HYD_psig_PT467_HydraulicInlet1 >= switchingPsi1A_Override?switchingPsi1A_Override:switchingPsi1A){ DO_HYD_XV460_DCV1_A = false;}

      if(millis() - timer[2] > 30000 && timer[2]){STATE = FAULT; faultString = faultString + "|1A Timeout|";}

      if(millis() - timer[2] > switchingTimeLow_Override?switchingTimeLow_Override:switchingTimeLow && timer[2] && !DO_HYD_XV460_DCV1_A){//check if minimum time has passed
        SUB_STATE1 = SIDE_B;
      }
    break;

    case SIDE_B:
      if(!timer[2]){ timer[2] = millis(); DO_HYD_XV463_DCV1_B = true; stateHistory1 = stateHistory1 + "-";}

      switchingPsi1B = getStage1SwitchingPsi_90_63();

      if(millis() - timer[2] > 250 && timer[2] && AI_HYD_psig_PT467_HydraulicInlet1 >= switchingPsi1B_Override?switchingPsi1B_Override:switchingPsi1B){ DO_HYD_XV463_DCV1_B = false;}
        
      if(millis() - timer[2] > 30000 && timer[2]){STATE = FAULT; faultString = faultString + "|1B Timeout|";}

      if(millis() - timer[2] > switchingTimeLow_Override?switchingTimeLow_Override:switchingTimeLow && timer[2] && !DO_HYD_XV463_DCV1_B){//check if minimum time has passed
        CPMlow = 60 / (millis() - CPMlowTimer);
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
        CPMhighTimer = millis();
      }

      switchingPsi2A = getStage2SwitchingPsi_150_90()-150;

      if(millis() - timer[3] > 250 && timer[3] && AI_HYD_psig_PT561_HydraulicInlet2 >= switchingPsi2A_Override?switchingPsi2A_Override:switchingPsi2A){ DO_HYD_XV554_DCV2_A = false; }

      if(millis() - timer[3] > 30000 && timer[3]){STATE = FAULT; faultString = faultString + "|2A Timeout|";}

      if(millis() - timer[3] > switchingTimeHigh_Override?switchingTimeHigh_Override:switchingTimeHigh && timer[3] && !DO_HYD_XV554_DCV2_A){//check if minimum time has passed
        SUB_STATE2 = SIDE_B;
      }
    break;

    case SIDE_B:
      if(!timer[3]){ timer[3] = millis(); DO_HYD_XV557_DCV2_B = true; stateHistory2 = stateHistory2 + "-";}
      
      switchingPsi2B = getStage3SwitchingPsi_150_90();

      if(millis() - timer[3] > 250 && timer[3] && AI_HYD_psig_PT561_HydraulicInlet2 >= switchingPsi2B_Override?switchingPsi2B_Override:switchingPsi2B){ DO_HYD_XV557_DCV2_B = false; }
        
      if(millis() - timer[3] > 30000 && timer[3]){STATE = FAULT; faultString = faultString + "|2B Timeout|";}
      
      if(millis() - timer[3] > switchingTimeHigh_Override?switchingTimeHigh_Override:switchingTimeHigh && timer[3] && !DO_HYD_XV557_DCV2_B){//check if minimum time has passed
        CPMhigh = 60 / (millis() - CPMhighTimer);
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
