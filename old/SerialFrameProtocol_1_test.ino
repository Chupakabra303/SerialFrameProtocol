#include "SerialFrameProtocol_1.h"
#define SFPport Serial

SerialFrameProtocol SFP(SFPport);

void setup() {
	SFPport.begin(57600);
	delay(2000);
	while (!SFPport && millis() < 1000);
	SFP.logPrint("Serial инициализирован");
	//for (int i = 0; i < FRAME_BUFF_SIZE; ++i)
	//	SFPport.print(SFP.frameBuffer[i], HEX);
}

constexpr uint32_t DATA_SIZE = 300;
int32_t iVal = 54321;
float fVal = 1.23456789;
float fCurrent_A1_Iobr = 3.321;

void customFrame() {
	SFP.print("Это часть фрейма ");
	SFP.print("iVal=" + String(iVal) + "; fVal=" + String(fVal));
}

void loop() {
//	SFP.frameStart("TEST"); // код команды передачи массива ВАХ
//	SFP.frameEnd();

// -------------------- Обработка принятого фрейма --------------------
if (SFP.cmdReceiver() != 0) {
	// SFP.print(F("COMMAND: C"));
	// SFP.frameStart(LOG_CMD);
	//	SFP.print(F("COMMAND: C"));
	//	SFP.write(SFP.cmdQueue);
	//SFP.frameEnd();
	switch (SFP.cmdQueue) {
		case 'N': // обработка команды CN. Ответ в виде текстовой строки
			SFP.logPrint("Ответ в виде текстовой строки через конструкцию frameStart() ... frameEnd(). Это рекомендуемый способ заполнения фрейма");
			SFP.frameStart("Это начало фрейма "); // Начало последовательного заполнения фрейма
				SFP.print("iVal=");
				SFP.print(iVal);
				SFP.print("; ");
				SFP.print("fVal=" + String(fVal));
				SFP.print(" А это просто часть фрейма");
			SFP.frameEnd(); // Окончание заполнения фрейма
			break;
		case 'Y':
			SFP.logPrint("Ответ в виде текстовой строки через обратный вызов с лямбда-функцией ");
			SFP.framePrintCallback([]() { // обратный вызов с лямбда-функцией в качестве аргумента для заполнения фрейма
				SFP.print("Это часть фрейма ");
				SFP.print("iVal=" + String(iVal) + "; fVal=" + String(fVal));
				SFP.print(" Это тоже часть фрейма");
			} );
			break;
		case 'Z':
			SFP.logPrint("Ответ в виде текстовой строки через обратный вызов customFrame()");
			SFP.framePrintCallback(customFrame); // обратный вызов функции customFrame() в качестве аргумента для заполнения фрейма
			break;
		case 'L':
			SFP.logPrint("Просто строка с префиксом LOG_CMD=" + String(LOG_CMD));
			SFP.logWrite(SFP.frameBuffer, 2); // Еще просто пошлем в log пришедшую команду "CL"
			break;
		case 'B': // обработка команды CB. Ответ блоком двоичных даных B0<iVal><fVal>
			SFP.logPrint("Передача блока данных B0");
			SFP.frameStart('B'); // или SFP.frameStart("B0") // Начало последовательного заполнения фрейма
				SFP.write('0'); // блок данных B0;
				SFP.write(iVal);
				SFP.write(fVal);
			SFP.frameEnd(); // Окончание заполнения фрейма
			break;
		case 'A':
			SFP.logPrint("Следом будет отправлен массив данных int32_t с прификсом A0");
			int32_t Data1[DATA_SIZE];
			for (uint32_t i = 0; i < DATA_SIZE; ++i)
				Data1[i] = i % 255;
			SFP.frameStart("A0");
				SFP.write((uint32_t)DATA_SIZE); // сначала отсылаем размер массива
				SFP.write(Data1, DATA_SIZE * 4); // теперь сам массив data1 (int32_t) побайтно x4
			SFP.frameEnd();

			SFP.logPrint("Почему бы в следующем фрейме не отправить еще один массив данных uint16_t прификсом A1");

			uint16_t Data2[DATA_SIZE];
			for (uint32_t i = 0; i < DATA_SIZE; ++i)  
				Data2[i] = i % 256;

			SFP.frameStart('A');
				SFP.write('1'); // ASCII !
				SFP.write(uint32_t(DATA_SIZE)); // сначала отсылаем размер массива
				SFP.write(Data2, DATA_SIZE * 2); // теперь сам массив, из-за uint16_t кол-во байт x2
			SFP.frameEnd();
			break;
		case 'X':
			SFP.logPrint("Посылаем принятый по команде CX массив обратно c префиксом AX");
			SFP.frameStart("AX");
				SFP.write(&SFP.frameBuffer[2], SFP.cmdLength - (CHECK_LEN + 2)); // вычитаем из длины котрольную сумму и двубайтную команду
			SFP.frameEnd();
			break;
		default:
			//float fVoltageCapacitor = 1.23456789;
			//SFP.logPrint(F("fVoltage_A1_R9_3=") + String(fVoltageCapacitor, 2));
			SFP.logPrint("000");
			break;
	}
	SFP.cmdClear();
}
}
