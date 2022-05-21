// Based on OpenMusicLabs previous works.
// Цифровой усилитель: нажатие кнопки 1 или кнопки 2 увеличивает или уменьшает громкость.

// #include <iarduino_encoder_tmr.h> // Подключаем библиотеку iarduino_Encoder_tmr для работы с энкодерами через аппаратный таймер
// #include "U8glib.h"

// iarduino_Encoder_tmr enc(ENC_CLK_PIN, ENC_DT_PIN);  // Енкодер
// U8GLIB_SH1106_128X64 u8g(U8G_I2C_OPT_NO_ACK);       // Display which does not send ACK

// ------------- Pins --------------------------------------------------------------
#define FOOT_PIN        12 // Нажатие "Ножной" кнопки ВКЛ/ВЫКЛ устройсва
#define MODE_PIN        5  // D5 мастер кнопка переключатель режимов
#define ENC_CLK_PIN     2  // CLK Провод Енкодера
#define ENC_DT_PIN      3  // DT провод Енкодера
// ------------- ШИМ ---------------------------------------------------------------
#define PWM_FREQ        0x00FF // установка частоты ШИМ-а (режим Фазовый - 2х8 Бит 31.3 кГц)
#define PWM_MODE        0 // Устанавливает режим коррекции фазы (0) или быстрый режим (1)
#define PWM_QTY         2 // Используем 2 ШИМ-а параллельно 2х8 Бит
// ------------- РЕЖИМ -------------------------------------------------------------
enum _mode_en {
    MODE_OFF = 0, // clear
    MODE_DISTORTION,
    MODE_OVERDRIVE,
    MODE_HORUS,
    MODE_COMPRESS,
    MODE_ELECTRO
};
byte Mode = MODE_ELECTRO; //MODE_OFF; // перменная выбора режима работы
// ------------- АЦП - ЦАП ---------------------------------------------------------
volatile int Signal = 0; // Задаем значение громкости
// -------------- Энкодер + кнопка -------------------------------------------------
#define BTN_CHK_LOOPS 100 // Колличество повторений между проверками кнопки
byte LoopCount = 0; // Перменная Счетчика циклов пропуска
// ---------------------------------------------------------------------------------
byte DistortK = 8; // Коэффициент для Дистошн
// ---------------------------------------------------------------------------------
byte OverK = 8; // Коэффициент для Овердрайва
// ---------------------------------------------------------------------------------
int Horus[256]; // Horus эффект
byte HorusX = 0;
byte HorusY = 0; // перменная массива 0 - 256
byte HorusZ = 0;
// ---------------------------------------------------------------------------------
int ComprT = 30; // Compressor эффект
// ---------------------------------------------------------------------------------
int ElectroLevel = 15; // Electro
int ElectroCounter = 0;
int ElectroOut = 0;
// ---------------------------------------------------------------------------------

void setup(){
    Serial.begin(115200);
    Serial.println("Starting...");
    // enc.begin(); // Инициируем работу с энкодером

    pinMode(FOOT_PIN, INPUT_PULLUP);
    pinMode(MODE_PIN, INPUT_PULLUP);
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
    sei(); // включить прерывания - на самом деле с arduino необязательно
}

bool mode_printed = false;
void loop() {
    LoopCount = LoopCount + 1;
    if (LoopCount == BTN_CHK_LOOPS) {
        if (digitalRead(FOOT_PIN)==HIGH) {
            int8_t step = 0;
            //step = enc.read();
            switch(Mode){
            case MODE_OFF:
                if (!mode_printed) { Serial.println("Mode: Clear"); mode_printed = true;}
                // u8g.firstPage();
                // do {
                //     u8g.setFont(u8g_font_helvR18r);
                //     u8g.drawStr( 3, 20, "KL BoosT");
                //     u8g.drawStr( 18, 50, "CLEAN");
                //     if (Signal < -25000) {
                //         u8g.drawBox(0,56,8,8);
                //     }
                //     if (Signal > 25000) {
                //         u8g.drawBox(112,56,8,8);
                //     }
                // // u8g.drawBox(0, 60, (Signal/256),4);
                // } while (u8g.nextPage());
                break;
            case MODE_DISTORTION:
                if (!mode_printed) { Serial.println("Mode: Distortion"); mode_printed = true;}
                DistortK = DistortK + step; // Если энкодер зафиксирует поворот, то значение переменной i изменится:
                if ( DistortK <10 ) { DistortK = 10; }
                if ( DistortK >26 ) { DistortK = 26; }
                // u8g.firstPage();
                // do {
                //     u8g.setFont(u8g_font_helvR18r);
                //     u8g.drawStr( 3, 20, "KL BoosT");
                //     u8g.drawStr( 31, 50, "FUZZ");
                //     u8g.drawBox(0,60,((DistortK-10)*9),4);
                // } while (u8g.nextPage());
                break;
            case MODE_OVERDRIVE:
                if (!mode_printed) { Serial.println("Mode: Overdrive"); mode_printed = true;}
                OverK = OverK + step; // Если энкодер зафиксирует поворот, то значение переменной i изменится:
                if ( OverK < 0) { OverK = 0; }
                if ( OverK >26 ) { OverK = 26; }
                // u8g.firstPage();
                // do {
                //     u8g.setFont(u8g_font_helvR18r);
                //     u8g.drawStr(3, 20, "KL BoosT");
                //     u8g.drawStr(8, 50, "Overdrive");
                //     u8g.drawBox(0,60,(OverK*5),4);
                // } while (u8g.nextPage());
                break;
            case MODE_HORUS:
                if (!mode_printed) { Serial.println("Mode: Horus"); mode_printed = true;}
                HorusY = HorusY + step*10; // Если энкодер зафиксирует поворот, то значение переменной i изменится:
                // u8g.firstPage();
                // do {
                //     u8g.setFont(u8g_font_helvR18r);
                //     u8g.drawStr(3, 20, "KL BoosT");
                //     u8g.drawStr(24 , 50, "Chorus");
                //     u8g.drawBox(0,60,(HorusY/2),4);
                // } while (u8g.nextPage());
                break;
            case MODE_COMPRESS:
                if (!mode_printed) { Serial.println("Mode: Compress"); mode_printed = true;}
                ComprT = ComprT - step; // Если энкодер зафиксирует поворот, то значение переменной i изменится:
                if ( ComprT <10 ) { ComprT = 10; }
                if ( ComprT >30 ) { ComprT = 30; }
                // u8g.firstPage();
                // do {
                //     u8g.setFont(u8g_font_helvR18r);
                //     u8g.drawStr(3, 20, "KL BoosT");
                //     u8g.drawStr( 4, 50, "Compress");
                //     u8g.drawBox(0, 60, ((30-ComprT)*6),4);
                // } while (u8g.nextPage());
                break;
            case MODE_ELECTRO:
                if (!mode_printed) { Serial.println("Mode: Electro"); mode_printed = true;}
                ElectroLevel = ElectroLevel + step; // Если энкодер зафиксирует поворот, то значение переменной i изменится:
                if ( ElectroLevel < 1 ) { ElectroLevel = 1; }
                if ( ElectroLevel > 64 ) { ElectroLevel = 64; }
                // u8g.firstPage();
                // do {
                //     u8g.setFont(u8g_font_helvR18r);
                //     u8g.drawStr(3, 20, "KL BoosT");
                //     u8g.drawStr( 15, 50, "Electro");
                //     u8g.drawBox(0, 60, (ElectroLevel*2), 4);
                // } while (u8g.nextPage());
                break;
            }
            if ((digitalRead(MODE_PIN)) == LOW ) {
                Mode = Mode + 1;
                delay (500);
                mode_printed = false;
            }
            if (Mode > 5 ) {
                Mode = MODE_OFF;
                delay (500);
            }
        //---------------------------------------------------------------------------------------------
        } else { // устройсво электрически не включено
        //     u8g.firstPage();
        //     do {
        //         u8g.setFont(u8g_font_helvR18r);
        //         u8g.drawStr( 2, 20, "KL BoosT");
        //         u8g.drawStr( 42, 64, "OFF");
        //     } while (u8g.nextPage());
        }
        LoopCount = 0;
    }
}

//************* Обработчик прерывания захвата ввода из АЦП 0 - Timer 1
ISR(TIMER1_CAPT_vect) // Команда обработки прерывания Timer 1
{
    unsigned int ADC_low, ADC_high; // Младший и Старший байты из АЦП
    ADC_low = ADCL; // читаем младший байт АЦП прочитан первым ( важно! )
    ADC_high = ADCH; // читаем старший байт АЦП прочитан вторым
    Signal = ((ADC_high << 8) | ADC_low) + 0x8000; // Собираем 16-битн. из 2-х байт полученных в АЦП
    //-----------------------------------------------------------------------------------------
    switch (Mode) {
    case MODE_DISTORTION: // ****** Distortion эффект ******
        if (Signal > 0) {
            if (Signal > 24576) {Signal = 24576;}
            Signal = (Signal/10)*DistortK; // Все что выше 24576 удваиваем и обрезаем по уровню (65535)
            if(Signal < 0) { Signal = 32767; }
        }
        if (Signal < 0) {
            if (Signal < -24576) {Signal = -24576;}
            Signal = (Signal/10)*DistortK; // Все что ниже -24576 удваиваем и обрезаем по уровню (0)
            if(Signal > 0) { Signal = -32767; }
        }
        break;
    //-----------------------------------------------------------------------------------------
    case MODE_OVERDRIVE: // ****** Overdrive Эффект ******
        if (Signal > 24576 || Signal < -24576) {
            if (Signal > 24576) {
                Signal = Signal - ((Signal - 24576)/8)*OverK; // Все что больше 57344 Уменьшаем на OverK
            }
            if (Signal < -24576) {
                Signal = Signal - ((Signal + 24576)/8)*OverK; // Все что меньше 8192 Уменьшаем на OverK
            }
        }
        break;
    //-----------------------------------------------------------------------------------------
    case MODE_HORUS: // ****** Horus эффект ******
        Horus[HorusX] = Signal; // Все время записываем 256 значений -> Horus[HorusX]
        HorusZ=HorusX-HorusY;
        if (Signal < Horus[HorusZ]) {
            Signal = Horus[HorusZ];
        }
        HorusX++;
        break;
    //-----------------------------------------------------------------------------------------
    case MODE_COMPRESS: // ****** Compress ******
        if (Signal > 512 ) {
            Signal = Signal + (24575 - Signal)/ComprT;
        }
        if (Signal < -512 ) {
            Signal = Signal - (24575 + Signal)/ComprT;
        }
        break;
    //-----------------------------------------------------------------------------------------
    case MODE_ELECTRO: // ****** Electro ******
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