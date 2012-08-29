/*
 * Copyright (c) 2011, Code Aurora Forum. All rights reserved.
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <poll.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/select.h>
#include <cutils/log.h>

#include "BMA222Sensor.h"

#define CONVERT                     (GRAVITY_EARTH / 256)
//#define CONVERT_GRAVITY_X           (CONVERT)
//#define CONVERT_GRAVITY_Y           (-1 * CONVERT)
//#define CONVERT_GRAVITY_Z           (-1 * CONVERT)
#define SENSOR_NAME                 "bma222"
#define ID_GY

/*****************************************************************************/

BMA222Sensor::BMA222Sensor()
    : SensorBase(NULL, SENSOR_NAME),
      mEnabled(0),
      mInputReader(4),
      mHasPendingEvent(false),
      mEnabledTime(0)
{
    mPendingEvent.version = sizeof(sensors_event_t);
    mPendingEvent.sensor = ID_GY
    mPendingEvent.type = SENSOR_TYPE_ACCELEROMETER;
    memset(mPendingEvent.data, 0, sizeof(mPendingEvent.data));

    if (sensor_get_class_path() < 0)
		LOGD("Failed to get class path\n");
}

BMA222Sensor::~BMA222Sensor() {
    if (mEnabled) {
        enable(0, 0);
    }
}

int BMA222Sensor::enable(int32_t, int en) {
    int flags = en ? 1 : 0;
    char buf[2];
    int count = 0;;

    if (flags != mEnabled) {
            buf[1] = 0;
            if (flags) {
                buf[0] = '1';
                mEnabledTime = getTimestamp();
            } else {
                buf[0] = '0';
            }
	    set_sysfs_input_attr(class_path, "enable", buf, 1);
            mEnabled = flags;
    }
    return 0;
}

bool BMA222Sensor::hasPendingEvents() const {
    return mHasPendingEvent;
}

int BMA222Sensor::setDelay(int32_t handle, int64_t delay_ns)
{
    char buf[80];
    int count = 0;;
    int64_t delay_ms = delay_ns / 1000000;

    count = sprintf(buf, "%lld", delay_ms);
    set_sysfs_input_attr(class_path, "delay", buf, count);

    return 0;
}

int BMA222Sensor::readEvents(sensors_event_t* data, int count)
{
    if (count < 1)
        return -EINVAL;

    if (mHasPendingEvent) {
        mHasPendingEvent = false;
        mPendingEvent.timestamp = getTimestamp();
        *data = mPendingEvent;
        return mEnabled ? 1 : 0;
    }

    ssize_t n = mInputReader.fill(data_fd);
    if (n < 0)
        return n;

    int numEventReceived = 0;
    input_event const* event;

    while (count && mInputReader.readEvent(&event)) {
        int type = event->type;
        if (type == EV_ABS) {
            float value = event->value;
            if (event->code == ABS_X) {
                //mPendingEvent.data[1] = value * CONVERT_GRAVITY_Y;
                mPendingEvent.data[0] = value * CONVERT;
            } else if (event->code == ABS_Y) {
                //mPendingEvent.data[0] = value * CONVERT_GRAVITY_X;
                mPendingEvent.data[1] = value * CONVERT;
            } else if (event->code == ABS_Z) {
                //mPendingEvent.data[2] = value * CONVERT_GRAVITY_Z;
                mPendingEvent.data[2] = value * CONVERT;
            }
        } else if (type == EV_SYN) {
            mPendingEvent.version = sizeof(sensors_event_t);
            mPendingEvent.sensor = ID_A;
            mPendingEvent.type = SENSOR_TYPE_ACCELEROMETER;
            mPendingEvent.timestamp = timevalToNano(event->time);
            *data++ = mPendingEvent;
            numEventReceived++;
            count--;
        } else {
            LOGE("BMA222Sensor: unknown event (type=%d, code=%d)",
                    type, event->code);
        }
        mInputReader.next();
    }

    return numEventReceived;
}

bool BMA222Sensor::isEnabled(int32_t handle) {
	return (0 != (mEnabled)) ? true : false;
}

int BMA222Sensor::sensor_get_class_path()
{
	char dirname[] = "/sys/class/input";
	char buf[256];
	int res;
	DIR *dir;
	struct dirent *de;
	int fd = -1;
	int found = 0;

	dir = opendir(dirname);
	if (dir == NULL)
		return -1;

	while((de = readdir(dir))) {
		if (strncmp(de->d_name, "input", strlen("input")) != 0) {
				continue;
		}

		sprintf(class_path, "%s/%s", dirname, de->d_name);
		snprintf(buf, sizeof(buf), "%s/name", class_path);

		fd = open(buf, O_RDONLY);
		if (fd < 0) {
				continue;
		}
		if ((res = read(fd, buf, sizeof(buf))) < 0) {
				close(fd);
				continue;
		}
		buf[res - 1] = '\0';
		if (strcmp(buf, SENSOR_NAME) == 0) {
				found = 1;
				close(fd);
				break;
		}

		close(fd);
		fd = -1;
	}
	closedir(dir);

	if (found) {
		return 0;
	}else {
		*class_path = '\0';
		return -1;
	}
}

int BMA222Sensor:: set_sysfs_input_attr(char *class_path,
				const char *attr, char *value, int len)
{
	char path[256];
	int fd;

	if (class_path == NULL || *class_path == '\0'
			|| attr == NULL || value == NULL || len < 1) {
		return -EINVAL;
	}
	snprintf(path, sizeof(path), "%s/%s", class_path, attr);
	path[sizeof(path) - 1] = '\0';
	fd = open(path, O_RDWR);
	if (fd < 0) {
			return -errno;
	}
	if (write(fd, value, len) < 0) {
		close(fd);
		return -errno;
	}
	close(fd);
	return 0;
}
