
double getStage1SwitchingPsi_90_63(){
  if(70 <= AI_H2_psig_PT911_Stage1_SuctionTank < 80){ return 1.1524*AI_H2_psig_PT716_Stage1_Discharge + 196.83;}
  else if(80 <= AI_H2_psig_PT911_Stage1_SuctionTank < 90){ return 1.1641*AI_H2_psig_PT716_Stage1_Discharge + 181.37;}
  else if(90 <= AI_H2_psig_PT911_Stage1_SuctionTank < 100){ return 1.1735*AI_H2_psig_PT716_Stage1_Discharge + 165.88;}
  else if(100 <= AI_H2_psig_PT911_Stage1_SuctionTank < 110){ return 1.1625*AI_H2_psig_PT716_Stage1_Discharge + 152.75;}
  else if(110 <= AI_H2_psig_PT911_Stage1_SuctionTank < 120){ return 1.1705*AI_H2_psig_PT716_Stage1_Discharge + 137.25;}
  else if(120 <= AI_H2_psig_PT911_Stage1_SuctionTank < 130){ return 1.1773*AI_H2_psig_PT716_Stage1_Discharge + 121.73;}
  else if(130 <= AI_H2_psig_PT911_Stage1_SuctionTank < 140){ return 1.1831*AI_H2_psig_PT716_Stage1_Discharge + 106.2;}
  else if(140 <= AI_H2_psig_PT911_Stage1_SuctionTank < 150){ return 1.1882*AI_H2_psig_PT716_Stage1_Discharge + 90.658;}
  else if(150 <= AI_H2_psig_PT911_Stage1_SuctionTank < 160){ return 1.1796*AI_H2_psig_PT716_Stage1_Discharge + 77.556;}
  else if(160 <= AI_H2_psig_PT911_Stage1_SuctionTank < 170){ return 1.1843*AI_H2_psig_PT716_Stage1_Discharge + 62.012;}
  else if(170 <= AI_H2_psig_PT911_Stage1_SuctionTank < 180){ return 1.1885*AI_H2_psig_PT716_Stage1_Discharge + 46.462;}
  else if(180 <= AI_H2_psig_PT911_Stage1_SuctionTank < 190){ return 1.1922*AI_H2_psig_PT716_Stage1_Discharge + 30.906;}
  else if(190 <= AI_H2_psig_PT911_Stage1_SuctionTank < 200){ return 1.1955*AI_H2_psig_PT716_Stage1_Discharge + 15.347;}
  else if(200 <= AI_H2_psig_PT911_Stage1_SuctionTank < 210){ return 1.1986*AI_H2_psig_PT716_Stage1_Discharge - 0.217;}
  else if(210 <= AI_H2_psig_PT911_Stage1_SuctionTank < 220){ return 1.1919*AI_H2_psig_PT716_Stage1_Discharge - 13.297;}
  else if(220 <= AI_H2_psig_PT911_Stage1_SuctionTank < 230){ return 1.1948*AI_H2_psig_PT716_Stage1_Discharge - 28.862;}
  else if(230 <= AI_H2_psig_PT911_Stage1_SuctionTank < 240){ return 1.1936*AI_H2_psig_PT716_Stage1_Discharge - 41.026;}
  else if(240 <= AI_H2_psig_PT911_Stage1_SuctionTank < 250){ return 1.1948*AI_H2_psig_PT716_Stage1_Discharge - 52.759;}
  else if(250 <= AI_H2_psig_PT911_Stage1_SuctionTank < 260){ return 1.1961*AI_H2_psig_PT716_Stage1_Discharge - 64.522;}
  else if(260 <= AI_H2_psig_PT911_Stage1_SuctionTank < 270){ return 1.1974*AI_H2_psig_PT716_Stage1_Discharge - 76.316;}
  else if(270 <= AI_H2_psig_PT911_Stage1_SuctionTank < 280){ return 1.1987*AI_H2_psig_PT716_Stage1_Discharge - 88.142;}
  else if(280 <= AI_H2_psig_PT911_Stage1_SuctionTank < 290){ return 1.1906*AI_H2_psig_PT716_Stage1_Discharge - 96.875;}
  else if(290 <= AI_H2_psig_PT911_Stage1_SuctionTank < 300){ return 1.1919*AI_H2_psig_PT716_Stage1_Discharge - 108.65;}
  else if(300 <= AI_H2_psig_PT911_Stage1_SuctionTank < 310){ return 1.1932*AI_H2_psig_PT716_Stage1_Discharge - 120.45;}
  else if(310 <= AI_H2_psig_PT911_Stage1_SuctionTank < 320){ return 1.1945*AI_H2_psig_PT716_Stage1_Discharge - 132.29;}
  else if(320 <= AI_H2_psig_PT911_Stage1_SuctionTank < 330){ return 1.1958*AI_H2_psig_PT716_Stage1_Discharge - 144.17;}
  else if(330 <= AI_H2_psig_PT911_Stage1_SuctionTank < 340){ return 1.1972*AI_H2_psig_PT716_Stage1_Discharge - 156.07;}
  else if(340 <= AI_H2_psig_PT911_Stage1_SuctionTank < 350){ return 1.1986*AI_H2_psig_PT716_Stage1_Discharge - 168.02;}
  else if(350 <= AI_H2_psig_PT911_Stage1_SuctionTank < 360){ return 1.1900*AI_H2_psig_PT716_Stage1_Discharge - 176;}
  else if(360 <= AI_H2_psig_PT911_Stage1_SuctionTank < 370){ return 1.1913*AI_H2_psig_PT716_Stage1_Discharge - 187.88;}
  else{return -1;}
}


double getStage2SwitchingPsi_90_63(){
  if(460 <= AI_H2_psig_PT712_Stage1_DischargeTank < 480){ return 0.2197*AI_H2_psig_PT519_Stage2_Discharge + 261.54;}
  else if(480 <= AI_H2_psig_PT712_Stage1_DischargeTank < 540){ return 0.2076*AI_H2_psig_PT519_Stage2_Discharge + 275.86;}
  else if(540 <= AI_H2_psig_PT712_Stage1_DischargeTank < 600){ return 0.2251*AI_H2_psig_PT519_Stage2_Discharge + 257.14;}
  else if(600 <= AI_H2_psig_PT712_Stage1_DischargeTank < 660){ return 0.2437*AI_H2_psig_PT519_Stage2_Discharge + 244.93;}
  else if(660 <= AI_H2_psig_PT712_Stage1_DischargeTank < 720){ return 0.2161*AI_H2_psig_PT519_Stage2_Discharge + 267.14;}
  else if(720 <= AI_H2_psig_PT712_Stage1_DischargeTank < 780){ return 0.2174*AI_H2_psig_PT519_Stage2_Discharge + 266.37;}
  else if(780 <= AI_H2_psig_PT712_Stage1_DischargeTank < 840){ return 0.2119*AI_H2_psig_PT519_Stage2_Discharge + 271.03;}
  else if(840 <= AI_H2_psig_PT712_Stage1_DischargeTank < 900){ return 0.2036*AI_H2_psig_PT519_Stage2_Discharge + 279.12;}
  else if(900 <= AI_H2_psig_PT712_Stage1_DischargeTank < 960){ return 0.2187*AI_H2_psig_PT519_Stage2_Discharge + 259.41;}
  else if(960 <= AI_H2_psig_PT712_Stage1_DischargeTank < 1020){ return 0.2108*AI_H2_psig_PT519_Stage2_Discharge + 272.18;}
  else if(1020 <= AI_H2_psig_PT712_Stage1_DischargeTank < 1080){ return 0.1963*AI_H2_psig_PT519_Stage2_Discharge + 290.03;}
  else if(1080 <= AI_H2_psig_PT712_Stage1_DischargeTank < 1140){ return 0.2037*AI_H2_psig_PT519_Stage2_Discharge + 285.88;}
  else if(1140 <= AI_H2_psig_PT712_Stage1_DischargeTank < 1200){ return 0.2098*AI_H2_psig_PT519_Stage2_Discharge + 275.34;}
  else if(1200 <= AI_H2_psig_PT712_Stage1_DischargeTank < 1260){ return 0.206*AI_H2_psig_PT519_Stage2_Discharge + 279.54;}
  else if(1260 <= AI_H2_psig_PT712_Stage1_DischargeTank < 1320){ return 0.2093*AI_H2_psig_PT519_Stage2_Discharge + 272.96;}
  else if(1320 <= AI_H2_psig_PT712_Stage1_DischargeTank < 1380){ return 0.2007*AI_H2_psig_PT519_Stage2_Discharge + 290.09;}
  else if(1380 <= AI_H2_psig_PT712_Stage1_DischargeTank < 1400){ return 0.2041*AI_H2_psig_PT519_Stage2_Discharge + 289.92;}
  else{return -1;}
}

double getStage2SwitchingPsi_150_90(){
  if(460 <= AI_H2_psig_PT712_Stage1_DischargeTank < 480){PTdata[4].max = 5560;}
  else if(480 <= AI_H2_psig_PT712_Stage1_DischargeTank < 540){PTdata[4].max = 5800;}
  else if(540 <= AI_H2_psig_PT712_Stage1_DischargeTank < 600){PTdata[4].max = 6600;}
  else if(600 <= AI_H2_psig_PT712_Stage1_DischargeTank <= 660){PTdata[4].max = 7280;}
  else if(660 < AI_H2_psig_PT712_Stage1_DischargeTank){PTdata[4].max = 8000;}
  PTdata[4].maxRecovery = PTdata[4].max - 500;

  return 0.7498*AI_H2_psig_PT519_Stage2_Discharge + 263.83;
}


double getStage3SwitchingPsi_90_63(){
  if(450 <= AI_H2_psig_PT519_Stage2_Discharge < 550){ return 0.2*AI_H2_psig_PT407_Stage3_Discharge + 180;}
  else if(550 <= AI_H2_psig_PT519_Stage2_Discharge < 650){ return 0.2*AI_H2_psig_PT407_Stage3_Discharge + 160;}
  else if(650 <= AI_H2_psig_PT519_Stage2_Discharge < 750){ return 0.208*AI_H2_psig_PT407_Stage3_Discharge + 127;}
  else if(750 <= AI_H2_psig_PT519_Stage2_Discharge < 850){ return 0.198*AI_H2_psig_PT407_Stage3_Discharge + 133;}
  else if(850 <= AI_H2_psig_PT519_Stage2_Discharge < 950){ return 292.9*log(0.0003*AI_H2_psig_PT407_Stage3_Discharge);}
  else if(950 <= AI_H2_psig_PT519_Stage2_Discharge < 1050){ return 309.83*log(0.0003*AI_H2_psig_PT407_Stage3_Discharge);}
  else if(1050 <= AI_H2_psig_PT519_Stage2_Discharge < 1150){ return 325.58*log(0.0002*AI_H2_psig_PT407_Stage3_Discharge);}
  else if(1150 <= AI_H2_psig_PT519_Stage2_Discharge < 1250){ if(AI_H2_psig_PT407_Stage3_Discharge < 2500){ return 535; }else{ return 0.2046*AI_H2_psig_PT407_Stage3_Discharge + 29.524; }}
  else if(1250 <= AI_H2_psig_PT519_Stage2_Discharge < 1350){ if(AI_H2_psig_PT407_Stage3_Discharge < 2500){ return 545; }else{ return 0.1971*AI_H2_psig_PT407_Stage3_Discharge + 45.714; }}
  else if(1350 <= AI_H2_psig_PT519_Stage2_Discharge < 1450){ if(AI_H2_psig_PT407_Stage3_Discharge < 2500){ return 570; }else{ return 310.98*log(0.0002*AI_H2_psig_PT407_Stage3_Discharge); }}
  else if(1450 <= AI_H2_psig_PT519_Stage2_Discharge < 1550){ if(AI_H2_psig_PT407_Stage3_Discharge < 3000){ return 590; }else{ return 0.2043*AI_H2_psig_PT407_Stage3_Discharge - 25; }}
  else if(1550 <= AI_H2_psig_PT519_Stage2_Discharge < 1650){ if(AI_H2_psig_PT407_Stage3_Discharge < 3000){ return 615; }else{ return 315.32*log(0.0002*AI_H2_psig_PT407_Stage3_Discharge); }}
  else if(1650 <= AI_H2_psig_PT519_Stage2_Discharge < 1750){ if(AI_H2_psig_PT407_Stage3_Discharge < 3500){ return 640; }else{ return 0.2043*AI_H2_psig_PT407_Stage3_Discharge - 67.143; }}
  else if(1750 <= AI_H2_psig_PT519_Stage2_Discharge < 1850){ if(AI_H2_psig_PT407_Stage3_Discharge < 3500){ return 660; }else{ return 0.1988*AI_H2_psig_PT407_Stage3_Discharge - 55; }}
  else if(1850 <= AI_H2_psig_PT519_Stage2_Discharge < 1950){ if(AI_H2_psig_PT407_Stage3_Discharge < 4000){ return 690; }else{ return 0.2057*AI_H2_psig_PT407_Stage3_Discharge - 112.86; }}
  else if(1950 <= AI_H2_psig_PT519_Stage2_Discharge < 2050){ if(AI_H2_psig_PT407_Stage3_Discharge < 4000){ return 700; }else{ return 0.2033*AI_H2_psig_PT407_Stage3_Discharge - 115.56; }}
  else if(2050 <= AI_H2_psig_PT519_Stage2_Discharge < 2150){ if(AI_H2_psig_PT407_Stage3_Discharge < 4000){ return 720; }else{ return 0.1993*AI_H2_psig_PT407_Stage3_Discharge - 104.89; }}
  else if(2150 <= AI_H2_psig_PT519_Stage2_Discharge < 2250){ if(AI_H2_psig_PT407_Stage3_Discharge < 4500){ return 740; }else{ return 0.2052*AI_H2_psig_PT407_Stage3_Discharge - 165.24; }}
  else if(2250 <= AI_H2_psig_PT519_Stage2_Discharge < 2350){ if(AI_H2_psig_PT407_Stage3_Discharge < 4500){ return 760; }else{ return 0.2002*AI_H2_psig_PT407_Stage3_Discharge - 150.24; }}
  else if(2350 <= AI_H2_psig_PT519_Stage2_Discharge < 2450){ if(AI_H2_psig_PT407_Stage3_Discharge < 5000){ return 785; }else{ return 0.2043*AI_H2_psig_PT407_Stage3_Discharge - 202.14; }}
  else if(2450 <= AI_H2_psig_PT519_Stage2_Discharge < 2550){ if(AI_H2_psig_PT407_Stage3_Discharge < 5000){ return 805; }else{ return 0.2036*AI_H2_psig_PT407_Stage3_Discharge - 210.36; }}
  else if(2550 <= AI_H2_psig_PT519_Stage2_Discharge < 2650){ if(AI_H2_psig_PT407_Stage3_Discharge < 5000){ return 825; }else{ return 0.1957*AI_H2_psig_PT407_Stage3_Discharge - 172.14; }}
  else if(2650 <= AI_H2_psig_PT519_Stage2_Discharge < 2750){ if(AI_H2_psig_PT407_Stage3_Discharge < 5500){ return 845; }else{ return 0.2046*AI_H2_psig_PT407_Stage3_Discharge - 254.19; }}
  else if(2750 <= AI_H2_psig_PT519_Stage2_Discharge < 2850){ if(AI_H2_psig_PT407_Stage3_Discharge < 5500){ return 865; }else{ return 0.2017*AI_H2_psig_PT407_Stage3_Discharge - 246.57; }}
  else if(2850 <= AI_H2_psig_PT519_Stage2_Discharge < 2950){ if(AI_H2_psig_PT407_Stage3_Discharge < 5500){ return 885; }else{ return 341.32*log(0.0002*AI_H2_psig_PT407_Stage3_Discharge); }}
  else if(2950 <= AI_H2_psig_PT519_Stage2_Discharge < 3050){ if(AI_H2_psig_PT407_Stage3_Discharge < 6000){ return 910; }else{ return 0.21*AI_H2_psig_PT407_Stage3_Discharge - 346; }}
  else if(3050 <= AI_H2_psig_PT519_Stage2_Discharge < 3150){ if(AI_H2_psig_PT407_Stage3_Discharge < 6000){ return 930; }else{ return 0.192*AI_H2_psig_PT407_Stage3_Discharge - 230; }}
  else if(3150 <= AI_H2_psig_PT519_Stage2_Discharge < 3250){ if(AI_H2_psig_PT407_Stage3_Discharge < 6000){ return 950; }else{ return 352.18*log(0.0002*AI_H2_psig_PT407_Stage3_Discharge); }}
  else if(3250 <= AI_H2_psig_PT519_Stage2_Discharge < 3350){ if(AI_H2_psig_PT407_Stage3_Discharge < 6500){ return 978; }else{ return 0.197*AI_H2_psig_PT407_Stage3_Discharge - 302; }}
  else if(3350 <= AI_H2_psig_PT519_Stage2_Discharge < 3450){ if(AI_H2_psig_PT407_Stage3_Discharge < 6000){ return 995; }else{ return 352.5*log(0.0002*AI_H2_psig_PT407_Stage3_Discharge); }}
  else{return -1;}
}

double getStage3SwitchingPsi_150_90(){
  if(450 <= AI_H2_psig_PT519_Stage2_Discharge < 550){ return 0.42*AI_H2_psig_PT407_Stage3_Discharge+70; }
  else if(550 <= AI_H2_psig_PT519_Stage2_Discharge < 650){ return 0.37*AI_H2_psig_PT407_Stage3_Discharge+146.67; }
  else if(650 <= AI_H2_psig_PT519_Stage2_Discharge < 750){ if(AI_H2_psig_PT407_Stage3_Discharge < 2000){return 0.08*AI_H2_psig_PT407_Stage3_Discharge+670;}else{return 0.42*AI_H2_psig_PT407_Stage3_Discharge-10; }}
  else if(750 <= AI_H2_psig_PT519_Stage2_Discharge < 850){ return 0.366*AI_H2_psig_PT407_Stage3_Discharge+116; }
  else if(850 <= AI_H2_psig_PT519_Stage2_Discharge < 950){ return 0.42*AI_H2_psig_PT407_Stage3_Discharge-100; }
  else if(950 <= AI_H2_psig_PT519_Stage2_Discharge < 1050){ return 0.348*AI_H2_psig_PT407_Stage3_Discharge+124; }
  else if(1050 <= AI_H2_psig_PT519_Stage2_Discharge < 1150){ return 0.402*AI_H2_psig_PT407_Stage3_Discharge-110; }
  else if(1150 <= AI_H2_psig_PT519_Stage2_Discharge < 1250){ return 0.358*AI_H2_psig_PT407_Stage3_Discharge+50; }
  else if(1250 <= AI_H2_psig_PT519_Stage2_Discharge < 1350){ return 0.384*AI_H2_psig_PT407_Stage3_Discharge-102; }
  else if(1350 <= AI_H2_psig_PT519_Stage2_Discharge < 1450){ return 0.42*AI_H2_psig_PT407_Stage3_Discharge-310; }
  else if(1450 <= AI_H2_psig_PT519_Stage2_Discharge < 1550){ return 0.388*AI_H2_psig_PT407_Stage3_Discharge-174; }
  else if(1550 <= AI_H2_psig_PT519_Stage2_Discharge < 1650){ return 0.42*AI_H2_psig_PT407_Stage3_Discharge-390; }
  else if(1650 <= AI_H2_psig_PT519_Stage2_Discharge < 1750){ return 0.372*AI_H2_psig_PT407_Stage3_Discharge-146; }
  else if(1750 <= AI_H2_psig_PT519_Stage2_Discharge < 1850){ return 0.406*AI_H2_psig_PT407_Stage3_Discharge-386; }
  else if(1850 <= AI_H2_psig_PT519_Stage2_Discharge < 1950){ return 0.3745*AI_H2_psig_PT407_Stage3_Discharge-214.72; }
  else if(1950 <= AI_H2_psig_PT519_Stage2_Discharge < 2050){ return 0.4029*AI_H2_psig_PT407_Stage3_Discharge-434.29; }
  else if(2050 <= AI_H2_psig_PT519_Stage2_Discharge < 2150){ if(AI_H2_psig_PT407_Stage3_Discharge < 6000){return 0.06*AI_H2_psig_PT407_Stage3_Discharge+1560;}else{return 0.42*AI_H2_psig_PT407_Stage3_Discharge-600; }}
  else if(2150 <= AI_H2_psig_PT519_Stage2_Discharge < 2250){ return 0.396*AI_H2_psig_PT407_Stage3_Discharge-450; }
  else if(2250 <= AI_H2_psig_PT519_Stage2_Discharge < 2350){ if(AI_H2_psig_PT407_Stage3_Discharge < 6500){return 0.1*AI_H2_psig_PT407_Stage3_Discharge+1420;}else{return 0.414*AI_H2_psig_PT407_Stage3_Discharge-619; }}
  else if(2350 <= AI_H2_psig_PT519_Stage2_Discharge < 2450){ return 0.384*AI_H2_psig_PT407_Stage3_Discharge-424; }
  else if(2450 <= AI_H2_psig_PT519_Stage2_Discharge < 2550){ if(AI_H2_psig_PT407_Stage3_Discharge < 7000){return 0.1*AI_H2_psig_PT407_Stage3_Discharge+1510;}else{return 0.41*AI_H2_psig_PT407_Stage3_Discharge-658.33; }}
  else if(2550 <= AI_H2_psig_PT519_Stage2_Discharge < 2650){ return 0.36*AI_H2_psig_PT407_Stage3_Discharge-300; }
  else if(2650 <= AI_H2_psig_PT519_Stage2_Discharge < 2750){ if(AI_H2_psig_PT407_Stage3_Discharge < 7500){return 0.08*AI_H2_psig_PT407_Stage3_Discharge+1750;}else{return 0.4*AI_H2_psig_PT407_Stage3_Discharge-650; }}
  else if(2750 <= AI_H2_psig_PT519_Stage2_Discharge < 2850){ return 0.26*AI_H2_psig_PT407_Stage3_Discharge+440; }
  else if(2850 <= AI_H2_psig_PT519_Stage2_Discharge < 2950){ return 0.04*AI_H2_psig_PT407_Stage3_Discharge+2160; }
  else if(2950 <= AI_H2_psig_PT519_Stage2_Discharge < 3050){ return 2450; }
  else if(2950 <= AI_H2_psig_PT519_Stage2_Discharge < 3050){ return 2450;}
  else if(3050 <= AI_H2_psig_PT519_Stage2_Discharge < 3150){ return 2420;}
  else if(3150 <= AI_H2_psig_PT519_Stage2_Discharge < 3250){ return 2390;}
  else if(3250 <= AI_H2_psig_PT519_Stage2_Discharge < 3350){ return 2360;}
  else if(3350 <= AI_H2_psig_PT519_Stage2_Discharge < 3450){ return 2330;}
  else if(3450 <= AI_H2_psig_PT519_Stage2_Discharge < 3550){ return 2300;}
  else if(3550 <= AI_H2_psig_PT519_Stage2_Discharge < 3650){ return 2270;}
  else if(3650 <= AI_H2_psig_PT519_Stage2_Discharge < 3750){ return 2240;}
  else if(3750 <= AI_H2_psig_PT519_Stage2_Discharge < 3850){ return 2210;}
  else if(3850 <= AI_H2_psig_PT519_Stage2_Discharge < 3950){ return 2180;}
  else if(3950 <= AI_H2_psig_PT519_Stage2_Discharge < 4050){ return 2150;}
  else if(4050 <= AI_H2_psig_PT519_Stage2_Discharge < 4150){ return 2120;}
  else if(4150 <= AI_H2_psig_PT519_Stage2_Discharge < 4250){ return 2090;}
  else if(4250 <= AI_H2_psig_PT519_Stage2_Discharge < 4350){ return 2060;}
  else if(4350 <= AI_H2_psig_PT519_Stage2_Discharge < 4450){ return 2030;}
  else if(4450 <= AI_H2_psig_PT519_Stage2_Discharge < 4550){ return 2000;}
  else{return -1;}
}



