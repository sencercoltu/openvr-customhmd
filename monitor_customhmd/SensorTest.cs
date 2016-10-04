using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace monitor_customhmd
{
    public class SensorTest
    {
        public List<float[]> PointsRaw = new List<float[]>();
        public List<float[]> PointsFiltered = new List<float[]>();        

        public SensorTest()
        {
            PosScale = new float[3];
            NegScale = new float[3];
            //FastMedian = new float[3];
            //SlowMedian = new float[3];
            LastRaw = new Int16[3];
            Deviation = new float[3];
            Filtered = new float[3];
            Compensated = new float[3];
            Converted = new float[3];
            Raw = new Int16[3];
            Resolution = 1;
            for (int i = 0; i < 3; i++)
            {
                PosScale[i] =
                NegScale[i] = 1.0f;

                Raw[i] = 0;
                Compensated[i] = 
                Filtered[i] = 0.0f;
            }
        }

        public int MaxPoints = 100;

        public void SetResolution(float res)
        {
            Resolution = res;
        }

        public float Resolution;
        //public float[] FastMedian;
        //public float[] SlowMedian;
        public float[] PosScale;
        public float[] Deviation;
        public float[] NegScale;
        public Int16[] Raw;
        public Int16[] LastRaw;
        public float[] Filtered;
        public float[] Compensated;
        public float[] Converted;

        public void ProcessNew()
        {
            for (int i = 0; i < 1; i++)
            {
                Deviation[i] = (int)(Deviation[i] * 0.9f + Math.Abs(LastRaw[i] - Raw[i] + 30) * 0.1f);

                Compensated[i] = Raw[i] * ((Raw[i] > 0) ? PosScale[i] : NegScale[i]);
                //FastMedian[i] = FastMedian[i] * 0.3f + Compensated[i] * 0.7f;
                //SlowMedian[i] = SlowMedian[i] * 0.9f + Compensated[i] * 0.1f;
                
                

                if (Math.Abs(Filtered[i] - Compensated[i]) > Deviation[i])
                {
                    Filtered[i] = Compensated[i];
                    //Deviation[i] *= 0.5f;
                }

                Converted[i] = Filtered[i] * Resolution;

                LastRaw[i] = Raw[i];
            }

            PointsRaw.Add(new float[] { Compensated[0], Deviation[0], Filtered[0] });
            //PointsFiltered.Add(new float[] { Compensated[0], FastMedian[0], SlowMedian[0] });

            Debug.WriteLine("Compensated: {0}\tSlow: {1}\tFilt: {2}", Compensated[0], Deviation[0], Filtered[0]);

            if (PointsRaw.Count >= MaxPoints)
            {
                PointsRaw.RemoveAt(0);
                //PointsFiltered.RemoveAt(0);
            }
            
        }
    }
}
