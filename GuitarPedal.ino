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
#define LEFT_PIN        4  // D4 мастер кнопка переключатель режимов left
#define RIGHT_PIN       5  // D5 мастер кнопка переключатель режимов right
#define ENTER_PIN       6  // D6 мастер кнопка переключатель режимов enter
#define EXIT_PIN        7  // D7 мастер кнопка переключатель режимов exit
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
    MODE_CHORUS,
    MODE_COMPRESS,
    MODE_ELECTRO,
    MODE_MAX
};

const char titleString[] PROGMEM = "Guitar Pedal v.1";
const char EffectString[] PROGMEM  = "Effect";

const char ModeString[MODE_MAX][17] PROGMEM = {
    "Clear",
    "Distortion",
    "Overdrive",
    "Chorus",
    "Compress",
    "Electro"
};

enum DistParamsEn {
    DIST_PARAM_LEVEL = 0,
    DIST_PARAM_AMP
};

const char DistParamShort[2][17] PROGMEM = {
    "Level",
    "Amplification"
};
const char DistParam[2][17] PROGMEM = {
    "Dist. level",
    "Dist. ampl."
};

enum OverdParamsEn {
    OVERD_PARAM_LEVEL = 0,
    OVERD_PARAM_AMP
};

const char OverdParamShort[2][17] PROGMEM = {
    "Level",
    "Amplification"
};
const char OverdParam[2][17] PROGMEM = {
    "Overd. level",
    "Overd. ampl."
};

const char ChorusStrShort[] PROGMEM = "Delay";
const char ChorusStr[] PROGMEM = "Chorus delay";

const char CompressTresholdStrShort[] PROGMEM = "Threshold";
const char CompressTresholdStr[] PROGMEM = "Compr. threshold";
const char CompressKoeffStrShort[] PROGMEM = "Koefficient";
const char CompressKoeffStr[] PROGMEM = "Compr. koeff.";

char displayedParamString[17] = "";

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
    MENUITEM_CHORUS,
    MENUITEM_COMPRESS,
    MENUITEM_ELECTRO,
    MENUITEM_DISTORTION_LEVEL,
    MENUITEM_DISTORTION_AMP,
    MENUITEM_DISTORTION_LEVEL_VALUE,
    MENUITEM_DISTORTION_AMP_VALUE,
    MENUITEM_OVERDRIVE_LEVEL,
    MENUITEM_OVERDRIVE_AMP,
    MENUITEM_OVERDRIVE_LEVEL_VALUE,
    MENUITEM_OVERDRIVE_AMP_VALUE,
    MENUITEM_CHORUS_PARAM,
    MENUITEM_CHORUS_DELAY,
    MENUITEM_COMPRESS_TRESHOLD_PARAM,
    MENUITEM_COMPRESS_KOEFF_PARAM,
    MENUITEM_COMPRESS_TRESHOLD_VALUE,
    MENUITEM_COMPRESS_KOEFF_VALUE,
    MENUITEM_MAX
};

byte currentMenuItem = 0;

#define EMPTY 0xFF

 MenuItem_t menuItem[]  = {
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
        .rightIndex = MENUITEM_CHORUS,
        .upIndex = EMPTY,
        .downIndex = EMPTY // todo: goto params 
    },
    {
        .mode = MODE_CHORUS,
        .title = EffectString,
        .string = ModeString[MODE_CHORUS],
        .leftIndex = MENUITEM_OVERDRIVE,
        .rightIndex = MENUITEM_COMPRESS,
        .upIndex = EMPTY,
        .downIndex = EMPTY // todo: goto params 
    },
    {
        .mode = MODE_COMPRESS,
        .title = EffectString,
        .string = ModeString[MODE_COMPRESS],
        .leftIndex = MENUITEM_CHORUS,
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
        .downIndex = MENUITEM_DISTORTION_LEVEL_VALUE
    },
    {
        .mode = MODE_DISTORTION,
        .title = ModeString[MODE_DISTORTION],
        .string = DistParamShort[DIST_PARAM_AMP],
        .leftIndex = MENUITEM_DISTORTION_LEVEL,
        .rightIndex = MENUITEM_DISTORTION_LEVEL,
        .upIndex = MENUITEM_DISTORTION,
        .downIndex = MENUITEM_DISTORTION_AMP_VALUE
    },
    {
        .mode = MODE_DISTORTION,
        .title = DistParam[DIST_PARAM_LEVEL],
        .string = displayedParamString,
        .leftIndex = EMPTY,
        .rightIndex = EMPTY,
        .upIndex = MENUITEM_DISTORTION_AMP,
        .downIndex = EMPTY // todo:
    },
    {
        .mode = MODE_DISTORTION,
        .title = DistParam[DIST_PARAM_AMP],
        .string = displayedParamString,
        .leftIndex = EMPTY,
        .rightIndex = EMPTY,
        .upIndex = MENUITEM_DISTORTION_LEVEL,
        .downIndex = EMPTY // todo:
    },
    {
        .mode = MODE_OVERDRIVE,
        .title = ModeString[MODE_OVERDRIVE],
        .string = OverdParamShort[OVERD_PARAM_LEVEL],
        .leftIndex = MENUITEM_OVERDRIVE_AMP,
        .rightIndex = MENUITEM_OVERDRIVE_AMP,
        .upIndex = MENUITEM_OVERDRIVE,
        .downIndex = MENUITEM_OVERDRIVE_LEVEL_VALUE
    },
    {
        .mode = MODE_OVERDRIVE,
        .title = ModeString[MODE_OVERDRIVE],
        .string = OverdParamShort[OVERD_PARAM_AMP],
        .leftIndex = MENUITEM_OVERDRIVE_LEVEL,
        .rightIndex = MENUITEM_OVERDRIVE_LEVEL,
        .upIndex = MENUITEM_OVERDRIVE,
        .downIndex = MENUITEM_OVERDRIVE_AMP_VALUE
    },
    {
        .mode = MODE_OVERDRIVE,
        .title = OverdParam[OVERD_PARAM_LEVEL],
        .string = displayedParamString,
        .leftIndex = EMPTY,
        .rightIndex = EMPTY,
        .upIndex = MENUITEM_OVERDRIVE_AMP,
        .downIndex = EMPTY // todo:
    },
    {
        .mode = MODE_OVERDRIVE,
        .title = OverdParam[OVERD_PARAM_AMP],
        .string = displayedParamString,
        .leftIndex = EMPTY,
        .rightIndex = EMPTY,
        .upIndex = MENUITEM_OVERDRIVE_AMP,
        .downIndex = EMPTY // todo:
    },
    {
        .mode = MODE_CHORUS,
        .title = ModeString[MODE_CHORUS],
        .string = ChorusStrShort,
        .leftIndex = EMPTY,
        .rightIndex = EMPTY,
        .upIndex = MENUITEM_CHORUS,
        .downIndex = MENUITEM_CHORUS_DELAY // todo:
    },
    {
        .mode = MODE_CHORUS,
        .title = ChorusStr,
        .string = displayedParamString,
        .leftIndex = EMPTY,
        .rightIndex = EMPTY,
        .upIndex = MENUITEM_CHORUS_PARAM,
        .downIndex = EMPTY // todo:
    },
    {
        .mode = MODE_COMPRESS,
        .title = ModeString[MODE_COMPRESS],
        .string = CompressTresholdStrShort,
        .leftIndex = MENUITEM_COMPRESS_KOEFF_PARAM,
        .rightIndex = MENUITEM_COMPRESS_KOEFF_PARAM,
        .upIndex = MENUITEM_COMPRESS,
        .downIndex = MENUITEM_COMPRESS_TRESHOLD_VALUE // todo:
    },
    {
        .mode = MODE_COMPRESS,
        .title = ModeString[MODE_COMPRESS],
        .string = CompressKoeffStrShort,
        .leftIndex = MENUITEM_COMPRESS_TRESHOLD_PARAM,
        .rightIndex = MENUITEM_COMPRESS_TRESHOLD_PARAM,
        .upIndex = MENUITEM_COMPRESS,
        .downIndex = MENUITEM_COMPRESS_KOEFF_VALUE // todo:
    },
    {
        .mode = MODE_COMPRESS,
        .title = CompressKoeffStr,
        .string = displayedParamString,
        .leftIndex = EMPTY,
        .rightIndex = EMPTY,
        .upIndex = MENUITEM_COMPRESS_TRESHOLD_PARAM,
        .downIndex = EMPTY // todo:
    },
    {
        .mode = MODE_COMPRESS,
        .title = CompressKoeffStr,
        .string = displayedParamString,
        .leftIndex = EMPTY,
        .rightIndex = EMPTY,
        .upIndex = MENUITEM_COMPRESS_KOEFF_PARAM,
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

typedef struct LevelsSt {
    int16_t value;
    char string[5];
} Levels_t;
#define LEVELS_COUNT 19
const Levels_t Levels[LEVELS_COUNT] PROGMEM = {
    {LEVEL_5,   "5%%"}, {LEVEL_10, "10%%"}, {LEVEL_15, "15%%"}, {LEVEL_20, "20%%"}, 
    {LEVEL_25, "25%%"}, {LEVEL_30, "30%%"}, {LEVEL_35, "35%%"}, {LEVEL_40, "40%%"},
    {LEVEL_45, "45%%"}, {LEVEL_50, "50%%"}, {LEVEL_55, "55%%"}, {LEVEL_60, "60%%"}, 
    {LEVEL_65, "65%%"}, {LEVEL_70, "70%%"}, {LEVEL_75, "75%%"}, {LEVEL_80, "80%%"},
    {LEVEL_85, "85%%"}, {LEVEL_90, "90%%"}, {LEVEL_95, "95%%"}
};

typedef struct amplificationsSt {
    int16_t value;
    char string[5];
} Amps_t;
#define DIST_AMPS_COUNT 19
const Amps_t DistAmps[DIST_AMPS_COUNT] PROGMEM = {
    { 8,  " 0"}, { 9,  "+1"}, {10,  "+2"}, {11,  "+3"}, 
    {12,  "+4"}, {13,  "+5"}, {14,  "+6"}, {15,  "+7"}, 
    {16,  "+8"}, {17,  "+9"}, {18, "+10"}, {19, "+11"}, 
    {20, "+12"}, {21, "+13"}, {22, "+14"}, {23, "+15"}, 
    {24, "+16"}, {25, "+17"}, {26, "+18"},
};

byte ModeIndex = MODE_CLEAR; //MODE_OFF; // перменная выбора режима работы
// -------------- Энкодер + кнопка -------------------------------------------------
#define BTN_CHK_LOOPS 100 // Колличество повторений между проверками кнопки
byte LoopCount = 0; // Перменная Счетчика циклов пропуска
// ---------------------------------------------------------------------------------
byte DistAmpIndex = 8; // Коэффициент усиления Дисторшн, 8 - без усиления
int16_t DistLevelIndex = 14; //75%
// ---------------------------------------------------------------------------------

typedef struct OverAmpSt {
    int16_t value;
    char string[15];
} OverAmp_t;

#define OVER_AMP_COUNT 19
OverAmp_t OverAmps[OVER_AMP_COUNT] = {
    { 8,  "-8"}, { 9,  "-7"}, {10,  "-6"}, {11,  "-5"},
    {12,  "-4"}, {13,  "-3"}, {14,  "-2"}, {15,  "-1"},
    {16,   "0"}, {17,  "+1"}, {18,  "+2"}, {19,  "+3"},
    {20,  "+4"}, {21,  "+5"}, {22,  "+6"}, {24,  "+7"},
    {24,  "+8"}, {25,  "+9"}, {26, "+10"}
};

byte OverAmpIndex = 0; // Коэффициент усиления эффекта Овердрайв, 16 - без эффекта
int16_t OverLevelIndex = 14; //75;
// ---------------------------------------------------------------------------------

int Chorus[256]; // Chorus эффект
int16_t ChorusDelay = 0; // перменная массива 0 - 256
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
    MENU_CHORUS,
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
            if( (digitalRead(LEFT_PIN)) == LOW && menuItem[currentMenuItem].leftIndex != EMPTY ) {
                switch( currentMenuItem ) {
                    case MENUITEM_DISTORTION_LEVEL_VALUE:
                        if( DistLevelIndex > 0) DistLevelIndex--;
                        strncpy(displayedParamString, Levels[DistLevelIndex].string, 14);
                        break;
                    case MENUITEM_DISTORTION_AMP_VALUE:
                        if( DistAmpIndex > 0) DistAmpIndex--;
                        strncpy(displayedParamString, DistAmps[DistAmpIndex].string, 14);
                        break;
                    case MENUITEM_OVERDRIVE_LEVEL_VALUE:
                        if (OverLevelIndex > 0) OverLevelIndex--;
                        strncpy(displayedParamString, Levels[OverLevelIndex].string, 14);
                        break;
                    case MENUITEM_OVERDRIVE_AMP_VALUE:
                        if (OverAmpIndex > 0) OverAmpIndex--;
                        strncpy(displayedParamString, OverAmps[OverAmpIndex].string, 14);
                        break;

                }
                if (menuItem[currentMenuItem].leftIndex != EMPTY) {
                    currentMenuItem = menuItem[currentMenuItem].leftIndex;
                    if (menuItem[currentMenuItem].upIndex != EMPTY) {
                        menuItem[menuItem[currentMenuItem].upIndex].downIndex = currentMenuItem;
                    }
                }
                ProtectionDelay = true;
            }
            if( (digitalRead(RIGHT_PIN)) == LOW && menuItem[currentMenuItem].rightIndex != EMPTY ) {
                switch( currentMenuItem ) {
                    case MENUITEM_DISTORTION_LEVEL_VALUE:
                        if( DistLevelIndex < (LEVELS_COUNT-1)) DistLevelIndex++;
                        strncpy(displayedParamString, Levels[DistLevelIndex].string, 14);
                        break;
                    case MENUITEM_DISTORTION_AMP_VALUE:
                        if( DistAmpIndex < (DIST_AMPS_COUNT-1)) DistAmpIndex++;
                        strncpy(displayedParamString, DistAmps[DistAmpIndex].string, 14);
                        break;
                    case MENUITEM_OVERDRIVE_LEVEL_VALUE:
                        if (OverLevelIndex < (LEVELS_COUNT-1)) OverLevelIndex++;
                        strncpy(displayedParamString, Levels[OverLevelIndex].string, 14);
                        break;
                    case MENUITEM_OVERDRIVE_AMP_VALUE:
                        if (OverAmpIndex < (OVER_AMP_COUNT-1)) OverAmpIndex++;
                        strncpy(displayedParamString, OverAmps[OverAmpIndex].string, 14);
                        break;
                }
                if (menuItem[currentMenuItem].rightIndex != EMPTY) {
                    currentMenuItem = menuItem[currentMenuItem].rightIndex;
                    if (menuItem[currentMenuItem].upIndex != EMPTY) {
                        menuItem[menuItem[currentMenuItem].upIndex].downIndex = currentMenuItem;
                    }
                }
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
            

            // switch(ModeIndex)
            // {
            // case MODE_CLEAR: break;
            // case MODE_DISTORTION:
            //     DistAmp = DistAmp + step; // Если энкодер зафиксирует поворот, то значение переменной i изменится:
            //     if ( DistAmp < 10 ) { DistAmp = 10; }
            //     if ( DistAmp > 26 ) { DistAmp = 26; }
            //     break;
            // case MODE_OVERDRIVE:
            //     OverAmp = OverAmp + step; // Если энкодер зафиксирует поворот, то значение переменной i изменится:
            //     if ( OverAmp < 1)   { OverAmp = 1; }
            //     if ( OverAmp > 26 ) { OverAmp = 26; }
            //     break;
            // case MODE_CHORUS:
            //     ChorusDelay = ChorusDelay + step*10; // Если энкодер зафиксирует поворот, то значение переменной i изменится:
            //     if (ChorusDelay < 0)   ChorusDelay = 0;
            //     if (ChorusDelay > 250) ChorusDelay = 250;
            //     break;
            // case MODE_COMPRESS:
            //     ComprT = ComprT - step; // Если энкодер зафиксирует поворот, то значение переменной i изменится:
            //     if ( ComprT < 10 ) { ComprT = 10; }
            //     if ( ComprT > 30 ) { ComprT = 30; }
            //     break;
            // case MODE_ELECTRO:
            //     ElectroLevel = ElectroLevel + step; // Если энкодер зафиксирует поворот, то значение переменной i изменится:
            //     if ( ElectroLevel < 1 ) { ElectroLevel = 1; }
            //     if ( ElectroLevel > 64 ) { ElectroLevel = 64; }
            //     break;
            // }
            
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
                if (Signal > Levels[DistLevelIndex].value) { Signal = Levels[DistLevelIndex].value; } //
                Signal = (Signal/8) * DistAmps[DistAmpIndex].value; 
                if (Signal < 0) { Signal = INT16_MAX; } 
            } else {
                // Все что ниже -DistLevel обрезаем и умножаем на DistAmp
                if (Signal < -Levels[DistLevelIndex].value) { Signal = -Levels[DistLevelIndex].value; }
                Signal = (Signal/8) * DistAmps[DistAmpIndex].value; 
                if (Signal > 0) { Signal = INT16_MIN; }
            }
            break;

        //-----------------------------------------------------------------------------------------
        // ****** Эффект Overdrive ******
        case MODE_OVERDRIVE: 
            if (Signal > Levels[OverLevelIndex].value) {
                Signal = Signal - ((Signal - Levels[OverLevelIndex].value)/16)*OverAmps[OverAmpIndex].value; // Все что больше OverLevel Уменьшаем на OverAmp
            }
            if (Signal < -Levels[OverLevelIndex].value) {
                Signal = Signal - ((Signal + Levels[OverLevelIndex].value)/16)*OverAmps[OverAmpIndex].value; // Все что меньше -OverLevel Уменьшаем на OverAmp
            }
            break;

        //-----------------------------------------------------------------------------------------
        // ****** Эффект Chorus ******
        case MODE_CHORUS: 
            {
                static byte horus_write_index = 0;
                byte horus_out_index = 0;
                Chorus[horus_write_index] = Signal; // Все время записываем 256 значений -> Chorus[horus_write_index]
                horus_out_index = horus_write_index - ChorusDelay;
                if (Signal < Chorus[horus_out_index]) {
                    Signal = Chorus[horus_out_index];
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