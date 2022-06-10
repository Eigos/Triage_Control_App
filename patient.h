#ifndef PATIENT_H
#define PATIENT_H

#include <string>
#include <vector>

struct Patient
{
public:
    Patient() : mID(-1), mHeartRate(0.f), mSpO2(0.f), mTemp(0.f) {};

    struct PressureData{
        PressureData() : mHighPresure(0.f), mLowPresure(0.f), mHeartRate(0.f) {};

        std::string getString(){
            std::string str = "h";
            str.append(std::to_string(mHighPresure));
            str.push_back('/');
            str.append(std::to_string(mLowPresure));
            str.push_back('/');
            str.append(std::to_string(mHeartRate));

            return {str};
        }

        float mHighPresure;
        float mLowPresure;
        float mHeartRate;
    };

    std::string mName;
    int32_t mID = -1;
    float mHeartRate = 0.f;
    float mSpO2 = 0.f;
    float mTemp = 0.f;
    std::string mSymptom;
    PressureData mPressureData;

};

#endif // PATIENT_H

/*{
    "HeartRate": 91.55000305175781,
    "ID": -1,
    "Name": "",
    "Pressure": {
        "HeartRate": 70.38999938964844,
        "HighPressure": 117.38999938964844,
        "LowPressure": 67.66999816894531
    },
    "SpO2": 103.94000244140625,
    "Temp": 52.349998474121094
}
*/
