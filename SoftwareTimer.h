#ifndef SoftwareTimer_h
#define SoftwareTimer_h

// A quick and dirty software timer
class SoftwareTimer{
	public:
		void start(){ enable(true); }
		void stop(){ enable(false); }
		void setCallback(void(*func)(void)){
			 _callback = func; 
		}
		void setInterval(uint32_t ms){
			 _interval = ms; 
		}
		void setIntervalMins(uint16_t mins){
			setInterval(mins * 60 * 1000); 
		}
		void handle(){ 
			if (_enabled && _interval && millis() > _nextMillis){ 
				if (_callback != NULL) _callback(); 
				setNextTick(); 
			}
		}
	private:
		uint32_t _interval = 0;
		uint32_t _nextMillis = 0;
		bool _enabled = false;
		void (*_callback)(void) = NULL;
		void setNextTick(){
			_nextMillis = millis() + _interval; 
		}
		void enable(bool enable){
			_enabled = enable; 
			if (enable) setNextTick(); 
		}
};

#endif