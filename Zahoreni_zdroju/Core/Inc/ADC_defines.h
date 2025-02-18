#ifndef __ADC_DEFINES_H__
#define __ADC_DEFINES_H__

/* Enum pro řízení proměření všech kanálů ADC */
typedef enum{
	ADC_WAITING = 0U,
	U15V, U15V_CURRENT,		//kanál 7, 10
	U12V, U12V_CURRENT,		//kanál 14, 12
	U24VO2, U24VO2_CURRENT,	//kanál 5, 11
	U24V, U24V_CURRENT,		//kanál 9, 2
	U5VK, U5VK_CURRENT,		//kanál 15, 0
	U5V, U5V_CURRENT,		//kanál 8, 1
	U_BAT,					//kanál 6
	U48V_CURRENT,			//kanál 3
	INTERNAL_REF,			//kanál 17
	PAD9, PAD15,			//kanál 4, 13
}ADC_State_Type;

/* Přiřazení kanálů ADC k měřené hodnotě */
#define U15V_CHANNEL			ADC_CHSELR_CHSEL7
#define U15V_CURRENT_CHANNEL	ADC_CHSELR_CHSEL10
#define U12V_CHANNEL			ADC_CHSELR_CHSEL14
#define U12V_CURRENT_CHANNEL	ADC_CHSELR_CHSEL12
#define U24VO2_CHANNEL			ADC_CHSELR_CHSEL5
#define U24VO2_CURRENT_CHANNEL	ADC_CHSELR_CHSEL11
#define U24V_CHANNEL			ADC_CHSELR_CHSEL9
#define U24V_CURRENT_CHANNEL	ADC_CHSELR_CHSEL2
#define U5VK_CHANNEL			ADC_CHSELR_CHSEL15
#define U5VK_CURRENT_CHANNEL	ADC_CHSELR_CHSEL0
#define U5V_CHANNEL				ADC_CHSELR_CHSEL8
#define U5V_CURRENT_CHANNEL		ADC_CHSELR_CHSEL1
#define U_BAT_CHANNEL			ADC_CHSELR_CHSEL6
#define PAD9_CHANNEL			ADC_CHSELR_CHSEL4
#define PAD15_CHANNEL			ADC_CHSELR_CHSEL13
#define U48V_CURRENT_CHANNEL	ADC_CHSELR_CHSEL3
#define INTERNAL_REF_CHANNEL	ADC_CHSELR_CHSEL17

#endif //__ADC_DEFINES_H__
