// Pin Usage
// Function      **  Pin Name ** Board Pin Out
// TIM4 CH1 IC1  **  PB6      ** D10
// TIM3 CH1 PWM  **  PA6      ** D12

// TIM4 CH1 IC1  **  PB6      ** D10
#define TIM4_CH1_IT1_RCC_GPIO  RCC_APB2Periph_GPIOB
#define TIM4_CH1_IT1_GPIO      GPIOB
#define TIM4_CH1_IT1_PIN       GPIO_Pin_6

// TIM3 CH1 PWM  **  PA6      ** D12
#define TIM3_CH1_PWM_RCC_GPIO   RCC_APB2Periph_GPIOA
#define TIM3_CH1_PWM_GPIO       GPIOA
#define TIM3_CH1_PWM_PIN        GPIO_Pin_6

//Funcation prototypes
void TIM4_CH1_IC1_init(void);
void TIM3_CH1_PWM_init(void);
void USART2_init(void);
void USARTSend(char *pucBuffer, unsigned long ulCount);

//For debugging******************************
//LD2 PA5
#define LD2_RCC_GPIO  RCC_APB2Periph_GPIOA
#define LD2_GPIO      GPIOA
#define LD2_PIN       GPIO_Pin_5
//*******************************************
