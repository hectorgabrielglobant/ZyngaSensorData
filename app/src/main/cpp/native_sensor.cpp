#include <jni.h>
#include <string>
#include <sstream>
#include <android/sensor.h>
#include <android/log.h>

namespace sensor {

    class SensorData {
    private:
        float* temperature;
        float* pressure;
        ASensorEventQueue* sensorEventQueue;
        const ASensor* tempSensor;
        const ASensor* pressureSensor;

    public:
        SensorData(float temp, float pres) {
            temperature = new float(temp);
            pressure = new float(pres);

            ASensorManager* sensorManager = ASensorManager_getInstance();

            tempSensor = ASensorManager_getDefaultSensor(sensorManager, ASENSOR_TYPE_AMBIENT_TEMPERATURE);
            pressureSensor = ASensorManager_getDefaultSensor(sensorManager, ASENSOR_TYPE_PRESSURE);

            ALooper* looper = ALooper_forThread();
            if (looper == nullptr) {
                // If not already in a looper thread, create one
                looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
            }

            sensorEventQueue = ASensorManager_createEventQueue(sensorManager, looper, 1, SensorCallback, reinterpret_cast<void*>(this));

            // Set the event rate for the accelerometer sensor to 10000 microseconds (100 Hz)
            // Note: The actual rate may vary depending on the device and system load.
            const int32_t desiredRateUs = 10000; // 100 Hz

            if (tempSensor) {
                ASensorEventQueue_enableSensor(sensorEventQueue, tempSensor);
                ASensorEventQueue_setEventRate(sensorEventQueue, tempSensor, desiredRateUs);
            }
            if (pressureSensor) {
                ASensorEventQueue_enableSensor(sensorEventQueue, pressureSensor);
                ASensorEventQueue_setEventRate(sensorEventQueue, pressureSensor, desiredRateUs);
            }
        }

        static int SensorCallback(int fd, int events, void* data) {
            auto sensor = reinterpret_cast<SensorData*>(data);
            if (!sensor) return 0;

            ASensorEvent event;
            while (ASensorEventQueue_getEvents(sensor->sensorEventQueue, &event, 1) > 0) {
                if (event.type == ASENSOR_TYPE_AMBIENT_TEMPERATURE) {
                    *sensor->temperature = event.data[0]; // Value in Celsius
                } else if (event.type == ASENSOR_TYPE_PRESSURE) {
                    *sensor->pressure = event.data[0]; // Value in hPa (millibars)
                }
            }

            // Should return 1 to continue receiving callbacks
            return 1;
        }

        [[nodiscard]] std::string ToString() const {
            std::ostringstream ss;
            ss << "Temp: " << *temperature << ", Pres: " << *pressure;
            return ss.str();
        }

        ~SensorData() {
            delete temperature;
            delete pressure;

            ASensorEventQueue_disableSensor(sensorEventQueue, tempSensor);
            ASensorEventQueue_disableSensor(sensorEventQueue, pressureSensor);
            ASensorManager_destroyEventQueue(ASensorManager_getInstance(), sensorEventQueue);
        }
    };

} // namespace sensor

extern "C"
JNIEXPORT jstring JNICALL
Java_com_example_nativeapp_SensorBridge_getSensorData(JNIEnv* env, jobject /* this */) {
    sensor::SensorData data(23.5f, 1013.2f);
    std::string result = data.ToString();
    return env->NewStringUTF(result.c_str());
}
