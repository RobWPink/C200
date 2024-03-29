float fmap(float x, float a, float b, float c, float d){
  float f=x/(b-a)*(d-c)+c;
  return f;
}
float get_temp(int raw_adc) {
  float voltage = (5 / pow(2,12)) * raw_adc;
  float temp = (voltage - 0.3) / 0.005;
  return temp / 10;
}

void flashDriver(){
  if (!flashTimer[0] && flashGreen) { flashTimer[0] = millis(); }
  if (millis() - flashTimer[0] > flashGreen && flashGreen) {
    flashTog[0] = !flashTog[0];
    DO_Encl_PilotGreen = flashTog[0];
    flashTimer[0] = millis();
  }
  else if (!flashGreen && flashTimer[0]) {
    DO_Encl_PilotGreen = false;
    flashTimer[0] = 0;
  }

  if (!flashTimer[1] && flashAmber) { flashTimer[1] = millis(); }
  if (millis() - flashTimer[1] > flashAmber && flashAmber) {
    flashTog[1] = !flashTog[1];
    DO_Encl_PilotAmber = flashTog[1];
    flashTimer[1] = millis();
  }
  else if (!flashAmber && flashTimer[1]) {
    DO_Encl_PilotAmber = false;
    flashTimer[1] = 0;
  }

  if (!flashTimer[2] && flashRed) { flashTimer[2] = millis(); }
  if (millis() - flashTimer[2] > flashRed && flashRed) {
    flashTog[2] = !flashTog[2];
    DO_Encl_PilotRed = flashTog[2];
    flashTimer[2] = millis();
  }
  else if (!flashRed && flashTimer[2]) {
    DO_Encl_PilotRed = false;
    flashTimer[2] = 0;
  }
}

//void rs485Transceive(){;}

void i2cTransceive(){
  for(int i = 0; i < sizeof(TTdata);i++){
    if(TTdata[i].channel != -1){
      TTdata[i].raw = TTdata[i].adc.read(TTdata[i].channel,SD);
      if(0 <= TTdata[i].raw < 4096){
        TTdata[i].rawTemp = get_temp(TTdata[i].raw);
        TTdata[i].avg.addValue(TTdata[i].rawTemp);
        *TTdata[i].value = TTdata[i].avg.getAverage();
        if(TTdata[i].max != -1){
          if(*TTdata[i].value >= TTdata[i].max && !TTdata[i].overheat){
            TTdata[i].overheat = true;
            if(STATE >= PRODUCTION){
              PREV_STATE = STATE;
              STATE = PAUSE;
            }
          }
          else if(*TTdata[i].value <= TTdata[i].recovery && TTdata[i].overheat){
            TTdata[i].overheat = false;
             if(PREV_STATE >= PRODUCTION){
              STATE = PREV_STATE;
              PREV_STATE = PAUSE;
             }
          }
        }

        if(TTdata[i].min != -1){
          if(*TTdata[i].value <= TTdata[i].min && !TTdata[i].overheat){
            TTdata[i].overheat = true;
            if(STATE >= PRODUCTION){
              PREV_STATE = STATE;
              STATE = PAUSE;
            }
          }
          else if(*TTdata[i].value <= TTdata[i].recovery && TTdata[i].overheat){
            TTdata[i].overheat = false;
             if(PREV_STATE >= PRODUCTION){
              STATE = PREV_STATE;
              PREV_STATE = PAUSE;
             }
          }
        }
      }
    }
  }
  for(int i = 0; i < sizeof(PTdata);i++){
    if(PTdata[i].channel != -1){
      PTdata[i].raw = PTdata[i].adc.read(PTdata[i].channel,SD);
      if(0 <= PTdata[i].raw < 4096 && PTdata[i].mapA != -1 && PTdata[i].mapB != -1 && PTdata[i].mapC != -1 && PTdata[i].mapD != -1){
        PTdata[i].mapped = fmap(PTdata[i].raw, PTdata[i].mapA, PTdata[i].mapB, PTdata[i].mapC, PTdata[i].mapD) + PTdata[i].offset;
        PTdata[i].avg.addValue(PTdata[i].mapped);
        *PTdata[i].value = PTdata[i].avg.getAverage();
        if(PTdata[i].max != -1){
          if(*PTdata[i].value >= PTdata[i].max && !PTdata[i].overPressure){
            PTdata[i].overPressure = true;
            if(STATE >= PRODUCTION){
              PREV_STATE = STATE;
              STATE = PAUSE;
            }
          }
          else if(*PTdata[i].value <= PTdata[i].recovery && PTdata[i].overPressure){
            PTdata[i].overPressure = false;
          }
        }
      }
    }
  }
  for(int i = 0; i < sizeof(DIdata);i++){
    if(DIdata[i].addr != -1){
      DIdata[i].gpio.digitalRead(DIdata[i].addr,DIdata[i].value);
    }
  }
  for(int i = 0; i < sizeof(DOdata);i++){
    if(DOdata[i].addr != -1){
      DOdata[i].gpio.digitalWrite(DOdata[i].addr,*DOdata[i].value);
    }
  }
}

void i2cSetup(){
  adc1.init();
  adc2.init();
  adc3.init();

  gpio1.begin();
  gpio1.pinMode(P0_1, INPUT_PULLUP, true);
  gpio1.pinMode(P0_2, INPUT_PULLUP, true);
  gpio1.pinMode(P0_3, INPUT_PULLUP, true);
  gpio1.pinMode(P0_4, INPUT_PULLUP, true);
  gpio1.pinMode(P0_5, INPUT_PULLUP, true);
  gpio1.pinMode(P0_6, INPUT_PULLUP, true);
  gpio1.pinMode(P0_7, INPUT_PULLUP, true);
  gpio1.pinMode(P1_0, INPUT_PULLUP, true);
  gpio1.pinMode(P1_1, INPUT_PULLUP, true);
  gpio1.pinMode(P1_2, INPUT_PULLUP, true);
  gpio1.pinMode(P1_5, INPUT_PULLUP, true);
  gpio1.pinMode(P1_6, INPUT_PULLUP, true);
  gpio1.pinMode(P1_7, INPUT_PULLUP, true);
  gpio1.pinMode(P2_0, INPUT_PULLUP, true);
  gpio1.pinMode(P2_1, INPUT_PULLUP, true);
  gpio1.pinMode(P2_2, INPUT_PULLUP, true);
  gpio1.pinMode(P2_3, INPUT_PULLUP, true);
  gpio1.pinMode(P2_4, INPUT_PULLUP, true);
  gpio1.pinMode(P2_5, INPUT_PULLUP, true);
  gpio1.pinMode(P2_6, INPUT_PULLUP, true);
  gpio1.pinMode(P2_7, INPUT_PULLUP, true);
  gpio1.pinMode(P3_0, INPUT_PULLUP, true);
  gpio1.pinMode(P3_1, INPUT_PULLUP, true);
  gpio1.pinMode(P3_2, INPUT_PULLUP, true);
  gpio1.pinMode(P3_3, OUTPUT);
  gpio1.pinMode(P3_4, OUTPUT);
  gpio1.pinMode(P3_5, OUTPUT);
  gpio1.pinMode(P3_6, OUTPUT);
  gpio1.pinMode(P3_7, OUTPUT);
  gpio1.pinMode(P4_0, OUTPUT);
  gpio1.pinMode(P4_1, OUTPUT);
  gpio1.digitalWrite(P3_3, HIGH);
  gpio1.digitalWrite(P3_4, LOW);
  gpio1.digitalWrite(P3_5, LOW);
  gpio1.digitalWrite(P3_6, LOW);
  gpio1.digitalWrite(P3_7, LOW);
  gpio1.digitalWrite(P4_0, LOW);
  gpio1.digitalWrite(P4_1, LOW);

  gpio2.begin();
  gpio2.pinMode(P0_0, OUTPUT);
  gpio2.pinMode(P0_1, OUTPUT);
  gpio2.pinMode(P0_2, OUTPUT);
  gpio2.pinMode(P0_3, OUTPUT);
  gpio2.pinMode(P0_4, OUTPUT);
  gpio2.pinMode(P0_5, OUTPUT);
  gpio2.pinMode(P0_6, OUTPUT);
  gpio2.pinMode(P0_7, OUTPUT);
  gpio2.pinMode(P1_0, INPUT_PULLUP, true);
  gpio2.pinMode(P1_1, INPUT_PULLUP, true);
  gpio2.pinMode(P1_2, INPUT_PULLUP, true);
  gpio2.pinMode(P1_5, INPUT_PULLUP, true);
  gpio2.pinMode(P1_6, INPUT_PULLUP, true);
  gpio2.pinMode(P1_7, INPUT_PULLUP, true);
  gpio2.pinMode(P2_0, INPUT_PULLUP, true);
  gpio2.pinMode(P2_1, INPUT_PULLUP, true);
  gpio2.pinMode(P2_2, INPUT_PULLUP, true);
  gpio2.pinMode(P2_3, INPUT_PULLUP, true);
  gpio2.pinMode(P2_4, INPUT_PULLUP, true);
  gpio2.pinMode(P2_5, INPUT_PULLUP, true);
  gpio2.pinMode(P2_6, INPUT_PULLUP, true);

  gpio2.pinMode(P2_7, INPUT_PULLUP, true);
  gpio2.pinMode(P3_0, INPUT_PULLUP, true);
  gpio2.pinMode(P3_1, INPUT_PULLUP, true);
  gpio2.pinMode(P3_2, INPUT_PULLUP, true);
  gpio2.pinMode(P3_3, INPUT_PULLUP, true);
  gpio2.pinMode(P3_4, INPUT_PULLUP, true);
  gpio2.pinMode(P3_5, INPUT_PULLUP, true);
  gpio2.pinMode(P3_6, INPUT_PULLUP, true);
  gpio2.pinMode(P3_7, INPUT_PULLUP, true);
  gpio2.pinMode(P4_0, INPUT_PULLUP, true);
  gpio2.pinMode(P4_1, INPUT_PULLUP, true);
  gpio2.digitalWrite(P0_0, LOW);
  gpio2.digitalWrite(P0_1, LOW);
  gpio2.digitalWrite(P0_2, LOW);
  gpio2.digitalWrite(P0_3, LOW);
  gpio2.digitalWrite(P0_4, LOW);
  gpio2.digitalWrite(P0_5, LOW);
  gpio2.digitalWrite(P0_6, LOW);
  gpio2.digitalWrite(P0_7, HIGH);
}
void preTransmission() {
  digitalWrite(RE_DE1, 1);
}

void postTransmission() {
  digitalWrite(RE_DE1, 0);
}
