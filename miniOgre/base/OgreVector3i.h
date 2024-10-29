namespace Ogre
{
	struct Vector3i
	{
		int32_t x, y, z;
		Vector3i():
			x(0),y(0),z(0)
		{
		}
		Vector3i(int32_t v)
			:
			x(v), y(v), z(v)
		{

		}
		Vector3i(const Vector3i& other) :
			x(other.x), y(other.y), z(other.z)
		{
		}

		Vector3i(int32_t _x, int32_t _y, int32_t _z)
		{
			x = _x;
			y = _y;
			z = _z;
		}

		bool operator != (Vector3i& other)
		{
			return x != other.x ||
				y != other.y ||
				z != other.z;
		}

		inline Vector3i operator /  (const Vector3i& other) const
		{
			return Vector3i(
				x / other.x,
				y / other.y,
				z / other.z);
		}

		inline Vector3i operator /  (const uint32_t scalar) const
		{
			return Vector3i(
				x / scalar,
				y / scalar,
				z / scalar);
		}

		inline Vector3i operator +  (const Vector3i& other) const
		{
			return Vector3i(
				x + other.x,
				y + other.y,
				z + other.z);
		}

		inline Vector3i operator +  (const uint32_t scalar) const
		{
			return Vector3i(
				x + scalar,
				y + scalar,
				z + scalar);
		}

		inline Vector3i& operator = (const Vector3i& other)
		{
			x = other.x;
			y = other.y;
			z = other.z;
			return *this;
		}

		inline Vector3i& operator += (const Vector3i& other)
		{
			x += other.x;
			y += other.y;
			z += other.z;
			return *this;
		}

		inline Vector3i operator * (const uint32_t scalar) const
		{
			return Vector3i(
				x * scalar,
				y * scalar,
				z * scalar);
		}

		inline Vector3i operator * (const float scalar) const
		{
			return Vector3i(
				x * scalar,
				y * scalar,
				z * scalar);
		}

		inline Vector3i operator % (const uint32_t scalar) const
		{
			return Vector3i(
				x % scalar,
				y % scalar,
				z % scalar);
		}


		inline Vector3i operator % (const Vector3i& other) const
		{
			return Vector3i(
				x % other.x,
				y % other.y,
				z % other.z);
		}
	};
}