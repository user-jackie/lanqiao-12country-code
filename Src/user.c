#include "user.h"

//总结1：一般只需要单次执行的与按键有关的变量都放在key_down的判断中
//总结2：在循环中需要第二次循环进入的代码函数

//问题1：c语言中是否有函数能完成数组溢出后自动往数组最后一个元素添加变量，第一个元素自动被覆盖
//问题2：c语言中数组从大到小排序的函数
//问题3：
/*************************************************************************************/
#define data_ui 0
#define para_ui 1

u8 ui = 0;

float data_a;
float data_b;
u16 data_f;
u8 data_ax;
u8 data_bx;
char data_mode = 'A';

u8 para_Pax = 20;
u8 para_Pbx = 20;
u16 para_Pf = 1000;

//ic中的变量
float pa6_angle;
float pa7_angle;
float pa6_angle_old;
float pa7_angle_old;
//角度数组
float pa6_angle_buf[5];
float pa7_angle_buf[5];
u8 pa6_angle_pointer;
u8 pa7_angle_pointer;

_Bool data_abx_num;

float adc_volt;
float adc_volt_old;

/*************************************************************************************/

void led_disp(u8 led)
{
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOC, 0xFF00, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOC, led << 8, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_RESET);
}

u8 key_down, key_up, key_value, key_old;
void key_read(void)     /* 上拉输入，按键按下电平被拉低！！ */
{
    if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == GPIO_PIN_RESET)
        key_value = 1;
    else if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) == GPIO_PIN_RESET)
        key_value = 2;
    else if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2) == GPIO_PIN_RESET)
        key_value = 3;
    else if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_RESET)
        key_value = 4;
    else
        key_value = 0;
    key_down = key_value & (key_value ^ key_old);
    key_up = ~key_value & (key_value ^ key_old);
    key_old = key_value;
}

u32 led_tick = 0;
u8 led_num = 0;
void led_proc(void)
{
    if(uwTick - led_tick < 100)
        return;
    led_tick = uwTick;
    
    if(data_ax > para_Pax)
        led_num |= (1 << 0);
    else
        led_num &= ~(1 << 0);
    if(data_bx > para_Pbx)
        led_num |= (1 << 1);
    else
        led_num &= ~(1 << 1);
    if(data_f > para_Pf)
        led_num |= (1 << 2);
    else
        led_num &= ~(1 << 2);
    if(data_mode == 'A')
        led_num |= (1 << 3);
    else
        led_num &= ~(1 << 3);
    if(data_a - data_b > 80.0f)
        led_num |= (1 << 4);
    else
        led_num &= ~(1 << 4);
    
    led_disp(led_num);
}

u32 key_tick = 0;
void key_proc(void)
{
    if(uwTick - key_tick < 20)
        return;
    key_tick = uwTick;
    key_read();
    if(key_down == 1)
    {
        ui = (ui + 1) % 2;
        LCD_Clear(Black);
    }
    else if(key_down == 2)
    {
        if(ui == para_ui)
        {
//            para_Pax += 10;
//            if(para_Pax > 60)
//                para_Pax = 10;
            para_Pax = (para_Pax + 10) % 60;
            para_Pbx += 10;
            if(para_Pbx > 60)
                para_Pbx = 10;
        }
    }
    else if(key_down == 3)
    {
        if(ui == data_ui)
        {
            if(data_mode != 'B')
                data_mode = 'B';
            else
                data_mode = 'A';
        }
        else if(ui == para_ui)
        {
            para_Pf += 1000;
            if(para_Pf > 10000)
                para_Pf = 1000;
        }
    }
    else if(key_down == 4)
    {
        if(data_mode == 'A')
        {
            data_a = pa6_angle;
            data_b = pa7_angle;
            if(pa6_angle_pointer < 5)
                pa6_angle_buf[pa6_angle_pointer ++] = pa6_angle;        /* pa6_angle_pointer == 5 时数组满了 */
            else
            {
                pa6_angle_pointer = 5;
                for(u8 i = 0; i < 4; i ++)
                    pa6_angle_buf[i] = pa6_angle_buf[i + 1];
                pa6_angle_buf[4] = pa6_angle;
            }
            if(pa7_angle_pointer < 5)
                pa7_angle_buf[pa7_angle_pointer ++] = pa7_angle;        /* pa7_angle_pointer == 5 时数组满了 */
            else
            {
                pa7_angle_pointer = 5;
                for(u8 j = 0; j < 4; j ++)
                    pa7_angle_buf[j] = pa7_angle_buf[j + 1];
                pa7_angle_buf[4] = pa7_angle;
            }
        }
        //第一次进入该循环不执行，第二次才执行的函数逻辑代码
        if(data_abx_num != 1)
            data_abx_num = 1;
        else
        {
            if(pa6_angle - pa6_angle_old >= 0)
                data_ax = pa6_angle - pa6_angle_old + 0.5f;     //四舍五入
            else
                data_ax = pa6_angle_old - pa6_angle + 0.5f;
            if(pa7_angle - pa7_angle_old >= 0)
                data_bx = pa7_angle - pa7_angle_old + 0.5f;     //四舍五入
            else
                data_bx = pa7_angle_old - pa7_angle + 0.5f;
        }
        pa6_angle_old = pa6_angle;
        pa7_angle_old = pa7_angle;
    }
    if(data_mode == 'B')
    {
        if(adc_volt - adc_volt_old > 1.0f)
        {
            data_a = pa6_angle;
            data_b = pa7_angle;
            if(pa6_angle_pointer < 5)
                pa6_angle_buf[pa6_angle_pointer ++] = pa6_angle;        /* pa6_angle_pointer == 5 时数组满了 */
            else
            {
                pa6_angle_pointer = 5;
                for(u8 i = 0; i < 4; i ++)
                    pa6_angle_buf[i] = pa6_angle_buf[i + 1];
                pa6_angle_buf[4] = pa6_angle;
            }
            if(pa7_angle_pointer < 5)
                pa7_angle_buf[pa7_angle_pointer ++] = pa7_angle;        /* pa7_angle_pointer == 5 时数组满了 */
            else
            {
                pa7_angle_pointer = 5;
                for(u8 j = 0; j < 4; j ++)
                    pa7_angle_buf[j] = pa7_angle_buf[j + 1];
                pa7_angle_buf[4] = pa7_angle;
            }
            if(data_abx_num != 1)
                data_abx_num = 1;
            else
            {
                data_ax = pa6_angle - pa6_angle_old;
                data_bx = pa7_angle - pa7_angle_old;
            }
            pa6_angle_old = pa6_angle;
            pa7_angle_old = pa7_angle;
        }
        adc_volt_old = adc_volt;
    }
}

u32 lcd_tick = 0;
u8 lcd_buf[40];
void lcd_proc(void)
{
    if(uwTick - lcd_tick < 100)
        return;
    lcd_tick = uwTick;
    
    if(ui == data_ui)
    {
        sprintf((char *)lcd_buf, "        DATA              ");
        LCD_DisplayStringLine(Line1, lcd_buf);
        sprintf((char *)lcd_buf, "   a:%.1f                 ", data_a);
        LCD_DisplayStringLine(Line2, lcd_buf);
        sprintf((char *)lcd_buf, "   b:%.1f                 ", data_b);
        LCD_DisplayStringLine(Line3, lcd_buf);
        sprintf((char *)lcd_buf, "   f:%dHz                 ", data_f);
        LCD_DisplayStringLine(Line4, lcd_buf);
        sprintf((char *)lcd_buf, "   ax:%d                  ", data_ax);
        LCD_DisplayStringLine(Line6, lcd_buf);
        sprintf((char *)lcd_buf, "   bx:%d                  ", data_bx);
        LCD_DisplayStringLine(Line7, lcd_buf);
        sprintf((char *)lcd_buf, "   mode:%c                ", data_mode);
        LCD_DisplayStringLine(Line8, lcd_buf);
    }
    else if(ui == para_ui)
    {
        sprintf((char *)lcd_buf, "        PARA              ");
        LCD_DisplayStringLine(Line1, lcd_buf);   
        sprintf((char *)lcd_buf, "   Pax:%d                 ", para_Pax);
        LCD_DisplayStringLine(Line2, lcd_buf);
        sprintf((char *)lcd_buf, "   Pbx:%d                 ", para_Pbx);
        LCD_DisplayStringLine(Line3, lcd_buf);
        sprintf((char *)lcd_buf, "   Pf:%d                  ", para_Pf);
        LCD_DisplayStringLine(Line4, lcd_buf);      /* 这一行一定要存在！！ */
    }
}

//PA2作为PWM输出，模拟
uint32_t uwIC2Value1 = 0;
uint32_t uwIC2Value2 = 0;
uint32_t uwDiffCapture = 0;
uint16_t uhCaptureIndex = 0;
uint32_t pa1_frq_read = 0;

uint32_t uwIC2Value11 = 0;
uint32_t uwIC2Value21 = 0;
uint32_t uwIC2Value31 = 0;
uint16_t uhCaptureIndex1 = 0;
uint16_t high_time = 0;
uint16_t low_time = 0;
uint8_t pa6_duty = 0;
uint16_t pa6_frq = 0;

uint32_t uwIC2Value12 = 0;
uint32_t uwIC2Value22 = 0;
uint32_t uwIC2Value32 = 0;
uint16_t uhCaptureIndex2 = 0;
uint16_t high_time2 = 0;
uint16_t low_time2 = 0;
uint8_t pa7_duty = 0;
uint16_t pa7_frq = 0;

u32 ic_tick = 0;
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
  if(htim == &htim2)    //PA1
  {
    if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
      {
        if(uhCaptureIndex == 0)
        {
          uwIC2Value1 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
          uhCaptureIndex = 1;
        }
        else if(uhCaptureIndex == 1)
        {
          uwIC2Value2 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2); 

          if (uwIC2Value2 > uwIC2Value1)
          {
            uwDiffCapture = (uwIC2Value2 - uwIC2Value1); 
          }
          else if (uwIC2Value2 < uwIC2Value1)
          {
            uwDiffCapture = ((0xFFFFFFFF - uwIC2Value1) + uwIC2Value2) + 1;
          }
          pa1_frq_read = 1e6 / uwDiffCapture;
          uhCaptureIndex = 0;
        }
      }
  }
  if(htim == &htim3)
  {
    if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)      //PA6
      {
        if(uhCaptureIndex1 == 0)
        {
          uwIC2Value11 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);        /* 注意要变换为通道1 */
          uhCaptureIndex1 = 1;
          __HAL_TIM_SET_CAPTUREPOLARITY(&htim3, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_FALLING);
        }
        else if(uhCaptureIndex1 == 1)
        {
          uwIC2Value21 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1); 

          if (uwIC2Value21 > uwIC2Value11)
          {
            high_time = (uwIC2Value21 - uwIC2Value11); 
          }
          else if (uwIC2Value21 < uwIC2Value11)
          {
            high_time = ((0xFFFF - uwIC2Value11) + uwIC2Value21) + 1;
          }
          uhCaptureIndex1 = 2;
          __HAL_TIM_SET_CAPTUREPOLARITY(&htim3, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_RISING);
        }
        else if(uhCaptureIndex1 == 2)
        {
          uwIC2Value31 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1); 

          if (uwIC2Value31 > uwIC2Value21)
          {
            low_time = (uwIC2Value31 - uwIC2Value21); 
          }
          else if (uwIC2Value31 < uwIC2Value21)
          {
            low_time = ((0xFFFF - uwIC2Value21) + uwIC2Value31) + 1;
          }
          uhCaptureIndex1 = 0;
          pa6_duty = 100 * high_time / (high_time + low_time);
          pa6_frq = 1e6 / (high_time + low_time);
        }
      }
      if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)        //PA7
      {
        if(uhCaptureIndex2 == 0)
        {
          uwIC2Value12 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);        /* 注意要变换为通道1 */
          uhCaptureIndex2 = 1;
          __HAL_TIM_SET_CAPTUREPOLARITY(&htim3, TIM_CHANNEL_2, TIM_INPUTCHANNELPOLARITY_FALLING);
        }
        else if(uhCaptureIndex2 == 1)
        {
          uwIC2Value22 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2); 

          if (uwIC2Value22 > uwIC2Value12)
          {
            high_time2 = (uwIC2Value22 - uwIC2Value12); 
          }
          else if (uwIC2Value22 < uwIC2Value12)
          {
            high_time2 = ((0xFFFF - uwIC2Value12) + uwIC2Value22) + 1;
          }
          uhCaptureIndex2 = 2;
          __HAL_TIM_SET_CAPTUREPOLARITY(&htim3, TIM_CHANNEL_2, TIM_INPUTCHANNELPOLARITY_RISING);
        }
        else if(uhCaptureIndex2 == 2)
        {
          uwIC2Value32 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2); 

          if (uwIC2Value32 > uwIC2Value22)
          {
            low_time2 = (uwIC2Value32 - uwIC2Value22); 
          }
          else if (uwIC2Value32 < uwIC2Value22)
          {
            low_time2 = ((0xFFFF - uwIC2Value22) + uwIC2Value32) + 1;
          }
          uhCaptureIndex2 = 0;
          pa7_duty = 100 * high_time2 / (high_time2 + low_time2);
          pa7_frq = 1e6 / (high_time2 + low_time2);
        }
      }
      if(uwTick - ic_tick < 50)
          return;
      ic_tick = uwTick;
      data_f = pa1_frq_read;
      if(pa6_duty < 10)
          pa6_angle = 0;
      else if(pa6_duty > 90)
          pa6_angle = 180;
      else
          pa6_angle = (float)(pa6_duty - 10) * 2.25f;
      
      if(pa7_duty < 10)
          pa7_angle = 0;
      else if(pa7_duty > 90)
          pa7_angle = 90;
      else
          pa7_angle = (float)(pa7_duty - 10) * 1.125f;
  }
}

u32 adc_tick = 0;
u16 adc_value = 0;
float adc_volt = 0;
void adc_proc(void)
{
    if(uwTick - adc_tick < 100)
        return;
    adc_tick = uwTick;
    HAL_ADC_Start(&hadc2);
    adc_value = HAL_ADC_GetValue(&hadc2);
    //如果需要测量第二次的adc，需要再次使用HAL_ADC_Start(&hadc2)
    adc_volt = (float)adc_value * 3.3f / 4096.0f;
}

struct __FILE
{
  int handle;
};
FILE __stdout;
int fputc(int ch, FILE *f) 
{
  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 1000);
  return ch;
}


u32 rx_tick = 0;
u8 rx_pointer = 0;
u8 rx_buf[20] = {0};
u8 rx_data = 0;
float pa6_angle_buf_arr[5];
float pa7_angle_buf_arr[5];

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    rx_tick = uwTick;
    HAL_UART_Receive_IT(&huart1, &rx_data, 1);
    rx_buf[rx_pointer ++] = rx_data;
}

void rx_proc(void)
{
    if(uwTick - rx_tick < 50)
        return;
    rx_tick = uwTick;
    if(rx_pointer == 2 && rx_buf[0] == 'a' && rx_buf[1] == '?')     //memcmp(rx_buf, "a?")
        printf("a:%.1f\r\n", data_a);
    else if(rx_pointer == 2 && rx_buf[0] == 'b' && rx_buf[1] == '?')
        printf("a:%.1f\r\n", data_b);
    else if(rx_pointer == 3 && rx_buf[0] == 'a' && rx_buf[1] == 'a' && rx_buf[2] == '?')
        printf("aa:%.1f-%.1f-%.1f-%.1f-%.1f\r\n", pa6_angle_buf[4], pa6_angle_buf[3], pa6_angle_buf[2], pa6_angle_buf[1], pa6_angle_buf[0]);
    else if(rx_pointer == 3 && rx_buf[0] == 'b' && rx_buf[1] == 'b' && rx_buf[2] == '?')
        printf("bb:%.1f-%.1f-%.1f-%.1f-%.1f\r\n", pa7_angle_buf[4], pa7_angle_buf[3], pa7_angle_buf[2], pa7_angle_buf[1], pa7_angle_buf[0]);
    else if(rx_pointer == 3 && rx_buf[0] == 'q' && rx_buf[1] == 'a' && rx_buf[2] == '?')
    {
        float temp = 0;
        for(u8 m = 0; m < 5; m ++)
            pa6_angle_buf_arr[m] = pa6_angle_buf[m];
        /*数组为[0 ,1, 2, 3, 4]
        i = 0, 1, 2, 3
        j = 
        */
        for(u8 i = 0; i < 4; i ++)              //数组长度 n = 5，i < n - 1
        {
            for(u8 j = i + 1; j < 5; j ++)      //数组长度 n = 5, (j = i + 1; j < n; j ++)
            {
                if(pa6_angle_buf_arr[i] > pa6_angle_buf_arr[j])     //从小到大
                {
                    temp = pa6_angle_buf_arr[i];
                    pa6_angle_buf_arr[i] = pa6_angle_buf_arr[j];
                    pa6_angle_buf_arr[j] = temp;
                }
            }
        }
        printf("qa:%.1f-%.1f-%.1f-%.1f-%.1f\r\n", pa6_angle_buf_arr[0], pa6_angle_buf_arr[1], pa6_angle_buf_arr[2], pa6_angle_buf_arr[3], pa6_angle_buf_arr[4]);
    }
    else if(rx_pointer == 3 && rx_buf[0] == 'q' && rx_buf[1] == 'b' && rx_buf[2] == '?')
    {
        float temp = 0;
        for(u8 m = 0; m < 5; m ++)
            pa7_angle_buf_arr[m] = pa7_angle_buf[m];
        /*数组为[0 ,1, 2, 3, 4]
        i = 0, 1, 2, 3
        j = 
        */
        for(u8 i = 0; i < 4; i ++)              //数组长度 n = 5，i < n - 1
        {
            for(u8 j = i + 1; j < 5; j ++)      //数组长度 n = 5, (j = i + 1; j < n; j ++)
            {
                if(pa7_angle_buf_arr[i] > pa7_angle_buf_arr[j])     //从小到大
                {
                    temp = pa7_angle_buf_arr[i];
                    pa7_angle_buf_arr[i] = pa7_angle_buf_arr[j];
                    pa7_angle_buf_arr[j] = temp;
                }
            }
        }
        printf("qa:%.1f-%.1f-%.1f-%.1f-%.1f\r\n", pa7_angle_buf_arr[0], pa7_angle_buf_arr[1], pa7_angle_buf_arr[2], pa7_angle_buf_arr[3], pa7_angle_buf_arr[4]);
    }
    else if(rx_pointer > 0)
        printf("error\r\n");
    rx_pointer = 0;
    memset(rx_buf, 0, sizeof(rx_buf));      //注意需要清空数组，否则会一直在某个判断中循环，一直打印
}



