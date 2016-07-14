#include<stdio.h>
#include<stdint.h>

template <typename V, class D> 
class StrongType
{
public:
	V _v;
	inline StrongType(V const &v) : _v(v) {
	}

	/* Allows converstion to type V */
	//inline operator V() const {
	//	return _v;
	//}
};

typedef StrongType<int, class WidthTag> Width;
typedef StrongType<int, class HeightTag> Height;

//asm("xxx");
void square_width(Width w)
{
	w._v = w._v * w._v;
}

//asm("yyy");
void square_int(int *x)
{
	int oldval = *x;
	*x = oldval * oldval;
}
//asm("zzz");

int main()
{
	printf("%lu\n", sizeof(Width));

	/* Check that Width and Height cannot be mixed */
	Width w = 1;
	Height h = 2;
	//w = h; // Compiler error

	//uint32_t x = w;
	uint32_t y = 1;
	square_width(y);
}
