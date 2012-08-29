/*
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

#include <hardware/sensors.h>

#include "nusensors.h"

/*****************************************************************************/

/*
 * The SENSORS Module
 */

static const struct sensor_t sSensorList[] = {
        { "BMA222 3-axis Accelerometer",
		"BOSCH Sensortec",
		1, SENSORS_HANDLE_BASE+ID_A,
		SENSOR_TYPE_ACCELEROMETER, 4.0f*9.81f, (4.0f*9.81f)/256.0f, 0.2f, 0, { } },
	{ "TMD27711 proximity sensor",
		"TAOS",
		1, SENSORS_PROXIMITY_HANDLE,
		SENSOR_TYPE_PROXIMITY, 5.0f, 1.0f, 0.5f, 0,{ } },
	{ "TMD27711 light sensor",
		"TAOS",
		1, SENSORS_LIGHT_HANDLE,
		 SENSOR_TYPE_LIGHT, 10000.0f, 1.0f, 0.5f, 0,{ } },
};

static int open_sensors(const struct hw_module_t* module, const char* name,
        struct hw_device_t** device);

static int sensors__get_sensors_list(struct sensors_module_t* module,
        struct sensor_t const** list)
{
    *list = sSensorList;
    return ARRAY_SIZE(sSensorList);
}

static struct hw_module_methods_t sensors_module_methods = {
    .open = open_sensors
};

const struct sensors_module_t HAL_MODULE_INFO_SYM = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .version_major = 1,
        .version_minor = 0,
        .id = SENSORS_HARDWARE_MODULE_ID,
        .name = "Samsung Galaxy Cooper Sensors Module",
        .author = "Paweł Chmiel",
        .methods = &sensors_module_methods,
    },
    .get_sensors_list = sensors__get_sensors_list
};

/*****************************************************************************/

static int open_sensors(const struct hw_module_t* module, const char* name,
        struct hw_device_t** device)
{
    return init_nusensors(module, device);
}
