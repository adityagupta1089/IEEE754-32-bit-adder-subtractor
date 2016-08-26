#include <cstdlib>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <stdint.h>

using namespace std;

#define SIGN 0
#define EXP 1
#define FRAC 2

#define MANTISSA_MAX ((2<<22)-1)
#define HIDDEN_MANTISSA (2 << 22)

#define max(a,b) ((a>b)?a:b)
#define min(a,b) ((a>b)?b:a)
#define zero(x) (x[EXP] == 0 && x[FRAC] == 0)
#define denorm(x)  (x[EXP] == 0 && x[FRAC] != 0)
#define sign(x) ((x[SIGN]==1)?-1:1)
#define hideMantissaBit(x) (x=x&MANTISSA_MAX);
#define convertToSignificand(x) x[FRAC]=((!denorm(x))?(x[FRAC]+HIDDEN_MANTISSA):(x[FRAC]));x[EXP]+=(denorm(x))?1:0
#define shift(x,y) x[FRAC] = shiftAndRound(x[FRAC],max(y[EXP] - x[EXP], 0));
#define renormalize(x,d) x[FRAC]=(d>0)?(shiftAndRound(x[FRAC],d)):(x[FRAC]<<(-d));

int msb_length(int64_t l) {
	int cnt = 0;
	while (l >= 1) {
		l >>= 1;
		cnt++;
	}
	return cnt;
}

int64_t shiftAndRound(int64_t x, int d) {
	d = min(d, msb_length(x));
	int result = x >> d;
	if (d == 0)
		return result;
	int r = (x & ((d > 1) ? (2 << (d - 2)) : 1)) >> (d - 1);
	if (r == 1) {
		int lb = result & 1;
		int s = 0;
		for (int i = 0; i < d - 1 && s == 0; i++) {
			s |= x & 1;
			x >>= 1;
		}
		if (s == 1 || lb == 1)
			result += 1;
	}
	return result;
}

int64_t* get(int64_t x) {
	int64_t* arr;
	arr = new int64_t[3];
	arr[SIGN] = (x & 0x80000000) >> 31;
	arr[EXP] = (x & 0x7F800000) >> 23;
	arr[FRAC] = (x & 0x7FFFFF);
	return arr;
	delete arr;
}

int64_t add(int64_t x, int64_t y) {
	int64_t* a = get(x);
	int64_t* b = get(y);
	if (zero(a))
		return y;
	if (zero(b))
		return x;
	convertToSignificand(a);
	convertToSignificand(b);
	if (a[EXP] != b[EXP]) {
		shift(a, b)
		shift(b, a)
	}
	int64_t* sum;
	sum = new int64_t[3];
	sum[FRAC] = sign(a) * a[FRAC] + sign(b) * b[FRAC];
	sum[SIGN] = ((sum[FRAC] >= 0) ? 0 : 1);
	sum[FRAC] = abs(sum[FRAC]);
	if (sum[FRAC] != 0) {
		sum[EXP] = max(a[EXP], b[EXP]);
		int deltaExp = msb_length(sum[FRAC]) - 24;
		sum[EXP] += deltaExp;
		renormalize(sum, deltaExp)
		if (!denorm(sum))
			hideMantissaBit(sum[FRAC])
	} else {
		sum[EXP] = 0;
	}
	return (sum[SIGN] << 31) + (sum[EXP] << 23) + sum[FRAC];
	delete a;
	delete b;
	delete sum;
}

int main() {
	ifstream inputStream;
	inputStream.open("input.txt");
	int r = 0, w = 0;
	int64_t i = 0;
	while (!inputStream.eof()) {
		int64_t a;
		int64_t b;
		int64_t c;
		inputStream >> dec >> i;
		inputStream >> hex >> a;
		inputStream >> hex >> b;
		inputStream >> hex >> c;
		if (c == add(a, b)) {
			r++;
			cout << "Test " << i << " PASSED" << endl;
		} else {
			w++;
			cout << "Test " << i << " FAILED" << endl;
		}
	}
	inputStream.close();
	cout << "Total " << r << " " << "PASSED " << w << " FAILED." << endl;
}
