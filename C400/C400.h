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

#define MODBUS_ID 22
#define RE_DE1 12
#define MOVING_AVG_SIZE 200
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
             SHUTDOWN,
             MANUAL_CONTROL,
             FAULT,
             ESTOP
} STATE,
  PREV_STATE,
  CHANGED_STATE;

enum comp{
  OFF,
  START,
  ON,
  PAUSE
}INTENSE1,INTENSE2;

enum prnt {
  NONE,
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

ADS7828 adc1(0x48);
ADS7828 adc2(0x49);
ADS7828 adc3(0x4A);
SmallMatrix smallMatrix[3] = { SmallMatrix(0x70), SmallMatrix(0x71), SmallMatrix(0x72) };
LargeMatrix bigMatrix[3] = { LargeMatrix(0x73), LargeMatrix(0x74), LargeMatrix(0x75) };
Adafruit_LiquidCrystal lcd(0);

String faultString = "";
String errMsg[30] = {""};
uint8_t mcpExist = 0;
double loopTime = 0;
int switchingPsi1 = 1950;
int switchingPsi2 = 1700;
int cycleCnt,scrollCnt,errorCnt = 0;
int flashGreen, flashAmber, flashRed = 0;
int delayTime = 100;
int lowCycleCnt_,highCycleCnt_,lowCycleCnt,highCycleCnt = 0;
int j,k,l= 0;
bool flashTog[3] = { false };
bool tog[5] = {false};
bool firstLowSide, firstHighSide = false;
bool strokeLowSide, strokeHighSide = false;
bool plot, prettyPrint, rawPrint, errorPrint = false;
bool virtualRedButton, virtualGreenButton, virtualAmberButton = false;
bool prevG,prevA,prevR = false;
bool daughterTog, c50setup,lsrTog, manualPause, manualMode = false;
unsigned long timer[5] = { 0 };
unsigned long flashTimer[3] = { 0 };
unsigned long twoTimer,lsrReset,loopTimer,dataTimer,pauseTimer,holdR,lcdTimer,dataPrintTimer,daughterPrintTimer = 0;

//double COMPRESSION_RATIO = 5.75;


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
RunningAverage avgPT467(3);
RunningAverage avgPT561(3);
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

struct vars{
  String name;
  String key;
  int* value;
  int prev;
} varData[] = {
  {"SwitchingPressure1","SWPSI1",&switchingPsi1,0},
  {"SwitchingPressure2","SWPSI2",&switchingPsi2,0},
};int varSize = 2;

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
  int pause;
  bool overheat;
} TTdata[] = {
  { "AI_HYD_C_TT454_HydraulicTank", "TT454", 0, 0, avgTT454, &AI_HYD_C_TT454_HydraulicTank, 0, 20, 23, 55, 30, BCOEFFICIENT, adc1, -1, 0, 0, false },    
  //{ "AI_CLT_C_TT107_CoolantSupply1", "TT107", 0, 0, avgTT107, &AI_CLT_C_TT107_CoolantSupply1, 0, -1, -1, 140, 130, BCOEFFICIENT, adc1, -1, 1, 0, false },  
  { "AI_CLT_C_TT207_CoolantSupply2", "TT207", 0, 0, avgTT207, &AI_CLT_C_TT207_CoolantSupply2, 0, -1, -1, 140, 130, BCOEFFICIENT, adc1, -1, 2, 0, false },  
  { "AI_H2_C_TT917_Stage1_SuctionTank", "TT917", 0, 0, avgTT917, &AI_H2_C_TT917_Stage1_SuctionTank, 0, -1, -1, 140, 130, BCOEFFICIENT, adc1, -1, 3, 0, false },          
  { "AI_H2_C_TT809_Stage1_Discharge1", "TT809", 0, 0, avgTT809, &AI_H2_C_TT809_Stage1_Discharge1, 0, -1, -1, 140, 130, 1, -1, 1, -1, 0, false },      
  { "AI_H2_C_TT810_Stage1_Discharge2", "TT810", 0, 0, avgTT810, &AI_H2_C_TT810_Stage1_Discharge2, 0, -1, -1, 140, 130, 1, -1, 2, -1, 0, false },      
  //{ "AI_H2_C_TT701_Stage1_DischargePreTank", "TT701", 0, 0, avgTT701, &AI_H2_C_TT701_Stage1_DischargePreTank, 0, -1, -1, -1, -1, -1,-1, -1, -1, 0, false }  
  { "AI_H2_C_TT715_Stage2_SuctionTank", "TT715", 0, 0, avgTT715, &AI_H2_C_TT715_Stage2_SuctionTank, 0, -1, -1, 140, 130, 1, -1, 3, -1, 0, false },    
  { "AI_H2_C_TT520_Stage2_Discharge", "TT520", 0, 0, avgTT520, &AI_H2_C_TT520_Stage2_Discharge, 0, -1, -1, 140, 130, 1, -1, 4, -1, 0, false },      
  { "AI_H2_C_TT521_Stage3_Suction", "TT521", 0, 0, avgTT521, &AI_H2_C_TT521_Stage3_Suction, 0, -1, -1, 140, 130, 1, -1, 5, -1, 0, false },        
  { "AI_H2_C_TT522_Stage3_Discharge", "TT522", 0, 0, avgTT522, &AI_H2_C_TT522_Stage3_Discharge, 0, -1, -1, 140, 130, 1,-1, 6, -1, 0, false },  
}; int TTsize = 9;

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
  int pause;
  bool overPressure;
} PTdata[] = {
  //{ "AI_H2_psig_PT911_Stage1_SuctionTank", "PT911", 0, 0, 0, avgPT911, &AI_H2_psig_PT911_Stage1_SuctionTank, 0, 70, 80, 400, 390, 820, 4096, 0, 500, adc2, 3, 1, false },                       
  //{ "AI_H2_psig_PT716_Stage1_Discharge", "PT716", 0, 0, 0, avgPT716, &AI_H2_psig_PT716_Stage1_Discharge, 0, -1, -1, 1450, 1350, 820, 4096, 0, 2000, adc2, 4, 1, false },          
  { "AI_H2_psig_PT712_Stage1_DischargeTank", "PT712", 0, 0, 0, avgPT712, &AI_H2_psig_PT712_Stage1_DischargeTank, 0, 400, 450, -1, -1, 820, 4096, 0, 2000, adc2, 3, 2, false },  //changed from adc2, 5 to adc2, 3,
  //{ "AI_H2_psig_PT519_Stage2_Discharge", "PT519", 0, 0, 0, avgPT519, &AI_H2_psig_PT519_Stage2_Discharge, 0, -1, -1, -1, -1, 820, 4096, 0, 10000, adc2, 6, 2, false },                
  { "AI_H2_psig_PT407_Stage3_Discharge", "PT407", 0, 0, 0, avgPT407, &AI_H2_psig_PT407_Stage3_Discharge, 0, -1, -1, -1, -1, 820, 4096, 0, 20000, adc2, 7, 2, false },           
  { "AI_H2_psig_PT410_Stage3_DischargeTank", "PT410", 0, 0, 0, avgPT410, &AI_H2_psig_PT410_Stage3_DischargeTank, 0, -1, -1, 8100, 7500, 820, 4096, 0, 20000, adc3, 0, 2, false },                             
  //{ "AI_HYD_psig_PT467_HydraulicInlet1", "PT467", 0, 0, -300, avgPT467, &AI_HYD_psig_PT467_HydraulicInlet1, 0, -1, -1, -1, -1, 820, 4096, 0, 5000, adc3, 1, 0, false },         
  { "AI_HYD_psig_PT561_HydraulicInlet2", "PT561", 0, 0, -100, avgPT561, &AI_HYD_psig_PT561_HydraulicInlet2, 0, -1, -1, -1, -1, 820, 4096, 0, 5000, adc3, 1, 0, false }, //changed from adc3, 2 to adc3, 1,
  //{ "AI_HYD_psig_PT556_HydraulicIntake2_B", "PT556", 0, 0, 0, avgPT556, &AI_HYD_psig_PT556_HydraulicIntake2_B, 0, -1, -1, -1, -1, 820, 4096, 0, 20000, adc3, 3, 0, false },          
  //{ "AI_HYD_psig_PT555_HydraulicIntake2_A", "PT555", 0, 0, 0, avgPT555, &AI_HYD_psig_PT555_HydraulicIntake2_A, 0, -1, -1, -1, -1, 820, 4096, 0, 20000, adc3, 4, 0, false },          
  //{ "AI_CLT_psig_PT113_CoolantSupply1", "PT113", 0, 0, 0, avgPT113, &AI_CLT_psig_PT113_CoolantSupply1, 0, -1, -1, -1, -1, 820, 4096, 0, 250, adc3, 5, 0, false },                  
  //{ "AI_CLT_psig_PT213_CoolantSupply2", "PT213", 0, 0, 0, avgPT213, &AI_CLT_psig_PT213_CoolantSupply2, 0, -1, -1, -1, -1, 820, 4096, 0, 250, adc3, 6, 0, false },                  
  //{ "AI_HYD_psig_PT461_HydraulicIntake1_A", "PT461", 0, 0, 0, avgPT461, &AI_HYD_psig_PT461_HydraulicIntake1_A, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, false },           
  //{ "AI_HYD_psig_PT462_HydraulicIntake1_B", "PT462", 0, 0, 0, avgPT462, &AI_HYD_psig_PT462_HydraulicIntake1_B, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, false }
};int PTsize = 4;

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
    //{ "DO_HYD_XV460_DCV1_A", "XV460", &DO_HYD_XV460_DCV1_A, 0, gpio1, P4_1 },                            
    //{ "DO_HYD_XV463_DCV1_B", "XV463", &DO_HYD_XV463_DCV1_B, 0, gpio2, P0_0 },                                             
    { "DO_HYD_XV554_DCV2_A", "XV554", &DO_HYD_XV554_DCV2_A, 0, gpio1, P4_1 }, //switched from gpio2, P0_1 to gpio1, P4_1                           
    { "DO_HYD_XV557_DCV2_B", "XV557", &DO_HYD_XV557_DCV2_B, 0, gpio2, P0_0 }, //switched from gpio2, P0_2 to gpio2, P0_0                                       
    { "DO_CLT_FCU112_CoolantFan1_Enable", "FCU112", &DO_CLT_FCU112_CoolantFan1_Enable, 0, gpio2, P0_3 },                  
    { "DO_CLT_FCU212_CoolantFan2_Enable", "FCU212", &DO_CLT_FCU212_CoolantFan2_Enable, 0, gpio2, P0_4 },                  
    //{ "DO_HYD_PMP458_HydraulicPump1_Enable", "PMP458", &DO_HYD_PMP458_HydraulicPump1_Enable, 0, gpio2, P0_5 },            //12
    { "DO_HYD_PMP552_HydraulicPump2_Enable", "PMP552", &DO_HYD_PMP552_HydraulicPump2_Enable, 0, gpio2, P0_6 },            //13
    { "DO_CLT_PMP104_PMP204_CoolantPumps_Enable", "CLTPMP", &DO_CLT_PMP104_PMP204_CoolantPumps_Enable, 0, gpio2, P0_7 },  
  };
int DIsize = 25;
int DOsize = 12;
