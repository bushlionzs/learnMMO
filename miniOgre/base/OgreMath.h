#ifndef __Math_H__
#define __Math_H__

namespace Ogre
{
    class Radian
    {
        Real mRad;

    public:
        explicit Radian ( Real r=0 ) : mRad(r) {}
        Radian ( const Degree& d );
        Radian& operator = ( const Real& f ) { mRad = f; return *this; }
        Radian& operator = ( const Radian& r ) { mRad = r.mRad; return *this; }
        Radian& operator = ( const Degree& d );

        Real valueDegrees() const; // see bottom of this file
        Real valueRadians() const { return mRad; }
        Real valueAngleUnits() const;

        const Radian& operator + () const { return *this; }
        Radian operator + ( const Radian& r ) const { return Radian ( mRad + r.mRad ); }
        Radian operator + ( const Degree& d ) const;
        Radian& operator += ( const Radian& r ) { mRad += r.mRad; return *this; }
        Radian& operator += ( const Degree& d );
        Radian operator - () const { return Radian(-mRad); }
        Radian operator - ( const Radian& r ) const { return Radian ( mRad - r.mRad ); }
        Radian operator - ( const Degree& d ) const;
        Radian& operator -= ( const Radian& r ) { mRad -= r.mRad; return *this; }
        Radian& operator -= ( const Degree& d );
        Radian operator * ( Real f ) const { return Radian ( mRad * f ); }
        Radian operator * ( const Radian& f ) const { return Radian ( mRad * f.mRad ); }
        Radian& operator *= ( Real f ) { mRad *= f; return *this; }
        Radian operator / ( Real f ) const { return Radian ( mRad / f ); }
        Radian& operator /= ( Real f ) { mRad /= f; return *this; }

        bool operator <  ( const Radian& r ) const { return mRad <  r.mRad; }
        bool operator <= ( const Radian& r ) const { return mRad <= r.mRad; }
        bool operator == ( const Radian& r ) const { return mRad == r.mRad; }
        bool operator != ( const Radian& r ) const { return mRad != r.mRad; }
        bool operator >= ( const Radian& r ) const { return mRad >= r.mRad; }
        bool operator >  ( const Radian& r ) const { return mRad >  r.mRad; }

        inline  friend std::ostream& operator <<
            ( std::ostream& o, const Radian& v )
        {
            o << "Radian(" << v.valueRadians() << ")";
            return o;
        }
    };

    class Degree
    {
        Real mDeg;

    public:
        explicit Degree ( Real d=0 ) : mDeg(d) {}
        Degree ( const Radian& r ) : mDeg(r.valueDegrees()) {}
        Degree& operator = ( const Real& f ) { mDeg = f; return *this; }
        Degree& operator = ( const Degree& d ) { mDeg = d.mDeg; return *this; }
        Degree& operator = ( const Radian& r ) { mDeg = r.valueDegrees(); return *this; }

        Real valueDegrees() const { return mDeg; }
        Real valueRadians() const; // see bottom of this file
        Real valueAngleUnits() const;

        const Degree& operator + () const { return *this; }
        Degree operator + ( const Degree& d ) const { return Degree ( mDeg + d.mDeg ); }
        Degree operator + ( const Radian& r ) const { return Degree ( mDeg + r.valueDegrees() ); }
        Degree& operator += ( const Degree& d ) { mDeg += d.mDeg; return *this; }
        Degree& operator += ( const Radian& r ) { mDeg += r.valueDegrees(); return *this; }
        Degree operator - () const { return Degree(-mDeg); }
        Degree operator - ( const Degree& d ) const { return Degree ( mDeg - d.mDeg ); }
        Degree operator - ( const Radian& r ) const { return Degree ( mDeg - r.valueDegrees() ); }
        Degree& operator -= ( const Degree& d ) { mDeg -= d.mDeg; return *this; }
        Degree& operator -= ( const Radian& r ) { mDeg -= r.valueDegrees(); return *this; }
        Degree operator * ( Real f ) const { return Degree ( mDeg * f ); }
        Degree operator * ( const Degree& f ) const { return Degree ( mDeg * f.mDeg ); }
        Degree& operator *= ( Real f ) { mDeg *= f; return *this; }
        Degree operator / ( Real f ) const { return Degree ( mDeg / f ); }
        Degree& operator /= ( Real f ) { mDeg /= f; return *this; }

        bool operator <  ( const Degree& d ) const { return mDeg <  d.mDeg; }
        bool operator <= ( const Degree& d ) const { return mDeg <= d.mDeg; }
        bool operator == ( const Degree& d ) const { return mDeg == d.mDeg; }
        bool operator != ( const Degree& d ) const { return mDeg != d.mDeg; }
        bool operator >= ( const Degree& d ) const { return mDeg >= d.mDeg; }
        bool operator >  ( const Degree& d ) const { return mDeg >  d.mDeg; }

        inline  friend std::ostream& operator <<
            ( std::ostream& o, const Degree& v )
        {
            o << "Degree(" << v.valueDegrees() << ")";
            return o;
        }
    };

    class Angle
    {
        Real mAngle;
    public:
        explicit Angle ( Real angle ) : mAngle(angle) {}
        operator Radian() const;
        operator Degree() const;
    };

    inline Radian::Radian ( const Degree& d ) : mRad(d.valueRadians()) {
    }
    inline Radian& Radian::operator = ( const Degree& d ) {
        mRad = d.valueRadians(); return *this;
    }
    inline Radian Radian::operator + ( const Degree& d ) const {
        return Radian ( mRad + d.valueRadians() );
    }
    inline Radian& Radian::operator += ( const Degree& d ) {
        mRad += d.valueRadians();
        return *this;
    }
    inline Radian Radian::operator - ( const Degree& d ) const {
        return Radian ( mRad - d.valueRadians() );
    }
    inline Radian& Radian::operator -= ( const Degree& d ) {
        mRad -= d.valueRadians();
        return *this;
    }

    class  Math 
    {
    public:
       enum AngleUnit
       {
           AU_DEGREE,
           AU_RADIAN
       };
       class RandomValueProvider
       {
       public:
            virtual ~RandomValueProvider() {}

            virtual Real getRandomUnit() = 0;
       };

    protected:
        static AngleUnit msAngleUnit;

        static int mTrigTableSize;
        static Real mTrigTableFactor;
        static Real* mSinTable;
        static Real* mTanTable;

        static RandomValueProvider* mRandProvider;

        void buildTrigTables();

        static Real SinTable (Real fValue);
        static Real TanTable (Real fValue);
    public:
        Math(unsigned int trigTableSize = 4096);

        ~Math();

        static inline int IAbs (int iValue) { return ( iValue >= 0 ? iValue : -iValue ); }
        static inline int ICeil (float fValue) { return int(::ceil(fValue)); }
        static inline int IFloor (float fValue) { return int(floor(fValue)); }
        static int ISign (int iValue);
        static inline Real Abs (Real fValue) { return Real(fabs(fValue)); }

        static inline Degree Abs (const Degree& dValue) { return Degree(fabs(dValue.valueDegrees())); }

        static inline Radian Abs (const Radian& rValue) { return Radian(fabs(rValue.valueRadians())); }

        static Radian ACos (Real fValue);

        static Radian ASin (Real fValue);

   
        static inline Radian ATan (Real fValue) { return Radian(atan(fValue)); }

        static inline Radian ATan2 (Real fY, Real fX) { return Radian(atan2(fY,fX)); }

        static inline Real Ceil (Real fValue) { return Real(::ceil(fValue)); }
        static inline bool isNaN(Real f)
        {
            return f != f;
        }

        static inline Real Cos (const Radian& fValue, bool useTables = false) {
            return (!useTables) ? Real(cos(fValue.valueRadians())) : SinTable(fValue.valueRadians() + HALF_PI);
        }

        static inline Real Cos (Real fValue, bool useTables = false) {
            return (!useTables) ? Real(cos(fValue)) : SinTable(fValue + HALF_PI);
        }

        static inline Real Exp (Real fValue) { return Real(exp(fValue)); }

        static inline Real Floor (Real fValue) { return Real(floor(fValue)); }

        static inline Real Log (Real fValue) { return Real(log(fValue)); }

        static const Real LOG2;

        static inline Real Log2 (Real fValue) { return Real(log(fValue)/LOG2); }

        static inline Real LogN (Real base, Real fValue) { return Real(log(fValue)/log(base)); }

        static inline Real Pow (Real fBase, Real fExponent) { return Real(pow(fBase,fExponent)); }

        static Real Sign (Real fValue);
        static inline Radian Sign ( const Radian& rValue )
        {
            return Radian(Sign(rValue.valueRadians()));
        }
        static inline Degree Sign ( const Degree& dValue )
        {
            return Degree(Sign(dValue.valueDegrees()));
        }
        static inline float saturate(float t)
        {
            float tmp = std::max( t, 0.0f );
            tmp = std::min( tmp, 1.0f );
            return tmp;
        }
        static inline double saturate(double t)
        {
            double tmp = std::max( t, 0.0 );
            tmp = std::min( tmp, 1.0 );
            return tmp;
        }
        
        template<typename T, typename S> static  T lerp( const T& a, const T& b, const S& w )
        { 
            return a + w * (b - a);
        }

        static inline Real Sin (const Radian& fValue, bool useTables = false) {
            return (!useTables) ? Real(sin(fValue.valueRadians())) : SinTable(fValue.valueRadians());
        }
  
        static inline Real Sin (Real fValue, bool useTables = false) {
            return (!useTables) ? Real(sin(fValue)) : SinTable(fValue);
        }

        static inline Real Sqr (Real fValue) { return fValue*fValue; }

   
        static inline Real Sqrt (Real fValue) { return Real(sqrt(fValue)); }

        static inline Radian Sqrt (const Radian& fValue) { return Radian(sqrt(fValue.valueRadians())); }

        static inline Degree Sqrt (const Degree& fValue) { return Degree(sqrt(fValue.valueDegrees())); }

        static Real InvSqrt (Real fValue);


        static Real UnitRandom ();

        static Real RangeRandom (Real fLow, Real fHigh);

        static Real SymmetricRandom ();

        static void SetRandomValueProvider(RandomValueProvider* provider);
       
        static inline Real Tan (const Radian& fValue, bool useTables = false) {
            return (!useTables) ? Real(tan(fValue.valueRadians())) : TanTable(fValue.valueRadians());
        }
        static inline Real Tan (Real fValue, bool useTables = false) {
            return (!useTables) ? Real(tan(fValue)) : TanTable(fValue);
        }

        static inline Real DegreesToRadians(Real degrees) { return degrees * fDeg2Rad; }
        static inline Real RadiansToDegrees(Real radians) { return radians * fRad2Deg; }

       static void setAngleUnit(AngleUnit unit);

       static AngleUnit getAngleUnit(void);

       static Real AngleUnitsToRadians(Real units);

       static Real RadiansToAngleUnits(Real radians);

       static Real AngleUnitsToDegrees(Real units);
       static Real DegreesToAngleUnits(Real degrees);

        static bool pointInTri2D(const Vector2& p, const Vector2& a, 
            const Vector2& b, const Vector2& c);

        static bool pointInTri3D(const Vector3& p, const Vector3& a, 
            const Vector3& b, const Vector3& c, const Vector3& normal);
        static std::pair<bool, Real> intersects(const Ray& ray, const Plane& plane);

        static std::pair<bool, Real> intersects(const Ray& ray, const Sphere& sphere, 
            bool discardInside = true);
        
        static std::pair<bool, Real> intersects(const Ray& ray, const AxisAlignedBox& box);

        
        static bool intersects(const Ray& ray, const AxisAlignedBox& box,
            Real* d1, Real* d2);

        static std::pair<bool, Real> intersects(const Ray& ray, const Vector3& a,
            const Vector3& b, const Vector3& c, const Vector3& normal,
            bool positiveSide = true, bool negativeSide = true);
        static std::pair<bool, Real> intersects(const Ray& ray, const Vector3& a,
            const Vector3& b, const Vector3& c,
            bool positiveSide = true, bool negativeSide = true);
        static bool intersects(const Sphere& sphere, const AxisAlignedBox& box);

        static bool intersects(const Plane& plane, const AxisAlignedBox& box);

        static std::pair<bool, Real> intersects(
            const Ray& ray, const std::vector<Plane>& planeList, 
            bool normalIsOutside);
        static std::pair<bool, Real> intersects(
            const Ray& ray, const std::list<Plane>& planeList,
            bool normalIsOutside);

        static bool intersects(const Sphere& sphere, const Plane& plane);

        static bool RealEqual(Real a, Real b,
            Real tolerance = std::numeric_limits<Real>::epsilon());

        static Vector3 calculateTangentSpaceVector(
            const Vector3& position1, const Vector3& position2, const Vector3& position3,
            Real u1, Real v1, Real u2, Real v2, Real u3, Real v3);

        static Matrix4 buildReflectionMatrix(const Plane& p);
        static Vector4 calculateFaceNormal(const Vector3& v1, const Vector3& v2, const Vector3& v3);
        static Vector3 calculateBasicFaceNormal(const Vector3& v1, const Vector3& v2, const Vector3& v3);
        static Vector4 calculateFaceNormalWithoutNormalize(const Vector3& v1, const Vector3& v2, const Vector3& v3);
        static Vector3 calculateBasicFaceNormalWithoutNormalize(const Vector3& v1, const Vector3& v2, const Vector3& v3);

        static Real gaussianDistribution(Real x, Real offset = 0.0f, Real scale = 1.0f);

        template <typename T>
        inline static T Clamp(T val, T minval, T maxval)
        {
            assert (minval <= maxval && "Invalid clamp range");
            return std::max(std::min(val, maxval), minval);
        }

        static Vector3 trunc(const Vector3& v);
        static Vector3 ceil(const Vector3& v);
        static Matrix4 makeTranslateMatrix(const Vector3& position);
        static Matrix4 makeRotateMatrix(const Matrix4& m, float degree, const Ogre::Vector3& v);
        static Matrix4 makeRotateMatrixYX(const float radiansX, const float radiansY);
        static Matrix4 makeRotateMatrixXY(const float radiansX, const float radiansY);
        static Matrix4 makeScaleMatrix(const Vector3& scale);
        static Matrix4 makeViewMatrix(const Vector3& position, const Quaternion& orientation, 
            const Matrix4* reflectMatrix = 0);
        static Matrix4 makePerspectiveMatrixRH(
            Real FovAngleY, 
            Real AspectRatio, 
            Real zNear, 
            Real zFar);
        static Matrix4 makePerspectiveMatrixLH(
            Real FovAngleY, 
            Real AspectRatio, 
            Real zNear, 
            Real zFar);

        static Matrix4 makePerspectiveMatrixLHReverseZ(
            float fovxRadians,
            float aspectInverse,
            float zNear,
            float zFar);
        static Matrix4 makePerspectiveMatrixRHReverseZ(
            float fovxRadians,
            float aspectInverse,
            float zNear,
            float zFar);
        static Matrix4 makePerspectiveMatrix(Real left, Real right, Real bottom, Real top, 
            Real zNear, Real zFar);
        static Matrix4 makeOrthoRH(
            float ViewLeft,
            float ViewRight,
            float ViewBottom,
            float ViewTop,
            float NearZ,
            float FarZ);
        static Matrix4 makeOrthoLH(
            float ViewLeft,
            float ViewRight,
            float ViewBottom,
            float ViewTop,
            float NearZ,
            float FarZ);
        static Matrix4 makeLookAtRH(
            const Ogre::Vector3& position,
            const Ogre::Vector3& target,
            const Ogre::Vector3& up);
        static Matrix4 makeLookAtLH(
            const Ogre::Vector3& position,
            const Ogre::Vector3& target,
            const Ogre::Vector3& up);
        
        static Real boundingRadiusFromAABB(const AxisAlignedBox& aabb);

        static size_t gcd( size_t a, size_t b )
        {
            return b == 0u ? a : gcd( b, a % b );
        }
        /// Returns Least Common Multiple
        static size_t lcm( size_t a, size_t b )
        {
            return ( a / gcd( a, b ) ) * b;
        }



        static const Real POS_INFINITY;
        static const Real NEG_INFINITY;
        static const Real PI;
        static const Real TWO_PI;
        static const Real HALF_PI;
        static const Real fDeg2Rad;
        static const Real fRad2Deg;

    };

    template <>
    inline float Math::Clamp<float>(float val, float minval, float maxval)
    {
        assert (minval <= maxval && "Invalid clamp range");
        return std::max( std::min(val, maxval), minval );
    }
    template <>
    inline double Math::Clamp<double>(double val, double minval, double maxval)
    {
        assert (minval <= maxval && "Invalid clamp range");
        return std::max( std::min(val, maxval), minval );
    }

    inline Real Radian::valueDegrees() const
    {
        return Math::RadiansToDegrees ( mRad );
    }

    inline Real Radian::valueAngleUnits() const
    {
        return Math::RadiansToAngleUnits ( mRad );
    }

    inline Real Degree::valueRadians() const
    {
        return Math::DegreesToRadians ( mDeg );
    }

    inline Real Degree::valueAngleUnits() const
    {
        return Math::DegreesToAngleUnits ( mDeg );
    }

    inline Angle::operator Radian() const
    {
        return Radian(Math::AngleUnitsToRadians(mAngle));
    }

    inline Angle::operator Degree() const
    {
        return Degree(Math::AngleUnitsToDegrees(mAngle));
    }

    inline Radian operator * ( Real a, const Radian& b )
    {
        return Radian ( a * b.valueRadians() );
    }

    inline Radian operator / ( Real a, const Radian& b )
    {
        return Radian ( a / b.valueRadians() );
    }

    inline Degree operator * ( Real a, const Degree& b )
    {
        return Degree ( a * b.valueDegrees() );
    }

    inline Degree operator / ( Real a, const Degree& b )
    {
        return Degree ( a / b.valueDegrees() );
    }
}

#endif