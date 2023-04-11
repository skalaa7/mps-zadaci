#include "matrix.hpp"
#include <cstdlib>
#include <iostream>
#include <cassert>

using namespace std;

namespace matrix
{
	mat::mat(size_t n, size_t m) : n(n), m(m), d(n*m)
	{
	}

	void mat::rand()
	{
		for (int i = 0; i != n; ++i)
			for (int j = 0; j != m; ++j)
				d[i*m + j] = (MIN + (num_t)(std::rand()%((int)MAX-(int)MIN))  );
	}

	void mat::zero()
	{
		for (int i = 0; i != n; ++i)
			for (int j = 0; j != m; ++j)
				d[i*m + j] = 0;
	}

	void mat::mult(const mat& a, const mat& b)
	{
		assert(a.m == b.n);
		assert(n == a.n);
		assert(m == b.m);

		for (int i = 0; i != n; ++i)
			for (int j = 0; j != m; ++j)
			{
				num_t res = 0.0;
				for (int k = 0; k != a.m; ++k)
					res += a.d[i*a.m + k] * b.d[k*b.m + j];
				d[i*m + j] = res;
			}
	}

	void mat::tmult(const mat& a, const mat& b, int bsize)
	{
		assert(a.m == b.n);
		assert(n == a.n);
		assert(m == b.m);

		zero();
		size_t d0 = a.n, d1 = a.m, d2 = b.m;

		for (size_t bi = 0; bi < d0; bi+=bsize)
			for (size_t bj = 0; bj < d2; bj+=bsize)
			{
				size_t li = max(bi + bsize, d0);
				size_t lj = max(bj + bsize, d2);
				for (size_t i = bi; i != li; ++i)
					for (size_t j = bj; j != lj; ++j)
					{
						num_t sum = 0.0;
						for (int k = 0; k != bsize; ++k)
						{
							size_t ri = bi + k;
							size_t rj = bj + k;
							if (ri > d0) break;
							if (rj > d2) break;

							sum += a.d[ri*d1+k] * b.d[k*d2+rj];
						}

						d[i*d1+j] += sum;
					}
			}
	}

	std::ostream& operator<< (std::ostream& str, const mat& m)
	{
		for (int i = 0; i != m.n; ++i)
		{
			for (int j = 0; j != m.m; ++j)
				str << m.d[i*m.m + j] << " ";
			str << endl;
		}
		return str;
	}

	bool operator == (const mat& a, const mat& b)
	{
		if (a.n != b.n) return false;
		if (a.m != b.m) return false;

		num_t er = 0.0;

		for (int i = 0; i != a.n; ++i)
			for (int j = 0; j != a.m; ++j)
			{
				num_t tmp = a.d[i*a.m + j] - b.d[i*a.m + j];
				er += tmp * tmp;
			}

		return er < ABS_ER;
	}

	unsigned long get_mul_ops(const mat& a, const mat& b, double elapsed)
	{
		assert(a.m == b.n);
		return 2 * a.n * a.m * b.m;
	}
}
