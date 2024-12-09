#include "RotEncoder.h"

void IRAM_ATTR readEncoderISR() {
	rotEncoderInstance->readEncoder_ISR();
}
void IRAM_ATTR readButtonISR() {
	rotEncoderInstance->readButton_ISR();
}

RotEncoder *RotEncoder::getInstance(
	uint8_t encoder_APin, uint8_t encoder_APinMode, 
	uint8_t encoder_BPin, uint8_t encoder_BPinMode,
	uint8_t encoder_ButtonPin, uint8_t encoder_ButtonPinMode,
	uint8_t encoder_VccPin,
	uint8_t encoder_Steps) {
	rotEncoderInstance = new RotEncoder();
	rotEncoderInstance->setup(
		encoder_APin, encoder_APinMode,
		encoder_BPin, encoder_BPinMode,
		encoder_ButtonPin, encoder_ButtonPinMode, 
		encoder_VccPin,
		encoder_Steps,
		readEncoderISR, readButtonISR
	);
	return rotEncoderInstance;
}


void RotEncoder::setup( uint8_t encoder_APin, uint8_t encoder_APinMode, 
												uint8_t encoder_BPin, uint8_t encoder_BPinMode, 
                        uint8_t encoder_ButtonPin, uint8_t encoder_ButtonPinMode, 
												uint8_t encoder_VccPin,
                        uint8_t encoder_Steps,
                        void (*ISR_rotation)(void), void (*ISR_button)(void) ) {

	encoderAPin = encoder_APin;
	encoderBPin = encoder_BPin;
	encoderButtonPin = encoder_ButtonPin;
	encoderVccPin = encoder_VccPin;
	encoderSteps = encoder_Steps;

	old_AB = 0;
	lastReadEncoder0Pos = 0;

	// todo: hard coded for active low:
	isr_button_pin_state_prev = HIGH;
	isr_button_pin_state_change_millis = millis();

	buttonStateNow = BUT_PRESSED_STATE_UNKNOWN;
	buttonStateAtLastReportedEvent = BUT_PRESSED_STATE_UNKNOWN;
	buttonEventReported = BUT_EVENT_NONE;


	pinMode(encoderAPin, encoder_APinMode);
	pinMode(encoderBPin, encoder_BPinMode);

	if ( encoderVccPin > 0 ) {
		pinMode(encoderVccPin, OUTPUT);
		digitalWrite(encoderVccPin, 1); //Vcc for encoder
	}

	// Initialize rotary encoder button reading and decoding
	if (encoderButtonPin >= 0) {
		pinMode(encoderButtonPin, encoder_ButtonPinMode);
	}  

	attachInterrupt(digitalPinToInterrupt(encoderAPin), ISR_rotation, CHANGE);
	attachInterrupt(digitalPinToInterrupt(encoderBPin), ISR_rotation, CHANGE);
  if ( ISR_button != NULL ) {
	  //attachInterrupt(digitalPinToInterrupt(encoderButtonPin), ISR_button, RISING);
  	attachInterrupt(digitalPinToInterrupt(encoderButtonPin), ISR_button, CHANGE);
  }
}

void RotEncoder::setBoundaries(long minEncoderValue, long maxEncoderValue, bool circleValues) {
	_minEncoderValue = minEncoderValue * encoderSteps;
	_maxEncoderValue = maxEncoderValue * encoderSteps;
	_circleValues = circleValues;
}

void RotEncoder::enable() {
	bEnabled = true;
}
void RotEncoder::disable() {
	bEnabled = false;
}

void RotEncoder::reset(long newValue_) {
	newValue_ = newValue_ * encoderSteps;
	encoder0Pos = newValue_;
	lastReadEncoder0Pos = encoder0Pos;
	if (encoder0Pos > _maxEncoderValue) {
		encoder0Pos = _circleValues ? _minEncoderValue : _maxEncoderValue;
  }
	if (encoder0Pos < _minEncoderValue) {
		encoder0Pos = _circleValues ? _maxEncoderValue : _minEncoderValue;
  }
}


long RotEncoder::readEncoder() {
	return (encoder0Pos / encoderSteps);
}
void RotEncoder::setEncoderValue(long newValue) {
	reset(newValue);
}
long RotEncoder::encoderChanged() {
	long _encoder0Pos = readEncoder();
	long encoder0Diff = _encoder0Pos - lastReadEncoder0Pos;
	lastReadEncoder0Pos = _encoder0Pos;
	return encoder0Diff;
}

ButtonPressedState RotEncoder::getButtonPressedState() {
	ButtonPressedState result = BUT_PRESSED_STATE_UNKNOWN;
	// todo: does thisassign  require disabling interrupts?
	unsigned long button_pin_state_change_millis = isr_button_pin_state_change_millis;

//	if ( button_pin_state_change_millis != 0 ) {
		if ( (millis() - button_pin_state_change_millis) > DEBOUNCE_THRESHOLD_MS ) {
			// todo: hard coded for active low:
			result = digitalRead(encoderButtonPin) ? BUT_PRESSED_STATE_UP : BUT_PRESSED_STATE_DOWN;
		}
//	}
	return result;
}

ButtonEvent RotEncoder::getButtonEvent() {
	ButtonEvent result = BUT_EVENT_NONE;
	ButtonPressedState currentButtonState = getButtonPressedState();
	if ( currentButtonState != BUT_PRESSED_STATE_UNKNOWN ) {
		if ( currentButtonState != buttonStateAtLastReportedEvent ) {
			if ( buttonStateAtLastReportedEvent != BUT_PRESSED_STATE_UNKNOWN ) {
				result = (currentButtonState == BUT_PRESSED_STATE_DOWN) ? BUT_EVENT_PRESSED : BUT_EVENT_RELEASED;
			}
			buttonStateAtLastReportedEvent = currentButtonState;
		}
	}
	return result;
}


void IRAM_ATTR RotEncoder::readEncoder_ISR() {
	unsigned long now = millis();
	portENTER_CRITICAL_ISR(&rotationMux);

	if ( bEnabled ) {
		// code from https://www.circuitsathome.com/mcu/reading-rotary-encoder-on-arduino/
		/**/
		old_AB <<= 2; //remember previous state

		int8_t ENC_PORT = ((digitalRead(encoderBPin)) ? (1 << 1) : 0) | ((digitalRead(encoderAPin)) ? (1 << 0) : 0);

		old_AB |= (ENC_PORT & 0x03); //add current state

		//encoder0Pos += ( enc_states[( old_AB & 0x0f )]);
		int8_t currentDirection = (enc_states[(old_AB & 0x0f)]); //-1,0 or 1

		if (currentDirection != 0) {
			long prevRotaryPosition = encoder0Pos / encoderSteps;
			encoder0Pos += currentDirection;
			long newRotaryPosition = encoder0Pos / encoderSteps;

			if (newRotaryPosition != prevRotaryPosition && rotaryAccelerationCoef > 1) {
				//additional movements cause acceleration?
				// at X ms, there should be no acceleration.
				unsigned long accelerationLongCutoffMillis = 200;
				// at Y ms, we want to have maximum acceleration
				unsigned long accelerationShortCutffMillis = 4;

				// compute linear acceleration
				if ( (currentDirection == lastMovementDirection) && (currentDirection != 0) && (lastMovementDirection != 0) ) {
					// ... but only of the direction of rotation matched and there
					// actually was a previous rotation.
					unsigned long millisAfterLastMotion = now - lastMovementAt;

					if (millisAfterLastMotion < accelerationLongCutoffMillis) {
						if (millisAfterLastMotion < accelerationShortCutffMillis) {
							millisAfterLastMotion = accelerationShortCutffMillis; // limit to maximum acceleration
						}
						if (currentDirection > 0) {
							encoder0Pos += rotaryAccelerationCoef / millisAfterLastMotion;
						} else {
							encoder0Pos -= rotaryAccelerationCoef / millisAfterLastMotion;
						}
					}
				}
				lastMovementAt = now;
				lastMovementDirection = currentDirection;
			}

			//respect limits
			if (encoder0Pos > _maxEncoderValue) {
				encoder0Pos = _circleValues ? _minEncoderValue : _maxEncoderValue;
      }
			if (encoder0Pos < _minEncoderValue) {
				encoder0Pos = _circleValues ? _maxEncoderValue : _minEncoderValue;
      }
		}
	}
	portEXIT_CRITICAL_ISR(&rotationMux);
}

void IRAM_ATTR RotEncoder::readButton_ISR() {
	portENTER_CRITICAL_ISR(&buttonMux);

	uint8_t button_pin_state_now = !digitalRead(encoderButtonPin);
	if ( button_pin_state_now != isr_button_pin_state_prev ) {
		isr_button_pin_state_change_millis = millis();
		isr_button_pin_state_prev = button_pin_state_now;
	}

	portEXIT_CRITICAL_ISR(&buttonMux);
}






// void AiEsp32RotaryEncoder::begin()
// {
// 	this->lastReadEncoder0Pos = 0;
// 	if (this->encoderVccPin >= 0)
// 	{
// 		pinMode(this->encoderVccPin, OUTPUT);
// 		digitalWrite(this->encoderVccPin, 1); //Vcc for encoder
// 	}
// 
// 	// Initialize rotary encoder reading and decoding
// 	this->previous_butt_state = 0;
// 	if (this->encoderButtonPin >= 0)
// 	{
// 		pinMode(this->encoderButtonPin, INPUT_PULLUP);
// 	}
// }



