struct KalmanSingle
{        
	void SetParams(float process_noise, float sensor_noise)
	{
		q = process_noise;
		r = sensor_noise;
	}

	float q; //process noise covariance
	float r; //measurement noise covariance
	float x; //value
	float p; //estimation error covariance
	float k; //kalman gain
	
	KalmanSingle()
	{
		q=0.125;
		r=32;
		p=0;
		x=0;
	}
	
	KalmanSingle(float process_noise, float sensor_noise, float estimated_error, float intial_value)
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

	float GetFilteredValue(float measurement)
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
	
};
