
#include "artdaq-demo/Compression/Codes.hh"
#include "artdaq-demo/Compression/Accum.hh"

#include <stdexcept>

using namespace std;

namespace demo {

code_type unary_code(long n)
{
  long b = 0x8000000000000000L;
  long a = b >> (n - 1);
  unsigned long c = (unsigned long)a >> (sizeof(long) * 8 - (n + 1));
  c &= 0xffffffffffffffeUL;
  // cout << "unary: " << n << " c=" << (void*)c << "\n";
  return c;
}

code_type b_mask(long b)
{
  unsigned long x = (unsigned long)(-1L);
  unsigned long shift = sizeof(long) * 8 - b;
  unsigned long c = x >> (shift - 1);
  c >>= 1;
  // cout << "mask: " << b << " c=" << (void*)c << " s " << shift << "\n";
  return c;
}

// ------------------------------

// for decoding pod, the number of zeros found is the number of bits
// afterwards that define the run length i.e. the value we are looking for.
// The value is the run length.
// here is a reasonable online page that talks about it:
// http://www.firstpr.com.au/audiocomp/lossless/

Code pod(code_type n)
{
  Code result;
  // need to check the n==0, I think the value and length should be 1 and 1
  if (n == 0) { result.value_ = 1; result.length_ = 1; return result; } // one bit
  unsigned long num_bits = floor(log2(n) + 1);
  result.value_ = n<<num_bits;
  result.length_ = num_bits * 2;
  return result; // length = num_bits*2;
}

void generateTable(Code (*f)(code_type), SymTable& out, size_t total)
{
  out.clear();
  for(size_t i=0;i<total;++i)
    {
      Code p = f(i);
      out.push_back(SymCode(i,p.value_,p.length_));
    }
  reverseCodes(out);
  // shift left by 1/2 the bit length of each code
  for(size_t i=0;i<total;++i)
    {
      out[i].code_ <<= out[i].bit_count_>>1;
    }
}

  unsigned long rleAndCompress(size_t bits, ADCCountVec const& in, DataVec& out, SymTable const& syms, unsigned bias)
{
  return rleAndCompress(bits, &in[0], &in[in.size()], out, syms, bias);
}

  unsigned long rleAndCompress(size_t bits,
			       ADCCountVec::value_type const* in_start,
			       ADCCountVec::value_type const* in_end,
			       DataVec& out, SymTable const& syms, unsigned)
{
  // adc_type mask = ((1UL<<bits)-1UL);
  Accum acc(out,syms);
  unsigned long bit_count=0;

  // calculate run lengths and feed each number into the acc.
  for (auto b = in_start; b != in_end; ++b) 
    {
      // auto curr = (*b - bias)&mask;
      auto curr = (*b);
      for (size_t i = 0; i < bits; ++i) 
	{
	  if ((curr & 0x01)==0)
	    {
	      acc.put(bit_count);
	      bit_count = 0;
	    }
	  else
	    { 
	      ++bit_count;
	    }
	  curr >>=1;
	}
    }
  if(bit_count>0) acc.put(bit_count);
  return acc.totalBits();
}

#if 0
  adc_type applyBias(size_t total_bits, adc_type value, int shift_amount)
  {
    reg_type tmp = value<<(sizeof(adc_type)*8);
    return 0;
    
  }

  adc_type removeBias()
  {
    return 0;
  }
#endif

}
