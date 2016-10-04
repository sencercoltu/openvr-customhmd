using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace monitor_customhmd
{
    public class Gravity : Shape3d
    {
        public override Point3d[] pts { get; protected set; }

        protected override Point3d[] orig_pts { get; set; }

        float depthK = 1;
        float len;
        public Gravity(float a)
        {
            len = Math.Abs(a);
            pts = new Point3d[3];
            orig_pts = new Point3d[3];

            center = new Point3d(0, 0, 0);
            pts[0] = new Point3d(0, 0, -a); //x

            depthK = Math.Abs((18.0f / (float)(a *2)));

            Array.Copy(pts, orig_pts, pts.Length);
        }

        Pen gColor = new Pen(Color.LightGray, 3);


        Font f = new Font(FontFamily.GenericSansSerif, 8);
        Brush b = new SolidBrush(Color.LightGray);
        public override void Draw(Graphics g, Camera cam)
        {            
            PointF[] pts2d = cam.GetProjection(pts);
            g.DrawLine(gColor, (float)center.X, (float)center.Y, pts2d[0].X, pts2d[0].Y);
            g.DrawString(VecStr("G", pts[0]), new Font(FontFamily.GenericSansSerif, 8 + (float)((pts[0].Z + len) * depthK)), b, pts2d[0]);
        }
    }
}
