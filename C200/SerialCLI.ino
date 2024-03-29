// #define SPTR_SIZE   20
// void SerialCLI() {
//   String str;
//   char *argBuf[20] = { 0 };
//   double argVal = 0;
//   while (Serial.available()) { str = Serial.readStringUntil('\n'); }

//   if (str.length() > 0) {
    
//     //Parse string function wasn't working on GIGA
//     char s[100] = { 0 };
//     int numArgs = 0;
//     int size = sizeof(argBuf);
//     strcpy(s, str.c_str());
//     argBuf[0] = strtok(s, " ");
//     //Serial.print(argBuf[0]);
//     for (numArgs = 1; NULL != (argBuf[numArgs] = strtok(NULL, " ")); numArgs++) {
//       //Serial.println(argBuf[numArgs] );
//       if (size == numArgs) { break; }
//     }
    
//     for (int n = 0; n < numArgs; n++) {
//       String argStr = argBuf[n];
//       int digital = false;
//       for(int i = 0; i < sizeof(DOdata); i++){
//         if(argStr.equalsIgnoreCase(DOdata[i].key)){
//           String argStrVal = argBuf[++n];
//           if(argStrVal.equalsIgnoreCase("false")){
//             *DOdata[i].value = false;
//           }
//           else if(argStrVal.equalsIgnoreCase("true")){
//             *DOdata[i].value = true;
//           }
//           else{ *DOdata[i].value = !*DOdata[i].value; }
//           digital = true;
//           break;
//         }
//       }
//       if(digital){;}
//       else if(argStr.equalsIgnoreCase("delay")) {
//         String argStrVal = argBuf[++n];
//         argVal = argStrVal.toInt();
//         if(argVal > 0){printDelay = argVal;}
//       }
   
//       else if(argStr.equalsIgnoreCase("help") || argStr.equalsIgnoreCase("h")){
//         printHelp();
//       }

//       else if(argStr.equalsIgnoreCase("raw")){
//         rawPrint = !rawPrint;
//       }

//       else if(argStr.equalsIgnoreCase("packet")){
//         printMode = (printMode == PACKET)?OFF:PACKET;
//       }

//       else if(argStr.equalsIgnoreCase("alldata")){
//         printMode = (printMode == ALL)?OFF:ALL;
//       }

//       else if(argStr.equalsIgnoreCase("digitalin")){
//         printMode = (printMode == DIGITAL_IN)?OFF:DIGITAL_IN;
//       }

//       else if(argStr.equalsIgnoreCase("digitalout")){
//         printMode = (printMode == DIGITAL_OUT)?OFF:DIGITAL_OUT;
//       }
//       else if(argStr.equalsIgnoreCase("analogin")){
//         printMode = (printMode == ANALOG_IN)?OFF:ANALOG_IN;
//       }

//       else if(argStr.equalsIgnoreCase("analogout")){
//         printMode = (printMode == ANALOG_OUT)?OFF:ANALOG_OUT;
//       }

//       else if(argStr.equalsIgnoreCase("pretty")){
//         prettyPrint = !prettyPrint;
//       }

//       // else if(argStr.equalsIgnoreCase("keys")){
//       //   keyPrint = !keyPrint;
//       // }

//       else if(argStr.equalsIgnoreCase("stop") || argStr.equalsIgnoreCase("quiet")){
//         printMode = OFF;
//       }

//       else{
//         Serial.println("Invalid Entry");
//       }
//     }
//   }
// }

// void printHelp(){
//   Serial.println("################## HELP MENU ##################");
//   Serial.println("bl508   <speed>    -> Blower speed");
//   Serial.println("fcv134  <position> -> Burner Fuel Valve position (2000 is closed)");
//   Serial.println("pmp204  <speed>    -> Water Pump speed");
//   Serial.println("fcv141  <position> -> SR Fuel Valve position");
//   Serial.println("fcv205  <position> -> HX Water Valve position");
//   Serial.println("fcv549  <position> -> new sr hydrogen ??? Valve position");
//   Serial.println("bmm                -> BMM Enable");
//   Serial.println("psa                -> Toggle PSA enable output signal");
//   Serial.println("green              -> Toggle green pilot light");
//   Serial.println("amber              -> Toggle amber pilot light");
//   Serial.println("xv501              -> Toggle XV501");
//   Serial.println("xv419              -> Toggle XV419");
//   Serial.println("xv305              -> Toggle XV305");
//   Serial.println("xv422              -> Toggle XV422");
//   Serial.println("xv1100             -> Toggle XV1100");
//   Serial.println("xv143              -> Toggle XV143");
//   Serial.println("twv308             -> Toggle from vent <--> reformer");
//   Serial.println("twv310             -> Toggle from vent <--> PSA");
//   Serial.println("off                -> Turn off ALL devices");
//   Serial.println("packet             -> Print data in standardized packet format");
//   Serial.println("digitalin          -> Print all gpio digital inputs");
//   Serial.println("digitalout         -> Print all gpio digital outputs");
//   Serial.println("analogin           -> Print all ADC values");
//   Serial.println("analogout          -> Print all DAC values");
//   Serial.println("printall           -> Print all data");
//   Serial.println("delay              -> Designate print delay time in ms (default: 1000)");
//   Serial.println("pretty             -> Toggle print mode labeled lists <--> csv list");
//   Serial.println("keys               -> Toggle print mode labeled keys <--> csv list");
//   Serial.println("raw                -> Toggle to print only raw values");
//   Serial.println("stop               -> Silence all printouts");
//   Serial.println("quiet              -> Silence all printouts");
//   Serial.println("help/h             -> This help menu");
//   Serial.println("###############################################");

// }

void dataPrint(){
  if(!printMode){return;}
  String msg = "{";
  switch(printMode){
    case PACKET:
      for(int i = 0; i < sizeof(TTdata);i++){
        if(*TTdata[i].value != prevTT[i]){
          if(msg != "{"){msg = msg + ",";}
          msg = msg + "\"" + TTdata[i].key + "\":" + *TTdata[i].value;
          prevTT[i] = *TTdata[i].value;
        }
      }

      for(int i = 0; i < sizeof(PTdata);i++){
        if(*PTdata[i].value != prevPT[i]){
          if(msg != "{"){msg = msg + ",";}
          msg = msg + "\"" + PTdata[i].key + "\":" + *PTdata[i].value;
          prevPT[i] = *PTdata[i].value;
        }
      }

      for(int i = 0; i < sizeof(DOdata);i++){
        if(*DOdata[i].value != prevDO[i]){
          if(msg != "{"){msg = msg + ",";}
          msg = msg + "\"" + DOdata[i].key + "\":" + *DOdata[i].value;
          prevDO[i] = *DOdata[i].value;
        }
      }

      for(int i = 0; i < sizeof(DIdata);i++){
        if(*DIdata[i].value != prevDI[i]){
          if(msg != "{"){msg = msg + ",";}
          msg = msg + "\"" + DIdata[i].key + "\":" + *DIdata[i].value;
          prevDI[i] = *DIdata[i].value;
        }
      }
      if(msg != "{"){
        msg = msg + "}";
        Serial.println(msg);
      }
    break;

    case ANALOG_IN:
      for(int i = 0; i < sizeof(TTdata); i++){
        if(prettyPrint){
          Serial.print("\"");
          Serial.print(TTdata[i].name);
          Serial.print("\": ");
        }
        Serial.print(rawPrint?TTdata[i].raw:*TTdata[i].value);
        if(prettyPrint){ Serial.println(); }
        else{ Serial.print(", "); }
      }

      for(int i = 0; i < sizeof(PTdata); i++){
        if(prettyPrint){
          Serial.print("\"");
          Serial.print(PTdata[i].name);
          Serial.print("\": ");
        }
        Serial.print(rawPrint?PTdata[i].raw:*PTdata[i].value);
        if(prettyPrint){ Serial.println(); }
        else{ Serial.print(", "); }
      }
    break;

    case DIGITAL_IN:
      for(int i = 0; i < sizeof(PTdata); i++){
        if(prettyPrint){
          Serial.print("\"");
          Serial.print(DIdata[i].name);
          Serial.print("\": ");
        }
        Serial.print(*DIdata[i].value);
        if(prettyPrint){ Serial.println(); }
        else{ Serial.print(", "); }
      }
    break;

    case DIGITAL_OUT:
      for(int i = 0; i < sizeof(DOdata); i++){
        if(prettyPrint){
          Serial.print("\"");
          Serial.print(DOdata[i].name);
          Serial.print("\": ");
        }
        Serial.print(*DOdata[i].value);
        if(prettyPrint){ Serial.println(); }
        else{ Serial.print(", "); }
      }
    break;
  
  default:
  break;
  }
}
 