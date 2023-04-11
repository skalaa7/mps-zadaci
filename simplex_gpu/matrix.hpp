#ifndef MATRIX_HPP
#define MATRIX_HPP

#include <vector>
#include <iostream>

namespace matrix
{
	typedef float num_t;
	typedef std::vector<num_t> vec_t;
	const num_t MAX = 15.0;
	const num_t MIN = -15.0;
	const num_t ABS_ER = 0.01;

	class mat
	{
	public:
		mat(size_t n, size_t m);
		void rand();
		void zero();
		void mult(const mat& a, const mat& b);
		void tmult(const mat& a, const mat& b, int bsize);

		vec_t::iterator begin() {return d.begin();}
		vec_t::iterator end() {return d.end();}
		size_t size() {return n*m;}
		size_t get_row() {return n;}
		size_t get_col() {return m;}

		friend std::ostream& operator<< (std::ostream& str, const mat& m);
		friend bool operator == (const mat& a, const mat& b);
		friend unsigned long get_mul_ops(const mat& a, const mat& b, double elapsed);
	protected:
		size_t n, m;
		vec_t d;
	};
}

#endif
