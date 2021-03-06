# Serial Frame Protocol (SFP)
Simple Serial Frame Protocol for Arduino, Teensy, etc.

Пример мастера на Python 3.8.10
![изображение](https://user-images.githubusercontent.com/15260953/146767523-906494bf-eba5-4ef9-b970-81a20fc1eb45.png)

Пример мастера на LabVIEW 2015
![изображение](https://user-images.githubusercontent.com/15260953/146767226-7c7100ab-bd7a-4066-914d-3cbcb7c103fe.png)

Публикую на суд публики свой простой и эффективный коммуникационный протокол Serial Frame Protocol для связи через последовательный порт с платформами, работающими под библиотекой Arduino по типу точка-точка.
Тестировал его работу на Arduino Mega 2560 и Teensy 3.5.
Тут же можно найти пример для Arduino, Teensy и т.п. и пример мастера на Python 3.8.10 и LabVIEW 2015.

Я решил сделать свой собственный протокол после работы с Modbus и похожими протоколами. Основными ограничениеми Modbus для меня были максимальный размер пакета данных (125 16-битных регистров для Modbus RTU), некоторая переусложненность системы комманд, многие из которых редко используются, наличие концепции выделенной области памяти для регистров (которая занимает место в ОЗУ МК и не всегда нужна). Также в некоторых случаях трудности создает master-slave огранизация работы с Modbus. Также я хотел уйти от концепции отслеживания временных интервалов при передаче пакетов Modbus RTU, приблизившись к передаче с терминирующим символом как Modbus ASCII для эффективного разделения пакетов данных. В итоге это вылилось в разработку собственного протокола: Serial Frame Protocol (далее SFP).

**Основные концепции и характеристики**
- Нет принципиального ограничения на размер пересылаемых данных (есть регулируемый размер приемного буфера данных в библиотеке SFP, размер которого ограничивается объемом ОЗУ МК)
- Нет ограничений на время передачи данных, конец передачи определяется только по терминирующему символу
- Может вестись обмен двоичными и/или ASCII данными. Для этого используется быстрый и эффективный механизм вставки байтов (Byte stuffing) для упаковки и распаковки налету байтов данных, совпадающих с терминирующим символом
- Для проверки целостности данных используется контрольная сумма LRC или CRC16 

Описание состоит из двух частей. Первая часть основная, описывает собственно сам протокол SFP, низкоуровневую структуру обмена данными (Кадр или Frame) и механизм обмена кадрами. Протокол не включает структуры данных или какую-либо систему комманд пользователя, и оперирует только массивами байт!
Вторая часть дополнительно описывает простейшую реализацию пользовательских команд (надстройка над протоколом), не является частью основного протокола. Тут же описываются функции библиотеки SFP. Приводится возможная реализации пользовательской ситемы команд и структур данных.

**1. Serial Frame Protocol**

Данные SFP предаются последовательно в кадрах. Кадр оканчивается терминирующим символом `"\n"` (0x0A). С помощью терминирующиего символа каждый кадр эффективно отделяется от соседних в приемном буфере обмена устройсва. Кадр не имеет определенной длины, длина кадра определяется "нарезкой" терминирующими символами. Данные в кадре могут передаваться без ограничения по времени (посимвольно, например в терминальной программе при вводе с клавиатуры, кусками переменной длины). Кадр считается завершенным при приеме терминирующего символа. В библиотеке SFP выделяется буфер определенного размера (настраивается через #define) для приема кадра. При приеме чрезмерно длинного кадра может произойти контролируемое переполнение этого буфера, новые поступающие байты перестанут добавляться в конец, но библиотека продолжит принимать кадр до терминирующего символа. По окончанию приема кадра при "переполнении" будет выставлен флаг ошибки кадра.

Кадр имеет следующую структуру:

`<b1>...<bN><CRC16 hi byte><CRC16 lo byte><\n>` при использовании контрольной суммы CRC16, либо

`<b1>...<bN><LRC byte><\n>` при использовании контрольной суммы LRC.

`b1...bN` - байты данных
`CRC16/LRC` - байты контрольной суммы. Библиотека SFP позволяет выбрать тип контрольной суммы, по-умолчанию используется CRC16.

При передаче библиотека SFP "на лету" вычисляет контрольную сумму кадра, осуществляет упаковку (Byte stuffing) и передачу кадра. Для этого не требуется дополнительных буферов. При приеме библиотека осуществляет обратное приобразование: "на лету" распаковывает кадр, заносит его в приемный буфер кадра, вычисляет котрольную сумму, проверяет корректность принятого кадра по контрольной сумме и переполнению буфера кадра.

Как работает упаковка и распаковка (Byte stuffing):
Кадр и входящую в него контрольная сумма упаковывается и передается "на лету". При упакове (stuffing) байты данных транслируются один в один за исключением байта терминирующего символа `"\n"` (0x0A) и байта экранирующего символа `"\"` (0x5C). Если в исходном потоке данных встречаются эти символы, то в потоке, отправляемом в порт, они заменяются на пары:

`"\" (0x5C) 	--> "\" + "e" (0x5C, 0x65)`

`"\n" (0x0A) --> "\" + "n" (0x5C, 0x6E)`

Таким образом в отправляемом потоке присутствует только экранирующий символ `"\"` и следующий за ним байт, кодирующий исходный экранированный байт.
При распаковке выполняется обратное преобразование. Байты транслируются во входящий буфер кадра один в один из входящего потока, за исключением экранирующего символа `"\"`. При обнаружении которого с учетом следующего за ним байта-кода производится замена пары на распакованный байт:

`"\" + "n" (0x5C, 0x6E) --> "\n" (0x0A)`

`"\" + "e" (0x5C, 0x65) --> "\" (0x5C)`

В библиотеке SFP реализован расчет контрольной суммы "на лету" побайтно, поэтому не трубуется дополнительных массивов для промежуточного хранения/кодирования кадров, кроме входящего буфера кадра.
Такая организация передачи данных позволяет эффективно передавать байтовые массивы данных, начиная от 1 байта и более, при минимальном расходовании ОЗУ.
Размер распакованного кадра во входящем буфере кадра не зависит от кол-ва stuffing байтов в необработанном потоке последовательного порта, т.к все преобразования делаются "на лету" и stuffing байты не попадают в буфер кадра!

**2. Функции библиотеки SFP. Пример пользовательской надстройки протокола.**

В библиотеке присутствует основной класс SerialFrameProtocol, который реализует сам протокол SFP, а также пользовательскую надстройку (дополнительные функции класса SerialFrameProtocol) и вспомогательный класс для вычисления LRC или CRC16 (быстрое вычисление на базе таблицы).

Класс SerialFrameProtocol является наследником класса Print библиотеки Arduino и реализует (перегружает) все варианты функций write() и print() класса Print, а также имеет некоторые дополнительные варианты вызова write() и print().
При вызове этих функций класса происходит передача данных c упаковкой в кадр, как описано выше.

Начало передачи кадра в классе предваряется функцией SerialFrameProtocol::frameStart(). Функция инициализирует вычисление контрольной суммы, а также позволяет передать одиночный стартовый байт или последовательность (строку) кадра. Затем ведется передача данных в кадре при помощи функций SerialFrameProtocol::write() или SerialFrameProtocol::print().
Конец кадра обозначается вызовом функции SerialFrameProtocol::frameEnd(), которая дописывает в конец кадра контрольную сумму и терминирующий символ.
 
SerialFrameProtocol::frameWrite(), SerialFrameProtocol::frameWrite() - простейщие обертки над низкоуровневыми функциями кадра. Вызов формирует и отсылает целый кадр.

SerialFrameProtocol::framePrintCallback() - фунция для обратного вызова пользовательской функции формирования кадра. Пользовательская функция (customFrame(), например) формирует структуру кадра, framePrintCallback(customFrame()) - отсылает законченный кадр. Вместо customFrame() может быть использовано лямбда функция (см. пример). Я рекомендую использовать обрамление SerialFrameProtocol::frameStart() - SerialFrameProtocol::frameEnd() для заполнения кадра данными.

SerialFrameProtocol::frameReceiver() - основная низкоуровневая функция приема кадра из порта. функция считывает кусками или целиком за один вызов кадр из буфера порта (по наличию байт во входящем буфере порта), копирует его в буфер кадра frameBuffer[FRAME_BUFF_SIZE] и выставляет флаги frameCompleted и frameError по результам проверки текущего полученного кадра.
Длина данных в кадре пишется в переменную frameLength класса.

SerialFrameProtocol::frameClear() - сбрасывает флаги считанного кадра. До вызова этой функции, SerialFrameProtocol::frameReceiver() не будет принимать новый кадр (в случае завершенного приема предыдущего кадра, когда флаг frameCompleted==true). Вызов перед SerialFrameProtocol::frameReceiver() подготавливает SerialFrameProtocol::frameReceiver() для считывания нового кадра.

Пользовательская надстройка протокола. Дополнительные функции в классе SerialFrameProtocol
В моих проектах вторым узлом связи является ПК, точнее программа на LabVIEW. Для LabVIEW я создал функциональный аналог библиотеки SFP - LLB библиотеку, SFPFrameProtocol1.llb. Далее рассматривается связь через SFP между ПК (программа на LabVIEW) и МК.

В своих проектах я использовал следущию систему команд и структуры данных пользователя, которые инкапуслируются в кадр.
Программа ПК посылает в МК командные запросы, на которые отвечает (но не обязательно) программа МК.
Командный запрос состоит из 2х обязательный байт: 'C' - обязательный префикс команды и байта - самой команды. После байта команды может следовать опционально массив байт, см. пример.
Содержимое кадра команды:

`<'C'><байт команды><опциональные массив байт>...`

Для обработки таких команд в классе SerialFrameProtocol присутствует функция SerialFrameProtocol::cmdReceiver(), которая является надстройкой над SerialFrameProtocol::frameReceiver().
Функция SerialFrameProtocol::cmdReceiver() разбирает принятые кадры на "команды". При удовлетворении всех признаков наличия команды в принятом кадре, считаная команда копируется в одноэлементную очередь cmdQueue (по сути просто переменная типа uint8_t). Длина команды и опционального массива копируется в переменую cmdLength, указывающую на конец данных команды во входящем буфере кадра. Значение в cmdQueue "фиксируется" до тех пор, пока не будет вызвана фунция SerialFrameProtocol::cmdClear(), сбрасывающая (в 0) очередь команд cmdQueue. Тут важный момент командой может быть любое значения байта, кроме 0, который является признаком отсутствия команды в очереде команд. Итого работает это так: первая считанная из кадра команда встает в очередь команд, остальные принятые команды игнорируются до очистки очереди (SerialFrameProtocol::cmdClear()). Смотрите пример. Вы можете сами "пойти дальше", заменить cmdQueue на реальную очередь команд, если нужно сохранять все полученные команды.

Программа МК может отвечать на команды ПК. Реализована похожая структура данных и система команд.
МК может отвечать блоками данных с префиксом 'B' (block):

`<'B'><байт кода блока данных><опциональные массив байт>...`

или массивом байт с префиксом 'A' (array):

`<'A'><байт кода массива><массив байт>...`

Такие структуры формируются при заполнения кадра (frameStart() - frameEnd()) в программе МК.
Дополнительно в библиотеке SFP есть функции SerialFrameProtocol::logPrint() / SerialFrameProtocol::logWrite(). Которые формируют кадр для логирования на ПК. В основном предполагается отсылка строки, но может быть любой тип данных, поддерживаемый функциям SerialFrameProtocol::print() / SerialFrameProtocol::write() c префиксом LOG_CMD ('~' по умолчанию).

Соответсвенно в программе на LabVIEW отслеживаются принимаемый кадры и обрабатываются блоки и массивы от МК, команды логирования и т.п.
Как я уже упоминал, это один из возможных примеров реализаций пользовательской надстройки протокола.
Таким образом можно эффективно передавать между ПК и МК отдельные переменные либо массивы.
