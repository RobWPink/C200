#include <Wire.h>
#include <SPI.h>
#include <ADS7828.h>
#include <PI4IOE5V6534Q.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_I2CRegister.h>
#include "Adafruit_MCP9600.h"
#include <ModbusMaster_oneH2.h>
#include <ModbusRTU.h>
#include <RunningAverage.h>
#include <LedHelper.h>
/*

change hydraulic to be fast but everything else slow for data scan

1600->3000 = y = 1.9893x + 7521.3
1000->1600 = y = 7.1429x - 657.14

if(suction > 1600){
  max = 1.9893*suction + 7521.3
}
else{
  max = 7.1429*suction - 657.14
}


*/
#define MODBUS_ID 22
#define RE_DE1 12

#define ESTOP_BREAK 40
#define LED_PWR 22
#define TRACO_24VDC 23

// resistance at 25 degrees C
#define THERMISTORNOMINAL 10000
// temp. for nominal resistance (almost always 25 C)
#define TEMPERATURENOMINAL 25
#define SERIESRESISTOR 10000
#define BCOEFFICIENT -2750.4  //good
#define BCOEFFICIENT_d -2350
#define BCOEFFICIENT_c -2650.4
#define BCOEFFICIENT_e 3988

enum state { IDLE_OFF,
             IDLE_ON,
             PRODUCTION,
             PAUSE,
             SHUTDOWN,
             MANUAL_CONTROL,
             FAULT,
             ESTOP
} STATE,
  PREV_STATE,
  CHANGED_STATE;

enum prnt {
  OFF,
  ALL,
  PACKET,
  DIGITAL_IN,
  DIGITAL_OUT,
  PT,
  TT,
  debug
} printMode;

TwoWire i2c(20, 21);
//ModbusMaster mbLocal;
PI4IOE5V6534Q gpio1(0x22, i2c);
PI4IOE5V6534Q gpio2(0x23, i2c);
Adafruit_MCP9600 mcp1;
Adafruit_MCP9600 mcp2;
Adafruit_MCP9600 mcp3;
Adafruit_MCP9600 mcp4;
Adafruit_MCP9600 mcp5;
Adafruit_MCP9600 mcp6;
Adafruit_MCP9600 nul;

ADS7828 adc1(0x48);
ADS7828 adc2(0x49);
ADS7828 adc3(0x4A);
SmallMatrix smallMatrix[3] = { SmallMatrix(0x70), SmallMatrix(0x71), SmallMatrix(0x72) };
LargeMatrix bigMatrix[3] = { LargeMatrix(0x73), LargeMatrix(0x74), LargeMatrix(0x75) };
Adafruit_LiquidCrystal lcd(0);

char ln1[16] = {'\0'};
char ln2[16] = {'\0'};
char ln3[16] = {'\0'};

String lsrError = "";
int MOVING_AVG_SIZE = 5;
int switchingPsi1 = 1950;
int switchingPsi2 = 1600;
struct vars{
  String name;
  String key;
  int* value;
  int prev;
} varData[] = {
  {"SwitchingPressure1","SWPSI1",&switchingPsi1,0},
  {"SwitchingPressure2","SWPSI2",&switchingPsi2,0},
  {"MovingAverageSize","MAVGSZ",&MOVING_AVG_SIZE,0},
  {"SwitchingPressure1","SWPSI1",&switchingPsi1,0}
};
int varSize = 4;
unsigned long lsrReset = 0;
bool plot = false;
String faultString = "";
unsigned long daughterPrintTimer2 = 0;
unsigned long loopTimer = 0;
double loopTime = 0;
bool daughterTog = false;
String errMsg[30] = {""};
int lowCycleCnt = 0;
int highCycleCnt = 0;
int j,k,l= 0;
bool flashTog[3] = { false };
bool tog[5] = {false};
bool c50setup = true;
bool firstLowSide, firstHighSide = false;
bool strokeLowSide, strokeHighSide = false;
bool prettyPrint, rawPrint, errorPrint = false;
bool virtualRedButton, virtualGreenButton, virtualAmberButton = false;
bool prevG,prevA,prevR = false;
bool manualPause = false;
bool manualMode = false;
unsigned long holdR = 0;
unsigned long timer[5] = { 0 };
unsigned long flashTimer[3] = { 0 };
unsigned long dataPrintTimer,daughterPrintTimer = 0;

int flashGreen, flashAmber, flashRed = 0;
int delayTime = 100;
int cycleCnt = 0;
int scrollCnt = 0;

//double COMPRESSION_RATIO = 5.75;

double AI_HYD_C_TT454_HydraulicTank = 0;
double AI_CLT_C_TT107_CoolantSupply1 = 0;
double AI_CLT_C_TT207_CoolantSupply2 = 0;
double AI_H2_C_TT917_Stage1SuctionTank = 0;
double AI_H2_C_TT809_Stage1Discharge1 = 0;
double AI_H2_C_TT810_Stage1Discharge2 = 0;
double AI_H2_C_TT715_Stage2SuctionTank = 0;
double AI_H2_C_TT520_Stage2Discharge = 0;
double AI_H2_C_TT521_Stage3Suction = 0;
double AI_H2_C_TT522_Stage3Discharge = 0;
double AI_H2_C_TT701_DischargePreTank = 0;
double AI_H2_psig_PT911_SuctionTank = 0;
double AI_H2_psig_PT716_Stage1_Discharge = 0;
double AI_H2_psig_PT712_Stage1_DischargeTank = 0;
double AI_H2_psig_PT519_Stage2_Discharge = 0;
double AI_H2_psig_PT407_Stage3_Discharge = 0;
double AI_H2_psig_PT410_Output = 0;
double AI_HYD_psig_PT467_HydraulicInlet1 = 0;
double AI_HYD_psig_PT561_HydraulicInlet2 = 0;
double AI_HYD_psig_PT556_HydraulicIntake2_B = 0;
double AI_HYD_psig_PT555_HydraulicIntake2_A = 0;
double AI_CLT_psig_PT113_CoolantSupply1 = 0;
double AI_CLT_psig_PT213_CoolantSupply2 = 0;
double AI_HYD_psig_PT461_HydraulicIntake1_A = 0;
double AI_HYD_psig_PT462_HydraulicIntake1_B = 0;
uint8_t DI_Comm_GSR_Global = 0;
uint8_t DI_Comm_LSRfb_Local = 0;
uint8_t DI_Comm_LSR_Local = 0;
uint8_t DI_Encl_PilotGreen_Feedback = 0;
uint8_t DI_Encl_PilotAmber_Feedback = 0;
uint8_t DI_Encl_PilotRed_Feedback = 0;
uint8_t DI_H2_XV907_SuctionPreTank_Feedback = 0;
uint8_t DI_HYD_XV460_DCV1_A_feedback = 0;
uint8_t DI_HYD_XV463_DCV1_B_feedback = 0;
uint8_t DI_HYD_XV554_DCV2_A_feedback = 0;
uint8_t DI_HYD_XV557_DCV2_B_feedback = 0;
uint8_t DI_CLT_FCU112_CoolantFan1_Feedback = 0;
uint8_t DI_CLT_FCU212_CoolantFan2_Feedback = 0;
uint8_t DI_HYD_PMP458_HydraulicPump1_Feedback = 0;
uint8_t DI_HYD_PMP552_HydraulicPump2_Feedback = 0;
uint8_t DI_CLT_PMP104_PMP204_CoolantPumps_Feedback = 0;
uint8_t DI_Encl_ESTOP = 0;
uint8_t DI_Encl_ButtonGreen = 0;
uint8_t DI_Encl_ButtonAmber = 0;
uint8_t DI_Encl_ButtonRed = 0;
uint8_t DI_HYD_LS000_HydraulicLevelSwitch = 0;
uint8_t DI_CLT_FS000_CoolantFlowSwitch = 0;
uint8_t DI_HYD_IS000_HydraulicFilterSwitch = 0;
uint8_t DI_H2_AN000_ContaminationOnSide1 = 0;
uint8_t DI_H2_AN000_ContaminationOnSide2 = 0;
uint8_t DO_Comm_LSR_Local = 0;
uint8_t DO_Comm_LSR_Reset = 0;
uint8_t DO_Encl_PilotGreen = 0;
uint8_t DO_Encl_PilotAmber = 0;
uint8_t DO_Encl_PilotRed = 0;
uint8_t DO_H2_XV907_SuctionPreTank = 0;
uint8_t DO_HYD_XV460_DCV1_A = 0;
uint8_t DO_HYD_XV463_DCV1_B = 0;
uint8_t DO_HYD_XV554_DCV2_A = 0;
uint8_t DO_HYD_XV557_DCV2_B = 0;
uint8_t DO_CLT_FCU112_CoolantFan1_Enable = 0;
uint8_t DO_CLT_FCU212_CoolantFan2_Enable = 0;
uint8_t DO_HYD_PMP458_HydraulicPump1_Enable = 0;
uint8_t DO_HYD_PMP552_HydraulicPump2_Enable = 0;
uint8_t DO_CLT_PMP104_PMP204_CoolantPumps_Enable = 0;


RunningAverage avgTT454(MOVING_AVG_SIZE);
RunningAverage avgTT107(MOVING_AVG_SIZE);
RunningAverage avgTT207(MOVING_AVG_SIZE);
RunningAverage avgTT917(MOVING_AVG_SIZE);
RunningAverage avgTT809(MOVING_AVG_SIZE);
RunningAverage avgTT810(MOVING_AVG_SIZE);
RunningAverage avgTT715(MOVING_AVG_SIZE);
RunningAverage avgTT520(MOVING_AVG_SIZE);
RunningAverage avgTT521(MOVING_AVG_SIZE);
RunningAverage avgTT522(MOVING_AVG_SIZE);
RunningAverage avgTT701(MOVING_AVG_SIZE);
RunningAverage avgPT911(MOVING_AVG_SIZE);
RunningAverage avgPT716(MOVING_AVG_SIZE);
RunningAverage avgPT712(MOVING_AVG_SIZE);
RunningAverage avgPT519(MOVING_AVG_SIZE);
RunningAverage avgPT407(MOVING_AVG_SIZE);
RunningAverage avgPT410(MOVING_AVG_SIZE);
RunningAverage avgPT467(MOVING_AVG_SIZE);
RunningAverage avgPT561(MOVING_AVG_SIZE);
RunningAverage avgPT556(MOVING_AVG_SIZE);
RunningAverage avgPT555(MOVING_AVG_SIZE);
RunningAverage avgPT113(MOVING_AVG_SIZE);
RunningAverage avgPT213(MOVING_AVG_SIZE);
RunningAverage avgFM904(MOVING_AVG_SIZE);
RunningAverage avgFM110(MOVING_AVG_SIZE);
RunningAverage avgPT461(MOVING_AVG_SIZE);
RunningAverage avgPT462(MOVING_AVG_SIZE);
RunningAverage avgPMP458(MOVING_AVG_SIZE);
RunningAverage avgFCU112(MOVING_AVG_SIZE);

//   { "AI_H2_?_FM904_SuctionFlow", "FM904", 0, 0, 0, avgFM904, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, false },             //
//   { "AI_H2_?_FM110_CoolantFlow1", "FM110", 0, 0, 0, avgFM110, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, false },            //

struct tt {
  String name;
  String key;
  double raw;
  double rawTemp;
  RunningAverage avg;
  double* value;
  double prev;
  int min;
  int minRecovery;
  int max;
  int maxRecovery;
  double coef;
  ADS7828 adc;
  int mcp;
  int channel;
  bool overheat;
} TTdata[] = {
  { "AI_HYD_C_TT454_HydraulicTank", "TT454", 0, 0, avgTT454, &AI_HYD_C_TT454_HydraulicTank,       0, 20, 23, 55, 30, BCOEFFICIENT, adc1, -1, 0, false },    //0
  //{ "AI_CLT_C_TT107_CoolantSupply1", "TT107", 0, 0, avgTT107, &AI_CLT_C_TT107_CoolantSupply1,   0, -1, -1, -1, -1, BCOEFFICIENT, adc1, 1, -1, false },  //1
  { "AI_CLT_C_TT207_CoolantSupply2", "TT207", 0, 0, avgTT207, &AI_CLT_C_TT207_CoolantSupply2,     0, -1, -1, -1, -1, BCOEFFICIENT, adc1, -1, 1, false },  //2
  { "AI_H2_C_TT917_Stage1SuctionTank", "TT917", 0, 0, avgTT917, &AI_H2_C_TT917_Stage1SuctionTank, 0, -1, -1, 50, 30, BCOEFFICIENT, adc1, -1, 3, false },          //3
  { "AI_H2_C_TT809_Stage1Discharge1", "TT809", 0, 0, avgTT809, &AI_H2_C_TT809_Stage1Discharge1,   0, -1, -1, 135, 110, 1, -1, 1, -1, false },      //4
  { "AI_H2_C_TT810_Stage1Discharge2", "TT810", 0, 0, avgTT810, &AI_H2_C_TT810_Stage1Discharge2,   0, -1, -1, 135, 110, 1, -1, 2, -1, false },      //5
  //{ "AI_H2_C_TT701_DischargePreTank", "TT701", 0, 0, avgTT701, &AI_H2_C_TT701_DischargePreTank, 0, -1, -1, -1, -1, 1, -1, -1, -1, false },
  { "AI_H2_C_TT715_Stage2SuctionTank", "TT715", 0, 0, avgTT715, &AI_H2_C_TT715_Stage2SuctionTank, 0, -1, -1, 150, 140, 1, -1, 3, -1, false },    //6
  { "AI_H2_C_TT520_Stage2Discharge", "TT520", 0, 0, avgTT520, &AI_H2_C_TT520_Stage2Discharge,     0, -1, -1, 150, 140, 1, -1, 4, -1, false },      //7
  { "AI_H2_C_TT521_Stage3Suction", "TT521", 0, 0, avgTT521, &AI_H2_C_TT521_Stage3Suction,         0, -1, -1, 150, 140, 1, -1, 5, -1, false },        //8
  { "AI_H2_C_TT522_Stage3Discharge", "TT522", 0, 0, avgTT522, &AI_H2_C_TT522_Stage3Discharge,     0, -1, -1, 150, 140, 1, -1, 6, -1, false }  //9
};
int TTsize = 9;

struct pt {
  String name;
  String key;
  double raw;
  double mapped;
  double offset;
  RunningAverage avg;
  double* value;
  double prev;
  int min;
  int minRecovery;
  int max;
  int maxRecovery;
  int mapA;
  int mapB;
  int mapC;
  int mapD;
  ADS7828 adc;
  int channel;
  bool overPressure;
} PTdata[] = {
  { "AI_H2_psig_PT712_Stage1_DischargeTank", "PT712", 0, 0, 0, avgPT712, &AI_H2_psig_PT712_Stage1_DischargeTank, 0, 1000, 1250, 4000, 3800, 820, 4096, 0, 10000, adc2, 3, false }, //2
  { "AI_H2_psig_PT407_Stage3_Discharge", "PT407", 0, 0, 0, avgPT407, &AI_H2_psig_PT407_Stage3_Discharge, 0, -1, -1, -1, -1, 820, 4096, 0, 20000, adc2, 7, false },           //4 //used as final tank
  { "AI_H2_psig_PT410_Output",           "PT410", 0, 0, 0, avgPT410, &AI_H2_psig_PT410_Output, 0, -1, -1, -1, -1, 820, 4096, 0, 20000, adc3, 0, false },                             //5 //used
  //{ "AI_HYD_psig_PT467_HydraulicInlet1", "PT467", 0, 0, 0, avgPT467, &AI_HYD_psig_PT467_HydraulicInlet1, 0, -1, -1, -1, -1, 820, 4096, 0, 5000, adc3, 1, false },         //6 //used
  { "AI_HYD_psig_PT561_HydraulicInlet2", "PT561", 0, 0, 0, avgPT561, &AI_HYD_psig_PT561_HydraulicInlet2, 0, -1, -1, -1, -1, 820, 4096, 0, 5000, adc3, 1, false }/*,         //7 //used
  { "AI_CLT_psig_PT113_CoolantSupply1", "PT113", 0, 0, 0, avgPT113, &AI_CLT_psig_PT113_CoolantSupply1, 0, -1, -1, -1, -1, -1, -1, -1, -1, adc3, 5, false },                  //10
  { "AI_CLT_psig_PT213_CoolantSupply2", "PT213", 0, 0, 0, avgPT213, &AI_CLT_psig_PT213_CoolantSupply2, 0, -1, -1, -1, -1, -1, -1, -1, -1, adc3, 6, false },                  //11
  { "AI_HYD_psig_PT461_HydraulicIntake1_A", "PT461", 0, 0, 0, avgPT461, &AI_HYD_psig_PT461_HydraulicIntake1_A, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, false },           //12
  { "AI_HYD_psig_PT462_HydraulicIntake1_B", "PT462", 0, 0, 0, avgPT462, &AI_HYD_psig_PT462_HydraulicIntake1_B, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, false }*/            //13
};

// { "AI_H2_psig_PT911_SuctionTank", "PT911", 0, 0, -150, avgPT911, &AI_H2_psig_PT911_SuctionTank, -1, -1, -1, 819, 3900, 0  , 500, adc2, 3, false },                       //0
//   { "AI_H2_psig_PT716_Stage1_Discharge", "PT716", 0, 0, 50, avgPT716, &AI_H2_psig_PT716_Stage1_Discharge, -1, -1, -1, 820, 4096, 0, 2000, adc2, 4, false },          //1
//   { "AI_H2_psig_PT712_Stage1_DischargeTank", "PT712", 0, 0, -65, avgPT712, &AI_H2_psig_PT712_Stage1_DischargeTank, -1, -1, -1, 810, 3090, 0, 2000, adc2, 5, false },  //2
//   { "AI_H2_psig_PT519_Stage2_Discharge", "PT519", 0, 0, 0, avgPT519, &AI_H2_psig_PT519_Stage2_Discharge, -1, -1, -1, -1, -1, -1, -1, adc2, 6, false },                //3
//   { "AI_H2_psig_PT407_Stage3_Discharge", "PT407", 0, 0, 0, avgPT407, &AI_H2_psig_PT407_Stage3_Discharge, -1, -1, -1, 819, 3900, 0, 2000, adc2, 7, false },           //4
//   { "AI_H2_psig_PT410_Output", "PT410", 0, 0, 200, avgPT410, &AI_H2_psig_PT410_Output, -1, -1, -1, 820, 4096, 0, 2000, adc3, 0, false },                             //5
//   { "AI_HYD_psig_PT467_HydraulicInlet1", "PT467", 0, 0, -200, avgPT467, &AI_HYD_psig_PT467_HydraulicInlet1, -1, -1, -1, 820, 3900, 0, 5000, adc3, 1, false },         //6
//   { "AI_HYD_psig_PT561_HydraulicInlet2", "PT561", 0, 0, -200, avgPT561, &AI_HYD_psig_PT561_HydraulicInlet2, -1, -1, -1, 820, 3900, 0, 5000, adc3, 2, false },         //7
//   { "AI_HYD_psig_PT556_HydraulicIntake2_B", "PT556", 0, 0, 0, avgPT556, &AI_HYD_psig_PT556_HydraulicIntake2_B, -1, -1, -1, -1, -1, -1, -1, adc3, 3, false },          //8
//   { "AI_HYD_psig_PT555_HydraulicIntake2_A", "PT555", 0, 0, 0, avgPT555, &AI_HYD_psig_PT555_HydraulicIntake2_A, -1, -1, -1, -1, -1, -1, -1, adc3, 4, false },          //9
//   { "AI_CLT_psig_PT113_CoolantSupply1", "PT113", 0, 0, 0, avgPT113, &AI_CLT_psig_PT113_CoolantSupply1, -1, -1, -1, -1, -1, -1, -1, adc3, 5, false },                  //10
//   { "AI_CLT_psig_PT213_CoolantSupply2", "PT213", 0, 0, 0, avgPT213, &AI_CLT_psig_PT213_CoolantSupply2, -1, -1, -1, -1, -1, -1, -1, adc3, 6, false },                  //11
//   { "AI_HYD_psig_PT461_HydraulicIntake1_A", "PT461", 0, 0, 0, avgPT461, &AI_HYD_psig_PT461_HydraulicIntake1_A, -1, -1, -1, -1, -1, -1, -1, -1, -1, false },           //12
//   { "AI_HYD_psig_PT462_HydraulicIntake1_B", "PT462", 0, 0, 0, avgPT462, &AI_HYD_psig_PT462_HydraulicIntake1_B, -1, -1, -1, -1, -1, -1, -1, -1, -1, false }            //13

int PTsize = 4;

int DIsize = 25;
int DOsize = 12;
struct digital {
  String name;
  String key;
  uint8_t* value;
  uint8_t prev;
  PI4IOE5V6534Q gpio;
  int addr;
} DIdata[] = {
 
 { "DI_Comm_GSR_Global", "GSR", &DI_Comm_GSR_Global, 0, gpio1, P0_0 },  //0
  { "DI_Comm_LSRfb_Local", "LSRfb", &DI_Comm_LSRfb_Local, 0, gpio1, P0_1 },   //1
  //{"","",&, 0,-1,-1},                            //2 //gpio1.digitalRead(P0_2,		REMOTE RESETRLY OP
  { "DI_Encl_PilotGreen_Feedback", "GPLfb", &DI_Encl_PilotGreen_Feedback, 0, gpio1, P0_3 },                                   //3
  { "DI_Encl_PilotAmber_Feedback", "APLfb", &DI_Encl_PilotAmber_Feedback, 0, gpio1, P0_4 },                                   //4  //D_HYDR_FLUID_LEVELS -martin
  { "DI_Encl_PilotRed_Feedback", "RPLfb", &DI_Encl_PilotRed_Feedback, 0, gpio1, P0_5 },                                       //5
  { "DI_H2_XV907_SuctionPreTank_Feedback", "XV907fb", &DI_H2_XV907_SuctionPreTank_Feedback, 0, gpio1, P0_6 },                 //6
  { "DI_HYD_XV460_DCV1_A_feedback", "XV460fb", &DI_HYD_XV460_DCV1_A_feedback, 0, gpio1, P0_7 },                               //7 //used
  { "DI_HYD_XV463_DCV1_B_feedback", "XV463fb", &DI_HYD_XV463_DCV1_B_feedback, 0, gpio1, P1_0 },                               //8 //used
  { "DI_HYD_XV554_DCV2_A_feedback", "XV554fb", &DI_HYD_XV554_DCV2_A_feedback, 0, gpio1, P1_1 },                               //9
  { "DI_HYD_XV557_DCV2_B_feedback", "XV557fb", &DI_HYD_XV557_DCV2_B_feedback, 0, gpio1, P1_2 },                               //10
  { "DI_CLT_FCU112_CoolantFan1_Feedback", "FCU112fb", &DI_CLT_FCU112_CoolantFan1_Feedback, 0, gpio1, P1_3 },                  //11 //used
  { "DI_CLT_FCU212_CoolantFan2_Feedback", "FCU212fb", &DI_CLT_FCU212_CoolantFan2_Feedback, 0, gpio1, P1_4 },                  //12 
  { "DI_HYD_PMP458_HydraulicPump1_Feedback", "PMP458fb", &DI_HYD_PMP458_HydraulicPump1_Feedback, 0, gpio1, P1_5 },            //13
  { "DI_HYD_PMP552_HydraulicPump2_Feedback", "PMP552fb", &DI_HYD_PMP552_HydraulicPump2_Feedback, 0, gpio1, P1_6 },            //14
  { "DI_CLT_PMP104_PMP204_CoolantPumps_Feedback", "CLTPMPfb", &DI_CLT_PMP104_PMP204_CoolantPumps_Feedback, 0, gpio1, P1_7 },  //15
  //{"","",&, 0,-1,-1},                                                       //16 //gpio1.digitalRead(P2_0,			COOLANT PUMPS LATCH RLY OP
  { "DI_Encl_ESTOP", "ESTOP", &DI_Encl_ESTOP, 0, gpio1, P2_1 },                                           //17
  { "DI_Encl_ButtonGreen", "GBN", &DI_Encl_ButtonGreen, 0, gpio1, P2_2 },                                 //18
  { "DI_Encl_ButtonAmber", "ABN", &DI_Encl_ButtonAmber, 0, gpio1, P2_3 },                                 //19
  { "DI_Encl_ButtonRed", "RBN", &DI_Encl_ButtonRed, 0, gpio1, P2_4 },                                     //20
  { "DI_Comm_LSR_Local", "LSR", &DI_Comm_LSR_Local, 0, gpio1, P2_5 },                                    //21
  { "DI_HYD_LS000_HydraulicLevelSwitch", "LVLSW", &DI_HYD_LS000_HydraulicLevelSwitch, 0, gpio1, P2_6 },   //22
  { "DI_CLT_FS000_CoolantFlowSwitch", "FLSW", &DI_CLT_FS000_CoolantFlowSwitch, 0, gpio1, P2_7 },          //23
  { "DI_HYD_IS000_HydraulicFilterSwitch", "FISW", &DI_HYD_IS000_HydraulicFilterSwitch, 0, gpio1, P3_0 },  //24 //D_HYDR_DIF_SWITCH -martin
  { "DI_H2_AN000_ContaminationOnSide1", "CONT1", &DI_H2_AN000_ContaminationOnSide1, 0, gpio1, P3_1 },     //25
  { "DI_H2_AN000_ContaminationOnSide2", "CONT2", &DI_H2_AN000_ContaminationOnSide2, 0, gpio1, P3_2 },     //26
},
  DOdata[] = {
    { "DO_Comm_LSR_Local", "LSROUT", &DO_Comm_LSR_Local, 0, gpio1, P3_3 },
    { "DO_Comm_LSR_Reset", "LSRRST", &DO_Comm_LSR_Reset, 0, gpio1, P3_4 },                                                  //0
    { "DO_Encl_PilotGreen", "GPL", &DO_Encl_PilotGreen, 0, gpio1, P3_5 },                                                 //2
    { "DO_Encl_PilotAmber", "APL", &DO_Encl_PilotAmber, 0, gpio1, P3_6 },                                                 //3
    { "DO_Encl_PilotRed", "RPL", &DO_Encl_PilotRed, 0, gpio1, P3_7 },                                                     //4
    { "DO_H2_XV907_SuctionPreTank", "XV907", &DO_H2_XV907_SuctionPreTank, 0, gpio1, P4_0 },                               //5                                           //7
    { "DO_HYD_XV554_DCV2_A", "XV554", &DO_HYD_XV554_DCV2_A, 0, gpio1, P4_1 },                                             //8 //not used
    { "DO_HYD_XV557_DCV2_B", "XV557", &DO_HYD_XV557_DCV2_B, 0, gpio2, P0_0 },                                             //9 //not used
    { "DO_CLT_FCU112_CoolantFan1_Enable", "FCU112", &DO_CLT_FCU112_CoolantFan1_Enable, 0, gpio2, P0_3 },                  //10
    { "DO_CLT_FCU212_CoolantFan2_Enable", "FCU212", &DO_CLT_FCU212_CoolantFan2_Enable, 0, gpio2, P0_4 },                  //11
    { "DO_HYD_PMP552_HydraulicPump2_Enable", "PMP552", &DO_HYD_PMP552_HydraulicPump2_Enable, 0, gpio2, P0_5 },            //13 //not used
    { "DO_CLT_PMP104_PMP204_CoolantPumps_Enable", "CLTPMP", &DO_CLT_PMP104_PMP204_CoolantPumps_Enable, 0, gpio2, P0_7 },  //14
  };
