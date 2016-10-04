
#ifndef M_PI 
#define M_PI 3.14159265358979323846f 
#endif

#include <math.h>


enum PassType
{
	Highpass,
	Lowpass
};

struct Filter
{

private:
    float c, a1, a2, a3, b1, b2;

    /// <summary>
    /// Array of input values, latest are in front
    /// </summary>
    float inputHistory[2];

    /// <summary>
    /// Array of output values, latest are in front
    /// </summary>
    float outputHistory[3];

public:
    void Setup(PassType passType, float frequency, float sampleRate, float resonance)
    {
		/// rez amount, from sqrt(2) to ~ 0.1
        switch (passType)
        {
            case Lowpass:
                c = 1.0f / tan(M_PI * frequency / sampleRate);
                a1 = 1.0f / (1.0f + resonance * c + c * c);
                a2 = 2.0f * a1;
                a3 = a1;
                b1 = 2.0f * (1.0f - c * c) * a1;
                b2 = (1.0f - resonance * c + c * c) * a1;
                break;
            case Highpass:
                c = tan(M_PI * frequency / sampleRate);
                a1 = 1.0f / (1.0f + resonance * c + c * c);
                a2 = -2.0f * a1;
                a3 = a1;
                b1 = 2.0f * (c * c - 1.0f) * a1;
                b2 = (1.0f - resonance * c + c * c) * a1;
                break;
        }
    }

    float Update(float newInput)
    {
        float newOutput = a1 * newInput + a2 * inputHistory[0] + a3 * inputHistory[1] - b1 * outputHistory[0] - b2 * outputHistory[1];

        inputHistory[1] = inputHistory[0];
        inputHistory[0] = newInput;

        outputHistory[2] = outputHistory[1];
        outputHistory[1] = outputHistory[0];
        outputHistory[0] = newOutput;
		
		return outputHistory[0];
    }

};
