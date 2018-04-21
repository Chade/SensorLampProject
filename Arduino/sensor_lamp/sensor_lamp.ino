#include <Arduino.h>
#include <MedianFilter.h>


//  Luminch One - Copyright 2012 by Francisco Castro <http://fran.cc>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

const uint8_t LED_PIN = 3;
const uint8_t SENSOR_PIN = A3;

const uint8_t FILTERSIZE = 9;               // Sample size for median filter

const uint8_t MIN_BRIGHTNESS = 5;           // Min brightness (0 - 255)
const uint8_t MAX_BRIGHTNESS = 255;         // Max brightness (0 - 255)

const uint8_t CYCLETIME = 10;               // Cycle time in ms (0 - 255)

const uint16_t DEBOUNCE_CYCLES = 30;		    // Number of cycles to debounce on / off
const uint16_t START_TRACKING_CYCLES = 100; // Delay if hand is detected before starting tracking
const uint16_t END_TRACKING_CYCLES = 100;   // Delay if no hand is detected while tracking before stopping tracking
const uint16_t HAND_MINIMUM_CHANGE = 10;    // Minimal brightness change allowed
const uint16_t SENSE_MAX_OFFSET = 50;       // Defines offset from SENSE_MAX to prevent signal noise from triggering lamp

uint16_t SENSE_MIN = 200;                   // Minimum sensing height in mm, must not be smaller then min value in out[]
uint16_t SENSE_MAX = 1500;                  // Maximum sensing height in mm, must not be greater then max value in out[]
uint16_t TRACK_DISTANCE = 200;              // Distance in mm to move hand to go from min to max brightness

// in[] holds the measured analogRead() values for defined distances
// Note: The in array should have increasing values
const int in[]  = {90, 97, 105, 113, 124, 134, 147, 164, 185, 218, 255, 317, 414, 525};
// out[] holds the corresponding distances in mm
const int out[] = {1500, 1400, 1300, 1200, 1100, 1000, 900, 800, 700, 600, 500, 400, 300, 200};

boolean lamp_lighted = false;
boolean hand_tracking = false;

uint16_t tracking_lower_limit = SENSE_MIN;
uint16_t tracking_upper_limit = SENSE_MAX;

uint8_t pwm_output = 0;

uint8_t stored_bright = MAX_BRIGHTNESS;
uint8_t target_bright = 0;
uint8_t hand_tracked_bright = 0;

uint16_t hand_cycles = 0;
uint16_t debounce_cycles = 0;

MedianFilter filter(FILTERSIZE);

int multiMap (int val, const int *_in, const int *_out, const uint8_t& size)
{
	// Take care the value is within range
	// val = constrain(val, _in[0], _in[size-1]);
	if (val <= _in[0]) return _out[0];
	if (val >= _in[size-1]) return _out[size-1];

	// Search right interval
	uint8_t pos = 1;  // _in[0] allready tested
	while (val > _in[pos]) pos++;

	// This will handle all exact "points" in the _in array
	if (val == _in[pos]) return _out[pos];

	// Interpolate in the right segment for the rest
	return (val - _in[pos-1]) * (_out[pos] - _out[pos-1]) / (_in[pos] - _in[pos-1]) + _out[pos-1];
}

float multiMap (float val, const float * _in, const float * _out, const uint8_t& size)
{
	// Take care the value is within range
	// val = constrain(val, _in[0], _in[size-1]);
	if (val <= _in[0]) return _out[0];
	if (val >= _in[size-1]) return _out[size-1];

	// Search right interval
	uint8_t pos = 1;  // _in[0] allready tested
	while (val > _in[pos]) pos++;

	// This will handle all exact "points" in the _in array
	if (val == _in[pos]) return _out[pos];

	// Interpolate in the right segment for the rest
	return (val - _in[pos-1]) * (_out[pos] - _out[pos-1]) / (_in[pos] - _in[pos-1]) + _out[pos-1];
}

void setup ()  {
	analogWrite(LED_PIN, 0);

  // Fill median filter
	for (uint8_t i = 0; i < FILTERSIZE; i++){
		filter.add(analogRead(SENSOR_PIN));
		delay(10);
	}

	// Get median
	uint16_t adc_input = filter.get();

	// Convert analog value to distance in mm
	uint16_t distance = multiMap(adc_input, in, out, 14);

	// Set max sense value
  if (distance > SENSE_MAX) {
    ;
  }
	else if (distance > (SENSE_MIN + TRACK_DISTANCE + SENSE_MAX_OFFSET)) {
		SENSE_MAX = distance - SENSE_MAX_OFFSET;
	}
	else if (distance > (SENSE_MIN + SENSE_MAX_OFFSET)) {
		SENSE_MAX = distance - SENSE_MAX_OFFSET;
		TRACK_DISTANCE = SENSE_MAX - SENSE_MIN;

		// Operation might be impeded
		analogWrite(LED_PIN, 255);
		delay(250);
		analogWrite(LED_PIN, 0);
	}
	else {
		// Operation not possible
		while (true) {
			pwm_output = pwm_output ? 0 : 255;
			analogWrite(LED_PIN, pwm_output);
			delay(250);
		}
	}
}

void loop () {
  uint32_t start_time = millis();
  
	// Debounce if neccessary
	if(debounce_cycles) {
		debounce_cycles -= 1;
	}
	else {
		// Read analog value and add to median filter
		filter.add(analogRead(SENSOR_PIN));
		uint16_t adc_input = filter.get();

		// Convert analog value to distance in mm
		uint16_t distance = multiMap(adc_input, in, out, 14);
    
		if (hand_tracking) {
			if(distance < SENSE_MAX) {
				if(distance < tracking_lower_limit) {
					hand_tracked_bright = MIN_BRIGHTNESS;

					// Shift lower border
					tracking_lower_limit = distance;
					tracking_upper_limit = constrain(distance + TRACK_DISTANCE,SENSE_MIN, SENSE_MAX);

					// Blink to signal
					analogWrite(LED_PIN, 0);
					delay(25);
					analogWrite(LED_PIN, pwm_output);
				}
				else if (distance > tracking_upper_limit) {
					hand_tracked_bright = MAX_BRIGHTNESS;

					// Shift upper border
					tracking_lower_limit = constrain(distance - TRACK_DISTANCE,SENSE_MIN, SENSE_MAX);
					tracking_upper_limit = distance;

					// Blink to signal
					analogWrite(LED_PIN, 0);
					delay(25);
					analogWrite(LED_PIN, pwm_output);
				}
				else {
					hand_tracked_bright = map(distance, tracking_lower_limit, tracking_upper_limit, MIN_BRIGHTNESS, MAX_BRIGHTNESS);
				}

				if (abs(target_bright - hand_tracked_bright) > HAND_MINIMUM_CHANGE || !lamp_lighted) {
					target_bright = hand_tracked_bright;
					lamp_lighted = true;
				}

				hand_cycles = 0;
			}
			else {
				target_bright = pwm_output;
				stored_bright = pwm_output;
				hand_cycles += 1;
				if (hand_cycles == END_TRACKING_CYCLES) {
					// Stop hand tracking
					hand_tracking = false;
					hand_cycles = 0;

					// Blink to confirm
					analogWrite(LED_PIN, 0);
					delay(200);
					analogWrite(LED_PIN, pwm_output);
				}
			}
		}
		else {
			if (distance < SENSE_MAX) {
				hand_cycles += 1;
				if (hand_cycles == START_TRACKING_CYCLES) {
					// Set tracking range
					tracking_lower_limit = constrain(distance - map(stored_bright, 0, 255, 0, TRACK_DISTANCE), SENSE_MIN, SENSE_MAX);
					tracking_upper_limit = constrain(distance + map(stored_bright, 0, 255, TRACK_DISTANCE, 0), SENSE_MIN, SENSE_MAX);

					// Start hand tracking
					hand_tracking = true;
					hand_cycles = 0;

					// Blink to confirm
					analogWrite(LED_PIN, 0);
					delay(200);
					analogWrite(LED_PIN, pwm_output);
				}
			}
			else {
				if (hand_cycles) {
					lamp_lighted = !lamp_lighted;
					target_bright = lamp_lighted ? stored_bright : 0;
					debounce_cycles = DEBOUNCE_CYCLES;
					hand_cycles = 0;
				}
			}
		}
	}

	// Adjust brightness
	if (pwm_output != target_bright) {
		if(pwm_output > target_bright && pwm_output > 0) --pwm_output;
		if(pwm_output < target_bright && pwm_output < 0xFF) ++pwm_output;
		analogWrite(LED_PIN, pwm_output);
	}

  // Wait for constant cycle time
  uint32_t delta = millis() - start_time;
  uint8_t ms = (delta < CYCLETIME) ? CYCLETIME - delta : 0;
	delay(ms);
}

