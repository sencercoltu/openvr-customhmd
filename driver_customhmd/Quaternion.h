//****************************************************
//* quaternion.h                                     *
//*                                                  *
//* Implementaion for a generalized quaternion struct *   
//*                                                  *
//* Written 1.25.00 by Angela Bennett                *
//****************************************************


#ifndef _QUATERNION_H_
#define _QUATERNION_H_

#define _USE_MATH_DEFINES

//#include <iostream.h>
#include "Common.h"
#include <math.h>

struct Quaternion : vr::HmdQuaternion_t
{
	//Quaternion
	// -default constructor
	// -creates a new quaternion with all parts equal to zero
	Quaternion(void);

	//Quaternion
	// -constructor
	// -parametes : w, x, y, z elements of the quaternion
	// -creates a new quaternion based on the elements passed in
	Quaternion(double wi, double xi, double yi, double zi);

	//Quaternion
	// -constructor
	// -parameters : 4D vector
	// -creates a new quaternion based on the elements passed in
	Quaternion(double v[4]);
	Quaternion(float v[4]);

	//Quaternion
	// -copy constructor
	// -parameters : const quaternion q
	// -creates a new quaternion based on the quaternion passed in
	Quaternion(const Quaternion& q);

	Quaternion(const vr::HmdQuaternion_t& q);

	//~Quaternion
	// -default destructor
	~Quaternion();

	//operator=
	// -parameters : q1- Quaternion object
	// -return values : Quaternion
	// -when called on quaternion q2 sets q2 to be an object of  q3 
	Quaternion operator = (const Quaternion& q);

	//operator+
	// -parameters : q1 - Quaternion object
	// -return value : Quaternion 
	// -when called on quaternion q2 adds q1 + q2 and returns the sum in a new quaternion
	Quaternion operator + (const Quaternion& q);

	//operator-
	// -parameters : q1- Quaternion object
	// -return values : Quaternion 
	// -when called on q1 subtracts q1 - q2 and returns the difference as a new quaternion
	Quaternion operator - (const Quaternion& q);

	//operator*
	// -parameters : q1 - Quaternion object
	// -return values : Quaternion 
	// -when called on a quaternion q2, multiplies q2 *q1  and returns the product in a new quaternion 
	Quaternion operator * (const Quaternion& q);

	//operator/
	// -parameters : q1 and q2- Quaternion objects
	// -return values : Quaternion 
	// -divide q1 by q2 and returns the quotient as q1 
	Quaternion operator / (Quaternion& q);

	//operator+=
	// -parameters : q1- Quaternion object
	// -return values : Quaternion 
	// -when called on quaternion q3 adds q1 and q3 and returns the sum as q3 
	Quaternion& operator += (const Quaternion& q);

	//operator-=
	// -parameters : q1- Quaternion object
	// -return values : Quaternion 
	// -when called on quaternion q3, subtracts q1 from q3 and returns the difference as q3
	Quaternion& operator -= (const Quaternion& q);

	//operator*=
	// -parameters : q1- Quaternion object
	// -return values : Quaternion 
	// -when called on quaternion q3, multiplies q3 by q1 and returns the product as q3
	Quaternion& operator *= (const Quaternion& q);

	//operator/=
	// -parameters : q1- Quaternion object
	// -return values : quaternion
	// -when called on quaternion q3, divides q3 by q1 and returns the quotient as q3
	Quaternion& operator /= (Quaternion& q);

	//operator!=
	// -parameters : q1 and q2- Quaternion objects
	// -return value : bool
	// -determines if q1 and q2 and equal
	bool operator != (const Quaternion& q);

	//operator==
	// -parameters : q1 and q2- Quaternion objects
	// -return value : bool
	// -determines if q1 and q2 and equal
	bool operator == (const Quaternion& q);

	//other methods: norm, inverse, conjugate, toEuler

	//norm
	// -parameters : none
	// -return value : double
	// -when called on a quaternion object q, returns the norm of q
	double norm();

	//magnitude
	// -parameters : none
	// -return value : double
	// -when called on a quaternion object q, returns the magnitude q
	double magnitude();

	//scale
	// -parameters :  s- a value to scale q1 by
	// -return value: quaternion
	// -returns the original quaternion with each part, w,x,y,z, multiplied by some scalar s
	Quaternion scale(double s);

	//inverse
	// -parameters : none
	// -return value : quaternion
	// -when called on a quaternion object q, returns the inverse of q
	Quaternion inverse();

	//conjugate
	// -parameters : none
	// -return value : quaternion
	// -when called on a quaternion object q, returns the conjugate of q
	Quaternion conjugate();

	//UnitQuaternion
	// -parameters : none
	// -return value : quaternion
	// -when called on quaterion q, takes q and returns the unit quaternion of q
	Quaternion UnitQuaternion();

	// -parameters : 3D vector of type double
	// -return value : void
	// -when given a  3D vector, v, rotates v by the quaternion
	void QuatRotation(double v[3]);

	//// [w, (x, y, z)]
	//double w, x, y, z;

	vr::HmdVector3d_t ToEuler();

	static Quaternion FromEuler(const vr::HmdVector3d_t &e);

	static void HmdMatrix_SetIdentity(vr::HmdMatrix34_t *pMatrix);

	static void HmdMatrix_SetIdentity(vr::HmdMatrix44_t *pMatrix);
};

#endif