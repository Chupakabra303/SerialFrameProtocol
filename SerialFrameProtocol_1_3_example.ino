//#define LRC // LRC для SerialFrameProtocol вместо CRC16

#include "SerialFrameProtocol_1_3.h"
#define sfpPort Serial
#define LOG_PREFIX '~' // префикс для логов

SerialFrameProtocol sfp(sfpPort);

constexpr uint32_t DATA_SIZE = 300;
int32_t iVal = 54321;
float fVal = 1.23456789;
float fCurrent_A1_Iobr = 3.321;

/*
void setup() {
  Serial1.begin(115200);
  //DEBUG_PORT.begin(115200);
  while (!Serial1 && millis() < 1000);
  //Serial.println("Serial OK");
  //sfp.print("sfp OK");
  //Serial.printf("%.2f", 123.456);
  uint8_t arr1[3] = {'A', 'B', 'C'};
  
  sfp.frameStart('0');
  //sfp.printf("%s", "\\");
  //sfp.write('{', arr1, 2, '}');
  sfp.write('7');
  sfp.write('\n');
  sfp.write("TEST");
  sfp.write(&arr1[0], 3);
  sfp.writeT('{', "abc", '}');
  sfp.writeT("QWE");
  sfp.writeT('7');
  sfp.logWrite("LOG", "LOG2");
  sfp.frameEnd();
}*/

void setup() {
  sfpPort.begin(57600);
  delay(2000);
  while (!sfpPort && millis() < 1000);
  sfp.framedWriteT(LOG_PREFIX, F("Serial инициализирован"));
}

void customFrame() {
  sfp.print(F("Это часть фрейма "));
  sfp.print("iVal=" + String(iVal) + "; fVal=" + String(fVal));
}

void loop() {
  //  sfp.frameStart("TEST"); // код команды передачи массива ВАХ
  //  sfp.frameEnd();

  // -------------------- Обработка принятого фрейма --------------------
  if (sfp.cmdReceiver() != 0) {
    // sfp.print(F("COMMAND: C"));
    // sfp.frameStart(LOG_PREFIX);
    //  sfp.print(F("COMMAND: C"));
    //  sfp.write(sfp.cmdQueue);
    //sfp.frameEnd();
    switch (sfp.cmdQueue) {
      case 'N': // обработка команды CN. Ответ в виде текстовой строки
        sfp.framedWriteT(LOG_PREFIX, F("Ответ в виде текстовой строки через конструкцию frameStart() ... frameEnd(). Это рекомендуемый способ заполнения фрейма"));
        sfp.frameStart(F("Это начало фрейма ")); // Начало последовательного заполнения фрейма
          sfp.write("iVal=");
          sfp.print(iVal);
          sfp.print("; ");
          sfp.print("fVal=" + String(fVal));
          sfp.print(" fVal=");
          sfp.print(fVal, 5); // 5 знаков после запятой
          sfp.print(" А это просто часть фрейма");
        sfp.frameEnd(); // Окончание заполнения фрейма
        break;
      case 'Y':
        sfp.framedWriteT(LOG_PREFIX, F("Ответ в виде текстовой строки через обратный вызов с лямбда-функцией"));
        sfp.framedCallbackWrite([]() { // обратный вызов с лямбда-функцией в качестве аргумента для заполнения фрейма
          sfp.print("Это часть фрейма ");
          sfp.print("iVal=" + String(iVal) + "; fVal=" + String(fVal, 5) + "; ");
          sfp.writeT("iVal=", String(iVal).c_str(), "; fVal=", String(fVal, 5).c_str(), "; ");
          sfp.print("Это тоже часть фрейма");
        } );
        break;
      case 'Z':
        sfp.framedWriteT(LOG_PREFIX, "Ответ в виде текстовой строки через обратный вызов customFrame()");
        sfp.framedCallbackWrite(customFrame); // обратный вызов функции customFrame() в качестве аргумента для заполнения фрейма
        break;
      case 'L':
        sfp.framedWriteT(LOG_PREFIX, "Просто строка с префиксом LOG_PREFIX=", LOG_PREFIX);
        sfp.frameStart(LOG_PREFIX); // Еще просто пошлем в log пришедшую команду "CL"
          sfp.write(sfp.frameBuffer, 2);
        sfp.frameEnd();
        
        sfp.framedWriteT(LOG_PREFIX, *(uint16_t *)&sfp.frameBuffer[0]); // Еще просто пошлем в log пришедшую команду "CL"
        sfp.framedWriteT(LOG_PREFIX, "Кортеж параметров", '[', "строка", ']', String(123).c_str(), String(125).c_str());

        break;
      case 'B': // обработка команды CB. Ответ блоком двоичных даных B0<iVal><fVal>
        sfp.framedWriteT(LOG_PREFIX, "Передача блока данных B0");
        sfp.framedWriteT('B', '0', iVal, fVal); // или sfp.frameStart("B0") // Начало последовательного заполнения фрейма
        sfp.frameStart('B', '0', iVal, fVal); // или sfp.frameStart("B0") // Начало последовательного заполнения фрейма
          // sfp.write('0'); // блок данных B0;
          // sfp.write(iVal);
          // sfp.write(fVal);
        sfp.frameEnd(); // Окончание заполнения фрейма
        break;
      case 'A':
        sfp.framedWriteT(LOG_PREFIX, "Следом будет отправлен массив данных int32_t с прификсом A0");
        int32_t Data1[DATA_SIZE];
        for (uint32_t i = 0; i < DATA_SIZE; ++i)
          Data1[i] = i % 255;
        sfp.frameStart('A', '0');
          sfp.writeT((uint32_t)DATA_SIZE); // сначала отсылаем размер массива
          sfp.write(Data1, DATA_SIZE * 4); // теперь сам массив data1 (int32_t) побайтно x4
        sfp.frameEnd();

        sfp.framedWriteT(LOG_PREFIX, "Почему бы в следующем фрейме не отправить еще один массив данных uint16_t прификсом A1");

        uint16_t Data2[DATA_SIZE];
        for (uint32_t i = 0; i < DATA_SIZE; ++i)  
          Data2[i] = i % 256;

        sfp.frameStart('A');
          sfp.writeT('1', (uint32_t)DATA_SIZE);
          // sfp.write('1'); // ASCII !
          // sfp.write(uint32_t(DATA_SIZE)); // сначала отсылаем размер массива
          sfp.write(Data2, DATA_SIZE * 2); // теперь сам массив, из-за uint16_t кол-во байт x2
        sfp.frameEnd();
        break;
      case 'X':
        sfp.framedWriteT(LOG_PREFIX, "Посылаем принятый по команде CX массив обратно c префиксом AX");
        sfp.frameStart("AX");
          sfp.write(&sfp.frameBuffer[2], sfp.cmdLength - 2); // вычитаем из длины двубайтную команду
        sfp.frameEnd();
        break;
      default:
        //float fVoltageCapacitor = 1.23456789;
        sfp.framedWriteT(LOG_PREFIX, "000");
        break;
    }
    sfp.cmdClear();
  }
}
