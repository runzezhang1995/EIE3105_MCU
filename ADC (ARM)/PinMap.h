// Pin Usage
// Function      **  Pin Name ** Board Pin Out
// ADC1 CH0      **  PA0      ** A0
// TIM3 CH1 PWM  **  PA6      ** D12

// ADC1 Ch0      **  PA0      ** A0
#define ADC1_0_RCC_GPIO  RCC_APB2Periph_GPIOA
#define ADC1_0_GPIO      GPIOA
#define ADC1_0_PIN       GPIO_Pin_0

// TIM3 CH1 PWM  **  PA6      ** D12
#define TIM3_CH1_PWM_RCC_GPIO   RCC_APB2Periph_GPIOA
#define TIM3_CH1_PWM_GPIO       GPIOA
#define TIM3_CH1_PWM_PIN        GPIO_Pin_6

//Funcation prototypes
void ADC1_1channel_init(void);
void TIM3_CH1_PWM_init(void);
void USART2_init(void);
void USARTSend(char *pucBuffer, unsigned long ulCount);

//For debugging******************************
//LD2 PA5
#define LD2_RCC_GPIO  RCC_APB2Periph_GPIOA
#define LD2_GPIO      GPIOA
#define LD2_PIN       GPIO_Pin_5
//*******************************************
