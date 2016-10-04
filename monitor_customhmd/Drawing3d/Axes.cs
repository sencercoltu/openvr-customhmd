using System;
using System.Collections.Generic;
using System.Drawing;

namespace monitor_customhmd
{
    public class Axes : Shape3d
    {
        public override Point3d[] pts { get; protected set; }

        protected override Point3d[] orig_pts { get; set; }

        float depthK = 1;
        float len;

        public Axes(float a)
        {
            len = Math.Abs(a);
            pts = new Point3d[3];
            orig_pts = new Point3d[3];

            center = new Point3d(0, 0 ,0);            
            pts[0] = new Point3d(-a, 0, 0); //x
            pts[1] = new Point3d(0, -a, 0); //y                      
            pts[2] = new Point3d(0, 0, a); //z

            depthK = Math.Abs((18.0f / (float)(a * 2)));
            Array.Copy(pts, orig_pts, pts.Length);
        }

        Pen xColor = new Pen(Color.Red, 3);
        Pen yColor = new Pen(Color.Green, 3);
        Pen zColor = new Pen(Color.Blue, 3);


        Font f = new Font(FontFamily.GenericSansSerif, 8);
        Brush b = new SolidBrush(Color.White);
        public override void Draw(Graphics g, Camera cam)
        {            
            PointF[] pts2d = cam.GetProjection(pts);
            g.DrawLine(xColor, (float)center.X, (float)center.Y, pts2d[0].X, pts2d[0].Y);
            g.DrawLine(yColor, (float)center.X, (float)center.Y, pts2d[1].X, pts2d[1].Y);
            g.DrawLine(zColor, (float)center.X, (float)center.Y, pts2d[2].X, pts2d[2].Y);
            g.DrawString(VecStr("X", pts[0]), new Font(FontFamily.GenericSansSerif, 8 + (float)((pts[0].Z + len) * depthK)), b, pts2d[0]);
            g.DrawString(VecStr("Y", pts[1]), new Font(FontFamily.GenericSansSerif, 8 + (float)((pts[1].Z + len) * depthK)), b, pts2d[1]);
            g.DrawString(VecStr("Z", pts[2]), new Font(FontFamily.GenericSansSerif, 8 + (float)((pts[2].Z + len) * depthK)), b, pts2d[2]);
        }

    }
}