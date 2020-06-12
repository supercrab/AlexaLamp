#ifndef SoftwareTimer_h
#define SoftwareTimer_h

// A quick and dirty software timer
class SoftwareTimer{

	public:

		// Start the timer
		void start(){
			enable(true); 
		}

		// Stop the timer
		void stop(){
			enable(false); 
		}
		
		// Define method to be called when time isup
		void setCallback(void(*func)(void)){
			_callback = func; 
		}
		
		// Set timer interval (ms)
		void setInterval(uint32_t ms){
			_interval = ms;
			setNextTick(); 
		}

		// Set timer interval (seconds)
		void setIntervalSeconds(uint32_t seconds){
			setInterval(seconds * 1000); 
		}

		// Set timer interval (minutes)
		void setIntervalMinutes(uint32_t mins){
			setIntervalSeconds(mins * 60); 
		}

		// Set timer interval (hours)
		void setIntervalHours(uint32_t hours){
			setIntervalMinutes(hours * 60); 
		}

		// Set timer interval (days - 48 max)
		void setIntervalDays(uint8_t days){

			//  millis rollover occurs every 49.7 days so ensure 48 days for accuracy
			if (days > 48)
				days = 48;
			setIntervalHours(days * 24); 
		}

		// Routine to handle timer event
		void handle(){

			// Timer must be on and have a sensible interval and we've done a tick
			if (_enabled && _interval > 0 && millis() > _nextMillis){ 
				if (_callback != NULL)
					_callback();

				// Workout next interval 
				setNextTick(); 
			}
		}

	private:
		
		// How often should timer tick
		uint32_t _interval = 0;

		// When will the time tick next
		uint32_t _nextMillis = 0;

		// Is the timer on
		bool _enabled = false;
		
		// Address of callback method to call when a tick occurs
		void (*_callback)(void) = NULL;

		// Calculate next interval time
		void setNextTick(){
			_nextMillis = millis() + _interval; 
		}

		// Enable or disable timer
		void enable(bool enable){
			_enabled = enable;

			// If we're turning the timer on set the next tick
			if (enable)
				setNextTick(); 
		}
};

#endif