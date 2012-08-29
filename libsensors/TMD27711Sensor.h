#ifndef ANDROID_TMD27711_SENSOR_H
#define ANDROID_TMD27711_SENSOR_H

#include <stdint.h>
#include <errno.h>
#include <sys/cdefs.h>
#include <sys/types.h>

#include "nusensors.h"
#include "SensorBase.h"
#include "InputEventReader.h"

/*****************************************************************************/

struct input_event;

class TMD27711Sensor : public SensorBase {
public:
	TMD27711Sensor();
	virtual ~TMD27711Sensor();

	virtual int setEnable(int32_t handle, int enabled);
	virtual int readEvents(sensors_event_t* data, int count);
	virtual bool hasPendingEvents(void) const;
	void processEvent(int code, int value);
	int setInitialState(void);
	int getFd() const;
	virtual int setDelay(int32_t handle, int64_t ns);
	virtual int getEnable(int32_t handle);		//rockie
	float indexToValue(size_t index) const;

	int mAlsEnabled;
	int mProxEnabled;
	bool mHasPendingEvent;
private:
	InputEventCircularReader mInputReader;
	sensors_event_t mPendingEvent;
};
/*****************************************************************************/
#endif //ANDROID_TMD27711_SENSOR_H
