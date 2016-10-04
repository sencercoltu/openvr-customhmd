using System;
using System.Collections.Generic;
using System.Drawing;

namespace monitor_customhmd
{
    public abstract class Shape3d
    {
        public abstract Point3d[] pts { get; protected set; }

        protected abstract Point3d[] orig_pts { get; set; }

        public Point3d[] Point3dArray
        {
            get { return pts; }
        }

        protected Point3d center = new Point3d(0, 0, 0);
        public Point3d Center
        {
            set
            {
                double dx = value.X - center.X;
                double dy = value.Y - center.Y;
                double dz = value.Z - center.Z;
                Point3d.Offset(pts, dx, dy, dz);
                Array.Copy(pts, orig_pts, pts.Length);
                center = value;
            }
            get { return center; }
        }

        protected Color lineColor = Color.Black;
        public Color LineColor
        {
            set { lineColor = value; }
            get { return lineColor; }
        }

        public void RotateAt(Point3d pt, Quaternion q)
        {
            // transform origin to pt
            Point3d[] copy = Point3d.Copy(orig_pts);
            Point3d.Offset(copy,  - pt.X,  - pt.Y,  - pt.Z);

            // rotate
            q.Rotate(copy);
            q.Rotate(center);

            // transform to original origin
            Point3d.Offset(copy, pt.X, pt.Y, pt.Z);
            pts = copy;
        }
        
        public virtual void Draw(Graphics g,Camera cam)
        {
        }

        protected string VecStr(string v, Point3d f)
        {
            var s = v + "(";
            s += f.X.ToString("N0") + "," + f.Y.ToString("N0") + "," + f.Z.ToString("N0");
            return s + ")";

        }

    }
}
