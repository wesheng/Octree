#pragma once

class Mathf
{
public:
    template<typename T>
    inline static T clamp(T val, T min, T max)
    {
        return (val < min) ? min : ((val > max) ? max : val);
    }

};