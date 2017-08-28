#pragma once

#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <stdint.h>
#include <math.h>
#include <chrono>
#include <iostream>

class digital_analog_converter{
	public:
	digital_analog_converter(unsigned int, unsigned int, double , double, double, double, unsigned int);
	double voltage_in_range(double voltage);
	uint16_t voltage_to_code(double voltage);
	double code_to_voltage(uint16_t code);
	void transmit(uint16_t code = 0, uint8_t device = 3, unsigned int cs = 0);
	void transmit_voltage(double voltage = 0, uint8_t device = 0, unsigned int cs = 0);
	void fade(double from, double to, double time = 1, uint8_t device = 0, unsigned int cs = 0);
	uint16_t get_last_value();
	double get_voltage_step();

	private:
	unsigned int clock_speed;
	unsigned int bits;
	double min_voltage; // dac output at transmitting 0;
	double max_voltage;  // dac output at transmitting 2^bits (hightest value)   BEWARE ORIENTATION!!!
	double min_voltage_constrain;
	double max_voltage_constrain;
	unsigned int cs; 
	static int LDAC; // pi pin used for LDAC control
	uint16_t last_value;
	double voltage_step;
};

class analog_digital_converter{
	//tested with MCP3008
	public:
	analog_digital_converter(unsigned int, unsigned int, double, double, unsigned int);
	double code_to_voltage(uint16_t code);
	uint16_t read(uint8_t channel = 0, unsigned int cs = 1);
	double read_voltage(uint8_t channel = 0, unsigned int cs = 1);

	private:
	unsigned int clock_speed;
	unsigned int bits;
	double min_voltage; // dac output at transmitting 0;
	double max_voltage;  // dac output at transmitting 2^bits-1 (hightest value)   BEWARE ORIENTATION!!!
	unsigned int cs; 
};

class digital_potentiometer{
	public:
	digital_potentiometer(unsigned int, unsigned int, double , double, unsigned int);
	uint16_t resistance_to_code(double voltage);
	double code_to_resistance(uint8_t code);
	void transmit(int code = 0);//, unsigned int cs = 0);
	void transmit_resistance(double voltage = 0);//, unsigned int cs = 0);

	private:
	unsigned int clock_speed;
	unsigned int bits;
	double min_resistance; // dac output at transmitting 0;
	double max_resistance;  // dac output at transmitting 2^bits (hightest value)   BEWARE ORIENTATION!!!
	unsigned int cs; 
};


double map(double, double, double, double, double);

double time_since(auto input);
