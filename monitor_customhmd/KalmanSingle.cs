using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace monitor_customhmd
{
    public class KalmanGroup
    {
        public List<float[]> Points = new List<float[]>();

        public bool Enabled;

        private int MaxPoints = 100;

        public KalmanSingle X;
        public KalmanSingle Y;
        public KalmanSingle Z;

        public void SetParams(float process_noise, float sensor_noise)
        {
            X.SetParams(process_noise, sensor_noise);
            Y.SetParams(process_noise, sensor_noise);
            Z.SetParams(process_noise, sensor_noise);
        }


        public KalmanGroup(int maxPoints, float process_noise, float sensor_noise, float estimated_error, float intial_value)
        {
            MaxPoints = maxPoints;
            X = new KalmanSingle(process_noise, sensor_noise, estimated_error, intial_value);
            Y = new KalmanSingle(process_noise, sensor_noise, estimated_error, intial_value);
            Z = new KalmanSingle(process_noise, sensor_noise, estimated_error, intial_value);
        }
        
        public void Process(float[] values)
        {
            if (Enabled)
            {                
                values[0] = X.GetFilteredValue(values[0]);
                values[1] = Y.GetFilteredValue(values[1]);
                values[2] = Z.GetFilteredValue(values[2]);                
            }            
            Points.Add(values);
            if (Points.Count >= MaxPoints) Points.RemoveAt(0);
        }
    }

    public class KalmanSingle
    {        
        public void SetParams(float process_noise, float sensor_noise)
        {
            q = process_noise;
            r = sensor_noise;
        }

        private float q; //process noise covariance
        private float r; //measurement noise covariance
        private float x; //value
        private float p; //estimation error covariance
        private float k; //kalman gain
        
        public KalmanSingle(float process_noise, float sensor_noise, float estimated_error, float intial_value)
        {
            /* The variables are x for the filtered value, q for the process noise, 
                     r for the sensor noise, p for the estimated error and k for the Kalman Gain. 
                     The state of the filter is defined by the values of these variables.

                     The initial values for p is not very important since it is adjusted
                     during the process. It must be just high enough to narrow down.
                     The initial value for the readout is also not very important, since
                     it is updated during the process.
                     But tweaking the values for the process noise and sensor noise
                     is essential to get clear readouts.

                     For large noise reduction, you can try to start from: (see http://interactive-matter.eu/blog/2009/12/18/filtering-sensor-data-with-a-kalman-filter/ )
                     q = 0.125
                     r = 32
                     p = 1023 //"large enough to narrow down"
                     e.g.
                     myVar = Kalman(0.125,32,1023,0);
                  */
            q = process_noise;
            r = sensor_noise;
            p = estimated_error;
            x = intial_value; //x will hold the iterated filtered value
        }

        public float GetFilteredValue(float measurement)
        {
            /* Updates and gets the current measurement value */
            //prediction update
            //omit x = x
            p = p + q;

            //measurement update
            k = p / (p + r);
            x = x + k * (measurement - x);
            p = (1 - k) * p;

            return x;
        }
        
    }
}
