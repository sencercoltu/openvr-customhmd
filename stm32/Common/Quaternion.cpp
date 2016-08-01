//****************************************************
//* quaternion.c++                                   *
//*                                                  *
//* Implementaion for a generalized quaternion struct *   
//*                                                  *
//* Written 1.25.00 by Angela Bennett                *
//****************************************************


#include "Quaternion.h"

//Quaternion
// -default constructor
// -creates a new quaternion with all parts equal to zero

Quaternion::Quaternion(void)
{
	x = 0;
	y = 0;
	z = 0;
	w = 1;
}


//Quaternion
// -constructor
// -parametes : x, y, z, w elements of the quaternion
// -creates a new quaternion based on the elements passed in

Quaternion::Quaternion(float wi, float xi, float yi, float zi)
{
	w = wi;
	x = xi;
	y = yi;
	z = zi;
}


//Quaternion
// -constructor
// -parameters : vector/array of four elements
// -creates a new quaternion based on the elements passed in

Quaternion::Quaternion(float v[4])
{
	w = v[0];
	x = v[1];
	y = v[2];
	z = v[3];
}


//Quaternion
// -copy constructor
// -parameters : const quaternion q
// -creates a new quaternion based on the quaternion passed in

Quaternion::Quaternion(const Quaternion& q)
{
	w = q.w;
	x = q.x;
	y = q.y;
	z = q.z;
}


//~Quaternion
// -destructor
// -deleted dynamically allocated memory

Quaternion::~Quaternion()
{
}


//operator=
// -parameters : q1 - Quaternion object
// -return value : Quaternion
// -when called on quaternion q2 sets q2 to be an object of  q3 

Quaternion Quaternion::operator = (const Quaternion& q)
{
	w = q.w;
	x = q.x;
	y = q.y;
	z = q.z;

	return (*this);
}

//operator+
// -parameters : q1 - Quaternion object
// -return value : Quaternion 
// -when called on quaternion q2 adds q1 + q2 and returns the sum in a new quaternion

Quaternion Quaternion::operator + (const Quaternion& q)
{
	return Quaternion(w + q.w, x + q.x, y + q.y, z + q.z);
}

//operator-
// -parameters : q1- Quaternion object
// -return values : Quaternion 
// -when called on q1 subtracts q1 - q2 and returns the difference as a new quaternion

Quaternion Quaternion::operator - (const Quaternion& q)
{
	return Quaternion(w - q.w, x - q.x, y - q.y, z - q.z);
}


//operator*
// -parameters : q1 - Quaternion object
// -return values : Quaternion 
// -when called on a quaternion q2, multiplies q2 *q1  and returns the product in a new quaternion 

Quaternion Quaternion::operator * (const Quaternion& q)
{
	return Quaternion(
		w*q.w - x*q.x - y*q.y - z*q.z,
		w*q.x + x*q.w + y*q.z - z*q.y,
		w*q.y + y*q.w + z*q.x - x*q.z,
		w*q.z + z*q.w + x*q.y - y*q.x);
}

//operator/
// -parameters : q1 and q2- Quaternion objects
// -return values : Quaternion 
// -divide q1 by q2 and returns the quotient q1

Quaternion Quaternion::operator / (Quaternion& q)
{
	return ((*this) * (q.inverse()));
}


//operator+=
// -parameters : q1- Quaternion object
// -return values : Quaternion 
// -when called on quaternion q3, adds q1 and q3 and returns the sum as q3

Quaternion& Quaternion::operator += (const Quaternion& q)
{
	w += q.w;
	x += q.x;
	y += q.y;
	z += q.z;

	return (*this);
}


//operator-=
// -parameters : q1- Quaternion object
// -return values : Quaternion 
// -when called on quaternion q3, subtracts q1 from q3 and returns the difference as q3

Quaternion& Quaternion::operator -= (const Quaternion& q)
{
	w -= q.w;
	x -= q.x;
	y -= q.y;
	z -= q.z;

	return (*this);
}


//operator*=
// -parameters : q1- Quaternion object
// -return values : Quaternion 
// -when called on quaternion q3, multiplies q3 by q1 and returns the product as q3

Quaternion& Quaternion::operator *= (const Quaternion& q)
{
	float w_val = w*q.w - x*q.x - y*q.y - z*q.z;
	float x_val = w*q.x + x*q.w + y*q.z - z*q.y;
	float y_val = w*q.y + y*q.w + z*q.x - x*q.z;
	float z_val = w*q.z + z*q.w + x*q.y - y*q.x;

	w = w_val;
	x = x_val;
	y = y_val;
	z = z_val;

	return (*this);
}


//operator/=
// -parameters : q1- Quaternion object
// -return values : quaternion
// -when called on quaternion q3, divides q3 by q1 and returns the quotient as q3

Quaternion& Quaternion::operator /= (Quaternion& q)
{
	(*this) = (*this)*q.inverse();
	return (*this);
}


//operator!=
// -parameters : q1 and q2- Quaternion objects
// -return value : bool
// -determines if q1 and q2 are not equal

bool Quaternion::operator != (const Quaternion& q)
{
	return (w != q.w || x != q.x || y != q.y || z != q.z) ? true : false;
}

//operator==
// -parameters : q1 and q2- Quaternion objects
// -return value : bool
// -determines if q1 and q2 are equal

bool Quaternion::operator == (const Quaternion& q)
{
	return (w == q.w && x == q.x && y == q.y && z == q.z) ? true : false;
}

//norm
// -parameters : none
// -return value : float
// -when called on a quaternion object q, returns the norm of q

float Quaternion::norm()
{
	return (w*w + x*x + y*y + z*z);
}

//magnitude
// -parameters : none
// -return value : float
// -when called on a quaternion object q, returns the magnitude q

float Quaternion::magnitude()
{
	return sqrtf(norm());
}

//scale
// -parameters :  s- a value to scale q1 by
// -return value: quaternion
// -returns the original quaternion with each part, w,x,y,z, multiplied by some scalar s

Quaternion Quaternion::scale(float  s)
{
	return Quaternion(w*s, x*s, y*s, z*s);
}

// -parameters : none
// -return value : quaternion
// -when called on a quaternion object q, returns the inverse of q

Quaternion Quaternion::inverse()
{
	return conjugate().scale(1 / norm());
}

//conjugate
// -parameters : none
// -return value : quaternion
// -when called on a quaternion object q, returns the conjugate of q

Quaternion Quaternion::conjugate()
{
	return Quaternion(w, -x, -y, -z);
}

//UnitQuaternion
// -parameters : none
// -return value : quaternion
// -when called on quaterion q, takes q and returns the unit quaternion of q

Quaternion Quaternion::UnitQuaternion()
{
	return (*this).scale(1 / (*this).magnitude());
}

// -parameters : vector of type float
// -return value : void
// -when given a 3D vector, v, rotates v by this quaternion

void Quaternion::QuatRotation(float v[3])
{
	Quaternion  qv(0, v[0], v[1], v[2]);
	Quaternion  qm = (*this) * qv * (*this).inverse();

	v[0] = qm.x;
	v[1] = qm.y;
	v[2] = qm.z;
}

Vector3f Quaternion::ToEuler()
{
	Vector3f e;

	float test = x*y + z*w;
	if (test > 0.499) { // singularity at north pole
		e.v[0] = 2 * atan2f(x, w);
		e.v[1] = M_PI / 2.0;
		e.v[2] = 0;
		return e;
	}
	if (test < -0.499) { // singularity at south pole
		e.v[0] = -2 * atan2f(x, w);
		e.v[1] = -M_PI / 2;
		e.v[2] = 0;
		return e;
	}
	float sqx = x*x;
	float sqy = y*y;
	float sqz = z*z;
	e.v[0] = atan2f(2 * y*w - 2 * x*z, 1 - 2 * sqy - 2 * sqz);
	e.v[1] = asinf(2 * test);
	e.v[2] = atan2f(2 * x*w - 2 * y*z, 1 - 2 * sqx - 2 * sqz);
	return e;
}

Quaternion Quaternion::FromEuler(const Vector3f &e)
{
	Quaternion q;
	// Assuming the angles are in radians.
	float c1 = cosf(e.v[0] / 2);
	float s1 = sinf(e.v[0] / 2);
	float c2 = cosf(e.v[1] / 2);
	float s2 = sinf(e.v[1] / 2);
	float c3 = cosf(e.v[2] / 2);
	float s3 = sinf(e.v[2] / 2);
	float c1c2 = c1*c2;
	float s1s2 = s1*s2;
	q.w = c1c2*c3 - s1s2*s3;
	q.x = c1c2*s3 + s1s2*c3;
	q.y = s1*c2*c3 + c1*s2*s3;
	q.z = c1*s2*c3 - s1*c2*s3;
	return q;
}
