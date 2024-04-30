void intensifier1Operation(){
  if(INTENSE1 != PREV1){
    timer[2] = 0;
    DO_HYD_XV460_DCV1_A = false;
    DO_HYD_XV463_DCV1_B = false;
    stateHistory = stateHistory + String(INTENSE1);
    PREV1 = INTENSE1;
    return;
  }
  switch(INTENSE1){
    case OFF:
      DO_HYD_XV460_DCV1_A = false;
      DO_HYD_XV463_DCV1_B = false;
    break;

    case START:
      INTENSE1 = (!deadHeadPsi1A || !deadHeadPsi1B) ? DEADHEAD1 : SIDE_A;
    break;

    case DEADHEAD1:
      if(!timer[2]){timer[2] = millis();DO_HYD_XV460_DCV1_A = true;stateHistory = stateHistory + "+";}
      if(millis() - timer[2] > 5000 && timer[2]){
        deadHeadPsi1A = AI_HYD_psig_PT467_HydraulicInlet1;
        DO_HYD_XV460_DCV1_A = false;
        INTENSE1 = DEADHEAD2;
      }
    break;

    case DEADHEAD2:
      if(!timer[2]){timer[2] = millis();DO_HYD_XV463_DCV1_B = true;stateHistory = stateHistory + "-";}
      if(millis() - timer[2] > 5000 && timer[2]){
        deadHeadPsi1B = AI_HYD_psig_PT467_HydraulicInlet1;
        DO_HYD_XV463_DCV1_B = false;
        INTENSE1 = SIDE_A;
      }
    break;

    case SIDE_A:
      switch(SUBSTATE1){
        case STROKE:
          if(!timer[2]){ timer[2] = millis(); DO_HYD_XV460_DCV1_A = true; stateHistory = stateHistory + "+";}
          if(millis() - timer[2] > 500 && timer[2]){
            if(AI_HYD_psig_PT467_HydraulicInlet1 >= switchingPsi1A){
              tmp_inlet1 = AI_HYD_psig_PT467_HydraulicInlet1; //pressure will instantly change when solenoid is closed, making the following check inaccurate, so save psi for that check
              DO_HYD_XV460_DCV1_A = false;
              SUBSTATE1 = warmUp1A?NORMAL:WARMUP;
            }
          }
          if(millis() - timer[2] > 30000 && timer[2]){STATE = FAULT; faultString = "1A Timeout";}
        break;

        case WARMUP: //needed to prevent changes in switchingPsi from interfering with nested checks
          if(tmp_inlet1 >= (deadHeadPsi1A-switchingPsi1A)/2 + switchingPsi1A){ //deadheaded
            switchingPsi1A = switchingPsi1A - 10; // fine tune decrement
            count1A = 1; //switch from incrementing to fine tune decrementing
          }
          else{
            if(!count1A){switchingPsi1A = switchingPsi1A + 100; } //increment if we arent finetuning 
            else{
              if(count1A > 3){warmUp1A = true;} //make sure we dont deadhead 3 times in a row while decrement finetuning 
              else{count1A++;} //we didnt deadhead this time after finetuning 
            }
          }
          SUBSTATE1 = NORMAL;
        break;

        case NORMAL:
          if(millis() - timer[2] > switchingTime1A-500 && timer[2]){//check if minimum time has passed
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
          if(!timer[2]){ timer[2] = millis(); DO_HYD_XV463_DCV1_B = true;stateHistory = stateHistory + "-";}
          if(millis() - timer[2] > 500 && timer[2]){
            if(AI_HYD_psig_PT467_HydraulicInlet1 >= switchingPsi1B){
              tmp_inlet1 = AI_HYD_psig_PT467_HydraulicInlet1;
              DO_HYD_XV463_DCV1_B = false;
              SUBSTATE1 = warmUp1B?NORMAL:WARMUP;
            }
          }
          if(millis() - timer[2] > 30000 && timer[2]){STATE = FAULT; faultString = "1B Timeout"; timer[2] = 0;}
        break;

        case WARMUP:
          if(tmp_inlet1 >= (deadHeadPsi1B-switchingPsi1B)/2 + switchingPsi1B){
            switchingPsi1B = switchingPsi1B - 10;
            count1B = 1;
          }
          else{
            if(!count1B){switchingPsi1B = switchingPsi1B + 100; }
            else{
              if(count1B > 3){warmUp1B = true;}
              else{count1B++;}
            }
          }
          SUBSTATE1 = NORMAL;
        break;

        case NORMAL:
          if(millis() - timer[2] > switchingTime1B-500 && timer[2]){
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
      for(int i = 0; i < PTsize;i++){
        if(!manualPause && PTdata[i].overPressure && (PTdata[i].pause == 1 || !PTdata[i].pause)){ break; }
      }
      INTENSE1 = SIDE_A;
      SUBSTATE1 = STROKE;
    break;

    default:
    break;
  }
}

//side b is not detecting deadheads
void intensifier2Operation(){
  if(INTENSE2 != PREV2){
    timer[3] = 0;
    DO_HYD_XV554_DCV2_A = false;
    DO_HYD_XV557_DCV2_B = false;
    peakPsi2A = 0;
    peakPsi2B = 0;
    stateHistory = stateHistory + String(INTENSE2);
    PREV2 = INTENSE2;
    return;
  }
  switch(INTENSE2){
    case OFF:
      DO_HYD_XV554_DCV2_A = false;
      DO_HYD_XV557_DCV2_B = false;
    break;

    case START:
      if(!timer[3]){timer[3] = millis();DO_HYD_XV557_DCV2_B = true;stateHistory = stateHistory + "-";}
      if(millis() - timer[3] > 5000 && timer[3]){
        DO_HYD_XV557_DCV2_B = false;
        INTENSE2 = DEADHEAD1;//(!deadHeadPsi2A || !deadHeadPsi2B) ? DEADHEAD1 : SIDE_A;
      }
      
    break;

    case DEADHEAD1:
      if(!timer[3]){timer[3] = millis();DO_HYD_XV554_DCV2_A = true;stateHistory = stateHistory + "+";}
      if(millis() - timer[3] > 500 && timer[3]){
        if(AI_HYD_psig_PT561_HydraulicInlet2 > peakPsi2A){peakPsi2A = AI_HYD_psig_PT561_HydraulicInlet2;}
      }
      if(millis() - timer[3] > 5000 && timer[3]){
        deadHeadPsi2A = peakPsi2A;
        DO_HYD_XV554_DCV2_A = false;
        INTENSE2 = DEADHEAD2;
      }
    break;

    case DEADHEAD2:
      if(!timer[3]){timer[3] = millis();DO_HYD_XV557_DCV2_B = true;stateHistory = stateHistory + "-";}
      if(millis() - timer[3] > 500 && timer[3]){
        if(AI_HYD_psig_PT561_HydraulicInlet2 > peakPsi2B){peakPsi2B = AI_HYD_psig_PT561_HydraulicInlet2;}
      }
      if(millis() - timer[3] > 5000 && timer[3]){
        deadHeadPsi2B = peakPsi2B;
        DO_HYD_XV557_DCV2_B = false;
        INTENSE2 = SIDE_A;
      }
    break;

    case SIDE_A:
      switch(SUBSTATE2){
        case STROKE:
          if(!timer[3]){ timer[3] = millis(); DO_HYD_XV554_DCV2_A = true; stateHistory = stateHistory + "+";}
          if(millis() - timer[3] > 500 && timer[3]){
            if(AI_HYD_psig_PT561_HydraulicInlet2 > peakPsi2A){peakPsi2A = AI_HYD_psig_PT561_HydraulicInlet2;}
            if(AI_HYD_psig_PT561_HydraulicInlet2 >= switchingPsi2A){
              if(millis() - timer[3] > 50 && timer[3]){
                DO_HYD_XV554_DCV2_A = false;
                SUBSTATE2 = warmUp2A?NORMAL:WARMUP;
              }
            }
          }
          if((millis() - timer[3] > 30000 && timer[3]) || switchingPsi2A > deadHeadPsi2A-300){STATE = FAULT; faultString = "2A Timeout";}
        break;

        case WARMUP: //needed to prevent changes in switchingPsi from interfering with nested checks
          if(peakPsi2A >= deadHeadPsi2A - 300){ //deadheaded
            switchingPsi2A = switchingPsi2A - 20; // fine tune decrement
            count2A = 1; //switch from incrementing to fine tune decrementing
          }
          else{
            if(!count2A){switchingPsi2A = switchingPsi2A + 100; } //increment if we arent finetuning 
            else{
              if(count2A > 3){warmUp2A = true;} //make sure we dont deadhead 3 times in a row while decrement finetuning 
              else{count2A++;} //we didnt deadhead this time after finetuning 
            }
          }
          SUBSTATE2 = NORMAL;
        break;

        case NORMAL:
          if(millis() - timer[3] > switchingTime2A-550 && timer[3]){//check if minimum time has passed
            INTENSE2 = SIDE_B;
            SUBSTATE2 = STROKE;
            highCycleCnt++; //reached end of cycle time, switch sides 
          }
        break;

        default:
        break;
      }
    break;

    case SIDE_B:
      switch(SUBSTATE2){
        case STROKE:
          if(!timer[3]){ timer[3] = millis(); DO_HYD_XV557_DCV2_B = true;stateHistory = stateHistory + "-";}
          if(millis() - timer[3] > 500 && timer[3]){
            if(AI_HYD_psig_PT561_HydraulicInlet2 > peakPsi2B){peakPsi2B = AI_HYD_psig_PT561_HydraulicInlet2;}
            if(AI_HYD_psig_PT561_HydraulicInlet2 >= switchingPsi2B){
              if(millis() - timer[3] > 50 && timer[3]){
                DO_HYD_XV557_DCV2_B = false;
                SUBSTATE2 = warmUp2B?NORMAL:WARMUP;
              }
            }
          }
          if((millis() - timer[3] > 30000 && timer[3]) || switchingPsi2B > deadHeadPsi2B-300){STATE = FAULT; faultString = "2B Timeout"; timer[3] = 0;}
        break;

        case WARMUP:
          if(peakPsi2B >= deadHeadPsi2B - 300){ //deadheaded
            switchingPsi2B = switchingPsi2B - 20; // fine tune decrement
            count2B = 1; //switch from incrementing to fine tune decrementing
          }
          else{
            if(!count2B){switchingPsi2B = switchingPsi2B + 100; }
            else{
              if(count2B > 3){warmUp2B = true;}
              else{count2B++;}
            }
          }
          SUBSTATE2 = NORMAL;
        break;

        case NORMAL:
          if(millis() - timer[3] > switchingTime2B-550 && timer[3]){
            INTENSE2 = SIDE_A;
            SUBSTATE2 = STROKE;
            highCycleCnt++;
          }
        break;

        default:
        break;
      }
    break;

    case PAUSE:
      DO_HYD_XV554_DCV2_A = false;
      DO_HYD_XV557_DCV2_B = false;
      for(int i = 0; i < PTsize;i++){
        if(!manualPause && PTdata[i].overPressure && (PTdata[i].pause == 2 || !PTdata[i].pause)){ break; }
      }
      INTENSE2 = SIDE_A;
      SUBSTATE2 = STROKE;
    break;

    default:
    break;
  }
}
