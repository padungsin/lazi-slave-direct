#define MAX_TIMER 6

class Timer {
	public:
	String port;
	String status;
	String time;
	int duration;
};
class DeviceTemplateProperty {
  public:
  int foggySproutDuration;
  int foliarSproutDuration;
  int pureWateringValveDuration;
  int mixFertilizerPumpDuration;
  int mixFertilizerWateringValveDuration;
  int wateringPumpDuration;
  int potassiumPumpDuration;
  int potassiumWateringValveDuration;
  int potassiumWateringPumpDuration;
  int kneadDuration;
  int foliarDuration;
};
class DevicePort {
  public:
    int foggyValve;
    int wateringValve;
    int wateringPump;
    int mixFertilizerPump;
    int potassiumPump;
    int foliarPump;
    int otherWateringValve;
    int light; 
    bool relayActiveHigh;
};

class DeviceTemplate {
  public:
  String mode;
  String range;
  String cutOff;
  DevicePort *devicePort;
  DeviceTemplateProperty *property;
};


class DeviceConfig {
  public:
    String version;
    DeviceTemplate *deviceTemplate;
    int timerCount;
    Timer *timer[MAX_TIMER];
};
