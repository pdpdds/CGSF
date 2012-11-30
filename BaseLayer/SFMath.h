#pragma once

namespace SFMath
{
#define TOLERANCE 0.0001f

	class Vector2D
	{
	public:
		bool IsZeroVector(){return m_x == 0 && m_y == 0;}

////////////////////////////////////////////////////////////////
		Vector2D(){m_x = 0; m_y = 0;}
		Vector2D(float x, float y){m_x = x; m_y = y;}
		Vector2D(Vector2D& rhs){m_x = rhs.m_x; m_y = rhs.m_y;}

		float GetX(){return m_x;}
		float SetX(float x){m_x = x;}

		float GetY(){return m_y;}
		float SetY(float x){m_y = x;}

		Vector2D& operator = (Vector2D& rhs);

		Vector2D operator += (Vector2D& rhs);
		Vector2D operator -= (Vector2D& rhs);
		Vector2D operator *= (float Scalar);
		Vector2D operator /= (float Scalar);


		Vector2D operator + (Vector2D& rhs);
		Vector2D operator - (Vector2D& rhs);
		Vector2D operator * (float Scalar);
		Vector2D operator / (float Scalar);

		float Dot(Vector2D& rhs);
		float Norm();
		Vector2D Normalize(float tolerance = TOLERANCE);
		Vector2D Projection(Vector2D& rhs); // 이 벡터에 대한 투영
		Vector2D Reflection(Vector2D& rhs); // 이 벡터에 대한 반사


	protected:

	private:
		float m_x;
		float m_y;
	};

	inline Vector2D& Vector2D::operator = (Vector2D& rhs){ m_x = rhs.m_x; m_y = rhs.m_y; return *this;}
	
	inline Vector2D Vector2D::operator += (Vector2D& rhs){ m_x += rhs.m_x; m_y += rhs.m_y; return *this;}
	inline Vector2D Vector2D::operator -= (Vector2D& rhs){ m_x -= rhs.m_x; m_y -= rhs.m_y; return *this;}
	inline Vector2D Vector2D::operator *= (float Scalar){return Vector2D(m_x * Scalar, m_y * Scalar);}
	inline Vector2D Vector2D::operator /= (float Scalar){assert(Scalar != 0); return Vector2D(m_x / Scalar, m_y / Scalar);}

	inline Vector2D Vector2D::operator + (Vector2D& rhs){return Vector2D(m_x + rhs.m_x, m_y + rhs.m_y);}
	inline Vector2D Vector2D::operator - (Vector2D& rhs){return Vector2D(m_x - rhs.m_x, m_y - rhs.m_y);}
	inline Vector2D Vector2D::operator * (float Scalar){return Vector2D(m_x * Scalar, m_y * Scalar);}
	inline Vector2D Vector2D::operator / (float Scalar){assert(Scalar != 0); return Vector2D(m_x / Scalar, m_y / Scalar);}

	inline float Vector2D::Dot(Vector2D& rhs)
	{
		return (m_x * rhs.m_x) + (m_y * rhs.m_y); 
	}

	inline float Vector2D::Norm()
	{
		return sqrtf((m_x * m_x) + (m_y * m_y));
	}

	inline Vector2D Vector2D::Normalize(float tolerance)
	{
		float fNorm = Norm();
		
		if(fNorm < tolerance)
			return Vector2D(0.0f, 0.0f);

		return Vector2D(m_x / fNorm, m_y / fNorm);
	}

	inline Vector2D Vector2D::Projection(Vector2D& rhs)
	{
		Vector2D NormalVector = Normalize();

		if(NormalVector.IsZeroVector())
		{
			return NormalVector;
		}
		
		float DotProduct = rhs.Dot(NormalVector);

		return NormalVector * DotProduct;
	}

	inline Vector2D Vector2D::Reflection(Vector2D& rhs)
	{
		Vector2D NormalVector = Normalize();

		if(NormalVector.IsZeroVector())
		{
			return NormalVector;
		}

		float DotProduct = rhs.Dot(NormalVector);

		return rhs - (NormalVector * DotProduct * 2);
	}

	class Vector3D
	{
	public:

		bool IsZeroVector(){return m_x == 0 && m_y == 0 && m_y == 0;}
//////////////////////////////////////////////////////////////////////////////
		Vector3D(){m_x = 0; m_y = 0; m_z = 0;}
		Vector3D(float x, float y, float z){m_x = x; m_y = y; m_z = z;}
		Vector3D(Vector3D& rhs){m_x = rhs.m_x; m_y = rhs.m_y; m_z = rhs.m_z;}

		float GetX(){return m_x;}
		float SetX(float x){m_x = x;}

		float GetY(){return m_y;}
		float SetY(float x){m_y = x;}

		Vector3D& operator = (Vector3D& rhs);

		Vector3D operator += (Vector3D& rhs);
		Vector3D operator -= (Vector3D& rhs);
		Vector3D operator *= (float Scalar);
		Vector3D operator /= (float Scalar);

		Vector3D operator + (Vector3D& rhs);
		Vector3D operator - (Vector3D& rhs);
		Vector3D operator * (float Scalar);
		Vector3D operator / (float Scalar);

		float Dot(Vector3D& rhs);
		float Norm();
		Vector3D Cross(Vector3D& rhs);
		Vector3D Normalize(float tolerance = 0.0001f);
		Vector3D Projection(Vector3D& rhs);
		Vector3D Reflection(Vector3D& rhs); // 이 벡터에 대한 반사

	protected:

	private:
		float m_x;
		float m_y;
		float m_z;
	};

	inline Vector3D& Vector3D::operator = (Vector3D& rhs){ m_x = rhs.m_x; m_y = rhs.m_y; m_z = rhs.m_z; return *this;}

	inline Vector3D Vector3D::operator += (Vector3D& rhs){ m_x += rhs.m_x; m_y += rhs.m_y; m_z += rhs.m_z; return *this;}
	inline Vector3D Vector3D::operator -= (Vector3D& rhs){ m_x -= rhs.m_x; m_y -= rhs.m_y; m_z -= rhs.m_z; return *this;}
	inline Vector3D Vector3D::operator *= (float Scalar){return Vector3D(m_x * Scalar, m_y * Scalar,  m_z * Scalar);}
	inline Vector3D Vector3D::operator /= (float Scalar){assert(Scalar != 0); return Vector3D(m_x / Scalar, m_y / Scalar,  m_z / Scalar);}
	

	inline Vector3D Vector3D::operator + (Vector3D& rhs){return Vector3D(m_x + rhs.m_x, m_y + rhs.m_y, m_z + rhs.m_z);}
	inline Vector3D Vector3D::operator - (Vector3D& rhs){return Vector3D(m_x - rhs.m_x, m_y - rhs.m_y, m_z - rhs.m_z);}
	inline Vector3D Vector3D::operator * (float Scalar){return Vector3D(m_x * Scalar, m_y * Scalar, m_z / Scalar);}
	inline Vector3D Vector3D::operator / (float Scalar){assert(Scalar != 0); return Vector3D(m_x / Scalar, m_y / Scalar, m_z / Scalar);}

	inline float Vector3D::Dot(Vector3D& rhs)
	{
		return (m_x * rhs.m_x) + (m_y * rhs.m_y) + (m_z * rhs.m_z); 
	}

	inline float Vector3D::Norm()
	{
		return sqrtf((m_x * m_x) + (m_y * m_y) + (m_z * m_z));
	}

	inline Vector3D Vector3D::Cross(Vector3D& rhs)
	{
		return Vector3D(m_y * rhs.m_z - m_z * rhs.m_y, m_z * rhs.m_x - m_x * rhs.m_z, m_x * rhs.m_y - m_y * rhs.m_x);
	}

	inline Vector3D Vector3D::Normalize(float tolerance)
	{
		float fNorm = Norm();
		
		if(fNorm < tolerance)
			return Vector3D(0.0f, 0.0f, 0.0f);

		return Vector3D(m_x / fNorm, m_y / fNorm,  m_z / fNorm);
	}

	inline Vector3D Vector3D::Projection(Vector3D& rhs)
	{
		Vector3D NormalVector = Normalize();

		if(NormalVector.IsZeroVector())
		{
			return NormalVector;
		}
		
		float DotProduct = rhs.Dot(NormalVector);

		return NormalVector * DotProduct;
	}

	inline Vector3D Vector3D::Reflection(Vector3D& rhs)
	{
		Vector3D NormalVector = Normalize();

		if(NormalVector.IsZeroVector())
		{
			return NormalVector;
		}

		float DotProduct = rhs.Dot(NormalVector);

		return rhs - (NormalVector * DotProduct * 2);
	}

/////////////////////////////////////////////////////////////////////////////////////////
//Matrix 2x2
/////////////////////////////////////////////////////////////////////////////////////////
	class Matrix2x2
	{
	public:
		Matrix2x2(){SetIdentity();}
		Matrix2x2(float InitArray[2][2])
		{
			m_Elements[0][0] = InitArray[0][0];
			m_Elements[0][1] = InitArray[0][1];
			m_Elements[1][0] = InitArray[1][0];
			m_Elements[1][1] = InitArray[1][1];

		}

		Matrix2x2(float E00, float E01, float E10, float E11)
		{
			m_Elements[0][0] = E00;
			m_Elements[0][1] = E01;
			m_Elements[1][0] = E10;
			m_Elements[1][1] = E11;
		}

		void SetElement(UINT Row, UINT Col, float Value)
		{
			if(Row > 1 || Col > 1)
			{
				SFASSERT(0);
				return;
			}
				
			m_Elements[Row][Col] = Value;
		}

		float GetElement(int Row, int Col)
		{
			if(Row > 1 || Col > 1)
			{
				SFASSERT(0);
				return 0.0f;
			}

			return m_Elements[Row][Col];
		}

		void SetIdentity()
		{
			m_Elements[0][0] = 1;
			m_Elements[0][1] = 0;
			m_Elements[1][0] = 0;
			m_Elements[1][1] = 1;
		}

		Matrix2x2& operator = (float InitArray[2][2]);

		Matrix2x2 operator + (Matrix2x2& rhs);
		Matrix2x2 operator - (Matrix2x2& rhs);
		Matrix2x2 operator * (float Scalar);
		Matrix2x2 operator / (float Scalar);

		Matrix2x2 operator += (Matrix2x2& rhs);
		Matrix2x2 operator -= (Matrix2x2& rhs);
		Matrix2x2 operator *= (float Scalar);
		Matrix2x2 operator /= (float Scalar);

		Matrix2x2 operator * (Matrix2x2& rhs);

		Matrix2x2 Transpose();
		float Determinant();
		Matrix2x2 Inverse();
		
	protected:

	private:
		float m_Elements[2][2];
	};

	inline Matrix2x2& Matrix2x2::operator = (float InitArray[2][2])
	{ 
		m_Elements[0][0] = InitArray[0][0];
		m_Elements[0][1] = InitArray[0][1];
		m_Elements[1][0] = InitArray[1][0];
		m_Elements[1][1] = InitArray[1][1];

		return *this;
	}

	inline Matrix2x2 Matrix2x2::operator + (Matrix2x2& rhs)
	{
		m_Elements[0][0] += rhs.m_Elements[0][0];
		m_Elements[0][1] += rhs.m_Elements[0][1];
		m_Elements[1][0] += rhs.m_Elements[1][0];
		m_Elements[1][1] += rhs.m_Elements[1][1];

		return Matrix2x2(m_Elements);
	}

	inline Matrix2x2 Matrix2x2::operator - (Matrix2x2& rhs)
	{
		m_Elements[0][0] -= rhs.m_Elements[0][0];
		m_Elements[0][1] -= rhs.m_Elements[0][1];
		m_Elements[1][0] -= rhs.m_Elements[1][0];
		m_Elements[1][1] -= rhs.m_Elements[1][1];

		return Matrix2x2(m_Elements);
	}

	inline Matrix2x2 Matrix2x2::operator * (float Scalar)
	{
		m_Elements[0][0] *= Scalar;
		m_Elements[0][1] *= Scalar;
		m_Elements[1][0] *= Scalar;
		m_Elements[1][1] *= Scalar;

		return Matrix2x2(m_Elements);
	}

	inline Matrix2x2 Matrix2x2::operator / (float Scalar)
	{
		if(Scalar >= 0.0f && Scalar < TOLERANCE)
			return *this;

		m_Elements[0][0] /= Scalar;
		m_Elements[0][1] /= Scalar;
		m_Elements[1][0] /= Scalar;
		m_Elements[1][1] /= Scalar;

		return Matrix2x2(m_Elements);
	}
	
	inline Matrix2x2 Matrix2x2::operator += (Matrix2x2& rhs)
	{
		m_Elements[0][0] += rhs.m_Elements[0][0];
		m_Elements[0][1] += rhs.m_Elements[0][1];
		m_Elements[1][0] += rhs.m_Elements[1][0];
		m_Elements[1][1] += rhs.m_Elements[1][1];

		return *this;
	}

	inline Matrix2x2 Matrix2x2::operator -= (Matrix2x2& rhs)
	{
		m_Elements[0][0] -= rhs.m_Elements[0][0];
		m_Elements[0][1] -= rhs.m_Elements[0][1];
		m_Elements[1][0] -= rhs.m_Elements[1][0];
		m_Elements[1][1] -= rhs.m_Elements[1][1];

		return *this;
	}

	inline Matrix2x2 Matrix2x2::operator *= (float Scalar)
	{
		m_Elements[0][0] *= Scalar;
		m_Elements[0][1] *= Scalar;
		m_Elements[1][0] *= Scalar;
		m_Elements[1][1] *= Scalar;

		return *this;
	}

	inline Matrix2x2 Matrix2x2::operator /= (float Scalar)
	{
		if(Scalar >= 0.0f && Scalar < TOLERANCE)
			return *this;

		m_Elements[0][0] /= Scalar;
		m_Elements[0][1] /= Scalar;
		m_Elements[1][0] /= Scalar;
		m_Elements[1][1] /= Scalar;

		return *this;
	}

	inline Matrix2x2 Matrix2x2::operator * (Matrix2x2& rhs)
	{
		return Matrix2x2(m_Elements[0][0] * rhs.m_Elements[0][0] + m_Elements[0][1] * rhs.m_Elements[1][0],
						 m_Elements[0][0] * rhs.m_Elements[0][1] + m_Elements[0][1] * rhs.m_Elements[1][1],
						 m_Elements[1][0] * rhs.m_Elements[0][0] + m_Elements[1][1] * rhs.m_Elements[1][0],
						 m_Elements[1][0] * rhs.m_Elements[0][1] + m_Elements[1][1] * rhs.m_Elements[1][1]);
	}

	inline Matrix2x2 Matrix2x2::Transpose()
	{
		Matrix2x2 Matrix;
		Matrix.m_Elements[0][0] = m_Elements[0][0];
		Matrix.m_Elements[0][1] = m_Elements[1][0];
		Matrix.m_Elements[1][0] = m_Elements[0][1];
		Matrix.m_Elements[1][1] = m_Elements[1][1];
				
		return Matrix;

	}

	inline float Matrix2x2::Determinant()
	{
		return m_Elements[0][0] * m_Elements[1][1] - m_Elements[0][1] * m_Elements[1][0];
	}

	inline Matrix2x2 Matrix2x2::Inverse()
	{
		float Det = Determinant();

		if(Det == 0.0f)
		{
			SFASSERT(0);
			return Matrix2x2();
		}

		return Matrix2x2(m_Elements[1][1] / Det, -m_Elements[0][1] / Det, -m_Elements[1][0] / Det, m_Elements[0][0] / Det);
	}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Matrix 3x3
////////////////////////////////////////////////////////////////////////////////////////////////////
	class Matrix3x3
	{
	public:
		Matrix3x3(){SetIdentity();}
		Matrix3x3(float InitArray[3][3])
		{
			m_Elements[0][0] = InitArray[0][0];
			m_Elements[0][1] = InitArray[0][1];
			m_Elements[0][2] = InitArray[0][2];
			m_Elements[1][0] = InitArray[1][0];
			m_Elements[1][1] = InitArray[1][1];
			m_Elements[1][2] = InitArray[1][2];
			m_Elements[2][0] = InitArray[2][0];
			m_Elements[2][1] = InitArray[2][1];
			m_Elements[2][2] = InitArray[2][2];
		}

		Matrix3x3(float E00, float E01, float E02, float E10, float E11,float E12, float E20, float E21, float E22)
		{
			m_Elements[0][0] = E00;
			m_Elements[0][1] = E01;
			m_Elements[0][2] = E02;
			m_Elements[1][0] = E10;
			m_Elements[1][1] = E11;
			m_Elements[1][2] = E12;
			m_Elements[2][0] = E20;
			m_Elements[2][1] = E21;
			m_Elements[2][2] = E22;
		}

		void SetElement(UINT Row, UINT Col, float Value)
		{
			if(Row > 2 || Col > 2)
			{
				SFASSERT(0);
				return;
			}
				
			m_Elements[Row][Col] = Value;
		}

		float GetElement(int Row, int Col)
		{
			if(Row > 1 || Col > 1)
			{
				SFASSERT(0);
				return 0.0f;
			}

			return m_Elements[Row][Col];
		}

		void SetIdentity()
		{
			m_Elements[0][0] = 1;
			m_Elements[0][1] = 0;
			m_Elements[0][2] = 0;
			m_Elements[1][0] = 0;
			m_Elements[1][1] = 1;
			m_Elements[1][2] = 0;
			m_Elements[2][0] = 0;
			m_Elements[2][1] = 0;
			m_Elements[2][2] = 1;
		}

		Matrix3x3& operator = (float InitArray[3][3]);

		Matrix3x3 operator + (Matrix3x3& rhs);
		Matrix3x3 operator - (Matrix3x3& rhs);
		Matrix3x3 operator * (float Scalar);
		Matrix3x3 operator / (float Scalar);

		Matrix3x3 operator += (Matrix3x3& rhs);
		Matrix3x3 operator -= (Matrix3x3& rhs);
		Matrix3x3 operator *= (float Scalar);
		Matrix3x3 operator /= (float Scalar);

		Matrix3x3 operator * (Matrix3x3& rhs);

		Matrix3x3 Transpose();
		float Determinant();
		Matrix3x3 Inverse();
		
	protected:

	private:
		float m_Elements[3][3];
	};

	inline Matrix3x3& Matrix3x3::operator = (float InitArray[3][3])
	{ 
		m_Elements[0][0] = InitArray[0][0];
		m_Elements[0][1] = InitArray[0][1];
		m_Elements[0][2] = InitArray[0][2];
		m_Elements[1][0] = InitArray[1][0];
		m_Elements[1][1] = InitArray[1][1];
		m_Elements[1][2] = InitArray[1][2];
		m_Elements[2][0] = InitArray[2][0];
		m_Elements[2][1] = InitArray[2][1];
		m_Elements[2][2] = InitArray[2][2];

		return *this;
	}

	inline Matrix3x3 Matrix3x3::operator + (Matrix3x3& rhs)
	{
		m_Elements[0][0] += rhs.m_Elements[0][0];
		m_Elements[0][1] += rhs.m_Elements[0][1];
		m_Elements[0][2] += rhs.m_Elements[0][2];
		m_Elements[1][0] += rhs.m_Elements[1][0];
		m_Elements[1][1] += rhs.m_Elements[1][1];
		m_Elements[1][2] += rhs.m_Elements[1][2];
		m_Elements[2][0] += rhs.m_Elements[2][0];
		m_Elements[2][1] += rhs.m_Elements[2][1];
		m_Elements[2][2] += rhs.m_Elements[2][2];

		return Matrix3x3(m_Elements);
	}

	inline Matrix3x3 Matrix3x3::operator - (Matrix3x3& rhs)
	{
		m_Elements[0][0] -= rhs.m_Elements[0][0];
		m_Elements[0][1] -= rhs.m_Elements[0][1];
		m_Elements[0][2] -= rhs.m_Elements[0][2];
		m_Elements[1][0] -= rhs.m_Elements[1][0];
		m_Elements[1][1] -= rhs.m_Elements[1][1];
		m_Elements[1][2] -= rhs.m_Elements[1][2];
		m_Elements[2][0] -= rhs.m_Elements[2][0];
		m_Elements[2][1] -= rhs.m_Elements[2][1];
		m_Elements[2][2] -= rhs.m_Elements[2][2];

		return Matrix3x3(m_Elements);
	}

	inline Matrix3x3 Matrix3x3::operator * (float Scalar)
	{
		m_Elements[0][0] *= Scalar;
		m_Elements[0][1] *= Scalar;
		m_Elements[0][2] *= Scalar;
		m_Elements[1][0] *= Scalar;
		m_Elements[1][1] *= Scalar;
		m_Elements[1][2] *= Scalar;
		m_Elements[2][0] *= Scalar;
		m_Elements[2][1] *= Scalar;
		m_Elements[2][2] *= Scalar;

		return Matrix3x3(m_Elements);
	}

	inline Matrix3x3 Matrix3x3::operator / (float Scalar)
	{
		if(Scalar >= 0.0f && Scalar < TOLERANCE)
			return *this;

		m_Elements[0][0] /= Scalar;
		m_Elements[0][1] /= Scalar;
		m_Elements[0][2] /= Scalar;
		m_Elements[1][0] /= Scalar;
		m_Elements[1][1] /= Scalar;
		m_Elements[1][2] /= Scalar;
		m_Elements[2][0] /= Scalar;
		m_Elements[2][1] /= Scalar;
		m_Elements[2][2] /= Scalar;

		return Matrix3x3(m_Elements);
	}
	
	inline Matrix3x3 Matrix3x3::operator += (Matrix3x3& rhs)
	{
		m_Elements[0][0] += rhs.m_Elements[0][0];
		m_Elements[0][1] += rhs.m_Elements[0][1];
		m_Elements[0][2] += rhs.m_Elements[0][2];
		m_Elements[1][0] += rhs.m_Elements[1][0];
		m_Elements[1][1] += rhs.m_Elements[1][1];
		m_Elements[1][2] += rhs.m_Elements[1][2];
		m_Elements[2][0] += rhs.m_Elements[2][0];
		m_Elements[2][1] += rhs.m_Elements[2][1];
		m_Elements[2][2] += rhs.m_Elements[2][2];

		return *this;
	}

	inline Matrix3x3 Matrix3x3::operator -= (Matrix3x3& rhs)
	{
		m_Elements[0][0] -= rhs.m_Elements[0][0];
		m_Elements[0][1] -= rhs.m_Elements[0][1];
		m_Elements[0][2] -= rhs.m_Elements[0][2];
		m_Elements[1][0] -= rhs.m_Elements[1][0];
		m_Elements[1][1] -= rhs.m_Elements[1][1];
		m_Elements[1][2] -= rhs.m_Elements[1][2];
		m_Elements[2][0] -= rhs.m_Elements[2][0];
		m_Elements[2][1] -= rhs.m_Elements[2][1];
		m_Elements[2][2] -= rhs.m_Elements[2][2];

		return *this;
	}

	inline Matrix3x3 Matrix3x3::operator *= (float Scalar)
	{
		m_Elements[0][0] *= Scalar;
		m_Elements[0][1] *= Scalar;
		m_Elements[0][2] *= Scalar;
		m_Elements[1][0] *= Scalar;
		m_Elements[1][1] *= Scalar;
		m_Elements[1][2] *= Scalar;
		m_Elements[2][0] *= Scalar;
		m_Elements[2][1] *= Scalar;
		m_Elements[2][2] *= Scalar;

		return *this;
	}

	inline Matrix3x3 Matrix3x3::operator /= (float Scalar)
	{
		if(Scalar >= 0.0f && Scalar < TOLERANCE)
			return *this;

		m_Elements[0][0] /= Scalar;
		m_Elements[0][1] /= Scalar;
		m_Elements[0][2] /= Scalar;
		m_Elements[1][0] /= Scalar;
		m_Elements[1][1] /= Scalar;
		m_Elements[1][2] /= Scalar;
		m_Elements[2][0] /= Scalar;
		m_Elements[2][1] /= Scalar;
		m_Elements[2][2] /= Scalar;

		return *this;
	}

	inline Matrix3x3 Matrix3x3::operator * (Matrix3x3& rhs)
	{
		return Matrix3x3(m_Elements[0][0] * rhs.m_Elements[0][0] + m_Elements[0][1] * rhs.m_Elements[1][0] + m_Elements[0][2] * rhs.m_Elements[2][0],
						 m_Elements[0][0] * rhs.m_Elements[0][1] + m_Elements[0][1] * rhs.m_Elements[1][1] + m_Elements[0][2] * rhs.m_Elements[2][1],
						 m_Elements[0][0] * rhs.m_Elements[0][2] + m_Elements[0][1] * rhs.m_Elements[1][2] + m_Elements[0][2] * rhs.m_Elements[2][2],
						 
						 m_Elements[1][0] * rhs.m_Elements[0][0] + m_Elements[1][1] * rhs.m_Elements[1][0] + m_Elements[1][2] * rhs.m_Elements[2][0],
						 m_Elements[1][0] * rhs.m_Elements[0][1] + m_Elements[1][1] * rhs.m_Elements[1][1] + m_Elements[1][2] * rhs.m_Elements[2][1],
						 m_Elements[1][0] * rhs.m_Elements[0][2] + m_Elements[1][1] * rhs.m_Elements[1][2] + m_Elements[1][2] * rhs.m_Elements[2][2],
						 
						 m_Elements[2][0] * rhs.m_Elements[0][0] + m_Elements[2][1] * rhs.m_Elements[1][0] + m_Elements[2][2] * rhs.m_Elements[2][0],
						 m_Elements[2][0] * rhs.m_Elements[0][1] + m_Elements[2][1] * rhs.m_Elements[1][1] + m_Elements[2][2] * rhs.m_Elements[2][1],
						 m_Elements[2][0] * rhs.m_Elements[0][2] + m_Elements[2][1] * rhs.m_Elements[1][2] + m_Elements[2][2] * rhs.m_Elements[2][2]);
	}

	inline Matrix3x3 Matrix3x3::Transpose()
	{
		Matrix3x3 Matrix;
		Matrix.m_Elements[0][0] = m_Elements[0][0];
		Matrix.m_Elements[0][1] = m_Elements[1][0];
		Matrix.m_Elements[0][2] = m_Elements[2][0];
		Matrix.m_Elements[1][0] = m_Elements[0][1];
		Matrix.m_Elements[1][1] = m_Elements[1][1];
		Matrix.m_Elements[1][2] = m_Elements[2][1];
		Matrix.m_Elements[2][0] = m_Elements[0][2];
		Matrix.m_Elements[2][1] = m_Elements[1][2];
		Matrix.m_Elements[2][2] = m_Elements[2][2];
	
		return Matrix;

	}

	inline float Matrix3x3::Determinant()
	{
		return m_Elements[0][0] * m_Elements[1][1] * m_Elements[2][2] - 
			   m_Elements[0][0] * m_Elements[2][1] * m_Elements[1][2] + 
			   m_Elements[1][0] * m_Elements[2][1] * m_Elements[0][2] - 
			   m_Elements[1][0] * m_Elements[0][1] * m_Elements[2][2] + 
			   m_Elements[2][0] * m_Elements[0][1] * m_Elements[1][2] - 
			   m_Elements[2][0] * m_Elements[1][1] * m_Elements[0][2]; 
	}

	/*inline Matrix3x3 Matrix3x3::Inverse()
	{
		float Det = Determinant();

		if(Det == 0.0f)
		{
			SFASSERT(0);
			return Matrix3x3();
		}

		return Matrix3x3(m_Elements[1][1] / Det, -m_Elements[0][1] / Det, -m_Elements[1][0] / Det, m_Elements[0][0] / Det);
	}*/
};