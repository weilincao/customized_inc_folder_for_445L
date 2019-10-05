void init(void){ volatile int delay;
    SYSCTL_RCGCTIMER_R |= 0x10;      // 0) activate timer4
    delay++; // delay here
    TIMER4_CTL_R = 0x00000000;       // 1) disable timer4A during setup
    TIMER4_CFG_R = 0x00000000;       // 2) configure for 32-bit mode
    TIMER4_TAMR_R = 0x00000002;      // 3) configure for periodic mode, default down-count settings
    TIMER4_TAILR_R = 7999;           // 4) 100us reload value
    TIMER4_TAPR_R = 0;               // 5) bus clock resolution
    TIMER4_ICR_R = 0x00000001;       // 6) clear timer5A timeout flag
    TIMER4_IMR_R = 0x00000001;       // 7) arm timeout interrupt
    NVIC_PRI17_R = (NVIC_PRI17_R&0xFF00FFFF)|0x00E00000; // 8) priority 7
// interrupts enabled in the main program after all devices initialized
// vector number 86, interrupt number 70
    NVIC_EN2_R = 0x00000040;         // 9) enable interrupt 70 in NVIC
    TIMER4_CTL_R = 0x00000001;       // 10) enable timer4A
}

void StopTimer4(void){
  NVIC_DIS2_R = 0x00000040;        // 9) enable interrupt 70 in NVIC
  TIMER4_CTL_R = 0x00000000;       // 10) enable timer4A
}
void StartTimer4(void){
  NVIC_EN2_R = 0x00000040;         // 9) enable interrupt 70 in NVIC
  TIMER4_CTL_R = 0x00000001;       // 10) enable timer4A
}

void Timer4A_Handler(void){
  TIMER4_ICR_R = 0x00000001;        // acknowledge timer4A timeout
  
}