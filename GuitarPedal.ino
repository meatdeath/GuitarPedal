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

const char CompressThresholdStrShort[] PROGMEM = "Threshold";
const char CompressThresholdStr[] PROGMEM = "Compr. threshold";
const char CompressKoeffStrShort[] PROGMEM = "Koefficient";
const char CompressKoeffStr[] PROGMEM = "Compr. koeff.";
const char LevelStr[] PROGMEM = "Level";
const char ElectroLevelStr[] PROGMEM = "Electro level";
const char CompressNoiseCutLevelStrShort[] PROGMEM = "Noise cut lev.";
const char CompressNoiseCutLevelStr[] PROGMEM = "Compr. noise cut";

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
    MENUITEM_DISTORTION, //1
    MENUITEM_OVERDRIVE,
    MENUITEM_CHORUS,
    MENUITEM_COMPRESS,
    MENUITEM_ELECTRO,   // 5

    MENUITEM_DISTORTION_LEVEL,
    MENUITEM_DISTORTION_AMP,
    MENUITEM_DISTORTION_LEVEL_VALUE,
    MENUITEM_DISTORTION_AMP_VALUE,

    MENUITEM_OVERDRIVE_LEVEL,   // 10
    MENUITEM_OVERDRIVE_AMP,
    MENUITEM_OVERDRIVE_LEVEL_VALUE,
    MENUITEM_OVERDRIVE_AMP_VALUE,

    MENUITEM_CHORUS_DELAY,
    MENUITEM_CHORUS_DELAY_VALUE,    // 15

    MENUITEM_COMPRESS_THRESHOLD,
    MENUITEM_COMPRESS_KOEFF,
    MENUITEM_COMPRESS_NOISE_CUT,
    MENUITEM_COMPRESS_THRESHOLD_VALUE,
    MENUITEM_COMPRESS_KOEFF_VALUE,      // 20
    MENUITEM_COMPRESS_NOISE_CUT_VALUE,

    MENUITEM_ELECTRO_LEVEL,
    MENUITEM_ELECTRO_LEVEL_VALUE,

    MENUITEM_MAX
};

byte currentMenuItem = 0;

#define EMPTY 0xFF

 MenuItem_t menuItem[MENUITEM_MAX]  = {
    {   // 0 - MENUITEM_CLEAR
        .mode = MODE_CLEAR,
        .title = EffectString,
        .string = ModeString[MODE_CLEAR],
        .leftIndex = MENUITEM_ELECTRO,
        .rightIndex = MENUITEM_DISTORTION,
        .upIndex = EMPTY,
        .downIndex = EMPTY
    },
    {   // 1 - MENUITEM_DISTORTION
        .mode = MODE_DISTORTION,
        .title = EffectString,
        .string = ModeString[MODE_DISTORTION],
        .leftIndex = MENUITEM_CLEAR,
        .rightIndex = MENUITEM_OVERDRIVE,
        .upIndex = EMPTY,
        .downIndex = MENUITEM_DISTORTION_LEVEL
    },
    {   // MENUITEM_OVERDRIVE
        .mode = MODE_OVERDRIVE,
        .title = EffectString,
        .string = ModeString[MODE_OVERDRIVE],
        .leftIndex = MENUITEM_DISTORTION,
        .rightIndex = MENUITEM_CHORUS,
        .upIndex = EMPTY,
        .downIndex = MENUITEM_OVERDRIVE_LEVEL
    },
    {   // MENUITEM_CHORUS
        .mode = MODE_CHORUS,
        .title = EffectString,
        .string = ModeString[MODE_CHORUS],
        .leftIndex = MENUITEM_OVERDRIVE,
        .rightIndex = MENUITEM_COMPRESS,
        .upIndex = EMPTY,
        .downIndex = MENUITEM_CHORUS_DELAY 
    },
    {   // MENUITEM_COMPRESS
        .mode = MODE_COMPRESS,
        .title = EffectString,
        .string = ModeString[MODE_COMPRESS],
        .leftIndex = MENUITEM_CHORUS,
        .rightIndex = MENUITEM_ELECTRO,
        .upIndex = EMPTY,
        .downIndex = MENUITEM_COMPRESS_THRESHOLD
    },
    {   // MENUITEM_ELECTRO
        .mode = MODE_ELECTRO,
        .title = EffectString,
        .string = ModeString[MODE_ELECTRO],
        .leftIndex = MENUITEM_COMPRESS,
        .rightIndex = MENUITEM_CLEAR,
        .upIndex = EMPTY,
        .downIndex = MENUITEM_ELECTRO_LEVEL 
    },
    {   // MENUITEM_DISTORTION_LEVEL
        .mode = MODE_DISTORTION,
        .title = ModeString[MODE_DISTORTION],
        .string = DistParamShort[DIST_PARAM_LEVEL],
        .leftIndex = MENUITEM_DISTORTION_AMP,
        .rightIndex = MENUITEM_DISTORTION_AMP,
        .upIndex = MENUITEM_DISTORTION,
        .downIndex = MENUITEM_DISTORTION_LEVEL_VALUE
    },
    {   // MENUITEM_DISTORTION_AMP
        .mode = MODE_DISTORTION,
        .title = ModeString[MODE_DISTORTION],
        .string = DistParamShort[DIST_PARAM_AMP],
        .leftIndex = MENUITEM_DISTORTION_LEVEL,
        .rightIndex = MENUITEM_DISTORTION_LEVEL,
        .upIndex = MENUITEM_DISTORTION,
        .downIndex = MENUITEM_DISTORTION_AMP_VALUE
    },
    {   // MENUITEM_DISTORTION_LEVEL_VALUE
        .mode = MODE_DISTORTION,
        .title = DistParam[DIST_PARAM_LEVEL],
        .string = displayedParamString,
        .leftIndex = EMPTY,
        .rightIndex = EMPTY,
        .upIndex = MENUITEM_DISTORTION_LEVEL,
        .downIndex = EMPTY // todo:
    },
    {   // MENUITEM_DISTORTION_AMP_VALUE
        .mode = MODE_DISTORTION,
        .title = DistParam[DIST_PARAM_AMP],
        .string = displayedParamString,
        .leftIndex = EMPTY,
        .rightIndex = EMPTY,
        .upIndex = MENUITEM_DISTORTION_AMP,
        .downIndex = EMPTY // todo:
    },
    {   // MENUITEM_OVERDRIVE_LEVEL
        .mode = MODE_OVERDRIVE,
        .title = ModeString[MODE_OVERDRIVE],
        .string = OverdParamShort[OVERD_PARAM_LEVEL],
        .leftIndex = MENUITEM_OVERDRIVE_AMP,
        .rightIndex = MENUITEM_OVERDRIVE_AMP,
        .upIndex = MENUITEM_OVERDRIVE,
        .downIndex = MENUITEM_OVERDRIVE_LEVEL_VALUE
    },
    {   // MENUITEM_OVERDRIVE_AMP
        .mode = MODE_OVERDRIVE,
        .title = ModeString[MODE_OVERDRIVE],
        .string = OverdParamShort[OVERD_PARAM_AMP],
        .leftIndex = MENUITEM_OVERDRIVE_LEVEL,
        .rightIndex = MENUITEM_OVERDRIVE_LEVEL,
        .upIndex = MENUITEM_OVERDRIVE,
        .downIndex = MENUITEM_OVERDRIVE_AMP_VALUE
    },
    {   // MENUITEM_OVERDRIVE_LEVEL_VALUE
        .mode = MODE_OVERDRIVE,
        .title = OverdParam[OVERD_PARAM_LEVEL],
        .string = displayedParamString,
        .leftIndex = EMPTY,
        .rightIndex = EMPTY,
        .upIndex = MENUITEM_OVERDRIVE_LEVEL,
        .downIndex = EMPTY
    },
    {   // MENUITEM_OVERDRIVE_AMP_VALUE
        .mode = MODE_OVERDRIVE,
        .title = OverdParam[OVERD_PARAM_AMP],
        .string = displayedParamString,
        .leftIndex = EMPTY,
        .rightIndex = EMPTY,
        .upIndex = MENUITEM_OVERDRIVE_AMP,
        .downIndex = EMPTY
    },
    {   // MENUITEM_CHORUS_DELAY
        .mode = MODE_CHORUS,
        .title = ModeString[MODE_CHORUS],
        .string = ChorusStrShort,
        .leftIndex = EMPTY,
        .rightIndex = EMPTY,
        .upIndex = MENUITEM_CHORUS,
        .downIndex = MENUITEM_CHORUS_DELAY_VALUE
    },
    {   // MENUITEM_CHORUS_DELAY_VALUE
        .mode = MODE_CHORUS,
        .title = ChorusStr,
        .string = displayedParamString,
        .leftIndex = EMPTY,
        .rightIndex = EMPTY,
        .upIndex = MENUITEM_CHORUS_DELAY,
        .downIndex = EMPTY
    },
    {   // MENUITEM_COMPRESS_THRESHOLD
        .mode = MODE_COMPRESS,
        .title = ModeString[MODE_COMPRESS],
        .string = CompressThresholdStrShort,
        .leftIndex = MENUITEM_COMPRESS_NOISE_CUT,
        .rightIndex = MENUITEM_COMPRESS_KOEFF,
        .upIndex = MENUITEM_COMPRESS,
        .downIndex = MENUITEM_COMPRESS_THRESHOLD_VALUE
    },
    {   // MENUITEM_COMPRESS_KOEFF
        .mode = MODE_COMPRESS,
        .title = ModeString[MODE_COMPRESS],
        .string = CompressKoeffStrShort,
        .leftIndex = MENUITEM_COMPRESS_THRESHOLD,
        .rightIndex = MENUITEM_COMPRESS_NOISE_CUT,
        .upIndex = MENUITEM_COMPRESS,
        .downIndex = MENUITEM_COMPRESS_KOEFF_VALUE
    },
    {   //MENUITEM_COMPRESS_NOISE_CUT
        .mode = MODE_COMPRESS,
        .title = ModeString[MODE_COMPRESS],
        .string = CompressNoiseCutLevelStrShort,
        .leftIndex = MENUITEM_COMPRESS_KOEFF,
        .rightIndex = MENUITEM_COMPRESS_THRESHOLD,
        .upIndex = MENUITEM_COMPRESS,
        .downIndex = MENUITEM_COMPRESS_NOISE_CUT_VALUE
    },
    {   // MENUITEM_COMPRESS_THRESHOLD_VALUE
        .mode = MODE_COMPRESS,
        .title = CompressThresholdStr,
        .string = displayedParamString,
        .leftIndex = EMPTY,
        .rightIndex = EMPTY,
        .upIndex = MENUITEM_COMPRESS_THRESHOLD,
        .downIndex = EMPTY
    },
    {   // MENUITEM_COMPRESS_KOEFF_VALUE
        .mode = MODE_COMPRESS,
        .title = CompressKoeffStr,
        .string = displayedParamString,
        .leftIndex = EMPTY,
        .rightIndex = EMPTY,
        .upIndex = MENUITEM_COMPRESS_KOEFF,
        .downIndex = EMPTY
    },
    {   // MENUITEM_COMPRESS_NOISE_CUT
        .mode = MODE_COMPRESS,
        .title = CompressNoiseCutLevelStr,
        .string = displayedParamString,
        .leftIndex = EMPTY,
        .rightIndex = EMPTY,
        .upIndex = MENUITEM_COMPRESS_NOISE_CUT,
        .downIndex = EMPTY
    },
    {   // MENUITEM_ELECTRO_LEVEL
        .mode = MODE_ELECTRO,
        .title = ModeString[MODE_ELECTRO],
        .string = LevelStr,
        .leftIndex = EMPTY,
        .rightIndex = EMPTY,
        .upIndex = MENUITEM_ELECTRO,
        .downIndex = MENUITEM_ELECTRO_LEVEL_VALUE
    },
    {   // MENUITEM_ELECTRO_LEVEL_VALUE
        .mode = MODE_ELECTRO,
        .title = ElectroLevelStr,
        .string = displayedParamString,
        .leftIndex = EMPTY,
        .rightIndex = EMPTY,
        .upIndex = MENUITEM_ELECTRO_LEVEL,
        .downIndex = EMPTY
    }
};


byte ModeIndex = MODE_CLEAR; //MODE_OFF; // перменная выбора режима работы
// -------------- кнопка -------------------------------------------------
#define BTN_CHK_LOOPS 100 // Колличество повторений между проверками кнопки
byte LoopCount = 0; // Перменная Счетчика циклов пропуска

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

typedef struct ParamSt {
    int16_t value;
    char string[15];
} Param_t;

#define LEVELS_COUNT 19
const Param_t Levels[LEVELS_COUNT] PROGMEM = {
    {LEVEL_5,   "5%"}, {LEVEL_10, "10%"}, {LEVEL_15, "15%"}, {LEVEL_20, "20%"}, 
    {LEVEL_25, "25%"}, {LEVEL_30, "30%"}, {LEVEL_35, "35%"}, {LEVEL_40, "40%"},
    {LEVEL_45, "45%"}, {LEVEL_50, "50%"}, {LEVEL_55, "55%"}, {LEVEL_60, "60%"}, 
    {LEVEL_65, "65%"}, {LEVEL_70, "70%"}, {LEVEL_75, "75%"}, {LEVEL_80, "80%"},
    {LEVEL_85, "85%"}, {LEVEL_90, "90%"}, {LEVEL_95, "95%"}
};

// ---------------------------------------------------------------------------------
// Distorsion
// ---------------------------------------------------------------------------------

#define DIST_AMPS_COUNT 19
const Param_t DistAmps[DIST_AMPS_COUNT] PROGMEM = {
    { 8,  " 0"}, { 9,  "+1"}, {10,  "+2"}, {11,  "+3"}, 
    {12,  "+4"}, {13,  "+5"}, {14,  "+6"}, {15,  "+7"}, 
    {16,  "+8"}, {17,  "+9"}, {18, "+10"}, {19, "+11"}, 
    {20, "+12"}, {21, "+13"}, {22, "+14"}, {23, "+15"}, 
    {24, "+16"}, {25, "+17"}, {26, "+18"},
};
byte DistAmpIndex = 8; // Коэффициент усиления Дисторшн, 8 - без усиления
int16_t DistLevelIndex = 14; //75%

// ---------------------------------------------------------------------------------
// Overdrive
// ---------------------------------------------------------------------------------

#define OVER_AMP_COUNT 19
const Param_t OverAmps[OVER_AMP_COUNT] PROGMEM = {
    { 8,  "-8"}, { 9,  "-7"}, {10,  "-6"}, {11,  "-5"},
    {12,  "-4"}, {13,  "-3"}, {14,  "-2"}, {15,  "-1"},
    {16,   "0"}, {17,  "+1"}, {18,  "+2"}, {19,  "+3"},
    {20,  "+4"}, {21,  "+5"}, {22,  "+6"}, {24,  "+7"},
    {24,  "+8"}, {25,  "+9"}, {26, "+10"}
};

byte OverAmpIndex = 0; // Коэффициент усиления эффекта Овердрайв, 16 - без эффекта
int16_t OverLevelIndex = 14; //75;

// ---------------------------------------------------------------------------------
// Chorus эффект
// ---------------------------------------------------------------------------------

#define CHORUS_DEPTH        256
#define CHORUS_DELAYS_COUNT 11

const Param_t ChorusDelay[CHORUS_DELAYS_COUNT] PROGMEM = {
    {  256*0/100, "0% (disabled)"}, 
    { 256*10/100, "10% (25samp)"},  { 256*20/100, "20% (51samp)"},  { 256*30/100, "30% (76samp)"},  { 256*40/100, "40% (102samp)"}, { 256*50/100, "50% (127samp)"},
    { 256*60/100, "60% (152samp)"}, { 256*70/100, "70% (178samp)"}, { 256*80/100, "80% (203samp)"}, { 256*90/100, "90% (229samp)"}, { 256*100/100, "100% (256samp)"},
};

int Chorus[CHORUS_DEPTH]; 
int16_t ChorusDelayIndex = 0; // перменная массива 0 - 256

// ---------------------------------------------------------------------------------
// Compressor эффект
// ---------------------------------------------------------------------------------

#define COMPRESS_KOEFF_COUNT    21
const Param_t CompressKoeff[COMPRESS_KOEFF_COUNT] PROGMEM = {
    { 10, "10" }, { 11, "11" }, { 12, "12" }, { 13, "13" }, { 14, "14" },
    { 15, "15" }, { 16, "16" }, { 17, "17" }, { 18, "18" }, { 19, "19" },
    { 20, "20" }, { 21, "21" }, { 22, "22" }, { 23, "23" }, { 24, "24" },
    { 25, "25" }, { 26, "26" }, { 27, "27" }, { 28, "28" }, { 29, "29" },
    { 30, "30" },
};
#define COMPRESS_NOISE_CUT_LEVEL_COUNT    5
const Param_t CompressNoiseCutLevel[COMPRESS_NOISE_CUT_LEVEL_COUNT] PROGMEM = {
    { 0,   "none"   }, 
    { 128, "low"    }, 
    { 256, "middle" }, 
    { 512, "high"   }, 
    { 768, "extra"  }
};

int CompressKoeffIndex = 30; 
int16_t CompressNoiseCutLevelIndex = 3;
int16_t CompressThresholdIndex = 14;

// ---------------------------------------------------------------------------------
// Electro
// ---------------------------------------------------------------------------------

int ElectroLevelIndex = 15; 
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
    lcd.print("Guitar Pedal v.1");
    lcd.setCursor(0,1);
    lcd.print("Starting...");
    delay(2000);

    sei(); // включить прерывания - на самом деле с arduino необязательно
    
    lcd.clear();
}

//-----------------------------------------------------------------------------

void loop() {
    LoopCount = LoopCount + 1;
    if (LoopCount == BTN_CHK_LOOPS) {
        if (digitalRead(FOOT_PIN)==HIGH) {
            int8_t step = 0;
            bool ProtectionDelay = false;
            //step = enc.read();
            if( (digitalRead(LEFT_PIN)) == LOW ) {
                switch( currentMenuItem ) {
                    case MENUITEM_DISTORTION_LEVEL_VALUE:   if( DistLevelIndex > 0 )            DistLevelIndex--;           break;
                    case MENUITEM_DISTORTION_AMP_VALUE:     if( DistAmpIndex > 0 )              DistAmpIndex--;             break;
                    case MENUITEM_OVERDRIVE_LEVEL_VALUE:    if( OverLevelIndex > 0 )            OverLevelIndex--;           break;
                    case MENUITEM_OVERDRIVE_AMP_VALUE:      if( OverAmpIndex > 0 )              OverAmpIndex--;             break;
                    case MENUITEM_CHORUS_DELAY_VALUE:       if( ChorusDelayIndex > 0 )          ChorusDelayIndex--;         break;
                    case MENUITEM_COMPRESS_KOEFF_VALUE:     if( CompressKoeffIndex > 0 )        CompressKoeffIndex--;       break;
                    case MENUITEM_COMPRESS_THRESHOLD_VALUE: if( CompressThresholdIndex > 0 )    CompressThresholdIndex--;   break;
                    case MENUITEM_COMPRESS_NOISE_CUT_VALUE: if( CompressNoiseCutLevelIndex > 0 )CompressNoiseCutLevelIndex--;break;
                    case MENUITEM_ELECTRO_LEVEL_VALUE:      if( ElectroLevelIndex > 0 )         ElectroLevelIndex--;        break;
                }
                if (menuItem[currentMenuItem].leftIndex != EMPTY) {
                    currentMenuItem = menuItem[currentMenuItem].leftIndex;
                    if (menuItem[currentMenuItem].upIndex != EMPTY) {
                        menuItem[menuItem[currentMenuItem].upIndex].downIndex = currentMenuItem;
                    }
                }
                ProtectionDelay = true;
            }
            else if( (digitalRead(RIGHT_PIN)) == LOW ) {
                switch( currentMenuItem ) {
                    case MENUITEM_DISTORTION_LEVEL_VALUE:   if( DistLevelIndex < (LEVELS_COUNT-1) )             DistLevelIndex++;           break;
                    case MENUITEM_DISTORTION_AMP_VALUE:     if( DistAmpIndex < (DIST_AMPS_COUNT-1) )            DistAmpIndex++;             break;
                    case MENUITEM_OVERDRIVE_LEVEL_VALUE:    if( OverLevelIndex < (LEVELS_COUNT-1) )             OverLevelIndex++;           break;
                    case MENUITEM_OVERDRIVE_AMP_VALUE:      if( OverAmpIndex < (OVER_AMP_COUNT-1) )             OverAmpIndex++;             break;
                    case MENUITEM_CHORUS_DELAY_VALUE:       if( ChorusDelayIndex < (CHORUS_DELAYS_COUNT-1) )    ChorusDelayIndex++;         break;
                    case MENUITEM_COMPRESS_KOEFF_VALUE:     if( CompressKoeffIndex < (COMPRESS_KOEFF_COUNT-1) ) CompressKoeffIndex++;       break;
                    case MENUITEM_COMPRESS_THRESHOLD_VALUE: if( CompressThresholdIndex < (LEVELS_COUNT-1) )     CompressThresholdIndex++;   break;
                    case MENUITEM_COMPRESS_NOISE_CUT_VALUE: if( CompressNoiseCutLevelIndex < (COMPRESS_NOISE_CUT_LEVEL_COUNT-1) ) CompressNoiseCutLevelIndex++; break;
                    case MENUITEM_ELECTRO_LEVEL_VALUE:      if( ElectroLevelIndex < (LEVELS_COUNT-1) )          ElectroLevelIndex++;        break;
                }
                if (menuItem[currentMenuItem].rightIndex != EMPTY) {
                    currentMenuItem = menuItem[currentMenuItem].rightIndex;
                    if (menuItem[currentMenuItem].upIndex != EMPTY) {
                        menuItem[menuItem[currentMenuItem].upIndex].downIndex = currentMenuItem;
                    }
                }
                ProtectionDelay = true;
            }
            else if ((digitalRead(EXIT_PIN)) == LOW && menuItem[currentMenuItem].upIndex != EMPTY ) {
                currentMenuItem = menuItem[currentMenuItem].upIndex;
                ProtectionDelay = true;
            }
            else if ((digitalRead(ENTER_PIN)) == LOW && menuItem[currentMenuItem].downIndex != EMPTY ) {
                currentMenuItem = menuItem[currentMenuItem].downIndex;
                ProtectionDelay = true;
            }

            switch( currentMenuItem ) {
                case MENUITEM_DISTORTION_LEVEL_VALUE:   strncpy_P(displayedParamString, Levels[DistLevelIndex].string, 14);             break;
                case MENUITEM_DISTORTION_AMP_VALUE:     strncpy_P(displayedParamString, DistAmps[DistAmpIndex].string, 14);             break;
                case MENUITEM_OVERDRIVE_LEVEL_VALUE:    strncpy_P(displayedParamString, Levels[OverLevelIndex].string, 14);             break;
                case MENUITEM_OVERDRIVE_AMP_VALUE:      strncpy_P(displayedParamString, OverAmps[OverAmpIndex].string, 14);             break;
                case MENUITEM_CHORUS_DELAY_VALUE:       strncpy_P(displayedParamString, ChorusDelay[ChorusDelayIndex].string, 14);      break;
                case MENUITEM_COMPRESS_KOEFF_VALUE:     strncpy_P(displayedParamString, CompressKoeff[CompressKoeffIndex].string, 14);  break;
                case MENUITEM_COMPRESS_THRESHOLD_VALUE: strncpy_P(displayedParamString, Levels[CompressThresholdIndex].string, 14);     break;
                case MENUITEM_COMPRESS_NOISE_CUT_VALUE: strncpy_P(displayedParamString, CompressNoiseCutLevel[CompressNoiseCutLevelIndex].string, 14); break;
                case MENUITEM_ELECTRO_LEVEL_VALUE:      strncpy_P(displayedParamString, Levels[ElectroLevelIndex].string, 14);          break;
            }


            char tmp[17];
            lcd.setCursor(0,0);
            strcpy_P(tmp, menuItem[currentMenuItem].title);
            lcd.print( tmp );
            lcd.print("                ");
            lcd.setCursor(0,1);
            if( menuItem[currentMenuItem].leftIndex != -1 ) lcd.print("<"); else lcd.print(" ");
            uint16_t len = 0;
            if( menuItem[currentMenuItem].string == displayedParamString ) {
                len = strlen(menuItem[currentMenuItem].string);
            } else {
                len = strlen_P(menuItem[currentMenuItem].string);
            }
            byte spaces1 = (14-len)/2;
            byte spaces2 = (15-len)/2;
            while(spaces1--) lcd.print(" ");
            if( menuItem[currentMenuItem].string == displayedParamString ) {
                strncpy(tmp, menuItem[currentMenuItem].string, 16);
            } else {
                strncpy_P(tmp, menuItem[currentMenuItem].string, 16);
            }
            lcd.print(tmp);
            while(spaces2--) lcd.print(" ");
            if( menuItem[currentMenuItem].rightIndex != -1 ) lcd.print(">"); else lcd.print(" ");
            
            if (ProtectionDelay) {
                delay (250);
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
                horus_out_index = horus_write_index - ChorusDelay[ChorusDelayIndex].value;
                if (Signal < Chorus[horus_out_index]) {
                    Signal = Chorus[horus_out_index];
                }
                horus_write_index++;
            }
            break;

        //-----------------------------------------------------------------------------------------
        // ****** Эффект Compress ******
        case MODE_COMPRESS: 
            if (Signal > CompressNoiseCutLevel[CompressNoiseCutLevelIndex].value ) {
                Signal = Signal + (Levels[CompressThresholdIndex].value - Signal)/CompressKoeff[CompressKoeffIndex].value;
            }
            if (Signal < -CompressNoiseCutLevel[CompressNoiseCutLevelIndex].value ) {
                Signal = Signal - (Levels[CompressThresholdIndex].value + Signal)/CompressKoeff[CompressKoeffIndex].value;
            }
            break;

        //-----------------------------------------------------------------------------------------
        // ****** Эффект Electro ******
        case MODE_ELECTRO: 
            if (ElectroCounter > Levels[ElectroLevelIndex].value) {
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