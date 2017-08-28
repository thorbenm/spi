/*
TODO:
-Something is wrong with the cs
*/





#include "spi.h"

bool spi_setup[2] = {false, false};
bool LDAC_setup = false;
//int LDAC = 6;

digital_analog_converter::digital_analog_converter(unsigned int clock_s, unsigned int b, double minv, double maxv, double minvc, double maxvc, unsigned int c){
	clock_speed = clock_s;
	bits = b;
	min_voltage = minv;
	max_voltage = maxv;
	min_voltage_constrain = minvc;
	max_voltage_constrain = maxvc;
	cs = c; 

	if (spi_setup[c] == false){
		wiringPiSPISetup (c, clock_speed) ;
		wiringPiSetup () ;
		spi_setup[c] = true;
	}else{
		std::cerr << "chip select " << c << " is already in use"; 
		exit(0);
	}
	if(LDAC >= 0 && LDAC_setup == false){
		pinMode (LDAC, OUTPUT) ;
		digitalWrite (LDAC,HIGH);
		LDAC_setup = true;
	}

	voltage_step = fabs(max_voltage - min_voltage) / pow(2.0, (double) bits);

	transmit_voltage();
}

double digital_analog_converter::voltage_in_range(double voltage){
	if(max_voltage_constrain < voltage){
		std::cerr << "Voltage out of Range (high)\n";
		return max_voltage_constrain;
//		exit(0);
	}
	if(voltage < min_voltage_constrain){
		std::cerr << "Voltage out of Range (low)\n";
		return min_voltage_constrain;
//		exit(0);
	return voltage;
	}
}

uint16_t digital_analog_converter::voltage_to_code(double voltage){
	double value = map(voltage, min_voltage, max_voltage, 0.0 , pow(2.0, (double) bits) - 1.0); 
	uint16_t code = (uint16_t) value;
	return code;
}

double digital_analog_converter::code_to_voltage(uint16_t code){
	double value = (double) code;
	value = map(value, 0.0 , pow(2.0, (double) bits) - 1.0, min_voltage , max_voltage); 
	return value;
}

void digital_analog_converter::fade(double from, double to, double time, uint8_t device, unsigned int cs){
	to = voltage_in_range(to);
	from = voltage_in_range(from);

	const double pi = 3.14;
	double amplitude = from - (to + from) / 2.0 ;
	double offset = (from + to) / 2.0;

	auto function_start = std::chrono::high_resolution_clock::now(); 
	double voltage = 0;
	while(time_since(function_start) < time){
		voltage = offset + amplitude * cos (pi / time * time_since(function_start)); 
		transmit_voltage(voltage, device, cs);
	}
	transmit_voltage(to, device, cs);
}

void digital_analog_converter::transmit(uint16_t code, uint8_t device, unsigned int cs){
	if(bits != 16){
		std::cerr << "Programme is hardcoded for bits = 16";
		exit(0);
	}
	code = voltage_to_code(voltage_in_range(code_to_voltage(code)));
	uint8_t code1 = code >> 8;
	uint8_t code2 = code & 0xFF;
	wiringPiSPIDataRW (cs, (unsigned char*)&device, sizeof(device));
	wiringPiSPIDataRW (cs, (unsigned char*)&code1, sizeof(code1));
	wiringPiSPIDataRW (cs, (unsigned char*)&code2, sizeof(code2));
	if(LDAC >= 0){
		digitalWrite (LDAC,LOW);
//		delayMicroseconds(1);
		digitalWrite (LDAC,HIGH);
	}
	last_value = code;
}

void digital_analog_converter::transmit_voltage(double voltage, uint8_t device, unsigned int cs){
	voltage = voltage_in_range(voltage);
	transmit(voltage_to_code(voltage), device, cs);
}

uint16_t digital_analog_converter::get_last_value(){
	return last_value;
}

double digital_analog_converter::get_voltage_step(){
	return voltage_step;
}

double map(double value, double fromLow, double fromHigh, double toLow, double toHigh){
	if(fromLow == fromHigh || toLow == toHigh){
		std::cerr << "mapping not possible\n";
		exit(0);
	}
	return value * (toHigh - toLow) / (fromHigh - fromLow) + toHigh - (toHigh - toLow) / (fromHigh - fromLow) * fromHigh; 
}

double time_since(auto input){
	// to get time since programme start put
	// auto start_time = std::chrono::high_resolution_clock::now(); 
	// at the beginning of your programme
	//
	// run as : time_since(start)
	auto diff = std::chrono::high_resolution_clock::now() - input; // get difference 
	auto nsec = std::chrono::duration_cast<std::chrono::nanoseconds>(diff);
	double time1 = (double) nsec.count() / 1.0e9 ; 
	return time1;
}

analog_digital_converter::analog_digital_converter(unsigned int clock_s, unsigned int b, double minv, double maxv, unsigned int c){

	clock_speed = clock_s;
	bits = b;
	min_voltage = minv;
	max_voltage = maxv;
	cs = c; 

	if (spi_setup[c] == false){
		wiringPiSPISetup (c, clock_speed) ;
		wiringPiSetup () ;
		spi_setup[c] = true;
	}else{
		std::cerr << "chip select " << c << " is already in use"; 
		exit(0);
	}
	
}

uint16_t analog_digital_converter::read(uint8_t channel, unsigned int cs){
	uint16_t result = 0;
	uint8_t code1 = 1;
	uint8_t code2 = (8+channel) << 4;
	uint8_t code3 = 0;
	uint8_t code[3] = {code1,code2,code3};
	wiringPiSPIDataRW (cs, (unsigned char*)code, sizeof(code));
	code[1] = code[1] & 0b00001111;
	result = code[1];
	result = result << 8;
	result = result | code[2]; 
	return result;
}

double analog_digital_converter::code_to_voltage(uint16_t code){
	return map((double) code, 0.0, pow(2.0, (double) bits) - 1.0, min_voltage, max_voltage);
}

double analog_digital_converter::read_voltage(uint8_t channel, unsigned int cs){
	return code_to_voltage(read(channel,cs));
}

//new from here:

digital_potentiometer::digital_potentiometer(unsigned int _clock_speed, unsigned int _bits, double _min_resistance, double _max_resistance, unsigned int _cs){
	clock_speed = _clock_speed;
	bits = _bits;
	min_resistance = _min_resistance;
	max_resistance = _max_resistance;
	cs = _cs; 

	if (spi_setup[cs] == false){
		wiringPiSPISetup (cs, clock_speed) ;
		wiringPiSetup () ;
		spi_setup[cs] = true;
	}else{
		std::cerr << "chip select " << cs << " is already in use"; 
		exit(0);
	}

//	transmit_resistance();
}

uint16_t digital_potentiometer::resistance_to_code(double resistance){
	double value = map(resistance, min_resistance, max_resistance, 0.0 , pow(2.0, (double) bits) - 1.0); 
	uint8_t code = (uint8_t) value;
	return code;
}

double digital_potentiometer::code_to_resistance(uint8_t code){
	double value = (double) code;
	value = map(value, 0.0 , pow(2.0, (double) bits) - 1.0, min_resistance , max_resistance); 
	return value;
}

void digital_potentiometer::transmit(int input){//, unsigned int cs){
	if(input < resistance_to_code(min_resistance)){
		std::cerr << "resistance too low" << std::endl;
		input = resistance_to_code(min_resistance);
	}
	if(input > resistance_to_code(max_resistance)){
		std::cerr << "resistance too high" << std::endl;
		input = resistance_to_code(max_resistance);
	}
	if(bits != 8){
		std::cerr << "Programme is hardcoded for bits = 8";
		exit(0);
	}
	uint8_t input2 = (uint8_t) input;
	uint16_t transmission = input2 << 8;
	wiringPiSPIDataRW (cs, (unsigned char*)&transmission, sizeof(transmission));
}

void digital_potentiometer::transmit_resistance(double resistance){//, unsigned int cs){
	transmit(resistance_to_code(resistance));//, cs);
}

