
#include <Wire.h>
#include <SPI.h>
#include <ADS7828.h>
#include <PI4IOE5V6534Q.h>
#include <ModbusMaster_oneH2.h>
#include <ModbusRTU.h>
#include <RunningAverage.h>

#define OCI_MODBUS_ID 22
#define RE_DE1 12
#define MOVING_AVG_SIZE 20

enum state { IDLE,               //0
             HYDRAULIC_TEMP,     //1
             HYDRAULIC_FILTER,
             PRODUCTION,         //2
             PAUSE,              //3
             SHUTDOWN,           //6
             MANUAL_CONTROL,     //7
             ESTOP,              //8
             FAULT               //9
} STATE, PREV_STATE,CHANGED_STATE;

enum intensifiers{
  PAUSED,
  SIDE_A,
  SIDE_B
}DCV1, PREV_DCV1, CHANGED_DCV1, DCV2, PREV_DCV2, CHANGED_DCV2;


TwoWire i2c(20,21);
ModbusMaster mbLocal;
PI4IOE5V6534Q gpio1(0x44, i2c);
PI4IOE5V6534Q gpio2(0x46, i2c);
ADS7828 adc1(0x48);
ADS7828 adc2(0x49);
ADS7828 adc3(0x4A);
bool firstLowSide,firstHighSide = false;
unsigned long timer[10] = {0};
unsigned long flashTimer[3] = {0};
bool flashTog[3] = {false};
int flashGreen, flashAmber, flashRed = 0;
int delayTime = 1000;
bool prettyPrint = false;
bool rawPrint = false;
bool errorPrint = true;

bool prevR = false;
unsigned long holdR = 0;
enum prnt{
  OFF,
  ALL,
  PACKET,
  DIGITAL_IN,
  DIGITAL_OUT,
  ANALOG_IN,
  ANALOG_OUT
}printMode;

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

bool virtualRedButton,virtualGreenButton,virtualAmberButton = false;
double RATIO = 5.75;
bool strokeLowSide,strokeHighSide = false;

double AI_HYD_C_TT454_HydraulicTank = 0;
double AI_CLT_C_TT107_CoolantSupply1 = 0;
double AI_CLT_C_TT207_CoolantSupply2 = 0;
double AI_H2_C_TT917_SuctionTank = 0;
double AI_H2_C_TT809_Discharge1_A = 0;
double AI_H2_C_TT810_Discharge1_B = 0;
double AI_H2_C_TT715_DischargeTank1 = 0;
double AI_H2_C_TT520_Discharge2_A = 0;
double AI_H2_C_TT521_Discharge2_B = 0;
double AI_H2_C_TT522_DischargeTank2 = 0;
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

struct tt{
  String name;
  String key;
  double raw;
  double rawTemp;
  RunningAverage avg;
  double* value;
  int min;
  int max;
  int recovery;
  ADS7828 adc;
  int channel;
  bool overheat;
} TTdata[] = {
  { "AI_HYD_C_TT454_HydraulicTank", "TT454", 0, 0, avgTT454, &AI_HYD_C_TT454_HydraulicTank, 10, 55, 30, adc1, 0, false },   //0
  { "AI_CLT_C_TT107_CoolantSupply1", "TT107", 0, 0, avgTT107, &AI_CLT_C_TT107_CoolantSupply1, -1, -1, -1, adc1, 1, false },  //1
  { "AI_CLT_C_TT207_CoolantSupply2", "TT207", 0, 0, avgTT207, &AI_CLT_C_TT207_CoolantSupply2, -1, -1, -1, adc1, 2, false },   //2
  { "AI_H2_C_TT917_SuctionTank", "TT917", 0, 0, avgTT917, &AI_H2_C_TT917_SuctionTank, -1, 50, 30, adc1, 3, false },      //3
  { "AI_H2_C_TT809_Discharge1_A", "TT809", 0, 0, avgTT809, &AI_H2_C_TT809_Discharge1_A, -1, 135, 110, adc1, 4, false },   //4
  { "AI_H2_C_TT810_Discharge1_B", "TT810", 0, 0, avgTT810, &AI_H2_C_TT810_Discharge1_B, -1, 135, 110, adc1, 5, false },   //5
  { "AI_H2_C_TT715_DischargeTank1", "TT715", 0, 0, avgTT715, &AI_H2_C_TT715_DischargeTank1, -1, -1, -1, adc1, 6, false },   //6
  { "AI_H2_C_TT520_Discharge2_A", "TT520", 0, 0, avgTT520, &AI_H2_C_TT520_Discharge2_A, -1, 135, 110, adc1, 7, false },      //7
  { "AI_H2_C_TT521_Discharge2_B", "TT521", 0, 0, avgTT521, &AI_H2_C_TT521_Discharge2_B, -1, -1, -1, adc2, 0, false },      //8
  { "AI_H2_C_TT522_DischargeTank2", "TT522", 0, 0, avgTT522, &AI_H2_C_TT522_DischargeTank2, -1, 135, 110, adc2, 1, false },    //9
  { "AI_H2_C_TT701_DischargePreTank", "TT701", 0, 0, avgTT701, &AI_H2_C_TT701_DischargePreTank, -1, -1, -1, -1, -1, false }  //10
};


struct pt{
  String name;
  String key;
  double raw;
  double mapped;
  double offset;
  RunningAverage avg;
  double* value;
  int min;
  int max;
  int recovery;
  double mapA;
  double mapB;
  double mapC;
  double mapD;
  ADS7828 adc;
  int channel;
  bool overPressure;
} PTdata[] = {
  { "AI_H2_psig_PT911_SuctionTank", "PT911", 0, 0, 0, avgPT911, &AI_H2_psig_PT911_SuctionTank, -1, -1, -1, 819, 3900, 0, 500, adc2, 3, false },         //0
  { "AI_H2_psig_PT716_Stage1_Discharge", "PT716", 0, 0, 50, avgPT716, &AI_H2_psig_PT716_Stage1_Discharge, -1, -1, -1, 820, 4096, 0, 20000, adc2, 4, false },    //1
  { "AI_H2_psig_PT712_Stage1_DischargeTank", "PT712", 0, 0, -65, avgPT712, &AI_H2_psig_PT712_Stage1_DischargeTank, -1, -1, -1, 810, 3090, 0, 2000, adc2, 5, false },       //2
  { "AI_H2_psig_PT519_Stage2_Discharge", "PT519", 0, 0, 0, avgPT519, &AI_H2_psig_PT519_Stage2_Discharge, -1, -1, -1, -1, -1, -1, -1, adc2, 6, false },                     //3
  { "AI_H2_psig_PT407_Stage3_Discharge", "PT407", 0, 0, 0, avgPT407, &AI_H2_psig_PT407_Stage3_Discharge, -1, -1, -1, 819, 3900, 0, 20000, adc2, 7, false },                     //4
  { "AI_H2_psig_PT410_Output", "PT410", 0, 0, 200, avgPT410, &AI_H2_psig_PT410_Output, -1, -1, -1, 820, 4096, 0, 20000, adc3, 0, false },                     //5
  { "AI_HYD_psig_PT467_HydraulicInlet1", "PT467", 0, 0, -200, avgPT467, &AI_HYD_psig_PT467_HydraulicInlet1, -1, -1, -1, 820, 3900, 0, 5000, adc3, 1, false },    //6
  { "AI_HYD_psig_PT561_HydraulicInlet2", "PT561", 0, 0, -200, avgPT561, &AI_HYD_psig_PT561_HydraulicInlet2, -1, -1, -1, 820, 3900, 0, 5000, adc3, 2, false },     //7
  { "AI_HYD_psig_PT556_HydraulicIntake2_B", "PT556", 0, 0, 0, avgPT556, &AI_HYD_psig_PT556_HydraulicIntake2_B, -1, -1, -1, -1, -1, -1, -1, adc3, 3, false },  //8
  { "AI_HYD_psig_PT555_HydraulicIntake2_A", "PT555", 0, 0, 0, avgPT555, &AI_HYD_psig_PT555_HydraulicIntake2_A, -1, -1, -1, -1, -1, -1, -1, adc3, 4, false },  //9
  { "AI_CLT_psig_PT113_CoolantSupply1", "PT113", 0, 0, 0, avgPT113, &AI_CLT_psig_PT113_CoolantSupply1, -1, -1, -1, -1, -1, -1, -1, adc3, 5, false },      //10
  { "AI_CLT_psig_PT213_CoolantSupply2", "PT213", 0, 0, 0, avgPT213, &AI_CLT_psig_PT213_CoolantSupply2, -1, -1, -1, -1, -1, -1, -1, adc3, 6, false },      //11
  { "AI_HYD_psig_PT461_HydraulicIntake1_A", "PT461", 0, 0, 0, avgPT461, &AI_HYD_psig_PT461_HydraulicIntake1_A, -1, -1, -1, -1, -1, -1, -1, -1, -1, false },  //12
  { "AI_HYD_psig_PT462_HydraulicIntake1_B", "PT462", 0, 0, 0, avgPT462, &AI_HYD_psig_PT462_HydraulicIntake1_B, -1, -1, -1, -1, -1, -1, -1, -1, -1, false }  //13
};

struct digital{
  String name;
  String key;
  uint8_t* value;
  PI4IOE5V6534Q gpio;
  int addr;
} DIdata[] = {
  {"DI_Comm_GSR_Global","???",&DI_Comm_GSR_Global,gpio1,P0_0},  //0
  {"DI_Comm_LSR_Local","???1",&DI_Comm_LSR_Local,gpio1,P0_1},  //1
  //{"","",&,-1,-1},                            //2 //gpio1.digitalRead(P0_2,		REMOTE RESETRLY OP
  {"DI_Encl_PilotGreen_Feedback","GPLfb",&DI_Encl_PilotGreen_Feedback,gpio1,P0_3}, //3
  {"DI_Encl_PilotAmber_Feedback","APLfb",&DI_Encl_PilotAmber_Feedback,gpio1,P0_4}, //4  //D_HYDR_FLUID_LEVELS -martin
  {"DI_Encl_PilotRed_Feedback","RPLfb",&DI_Encl_PilotRed_Feedback,gpio1,P0_5},  //5
  {"DI_H2_XV907_SuctionPreTank_Feedback","XV907fb",&DI_H2_XV907_SuctionPreTank_Feedback,gpio1,P0_6}, //6
  {"DI_HYD_XV460_DCV1_A_feedback","XV460fb",&DI_HYD_XV460_DCV1_A_feedback,gpio1,P0_7}, //7
  {"DI_HYD_XV463_DCV1_B_feedback","XV463fb",&DI_HYD_XV463_DCV1_B_feedback,gpio1,P1_0}, //8
  {"DI_HYD_XV554_DCV2_A_feedback","XV554fb",&DI_HYD_XV554_DCV2_A_feedback,gpio1,P1_1}, //9
  {"DI_HYD_XV557_DCV2_B_feedback","XV557fb",&DI_HYD_XV557_DCV2_B_feedback,gpio1,P1_2}, //10
  {"DI_CLT_FCU112_CoolantFan1_Feedback","FCU112fb",&DI_CLT_FCU112_CoolantFan1_Feedback,gpio1,P1_3}, //11
  {"DI_CLT_FCU212_CoolantFan2_Feedback","FCU212fb",&DI_CLT_FCU212_CoolantFan2_Feedback,gpio1,P1_4}, //12
  {"DI_HYD_PMP458_HydraulicPump1_Feedback","PMP458fb",&DI_HYD_PMP458_HydraulicPump1_Feedback,gpio1,P1_5}, //13
  {"DI_HYD_PMP552_HydraulicPump2_Feedback","PMP552fb",&DI_HYD_PMP552_HydraulicPump2_Feedback,gpio1,P1_6}, //14
  {"DI_CLT_PMP104_PMP204_CoolantPumps_Feedback","CLTPMPfb",&DI_CLT_PMP104_PMP204_CoolantPumps_Feedback,gpio1,P1_7}, //15
  //{"","",&,-1,-1},                                                       //16 //gpio1.digitalRead(P2_0,			COOLANT PUMPS LATCH RLY OP
  {"DI_Encl_ESTOP","ESTOP",&DI_Encl_ESTOP,gpio1,P2_1},  //17
  {"DI_Encl_ButtonGreen","GBN",&DI_Encl_ButtonGreen,gpio1,P2_2}, //18
  {"DI_Encl_ButtonAmber","ABN",&DI_Encl_ButtonAmber,gpio1,P2_3}, //19
  {"DI_Encl_ButtonRed","RBN",&DI_Encl_ButtonRed,gpio1,P2_4}, //20
  {"DI_Comm_LSR_Local","???2",&DI_Comm_LSR_Local,gpio1,P2_5}, //21
  {"DI_HYD_LS000_HydraulicLevelSwitch","LVLSW",&DI_HYD_LS000_HydraulicLevelSwitch,gpio1,P2_6}, //22
  {"DI_CLT_FS000_CoolantFlowSwitch","FLSW",&DI_CLT_FS000_CoolantFlowSwitch,gpio1,P2_7}, //23
  {"DI_HYD_IS000_HydraulicFilterSwitch","FISW",&DI_HYD_IS000_HydraulicFilterSwitch,gpio1,P3_0}, //24 //D_HYDR_DIF_SWITCH -martin
  {"DI_H2_AN000_ContaminationOnSide1","CONT1",&DI_H2_AN000_ContaminationOnSide1,gpio1,P3_1}, //25
  {"DI_H2_AN000_ContaminationOnSide2","CONT2",&DI_H2_AN000_ContaminationOnSide2,gpio1,P3_2}, //26
},DOdata[] = {
  {"DO_Comm_LSR_Local","???3",&DO_Comm_LSR_Local,gpio1,P3_3}, //0
 // {"????","???",&????,gpio1,P3_4}, //1  //gpio1.digitalWrite(P3_4,    REMOTE RESETRLY OP
  {"DO_Encl_PilotGreen","GPL",&DO_Encl_PilotGreen,gpio1,P3_5}, //2
  {"DO_Encl_PilotAmber","APL",&DO_Encl_PilotAmber,gpio1,P3_6}, //3
  {"DO_Encl_PilotRed","RPL",&DO_Encl_PilotRed,gpio1,P3_7}, //4
  {"DO_H2_XV907_SuctionPreTank","XV907",&DO_H2_XV907_SuctionPreTank,gpio1,P4_0}, //5
  {"DO_HYD_XV460_DCV1_A","XV460",&DO_HYD_XV460_DCV1_A,gpio1,P4_1}, //6
  {"DO_HYD_XV463_DCV1_B","XV463",&DO_HYD_XV463_DCV1_B,gpio2,P0_0}, //7
  {"DO_HYD_XV554_DCV2_A","XV554",&DO_HYD_XV554_DCV2_A,gpio2,P0_1}, //8
  {"DO_HYD_XV557_DCV2_B","XV557",&DO_HYD_XV557_DCV2_B,gpio2,P0_2}, //9
  {"DO_CLT_FCU112_CoolantFan1_Enable","FCU112",&DO_CLT_FCU112_CoolantFan1_Enable,gpio2,P0_3}, //10
  {"DO_CLT_FCU212_CoolantFan2_Enable","FCU212",&DO_CLT_FCU212_CoolantFan2_Enable,gpio2,P0_4}, //11
  {"DO_HYD_PMP458_HydraulicPump1_Enable","PMP458",&DO_HYD_PMP458_HydraulicPump1_Enable,gpio2,P0_5}, //12
  {"DO_HYD_PMP552_HydraulicPump2_Enable","PMP552",&DO_HYD_PMP552_HydraulicPump2_Enable,gpio2,P0_6}, //13
  {"DO_CLT_PMP104_PMP204_CoolantPumps_Enable","CLTPMP",&DO_CLT_PMP104_PMP204_CoolantPumps_Enable,gpio2,P0_7}, //14
};
double prevPT[sizeof(TTdata)] = {0};
double prevTT[sizeof(PTdata)] = {0};
double prevDI[sizeof(DIdata)] = {0};
double prevDO[sizeof(DOdata)] = {0};

