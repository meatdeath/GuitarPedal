// Based on OpenMusicLabs previous works.
// Цифровой усилитель: нажатие кнопки 1 или кнопки 2 увеличивает или уменьшает громкость.

// #include <iarduino_encoder_tmr.h> // Подключаем библиотеку iarduino_Encoder_tmr для работы с энкодерами через аппаратный таймер
// #include "U8glib.h"
#include <LiquidCrystal_I2C.h>

// iarduino_Encoder_tmr enc(ENC_CLK_PIN, ENC_DT_PIN);  // Енкодер
// U8GLIB_SH1106_128X64 u8g(U8G_I2C_OPT_NO_ACK);       // Display which does not send ACK

LiquidCrystal_I2C lcd(0x3F,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

// ------------- Pins --------------------------------------------------------------
#define FOOT_PIN        12 // Нажатие "Ножной" кнопки ВКЛ/ВЫКЛ устройсва
#define ENTER_PIN       6  // D4 мастер кнопка переключатель режимов enter
#define EXIT_PIN        7  // D5 мастер кнопка переключатель режимов exit
#define LEFT_PIN        4  // D4 мастер кнопка переключатель режимов left
#define RIGHT_PIN       5  // D5 мастер кнопка переключатель режимов right
#define ENC_CLK_PIN     2  // CLK Провод Енкодера
#define ENC_DT_PIN      3  // DT провод Енкодера
// ------------- ШИМ ---------------------------------------------------------------
#define PWM_FREQ        0x00FF // установка частоты ШИМ-а (режим Фазовый - 2х8 Бит 31.3 кГц)
#define PWM_MODE        0 // Устанавливает режим коррекции фазы (0) или быстрый режим (1)
#define PWM_QTY         2 // Используем 2 ШИМ-а параллельно 2х8 Бит
// ------------- РЕЖИМ -------------------------------------------------------------
enum _mode_en {
    MODE_CLEAR,
    MODE_DISTORTION,
    MODE_OVERDRIVE,
    MODE_HORUS,
    MODE_COMPRESS,
    MODE_ELECTRO,
    MODE_MAX
};

const char titleString[] = "Guitar Pedal v.1";
const char EffectString[] = "Effect";

const char ModeString[MODE_MAX][17] = {
    "Clear",
    "Distortion",
    "Overdrive",
    "Horus",
    "Compress",
    "Electro"
};

enum DistParamsEn {
    DIST_PARAM_LEVEL = 0,
    DIST_PARAM_AMP
};

const char DistParamShort[2][17] = {
    "Level",
    "Amplification"
};
const char DistParam[2][17] = {
    "Dist. level",
    "Dist. ampl."
};

typedef struct MenuItemSt MenuItem_t;

struct MenuItemSt {
    byte mode;
    const char *title;
    const char *string;
    byte leftIndex;
    byte rightIndex;
    byte upIndex;
    byte downIndex;
};

enum MenuItemEn {
    MENUITEM_CLEAR = 0,
    MENUITEM_DISTORTION,
    MENUITEM_OVERDRIVE,
    MENUITEM_HORUS,
    MENUITEM_COMPRESS,
    MENUITEM_ELECTRO,
    MENUITEM_DISTORTION_LEVEL,
    MENUITEM_DISTORTION_AMP,
    MENUITEM_MAX
};

byte currentMenuItem = 0;

#define EMPTY 0xFF

MenuItem_t menuItem[] = {
    {
        .mode = MODE_CLEAR,
        .title = EffectString,
        .string = ModeString[MODE_CLEAR],
        .leftIndex = MENUITEM_ELECTRO,
        .rightIndex = MENUITEM_DISTORTION,
        .upIndex = EMPTY,
        .downIndex = EMPTY // todo: goto params 
    },
    {
        .mode = MODE_DISTORTION,
        .title = EffectString,
        .string = ModeString[MODE_DISTORTION],
        .leftIndex = MENUITEM_CLEAR,
        .rightIndex = MENUITEM_OVERDRIVE,
        .upIndex = EMPTY,
        .downIndex = MENUITEM_DISTORTION_LEVEL // todo: goto params 
    },
    {
        .mode = MODE_OVERDRIVE,
        .title = EffectString,
        .string = ModeString[MODE_OVERDRIVE],
        .leftIndex = MENUITEM_DISTORTION,
        .rightIndex = MENUITEM_HORUS,
        .upIndex = EMPTY,
        .downIndex = EMPTY // todo: goto params 
    },
    {
        .mode = MODE_HORUS,
        .title = EffectString,
        .string = ModeString[MODE_HORUS],
        .leftIndex = MENUITEM_OVERDRIVE,
        .rightIndex = MENUITEM_COMPRESS,
        .upIndex = EMPTY,
        .downIndex = EMPTY // todo: goto params 
    },
    {
        .mode = MODE_COMPRESS,
        .title = EffectString,
        .string = ModeString[MODE_COMPRESS],
        .leftIndex = MENUITEM_HORUS,
        .rightIndex = MENUITEM_ELECTRO,
        .upIndex = EMPTY,
        .downIndex = EMPTY // todo: goto params 
    },
    {
        .mode = MODE_ELECTRO,
        .title = EffectString,
        .string = ModeString[MODE_ELECTRO],
        .leftIndex = MENUITEM_COMPRESS,
        .rightIndex = MENUITEM_CLEAR,
        .upIndex = EMPTY,
        .downIndex = EMPTY // todo: goto params 
    },
    {
        .mode = MODE_DISTORTION,
        .title = ModeString[MODE_DISTORTION],
        .string = DistParamShort[DIST_PARAM_LEVEL],
        .leftIndex = MENUITEM_DISTORTION_AMP,
        .rightIndex = MENUITEM_DISTORTION_AMP,
        .upIndex = MENUITEM_DISTORTION,
        .downIndex = EMPTY // todo:
    },
    {
        .mode = MODE_DISTORTION,
        .title = ModeString[MODE_DISTORTION],
        .string = DistParamShort[DIST_PARAM_AMP],
        .leftIndex = MENUITEM_DISTORTION_LEVEL,
        .rightIndex = MENUITEM_DISTORTION_LEVEL,
        .upIndex = MENUITEM_DISTORTION,
        .downIndex = EMPTY // todo:
    }
};


enum signalLevelsEn {
    LEVEL_0  = 0,
    LEVEL_5  = ((int32_t)5  * INT16_MAX) / 100,
    LEVEL_10 = ((int32_t)10 * INT16_MAX) / 100,
    LEVEL_15 = ((int32_t)15 * INT16_MAX) / 100,
    LEVEL_20 = ((int32_t)20 * INT16_MAX) / 100,
    LEVEL_25 = ((int32_t)25 * INT16_MAX) / 100,
    LEVEL_30 = ((int32_t)30 * INT16_MAX) / 100,
    LEVEL_35 = ((int32_t)35 * INT16_MAX) / 100,
    LEVEL_40 = ((int32_t)40 * INT16_MAX) / 100,
    LEVEL_45 = ((int32_t)45 * INT16_MAX) / 100,
    LEVEL_50 = ((int32_t)50 * INT16_MAX) / 100,
    LEVEL_55 = ((int32_t)55 * INT16_MAX) / 100,
    LEVEL_60 = ((int32_t)60 * INT16_MAX) / 100,
    LEVEL_65 = ((int32_t)65 * INT16_MAX) / 100,
    LEVEL_70 = ((int32_t)70 * INT16_MAX) / 100,
    LEVEL_75 = ((int32_t)75 * INT16_MAX) / 100,
    LEVEL_80 = ((int32_t)80 * INT16_MAX) / 100,
    LEVEL_85 = ((int32_t)85 * INT16_MAX) / 100,
    LEVEL_90 = ((int32_t)90 * INT16_MAX) / 100,
    LEVEL_95 = ((int32_t)95 * INT16_MAX) / 100,
    LEVEL_100 = INT16_MAX,

    LEVEL_12_5 = INT16_MAX / 8,
    LEVEL_6_25 = INT16_MAX / 16
};

byte ModeIndex = MODE_CLEAR; //MODE_OFF; // перменная выбора режима работы
// -------------- Энкодер + кнопка -------------------------------------------------
#define BTN_CHK_LOOPS 100 // Колличество повторений между проверками кнопки
byte LoopCount = 0; // Перменная Счетчика циклов пропуска
// ---------------------------------------------------------------------------------
byte DistAmp = 8; // Коэффициент усиления Дисторшн, 8 - без усиления
int16_t DistLevel = LEVEL_75;
// ---------------------------------------------------------------------------------
byte OverAmp = 8; // Коэффициент усиления эффекта Овердрайв, 16 - без эффекта
int16_t OverLevel = LEVEL_75;
// ---------------------------------------------------------------------------------
int Horus[256]; // Horus эффект
int16_t HorusDelay = 0; // перменная массива 0 - 256
// ---------------------------------------------------------------------------------
int16_t CompNoiseCutLevel = 512;
int ComprT = 30; // Compressor эффект
int16_t CompThreshold = LEVEL_75;
// ---------------------------------------------------------------------------------
int ElectroLevel = 15; // Electro
int ElectroCounter = 0;
int ElectroOut = 0;
// ---------------------------------------------------------------------------------

void setup(){
    Serial.begin(115200);
    Serial.println("Starting...");
    // enc.begin(); // Инициируем работу с энкодером

    pinMode(FOOT_PIN,  INPUT_PULLUP);
    pinMode(ENTER_PIN, INPUT_PULLUP);
    pinMode(EXIT_PIN,  INPUT_PULLUP);
    pinMode(LEFT_PIN,  INPUT_PULLUP);
    pinMode(RIGHT_PIN, INPUT_PULLUP);
    //----------------------Настройка АЦП - настроен для чтения автоматически. ------------------
    ADMUX = 0x60; // внутренний ИОН 1.1 В, правое выравнивание, управление по ADC0,
    ADCSRA = 0xe5; // включить ADC, автозапуск, 5 - clk 1/32 - ПОДБИРАЕМ
    ADCSRB = 0x07; // без мультиплексора, запуска захват таймером Т1
    DIDR0 = 0x01; // разрешаем ТОЛЬКО аналоговый вход ADC0
    //--------------------- настройка ШИМ -------------------------------------------------------
    TCCR1A = (((PWM_QTY - 1) << 5) | 0x80 | (PWM_MODE << 1));
    TCCR1B = ((PWM_MODE << 3) | 0x11); // clk_io/1 (нет деления) Делитель тактовой частоты 16 МГц
    TIMSK1 = 0x20;
    ICR1H = (PWM_FREQ >> 8);
    ICR1L = (PWM_FREQ & 0xff);
    DDRB |= ((PWM_QTY << 1) | 0x02); // включить выходы

    lcd.init();                      // initialize the lcd 
    // Print a message to the LCD.
    lcd.backlight();
    lcd.setCursor(0,0);
    lcd.print(titleString);
    lcd.setCursor(0,1);
    lcd.print("Starting...");
    delay(2000);

    sei(); // включить прерывания - на самом деле с arduino необязательно
    
    lcd.clear();
    // lcd.setCursor(0,0);
    // lcd.print("MODE____________");
}

enum MenuLevelEn {
    MENU_MAIN = 0,
    MENU_DISTORTION,
    MENU_OVERDRIVE,
    MENU_HORUS,
    MENU_COMPRESS,
    MENU_ELECTRO,
    MENU_MAX
};

byte MenuLevel = MENU_MAIN;


void loop() {
    LoopCount = LoopCount + 1;
    if (LoopCount == BTN_CHK_LOOPS) {
        if (digitalRead(FOOT_PIN)==HIGH) {
            int8_t step = 0;


            bool ProtectionDelay = false;
            //step = enc.read();
            if ((digitalRead(LEFT_PIN)) == LOW && menuItem[currentMenuItem].leftIndex != EMPTY ) {
                currentMenuItem = menuItem[currentMenuItem].leftIndex;
                ProtectionDelay = true;
            }
            if ((digitalRead(RIGHT_PIN)) == LOW && menuItem[currentMenuItem].rightIndex != EMPTY ) {
                currentMenuItem = menuItem[currentMenuItem].rightIndex;
                ProtectionDelay = true;
            }
            if ((digitalRead(EXIT_PIN)) == LOW && menuItem[currentMenuItem].upIndex != EMPTY ) {
                currentMenuItem = menuItem[currentMenuItem].upIndex;
                ProtectionDelay = true;
            }
            if ((digitalRead(ENTER_PIN)) == LOW && menuItem[currentMenuItem].downIndex != EMPTY ) {
                currentMenuItem = menuItem[currentMenuItem].downIndex;
                ProtectionDelay = true;
            }


            lcd.setCursor(0,0);
            lcd.print( menuItem[currentMenuItem].title );
            lcd.print("                ");
            lcd.setCursor(0,1);
            if( menuItem[currentMenuItem].leftIndex != -1 ) lcd.print("<"); else lcd.print(" ");
            byte spaces1 = (14-strlen(menuItem[currentMenuItem].string))/2;
            byte spaces2 = (15-strlen(menuItem[currentMenuItem].string))/2;
            while(spaces1--) lcd.print(" ");
            lcd.print(menuItem[currentMenuItem].string);
            while(spaces2--) lcd.print(" ");
            if( menuItem[currentMenuItem].rightIndex != -1 ) lcd.print(">"); else lcd.print(" ");
            

            switch(ModeIndex)
            {
            case MODE_CLEAR: break;
            case MODE_DISTORTION:
                DistAmp = DistAmp + step; // Если энкодер зафиксирует поворот, то значение переменной i изменится:
                if ( DistAmp < 10 ) { DistAmp = 10; }
                if ( DistAmp > 26 ) { DistAmp = 26; }
                break;
            case MODE_OVERDRIVE:
                OverAmp = OverAmp + step; // Если энкодер зафиксирует поворот, то значение переменной i изменится:
                if ( OverAmp < 1)   { OverAmp = 1; }
                if ( OverAmp > 26 ) { OverAmp = 26; }
                break;
            case MODE_HORUS:
                HorusDelay = HorusDelay + step*10; // Если энкодер зафиксирует поворот, то значение переменной i изменится:
                if (HorusDelay < 0)   HorusDelay = 0;
                if (HorusDelay > 250) HorusDelay = 250;
                break;
            case MODE_COMPRESS:
                ComprT = ComprT - step; // Если энкодер зафиксирует поворот, то значение переменной i изменится:
                if ( ComprT < 10 ) { ComprT = 10; }
                if ( ComprT > 30 ) { ComprT = 30; }
                break;
            case MODE_ELECTRO:
                ElectroLevel = ElectroLevel + step; // Если энкодер зафиксирует поворот, то значение переменной i изменится:
                if ( ElectroLevel < 1 ) { ElectroLevel = 1; }
                if ( ElectroLevel > 64 ) { ElectroLevel = 64; }
                break;
            }
            
            if (ProtectionDelay) {
                delay (500);
            }
        //---------------------------------------------------------------------------------------------
        } else { // устройсво электрически не включено
            lcd.setCursor(0,0);
            lcd.print("                ");
            lcd.setCursor(0,1);
            lcd.print("      OFF       ");
        }
        LoopCount = 0;
    }
}

//************* Обработчик прерывания захвата ввода из АЦП 0 - Timer 1
ISR(TIMER1_CAPT_vect) // Команда обработки прерывания Timer 1
{
    // ------------- АЦП -> ЦАП ---------------------------------------------------------

    unsigned int ADC_low, ADC_high; // Младший и Старший байты из АЦП
    ADC_low  = ADCL; // читаем младший байт АЦП прочитан первым ( важно! )
    ADC_high = ADCH; // читаем старший байт АЦП прочитан вторым

    int16_t Signal = ( ((ADC_high << 8) | ADC_low) << 4 ) + 0x8000; // Собираем 16-битн. из 2-х байт полученных в АЦП

    switch (menuItem[currentMenuItem].mode) {
        //-----------------------------------------------------------------------------------------
        // ****** Эффект Distortion ******
        case MODE_DISTORTION: 
            if (Signal > 0) {
                // Все что выше DistLevel обрезаем по уровню (DistLevel) и умножаем на DistAmp
                if (Signal > DistLevel) { Signal = DistLevel; } //
                Signal = (Signal/8) * DistAmp; 
                if (Signal < 0) { Signal = INT16_MAX; } 
            } else {
                // Все что ниже -DistLevel обрезаем и умножаем на DistAmp
                if (Signal < -DistLevel) { Signal = -DistLevel; }
                Signal = (Signal/10) * DistAmp; 
                if (Signal > 0) { Signal = INT16_MIN; }
            }
            break;

        //-----------------------------------------------------------------------------------------
        // ****** Эффект Overdrive ******
        case MODE_OVERDRIVE: 
            if (Signal > OverLevel) {
                Signal = Signal - ((Signal - OverLevel)/16)*OverAmp; // Все что больше OverLevel Уменьшаем на OverAmp
            }
            if (Signal < -OverLevel) {
                Signal = Signal - ((Signal + OverLevel)/16)*OverAmp; // Все что меньше -OverLevel Уменьшаем на OverAmp
            }
            break;

        //-----------------------------------------------------------------------------------------
        // ****** Эффект Horus ******
        case MODE_HORUS: 
            {
                static byte horus_write_index = 0;
                byte horus_out_index = 0;
                Horus[horus_write_index] = Signal; // Все время записываем 256 значений -> Horus[horus_write_index]
                horus_out_index = horus_write_index - HorusDelay;
                if (Signal < Horus[horus_out_index]) {
                    Signal = Horus[horus_out_index];
                }
                horus_write_index++;
            }
            break;

        //-----------------------------------------------------------------------------------------
        // ****** Эффект Compress ******
        case MODE_COMPRESS: 
            if (Signal > CompNoiseCutLevel ) {
                Signal = Signal + (CompThreshold - Signal)/ComprT;
            }
            if (Signal < -CompNoiseCutLevel ) {
                Signal = Signal - (CompThreshold + Signal)/ComprT;
            }
            break;

        //-----------------------------------------------------------------------------------------
        // ****** Эффект Electro ******
        case MODE_ELECTRO: 
            if (ElectroCounter > ElectroLevel) {
                ElectroOut = Signal;
                ElectroCounter = 0;
            } else {
                Signal = ElectroOut;
            }
            ElectroCounter++;
            break;
    }
    //-----------------------------------------------------------------------------------------


    OCR1AL = ((Signal + 0x8000) >> 8); // старший байт -> OC1A
    OCR1BL = Signal; // младший байт -> OC1B
}