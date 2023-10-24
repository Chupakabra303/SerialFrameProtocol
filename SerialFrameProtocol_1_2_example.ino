#include "SerialFrameProtocol_1_2.h"

#define sfpPort Serial

SerialFrameProtocol sfp(sfpPort);

void setup() {
  sfpPort.begin(57600);
  delay(2000);
  while (!sfpPort && millis() < 1000);
  sfp.logPrint("Serial инициализирован");
  //for (int i = 0; i < FRAME_BUFF_SIZE; ++i)
  //  sfpPort.print(sfp.frameBuffer[i], HEX);
}

constexpr uint32_t DATA_SIZE = 300;
int32_t iVal = 54321;
float fVal = 1.23456789;
float fCurrent_A1_Iobr = 3.321;

void customFrame() {
  sfp.print("Это часть фрейма ");
  sfp.print("iVal=" + String(iVal) + "; fVal=" + String(fVal));
}

void loop() {
  //  sfp.frameStart("TEST"); // код команды передачи массива ВАХ
  //  sfp.frameEnd();

  // -------------------- Обработка принятого фрейма --------------------
  if (sfp.cmdReceiver() != 0) {
    // sfp.print(F("COMMAND: C"));
    // sfp.frameStart(LOG_CMD);
    //  sfp.print(F("COMMAND: C"));
    //  sfp.write(sfp.cmdQueue);
    //sfp.frameEnd();
    switch (sfp.cmdQueue) {
      case 'N': // обработка команды CN. Ответ в виде текстовой строки
        sfp.logPrint("Ответ в виде текстовой строки через конструкцию frameStart() ... frameEnd(). Это рекомендуемый способ заполнения фрейма");
        sfp.frameStart("Это начало фрейма "); // Начало последовательного заполнения фрейма
          sfp.print("iVal=");
          sfp.print(iVal);
          sfp.print("; ");
          sfp.print("fVal=" + String(fVal));
          sfp.print(" А это просто часть фрейма");
        sfp.frameEnd(); // Окончание заполнения фрейма
        break;
      case 'Y':
        sfp.logPrint("Ответ в виде текстовой строки через обратный вызов с лямбда-функцией ");
        sfp.framePrintCallback([]() { // обратный вызов с лямбда-функцией в качестве аргумента для заполнения фрейма
          sfp.print("Это часть фрейма ");
          sfp.print("iVal=" + String(iVal) + "; fVal=" + String(fVal));
          sfp.print(" Это тоже часть фрейма");
        } );
        break;
      case 'Z':
        sfp.logPrint("Ответ в виде текстовой строки через обратный вызов customFrame()");
        sfp.framePrintCallback(customFrame); // обратный вызов функции customFrame() в качестве аргумента для заполнения фрейма
        break;
      case 'L':
        sfp.logPrint("Просто строка с префиксом LOG_CMD=" + String(LOG_CMD));
        sfp.logWrite(sfp.frameBuffer, 2); // Еще просто пошлем в log пришедшую команду "CL"
        break;
      case 'B': // обработка команды CB. Ответ блоком двоичных даных B0<iVal><fVal>
        sfp.logPrint("Передача блока данных B0");
        sfp.frameStart('B'); // или sfp.frameStart("B0") // Начало последовательного заполнения фрейма
          sfp.write('0'); // блок данных B0;
          sfp.write(iVal);
          sfp.write(fVal);
        sfp.frameEnd(); // Окончание заполнения фрейма
        break;
      case 'A':
        sfp.logPrint("Следом будет отправлен массив данных int32_t с прификсом A0");
        int32_t Data1[DATA_SIZE];
        for (uint32_t i = 0; i < DATA_SIZE; ++i)
          Data1[i] = i % 255;
        sfp.frameStart("A0");
          sfp.write((uint32_t)DATA_SIZE); // сначала отсылаем размер массива
          sfp.write(Data1, DATA_SIZE * 4); // теперь сам массив data1 (int32_t) побайтно x4
        sfp.frameEnd();

        sfp.logPrint("Почему бы в следующем фрейме не отправить еще один массив данных uint16_t прификсом A1");

        uint16_t Data2[DATA_SIZE];
        for (uint32_t i = 0; i < DATA_SIZE; ++i)  
          Data2[i] = i % 256;

        sfp.frameStart('A');
          sfp.write('1'); // ASCII !
          sfp.write(uint32_t(DATA_SIZE)); // сначала отсылаем размер массива
          sfp.write(Data2, DATA_SIZE * 2); // теперь сам массив, из-за uint16_t кол-во байт x2
        sfp.frameEnd();
        break;
      case 'X':
        sfp.logPrint("Посылаем принятый по команде CX массив обратно c префиксом AX");
        sfp.frameStart("AX");
          sfp.write(&sfp.frameBuffer[2], sfp.cmdLength - 2); // вычитаем из длины двубайтную команду
        sfp.frameEnd();
        break;
      default:
        //float fVoltageCapacitor = 1.23456789;
        //sfp.logPrint(F("fVoltage_A1_R9_3=") + String(fVoltageCapacitor, 2));
        sfp.logPrint("000");
        break;
    }
    sfp.cmdClear();
  }
}