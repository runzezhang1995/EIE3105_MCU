// Pin Usage
// Function      **  Pin Name ** Board Pin Out
// TIM4 CH1 IC1  **  PB6      ** D10
// TIM3 CH1 PWM  **  PA6      ** D12

// EXTI WHEEL COUNT LEFT  **  PB6     
#define WHEEL_COUNT_LEFT_RCC_GPIO  RCC_APB2Periph_GPIOB
#define WHEEL_COUNT_LEFT_GPIO      GPIOB
#define WHEEL_COUNT_LEFT_PIN       GPIO_Pin_6

// EXTI WHEEL COUNT LEFT  **  PA1     
#define WHEEL_COUNT_RIGHT_RCC_GPIO  RCC_APB2Periph_GPIOA
#define WHEEL_COUNT_RIGHT_GPIO      GPIOA
#define WHEEL_COUNT_RIGHT_PIN       GPIO_Pin_1


// TIM3 CH1 PWM  **  PA6      ** D12
#define TIM3_CH1_PWM_RCC_GPIO   RCC_APB2Periph_GPIOA
#define TIM3_CH1_PWM_GPIO       GPIOA
#define TIM3_CH1_PWM_PIN        GPIO_Pin_6

// TIM3 CH1 PWM  **  PA7      ** D12
#define TIM3_CH2_PWM_RCC_GPIO   RCC_APB2Periph_GPIOA
#define TIM3_CH2_PWM_GPIO       GPIOA
#define TIM3_CH2_PWM_PIN        GPIO_Pin_7


// wheel left dir PA0
#define WHEEL_DIR_LEFT_RCC_GPIO RCC_APB2Periph_GPIOA
#define WHEEL_DIR_LEFT_GPIO       GPIOA
#define WHEEL_DIR_LEFT_PIN        GPIO_Pin_0
// wheel right dir PC15
#define WHEEL_DIR_RIGHT_RCC_GPIO RCC_APB2Periph_GPIOC
#define WHEEL_DIR_RIGHT_GPIO       GPIOC
#define WHEEL_DIR_RIGHT_PIN        GPIO_Pin_15

// user button  PB8
#define USER_BUTTON_RCC_GPIO RCC_APB2Periph_GPIOB
#define USER_BUTTON_GPIO       GPIOB
#define USER_BUTTON_PIN        GPIO_Pin_8

// user button PB7
#define LED_RCC_GPIO RCC_APB2Periph_GPIOB
#define LED_GPIO       GPIOB
#define LED_PIN        GPIO_Pin_7


//Funcation prototypes
void TIM4_CH1_IC1_init(void);
void TIM3_PWM_init(void);
//void TIM2_init(void);
void Wheel_Dir_Init(void);
void USART2_init(void);
void User_Button_Init(void);
void Board_LED_Init(void);
void USARTSend(char *pucBuffer, unsigned long ulCount);
void EXTI_wheel_counter_init(void);
void TIM2_Init(void);
//For debugging******************************
//LD2 PA5
#define LD2_RCC_GPIO  RCC_APB2Periph_GPIOA
#define LD2_GPIO      GPIOA
#define LD2_PIN       GPIO_Pin_5
//*******************************************
