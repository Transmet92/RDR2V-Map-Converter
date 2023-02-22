#pragma once
#include "common.h"
#include "SkyCommons/SkyMath.h"
#include "SkyCommons/SkyContainer.h"
#include "SkyCommons/SkyFunctional.h"
#include "pgBase.h"



namespace rage
{
	template<typename T>
	class pgCollectionBase
	{
	protected:
		T* mData = 0;
		uint16 mCount = 0;
		uint16 mSize = 0;
		uint32 mPad0 = 0;

	public:
		pgCollectionBase() { }

		pgCollectionBase(uint32 count) {
			Alloc(count);
		}

		~pgCollectionBase() {
			if (mData)
				delete[] mData;
		};

		void Alloc(uint32 count) {
			mData = new T[count];
			ZeroMemory(mData, sizeof(T) * count);
			mCount = count;
			mSize = count;
		};

		T& operator[](uint16 index) { return mData[index]; };
		uint16 GetCount() const { return mCount; };
		uint16 GetSize() const { return mSize; };

		T* begin() { return &(mData[0]); };
		T* end() { return &(mData[mCount]); };

		const T* begin() const { return &(mData[0]); };
		const T* end() const { return &(mData[mCount]); };
	};



	template<typename T, bool T_CallMap = true>
	class pgCollection : public pgCollectionBase<T>
	{
	public:
		pgCollection() : pgCollectionBase<T>() {}
		pgCollection(uint32 count) : pgCollectionBase<T*>(count) {}

		void Map(pgMap& map, pgPtrList* ptrList = 0)
		{
			if (!ptrList)
				ptrList = map.Push(this);

			if (ptrList)
			{
				ptrList->Push(&this->mData);

				if constexpr (T_CallMap)
				{
					static_assert(std::is_class<T>::value, "T doit etre une classe ou une structure");
					for (int i = 0; i < this->mCount; i++)
					{
						if (this->mData[i])
						{
							this->mData[i].Map(map);
						}
					}
				}
				else
				{
					map.PushBlock((uint64)this->mData, sizeof(T) * this->mCount, false);
				}
			}
		}
	};

	template<typename T, bool T_CallMap = true>
	class pgCollectionPtr : public pgCollectionBase<T*>
	{
	public:
		pgCollectionPtr() : pgCollectionBase<T*>() {}
		pgCollectionPtr(uint32 count) : pgCollectionBase<T*>(count) {}


		void Map(pgMap& map, pgPtrList* ptrList = 0)
		{
			if (!ptrList)
				ptrList = map.Push(this);

			if (ptrList)
			{
				ptrList->Push(&this->mData);

				auto pList = map.PushBlock((uint64)this->mData, sizeof(T*) * this->mCount, false);
				if (pList)
				{
					for (int i = 0; i < this->mCount; i++)
						*pList << &this->mData[i];

					for (int i = 0; i < this->mCount; i++)
					{
						if (this->mData[i])
						{
							if constexpr (T_CallMap) {
								static_assert(std::is_class<T>::value, "T doit etre une classe ou une structure");
								this->mData[i]->Map(map);
							}
							else
								map.PushBlock((uint64)this->mData[i], sizeof(T), false);
						}
					}
				}
			}
		}
	};
}

#pragma pack(pop)



template<typename T>
void printHex(T val, int nline = 1, bool espaceEachByte = true)
{
	for (int i = 0; i < sizeof(T); i++)
		printf(espaceEachByte ? "%02hhX " : "%02hhX", *(((uint8*)&val) + i));

	for (int i = 0; i < nline; i++)
		printf("\n");
}

template<typename T>
void printHexBlob(T* pVal, uint32 size, int nline = 1, bool espaceEachByte = true)
{
	for (int i = 0; i < size; i++)
		printf(espaceEachByte ? "%02hhX " : "%02hhX", *(((uint8*)pVal) + i));

	for (int i = 0; i < nline; i++)
		printf("\n");
}


static void printIndent(int count = 1)
{
	for (int i = 0; i < count; i++)
		printf("\t");
}



#pragma pack(push, 1)
template<typename T>
inline T InvEnd(const T v) {
	T o;
	for (int i = 0; i < sizeof(T); i++)
		((uint8*)&o)[i] = ((char*)&v)[sizeof(T) - i - 1];

	return o;
};

// file wrap parts
template<typename T = void>
struct ptr32
{
	uint32 m_Offset;

	bool isGPU() const { return (m_Offset & 0xF0) == 0x60; };
	bool isCPU() const { return (m_Offset & 0xF0) == 0x50; };

	uint32 Get() const
	{
		uint32 offset = InvEnd(m_Offset) & 0x0FFFFFFF;
		return offset;
	};

	template<typename X, typename Y> // CPU/GPU   SEGMENTS PTR
	T* Get(X* CpuSeg, Y* GpuSeg)
	{
		if (!m_Offset)
			return 0;

		uint8* pSeg = 0;

		uint8 memFlag = (m_Offset & 0xF0);
		if (memFlag == 0x50)
			pSeg = (uint8*)CpuSeg;
		else if (memFlag == 0x60)
			pSeg = (uint8*)GpuSeg;

		return (T*)(pSeg + Get());
	};
};

template<typename T>
struct pgCollect {
	ptr32<T> m_Off;
	uint16 m_Size;
	uint16 m_Count;

	template<typename TCPU, typename TGPU>
	T* GetAt(int index, TCPU* cpu, TGPU* gpu) { return &(m_Off.Get(cpu, gpu)[index]); };

	ptr32<T> Get() const { return m_Off; };

	// safe get count
	uint16 GetCount() const { return std::min(GetCount0(), GetSize()); };

	uint16 GetCount0() const { return InvEnd(m_Count); };
	uint16 GetSize() const { return InvEnd(m_Size); };
};


template<typename T>
static void printBits(T* ptr, uint32 size)
{
	uint8* b = (uint8*)ptr;

	int j;
	uint8 byte;
	for (int i = size - 1; i >= 0; i--)
	{
		for (j = 7; j >= 0; j--)
		{
			byte = b[i] & (1 << j);
			byte >>= j;
			printf("%u", byte);
		}
	}
}


static int IntPower(int base, int exponent)
{
	if (exponent == 0)
		return 1;

	int result = IntPower(base, exponent / 2);
	result *= result;

	if (exponent & 1)
		result *= base;

	return result;
}


static float DistToZeroIsOk(float val, float distanceMax) {
	return fabsf(val) <= distanceMax;
}

static Vector3c LittleEndian(const Vector3c& vec)
{
	Vector3c toret;
	toret.x = InvEnd(vec.x);
	toret.y = InvEnd(vec.y);
	toret.z = InvEnd(vec.z);
	return toret;
}
static Vector3c RdrVector3(const Vector3c& vec)
{
	return { vec.z, vec.x, vec.y };
}


class CBounds {
public:
	Vector3c Min;
	Vector3c Max;

	bool operator==(const CBounds c) const {
		return (Min == c.Min && Max == c.Max);
	};

	bool CoordsIsIn(Vector3c pos) const
	{
		return pos.x >= Min.x && pos.y >= Min.y && pos.z >= Min.z &&
			pos.x <= Max.x && pos.y <= Max.y && pos.z <= Max.z;
	}
};


struct GeoBound
{
	Vector4c m_AABBMin = { 0.f, 0.f, 0.f, 0.f };
	Vector4c m_AABBMax = { 0.f, 0.f, 0.f, 0.f };

	void ConvertRDR2V()
	{
		Vector4c newVec = { InvEnd(m_AABBMin.x), InvEnd(m_AABBMin.z), InvEnd(m_AABBMin.y), InvEnd(m_AABBMin.w) };
		m_AABBMin = newVec;

		newVec = { InvEnd(m_AABBMax.x), InvEnd(m_AABBMax.z), InvEnd(m_AABBMax.y), InvEnd(m_AABBMax.w) };
		m_AABBMax = newVec;
	}

	void Delocate(const Vector3c& position)
	{
		m_AABBMin -= position;
		m_AABBMax -= position;
	}
};


enum DetailLevel : int {
	DRAWABLE_HD,
	DRAWABLE_LOD, // LOD3/4
	DRAWABLE_SLOD // index buffer cassés (jamais utilisé)
};

