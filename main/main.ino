#define USE_RANDOM_SEED        true  //использовать ли аналоговый пин 0 для семя рандома
#define MIN_SIGNAL_TIME        1000  //минимальное время подачи высокого сигнала на указаные пины, милисикунды
#define MAX_SIGNAL_TIME        30000 //максимальное время подачи высокого сигнала на указаные пины, милисикунды             
#define NUMBER_OF_OUTPUTS_PINS 5     //количество выходных пинов
/*
	пины считаются 2, 3, 4... 
	устанавливаются автоматически(каждый пин указывать не нужно)
*/
#define MODE 2 //режим подачи сигналов на пины
/*
	режимы:
		1 - высокий сигнал подается на все пины разом;
		2 - высокий сигнал подается на N случайных пинов(N - параметр );
		3 - высокий сигнал подается последовательно
		(на каждом пине высокий сигнал (от MIN_SIGNAL_TIME до MAX_SIGNAL_TIME) / NUMBER_OF_OUTPUTS_PINS милисикунд);
*/ 
#define N_MODE2       2      //для режима 2
#define REQUIRED_TIME 1800000//максимальная возможная пауза между подачей сигнала, милисикунды
//END_SETTINGS

String sequence             = "";   //строка со временем и значение которое нужно подовать(ПР. 00036481 - 0003648 вемя 1 значение)
bool   to_generate_sequence = true; //генерировать ли последовательность
bool   ERROR                = false;//для отловки ошибки
unsigned long time_of_last_high_voltage = 0;//для проверки максимальной возможной паузы
unsigned long time                      = 0;//для проверки прошло ли время
unsigned long time_of_signal            = 0;//время подачи сигнала
unsigned long index                     = 0;//индекс текужего периода

String get_substring(String& str, unsigned long begin, unsigned long end);
String int_to_my_string(unsigned long number);//конвертация int в string и прибовление нулей, чтобы результат содержал 8 символов 
bool   to_change_pins(int mode = -1);//изменение состояния пинов
String operator*(String str, const int number);//перегрузка оператора * для строки и числа, потому что разработчики поленились)))


void setup()
{
	//проверка настроек. Если найдена ошибка поднимается флаг ERROR
	#if (MIN_SIGNAL_TIME < 0) || (MIN_SIGNAL_TIME > MAX_SIGNAL_TIME) || (MAX_SIGNAL_TIME < 0)
		ERROR = true;
		Serial.begin(9600);
		Serial.print("incorrect setting!!! Error in MIN_SIGNAL_TIME or MAX_SIGNAL_TIME!!!\n");
		while(true){}
	#elif (NUMBER_OF_OUTPUTS_PINS < 0) || (NUMBER_OF_OUTPUTS_PINS > NUM_DIGITAL_PINS - 2)
		ERROR = true;
		Serial.begin(9600);
		Serial.print("incorrect setting!!! Error in NUMBER_OF_OUTPUTS_PINS!!!\n");
		while(true){}
	#elif (MODE < 1) || (MODE > 3)
		ERROR = true;
		Serial.begin(9600);
		Serial.print("incorrect setting!!! error in MODE!!!\n");
		while(true){}
	#elif (N_MODE2 < 0) || (N_MODE2 > NUMBER_OF_OUTPUTS_PINS)
		ERROR = true;
		Serial.begin(9600);
		Serial.print("incorrect setting!!! Error in NUMBER_OF_OUTPUTS_PINS!!!\n");
		while(true){}
	#elif REQUIRED_TIME < 0
		ERROR = true;
		Serial.begin(9600);
		Serial.print("incorrect setting!!! Error in REQUIRED_TIME!!!\n");
		while(true){}
	//если ошибок не найдено
	#else
		//инициилизируем пины
		for(int i = 0; i < NUMBER_OF_OUTPUTS_PINS; ++i)
		pinMode(i + 2, OUTPUT); 
	#endif
}

//если была найдена ошибка, то этот код не выполняем
#if ERROR
void loop()
{	
	//использовать ли семя рандома
	#if USE_RANDOM_SEED
		randomSeed(analogRead(0));
	#endif

	//если генерировать последодвательность
	if(to_generate_sequence)
	{
		//"обнуляем" переменные 
		sequence = "";
		bool last_value = LOW;
		int  signal = 0; 
		to_generate_sequence = false;
		unsigned long random_time = 0;
		unsigned long general_time = 3600000;

		//цикл генерирует периоды на 1 час
		while(general_time != 0)
		{
			//проверка чтобы высокий сигнал подавался не дольше указаного в настройках
			if(last_value == LOW)
			{
				signal = random(0, 2);
				last_value = signal;
			}
			else
			{
				signal = LOW;
				last_value = signal;
			}

			random_time = random(MIN_SIGNAL_TIME, MAX_SIGNAL_TIME + 1);

			//проверка чтобы периоды в сумме больше часа не занимали
			if(random_time > general_time)
			{
				random_time = general_time;
				general_time -= random_time;
			}	
			else
				general_time -= random_time;

			//добавление к последовательности нового периода
			String Time   = int_to_my_string(random_time);
			String Signal = String(signal);
			sequence += Time + Signal;
		}	

	}

	//если нужно изменить состояние пинов
	if(millis() - time > time_of_signal)
	{
		time = millis();
		//получаем длительность нового периода 
		time_of_signal = (get_substring(sequence, index, index + 6)).toInt();

		//вызываем функцию изменения состояния пинов 
		to_change_pins(MODE);

		//увеличиваем индекс для перехода на следующий период
		if(index == sequence.length() - 8)
			to_generate_sequence = true;
		else
			index += 8;
	}

	//если превышен максимальная пауза
	if(millis() - time_of_last_high_voltage > REQUIRED_TIME)
	{
		//говорим что нужно сгенерировать новую последовательность
		to_generate_sequence = true;
		//вызываем функцию изменения состояния пинов 
		to_change_pins(0);
	}
}

bool to_change_pins(int mode)
{
	for(int i = 0; i < NUMBER_OF_OUTPUTS_PINS; ++i)
		digitalWrite(i + 2, LOW);
	switch (mode) 
	{
	    case 0:
	    {
	    	for(int i = 0; i < NUMBER_OF_OUTPUTS_PINS; ++i)
	    	{
				digitalWrite(i + 2, HIGH);
				delay(MAX_SIGNAL_TIME);
				digitalWrite(i + 2, LOW);
	    	}
			break;
	    }  
	    case 1:
	    {
	    	//посылаем на все пины одинаковый сигнал
	    	int signal = (get_substring(sequence, index + 7, index + 7)).toInt();
	    	if(signal > 0) 
	    	{
	    		//обновляем время для не превышеняи 
	    		time_of_last_high_voltage = millis();

	    		for(int i = 0; i < NUMBER_OF_OUTPUTS_PINS; ++i)
	    		digitalWrite(i + 2, signal);
	    	}
			break;
	    }   
	    case 2:
	    {
	    	//подаем сигнал на N случайных пинов
	    	int signal = (get_substring(sequence, index + 7, index + 7)).toInt();
	    	if(signal > 0)
	    	{
	    		time_of_last_high_voltage = millis();

	    		//массив для доступных пинов
	    		int all_pins[NUMBER_OF_OUTPUTS_PINS];
	    		for(int i = 0; i < NUMBER_OF_OUTPUTS_PINS; ++i)
	    			all_pins[i] = i + 2;

	    		//массив для нужных пинов
		    	int pins[N_MODE2];  
		    	//индекс в массиве all_pins
		    	int random_number = 0;

		    	for(int i = 0; i < N_MODE2; ++i)
		    	{
		    		//генерируем случайное число 
		    		random_number = random(0, NUMBER_OF_OUTPUTS_PINS);
		    		//контроль уникальности числа
		    		//-1 - значит что пин назят
		    		if(all_pins[random_number] != -1)
		    			pins[i] = all_pins[random_number];
		    		else
		    		{
		    			do
		    			{
		    				if(random_number == NUMBER_OF_OUTPUTS_PINS - 1)
		    					random_number = 0;
		    				else
		    					random_number++;
		    			}
		    			while(all_pins[random_number] == -1);
		    			pins[i] = all_pins[random_number];
		    		}
		    		all_pins[random_number] = -1;
		    	}

		    	//подаем на сгенерированые пины сигнал
		    	for(int i = 0; i < N_MODE2; ++i)
		    		digitalWrite(pins[i], signal);
	    	}  
	    	break;	
	    }
	    case 3:
	    {
	    	//последовательная подачя сигнала
	    	int signal = (get_substring(sequence, index + 7, index + 7)).toInt();
	    	if(signal > 0) 
	    	{
	    		time_of_last_high_voltage = millis();

	    		//вычисляем время подачи сигнала на каждый пин
	    		long time_for_every_pin = time_of_signal / NUMBER_OF_OUTPUTS_PINS;

	    		//подаем сигнал на каждый пин поочереди
	    		for(int i = 0; i < NUMBER_OF_OUTPUTS_PINS; ++i)
	    		{
	    			digitalWrite(i + 2, HIGH);
	    			delay(time_for_every_pin);
	    			digitalWrite(i + 2, LOW);
	    		}
	    	}
	    	break;
	    }	
	    default:
	    {
	    	return false;
	    }  
	}

	return true;
}

String int_to_my_string(unsigned long number)
{
	String result = String(number);

	//вычисляем сколько нулей нужно добавить к числу
	if(number < 1000000)
	{
		for(int i = 5; i > 0; --i)
		{
			if(number > pow(10, i))
			{
				//добавляем нули
				result = (String("0") * (6 - i)) + result;
				break;
			}
		}
	}

	return result;
}

String get_substring(String& str, unsigned long begin, unsigned long end)
{
	String result = "";

	for(unsigned long i = begin; i < end + 1; ++i)
		result += str[i];

	return result;
}

String operator*(String str, const int number)
{
	String result = "";
	for(int i = 0; i < number; ++i)
		result += str;

	return result;
}
#else
void loop(){}
#endif