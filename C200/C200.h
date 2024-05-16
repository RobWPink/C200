#include <Wire.h>
#include <SPI.h>
#include <RPC.h>
#include <ADS7828.h>
#include <PI4IOE5V6534Q.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_I2CRegister.h>
#include "Adafruit_MCP9600.h"
#include <ModbusMaster_oneH2.h>
#include <ModbusRTU.h>
#include <RunningAverage.h>
#include <LedHelper.h>
#include <Ewma.h>

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

enum state {
  IDLE_OFF,
  IDLE_ON,
  PRODUCTION,
  SHUTDOWN,
  MANUAL_STROKE,
  MANUAL_CONTROL,
  FAULT,
  ESTOP
} STATE,
  PREV_STATE,
  CHANGED_STATE;

enum comp {
  OFF,
  START,
  DEADHEAD1,
  DEADHEAD2,
  SIDE_A,
  SIDE_B,
  PAUSE
} SUB_STATE1,
  SUB_STATE2,
  PREV1,
  PREV2;

enum prnt {
  NONE,
  ALL,
  PACKET,
  DIGITAL_IN,
  DIGITAL_OUT,
  PT,
  TT,
  FM,
  VAR,
  DEBUG
} printMode;


TwoWire i2c(20, 21);

PI4IOE5V6534Q gpio1(0x22, i2c);
PI4IOE5V6534Q gpio2(0x23, i2c);

Adafruit_MCP9600 mcp1;
Adafruit_MCP9600 mcp2;
Adafruit_MCP9600 mcp3;
Adafruit_MCP9600 mcp4;
Adafruit_MCP9600 mcp5;
Adafruit_MCP9600 mcp6;

ADS7828 adc1(0x48);
ADS7828 adc2(0x49);
ADS7828 adc3(0x4A);

SmallMatrix smallMatrix[3] = { SmallMatrix(0x70), SmallMatrix(0x71), SmallMatrix(0x72) };
LargeMatrix bigMatrix[3] = { LargeMatrix(0x73), LargeMatrix(0x74), LargeMatrix(0x75) };
Adafruit_LiquidCrystal lcd(0);

String faultString = "";
String errMsg[30] = { "" };
String stateHistory1 = "";
String stateHistory2 = "";


uint8_t mcpExist = 0;
int scrollCnt, errCnt = 0;
int flashGreen, flashAmber, flashRed = 0;

bool flashTog[3] = { false };
bool tog[5] = { false };
bool plot, prettyPrint, rawPrint = false;
bool prevG, prevA, prevR = false;
bool daughterTog, manualPause, manualMode;
unsigned long timer[10] = { 0 };
unsigned long flashTimer[3] = { 0 };
unsigned long hydraulicSafetyTimer, twoTimer, loopTimer, dataTimer, pauseTimer, holdR, lcdTimer, dataPrintTimer, daughterPrintTimer = 0;
unsigned long virtualRedButton, virtualGreenButton, virtualAmberButton = 0;

double lowMax,highMax = 0;
double loopTime = 0;

double delayTime = 100;

double Stage1_Compression_RATIO = 6.5;

double lowCPMCnt_, highCPMCnt_, lowCPMCnt, highCPMCnt = 0;

double MOVING_AVG_SIZE = 0.02; //equivilent to 200 samples
double HYD_MOVING_AVG_SIZE = 0.95;

double switchingPsi1A = 0;
double switchingPsi1B = 0;
double switchingPsi2A = 0;
double switchingPsi2B = 0;

double switchingTimeLow = 1500;
double switchingTimeHigh = 1500;

double highCPM = 15;
double lowCPM = 15;

double time2CPMmult = 100;

struct var{
  String name;
  String key;
  double* value;
  double defaultValue;
  double prev;
};

var VARdata[] = {
  {"Ideal_Low_CPM","LCPM",&lowCPM,15,0},
  {"Ideal_High_CPM","HCPM",&highCPM,15,0},
  {"NonHydraulic_MovingAverage_size","NHMAS",&MOVING_AVG_SIZE,0.02,0},
  {"Hydraulic_MovingAverage_size","HYMAS",&HYD_MOVING_AVG_SIZE,0.95,0},
  {"Print_Delay_Time","PRDTM",&delayTime,100,0},
  {"Stage1_Compression_Ratio","S1CRO",&Stage1_Compression_RATIO,6.5,0}
};int VARsize = 6;


double peakPsi1A = 0;
double peakPsi1B = 0;
double peakPsi2A = 0;
double peakPsi2B = 0;

double deadHeadPsi1A = 0;
double deadHeadPsi1B = 0;
double deadHeadPsi2A = 0;
double deadHeadPsi2B = 0;

double AI_HYD_C_TT454_HydraulicTank = 0;
double AI_CLT_C_TT107_CoolantSupply1 = 0;
double AI_CLT_C_TT207_CoolantSupply2 = 0;
double AI_H2_C_TT917_Stage1_SuctionTank = 0;
double AI_H2_C_TT809_Stage1_Discharge1 = 0;
double AI_H2_C_TT810_Stage1_Discharge2 = 0;
double AI_H2_C_TT701_Stage1_DischargePreTank = 0;
double AI_H2_C_TT715_Stage2_SuctionTank = 0;
double AI_H2_C_TT520_Stage2_Discharge = 0;
double AI_H2_C_TT521_Stage3_Suction = 0;
double AI_H2_C_TT522_Stage3_Discharge = 0;
double AI_H2_psig_PT911_Stage1_SuctionTank = 0;
double AI_H2_psig_PT716_Stage1_Discharge = 0;
double AI_H2_psig_PT712_Stage1_DischargeTank = 0;
double AI_H2_psig_PT519_Stage2_Discharge = 0;
double AI_H2_psig_PT407_Stage3_Discharge = 0;
double AI_H2_psig_PT410_Stage3_DischargeTank = 0;
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

float AI_H2_KGPM_RHE28_Flow = 0;
float AI_H2_C_RHE28_Temp = 0;
float AI_H2_psig_RHE28_Pressure = 0;
float AI_H2_KGPD_RHE28_Total = 0;
float AI_H2_KGPM_RED_Flow = 0;
float AI_H2_C_RED_Temp = 0;
float AI_H2_psig_RED_Pressure = 0;
float AI_H2_KGPD_RED_Total = 0;


Ewma avgTT454(1);
Ewma avgTT107(1);
Ewma avgTT207(1);
Ewma avgTT917(1);
Ewma avgTT809(1);
Ewma avgTT810(1);
Ewma avgTT715(1);
Ewma avgTT520(1);
Ewma avgTT521(1);
Ewma avgTT522(1);
Ewma avgTT701(1);
Ewma avgPT911(MOVING_AVG_SIZE);
Ewma avgPT716(MOVING_AVG_SIZE);
Ewma avgPT712(MOVING_AVG_SIZE);
Ewma avgPT519(MOVING_AVG_SIZE);
Ewma avgPT407(MOVING_AVG_SIZE);
Ewma avgPT410(MOVING_AVG_SIZE);
Ewma avgPT467(HYD_MOVING_AVG_SIZE);
Ewma avgPT561(HYD_MOVING_AVG_SIZE);
Ewma avgPT556(MOVING_AVG_SIZE);
Ewma avgPT555(MOVING_AVG_SIZE);
Ewma avgPT113(MOVING_AVG_SIZE);
Ewma avgPT213(MOVING_AVG_SIZE);
Ewma avgFM904(MOVING_AVG_SIZE);
Ewma avgFM110(MOVING_AVG_SIZE);
Ewma avgPT461(MOVING_AVG_SIZE);
Ewma avgPT462(MOVING_AVG_SIZE);
Ewma avgPMP458(MOVING_AVG_SIZE);
Ewma avgFCU112(MOVING_AVG_SIZE);

struct fm {
  String name;
  String key;
  float* value;
  float prev;
};

struct dev {
  String name;
  uint8_t ID;
  double baud;
  fm flowData[4];

} flowMeters[] = {
  { "RHE28", 3, 19200, { { "AI_H2_KGPM_RHE28_Flow", "RHE28F", &AI_H2_KGPM_RHE28_Flow, 0 }, { "AI_H2_C_RHE28_Temp", "RHE28T", &AI_H2_C_RHE28_Temp, 0 }, { "AI_H2_psig_RHE28_Pressure", "RHE28P", &AI_H2_psig_RHE28_Pressure, 0 }, { "AI_H2_KGPD_RHE28_Total", "RHE28O", &AI_H2_KGPD_RHE28_Total, 0 } } },
  { "RED", 1, 19200, { { "AI_H2_KGPM_RED_Flow", "RHE28F", &AI_H2_KGPM_RED_Flow, 0 }, { "AI_H2_C_RED_Temp", "RHE28T", &AI_H2_C_RED_Temp, 0 }, { "AI_H2_psig_RED_Pressure", "RHE28P", &AI_H2_psig_RED_Pressure, 0 }, { "AI_H2_KGPD_RED_Total", "RHE28O", &AI_H2_KGPD_RED_Total, 0 } } }
};

struct tt {
  String name;
  String key;
  double raw;
  double rawTemp;
  Ewma avg;
  double* value;
  double prev;
  int min;
  int minRecovery;
  int minPause;
  int max;
  int maxRecovery;
  int maxPause;
  double coef;
  ADS7828 adc;
  int mcp;
  int channel;
  int pause;
  bool overheat;
} TTdata[] = {
  { "AI_HYD_C_TT454_HydraulicTank", "TT454", 0, 0, avgTT454, &AI_HYD_C_TT454_HydraulicTank, 0, 20, 23, 0, 55, 30, 0,  BCOEFFICIENT, adc1, -1, 0, 0, false },
  { "AI_CLT_C_TT107_CoolantSupply1", "TT107", 0, 0, avgTT107, &AI_CLT_C_TT107_CoolantSupply1, 0, -1, -1, 0, 140, 130, 0,  BCOEFFICIENT, adc1, -1, 1, 0, false },
  { "AI_CLT_C_TT207_CoolantSupply2", "TT207", 0, 0, avgTT207, &AI_CLT_C_TT207_CoolantSupply2, 0, -1, -1, 0, 140, 130, 0,  BCOEFFICIENT, adc1, -1, 2, 0, false },
  { "AI_H2_C_TT917_Stage1_SuctionTank", "TT917", 0, 0, avgTT917, &AI_H2_C_TT917_Stage1_SuctionTank, 0, -1, -1, 0, 140, 130, 0,  BCOEFFICIENT, adc1, -1, 3, 0, false },
  { "AI_H2_C_TT809_Stage1_Discharge1", "TT809", 0, 0, avgTT809, &AI_H2_C_TT809_Stage1_Discharge1, 0, -1, -1, 0, 140, 130, 0,  1, -1, 1, -1, 0, false },
  { "AI_H2_C_TT810_Stage1_Discharge2", "TT810", 0, 0, avgTT810, &AI_H2_C_TT810_Stage1_Discharge2, 0, -1, -1, 0, 140, 130, 0,  1, -1, 2, -1, 0, false },
  //{ "AI_H2_C_TT701_Stage1_DischargePreTank", "TT701", 0, 0, avgTT701, &AI_H2_C_TT701_Stage1_DischargePreTank, 0, -1, -1, 0, -1, -1,0,  -1,-1, -1, -1, 0, false }
  { "AI_H2_C_TT715_Stage2_SuctionTank", "TT715", 0, 0, avgTT715, &AI_H2_C_TT715_Stage2_SuctionTank, 0, -1, -1, 0, 140, 130, 0,  1, -1, 3, -1, 0, false },
  { "AI_H2_C_TT520_Stage2_Discharge", "TT520", 0, 0, avgTT520, &AI_H2_C_TT520_Stage2_Discharge, 0, -1, -1, 0, 140, 130, 0,  1, -1, 4, -1, 0, false },
  { "AI_H2_C_TT521_Stage3_Suction", "TT521", 0, 0, avgTT521, &AI_H2_C_TT521_Stage3_Suction, 0, -1, -1, 0, 140, 130, 0,  1, -1, 5, -1, 0, false },
  { "AI_H2_C_TT522_Stage3_Discharge", "TT522", 0, 0, avgTT522, &AI_H2_C_TT522_Stage3_Discharge, 0, -1, -1, 0, 140, 130, 0,  1, -1, 6, -1, 0, false },
}; int TTsize = 10;

struct pt {
  String name;
  String key;
  double raw;
  double mapped;
  double offset;
  Ewma avg;
  double* value;
  double prev;
  int min;
  int minRecovery;
  int minPause;
  int max;
  int maxRecovery;
  int maxPause;
  int mapA;
  int mapB;
  int mapC;
  int mapD;
  ADS7828 adc;
  int channel;
  bool overPressure;
} PTdata[] = {
  { "AI_H2_psig_PT911_Stage1_SuctionTank", "PT911", 0, 0, 0, avgPT911, &AI_H2_psig_PT911_Stage1_SuctionTank, 0, 75, 85, 1, 400, 390, 0, 820, 4096, 0, 500, adc2, 3, false },
  { "AI_H2_psig_PT716_Stage1_Discharge", "PT716", 0, 0, 0, avgPT716, &AI_H2_psig_PT716_Stage1_Discharge, 0, -1, -1, -1, -1, -1, -1,  820, 4096, 0, 2000, adc2, 4, false },
  { "AI_H2_psig_PT712_Stage1_DischargeTank", "PT712", 0, 0, 0, avgPT712, &AI_H2_psig_PT712_Stage1_DischargeTank, 0, 400, 450, 2, -1, -1, 1, 820, 4096, 0, 2000, adc2, 5, false },
  { "AI_H2_psig_PT519_Stage2_Discharge", "PT519", 0, 0, 0, avgPT519, &AI_H2_psig_PT519_Stage2_Discharge, 0, -1, -1, -1, -1, -1, -1,  820, 4096, 0, 10000, adc2, 6, false },
  { "AI_H2_psig_PT407_Stage3_Discharge", "PT407", 0, 0, 0, avgPT407, &AI_H2_psig_PT407_Stage3_Discharge, 0, -1, -1, -1, -1, -1, -1,  820, 4096, 0, 20000, adc2, 7, false },
  { "AI_H2_psig_PT410_Stage3_DischargeTank", "PT410", 0, 0, 0, avgPT410, &AI_H2_psig_PT410_Stage3_DischargeTank, 0, -1, -1, -1, 8100, 7500, 0,  820, 4096, 0, 20000, adc3, 0, false },
  { "AI_HYD_psig_PT467_HydraulicInlet1", "PT467", 0, 0, 0, avgPT467, &AI_HYD_psig_PT467_HydraulicInlet1, 0, -1, -1, -1, -1, -1, -1,  820, 4096, 0, 5000, adc3, 1, false },
  { "AI_HYD_psig_PT561_HydraulicInlet2", "PT561", 0, 0, 0, avgPT561, &AI_HYD_psig_PT561_HydraulicInlet2, 0, -1, -1, -1, -1, -1, -1,  820, 4096, 0, 5000, adc3, 2, false },
  { "AI_HYD_psig_PT556_HydraulicIntake2_B", "PT556", 0, 0, 0, avgPT556, &AI_HYD_psig_PT556_HydraulicIntake2_B, 0, -1, -1, -1, -1, -1, -1,  820, 4096, 0, 20000, adc3, 3, false },
  { "AI_HYD_psig_PT555_HydraulicIntake2_A", "PT555", 0, 0, 0, avgPT555, &AI_HYD_psig_PT555_HydraulicIntake2_A, 0, -1, -1, -1, -1, -1, -1,  820, 4096, 0, 20000, adc3, 4, false },
  { "AI_CLT_psig_PT113_CoolantSupply1", "PT113", 0, 0, 0, avgPT113, &AI_CLT_psig_PT113_CoolantSupply1, 0, -1, -1, -1, -1, -1, -1,  820, 4096, 0, 250, adc3, 5, false },
  { "AI_CLT_psig_PT213_CoolantSupply2", "PT213", 0, 0, 0, avgPT213, &AI_CLT_psig_PT213_CoolantSupply2, 0, -1, -1, -1, -1, -1, -1,  820, 4096, 0, 250, adc3, 6, false },
  { "AI_HYD_psig_PT461_HydraulicIntake1_A", "PT461", 0, 0, 0, avgPT461, &AI_HYD_psig_PT461_HydraulicIntake1_A, -1, -1, -1, -1, -1, -1, -1,  -1, -1, -1, -1, false },
  { "AI_HYD_psig_PT462_HydraulicIntake1_B", "PT462", 0, 0, 0, avgPT462, &AI_HYD_psig_PT462_HydraulicIntake1_B, -1, -1, -1, -1, -1, -1, -1,  -1, -1, -1, -1, false }
}; int PTsize = 14;

struct digital {
  String name;
  String key;
  uint8_t* value;
  uint8_t prev;
  PI4IOE5V6534Q gpio;
  int addr;
} DIdata[] = {
  { "DI_Comm_GSR_Global", "GSR", &DI_Comm_GSR_Global, 0, gpio1, P0_0 },
  { "DI_Comm_LSRfb_Local", "LSRfb", &DI_Comm_LSRfb_Local, 0, gpio1, P0_1 },
  { "DI_Encl_PilotGreen_Feedback", "GPLfb", &DI_Encl_PilotGreen_Feedback, 0, gpio1, P0_3 },
  { "DI_Encl_PilotAmber_Feedback", "APLfb", &DI_Encl_PilotAmber_Feedback, 0, gpio1, P0_4 },
  { "DI_Encl_PilotRed_Feedback", "RPLfb", &DI_Encl_PilotRed_Feedback, 0, gpio1, P0_5 },
  { "DI_H2_XV907_SuctionPreTank_Feedback", "XV907fb", &DI_H2_XV907_SuctionPreTank_Feedback, 0, gpio1, P0_6 },
  { "DI_HYD_XV460_DCV1_A_feedback", "XV460fb", &DI_HYD_XV460_DCV1_A_feedback, 0, gpio1, P0_7 },
  { "DI_HYD_XV463_DCV1_B_feedback", "XV463fb", &DI_HYD_XV463_DCV1_B_feedback, 0, gpio1, P1_0 },
  { "DI_HYD_XV554_DCV2_A_feedback", "XV554fb", &DI_HYD_XV554_DCV2_A_feedback, 0, gpio1, P1_1 },
  { "DI_HYD_XV557_DCV2_B_feedback", "XV557fb", &DI_HYD_XV557_DCV2_B_feedback, 0, gpio1, P1_2 },
  { "DI_CLT_FCU112_CoolantFan1_Feedback", "FCU112fb", &DI_CLT_FCU112_CoolantFan1_Feedback, 0, gpio1, P1_3 },
  { "DI_CLT_FCU212_CoolantFan2_Feedback", "FCU212fb", &DI_CLT_FCU212_CoolantFan2_Feedback, 0, gpio1, P1_4 },
  { "DI_HYD_PMP458_HydraulicPump1_Feedback", "PMP458fb", &DI_HYD_PMP458_HydraulicPump1_Feedback, 0, gpio1, P1_5 },
  { "DI_HYD_PMP552_HydraulicPump2_Feedback", "PMP552fb", &DI_HYD_PMP552_HydraulicPump2_Feedback, 0, gpio1, P1_6 },
  { "DI_CLT_PMP104_PMP204_CoolantPumps_Feedback", "CLTPMPfb", &DI_CLT_PMP104_PMP204_CoolantPumps_Feedback, 0, gpio1, P1_7 },
  { "DI_Encl_ESTOP", "ESTOP", &DI_Encl_ESTOP, 0, gpio1, P2_1 },
  { "DI_Encl_ButtonGreen", "GBN", &DI_Encl_ButtonGreen, 0, gpio1, P2_2 },
  { "DI_Encl_ButtonAmber", "ABN", &DI_Encl_ButtonAmber, 0, gpio1, P2_3 },
  { "DI_Encl_ButtonRed", "RBN", &DI_Encl_ButtonRed, 0, gpio1, P2_4 },
  { "DI_Comm_LSR_Local", "LSR", &DI_Comm_LSR_Local, 0, gpio1, P2_5 },
  { "DI_HYD_LS000_HydraulicLevelSwitch", "LVLSW", &DI_HYD_LS000_HydraulicLevelSwitch, 0, gpio1, P2_6 },
  { "DI_CLT_FS000_CoolantFlowSwitch", "FLSW", &DI_CLT_FS000_CoolantFlowSwitch, 0, gpio1, P2_7 },
  { "DI_HYD_IS000_HydraulicFilterSwitch", "FISW", &DI_HYD_IS000_HydraulicFilterSwitch, 0, gpio1, P3_0 },
  { "DI_H2_AN000_ContaminationOnSide1", "CONT1", &DI_H2_AN000_ContaminationOnSide1, 0, gpio1, P3_1 },
  { "DI_H2_AN000_ContaminationOnSide2", "CONT2", &DI_H2_AN000_ContaminationOnSide2, 0, gpio1, P3_2 },
},
  DOdata[] = {
    { "DO_Comm_LSR_Local", "LSROUT", &DO_Comm_LSR_Local, 0, gpio1, P3_3 },
    { "DO_Comm_LSR_Reset", "LSRRST", &DO_Comm_LSR_Reset, 0, gpio1, P3_4 },
    { "DO_Encl_PilotGreen", "GPL", &DO_Encl_PilotGreen, 0, gpio1, P3_5 },
    { "DO_Encl_PilotAmber", "APL", &DO_Encl_PilotAmber, 0, gpio1, P3_6 },
    { "DO_Encl_PilotRed", "RPL", &DO_Encl_PilotRed, 0, gpio1, P3_7 },
    { "DO_H2_XV907_SuctionPreTank", "XV907", &DO_H2_XV907_SuctionPreTank, 0, gpio1, P4_0 },
    { "DO_HYD_XV460_DCV1_A", "XV460", &DO_HYD_XV460_DCV1_A, 0, gpio1, P4_1 }, 
    { "DO_HYD_XV463_DCV1_B", "XV463", &DO_HYD_XV463_DCV1_B, 0, gpio2, P0_0 },
    { "DO_HYD_XV554_DCV2_A", "XV554", &DO_HYD_XV554_DCV2_A, 0, gpio2, P0_1 },
    { "DO_HYD_XV557_DCV2_B", "XV557", &DO_HYD_XV557_DCV2_B, 0, gpio2, P0_2 },
    { "DO_CLT_FCU112_CoolantFan1_Enable", "FCU112", &DO_CLT_FCU112_CoolantFan1_Enable, 0, gpio2, P0_3 },
    { "DO_CLT_FCU212_CoolantFan2_Enable", "FCU212", &DO_CLT_FCU212_CoolantFan2_Enable, 0, gpio2, P0_4 },
    { "DO_HYD_PMP458_HydraulicPump1_Enable", "PMP458", &DO_HYD_PMP458_HydraulicPump1_Enable, 0, gpio2, P0_5 },  //12
    { "DO_HYD_PMP552_HydraulicPump2_Enable", "PMP552", &DO_HYD_PMP552_HydraulicPump2_Enable, 0, gpio2, P0_6 },  //13
    { "DO_CLT_PMP104_PMP204_CoolantPumps_Enable", "CLTPMP", &DO_CLT_PMP104_PMP204_CoolantPumps_Enable, 0, gpio2, P0_7 },
  }; int DIsize = 25; int DOsize = 15;
