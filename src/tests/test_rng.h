/*
* (C) 2009 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_TESTS_FIXED_RNG_H__
#define BOTAN_TESTS_FIXED_RNG_H__

#include "tests.h"
#include <deque>
#include <string>
#include <botan/rng.h>
#include <botan/hex.h>

namespace Botan_Tests {

class Fixed_Output_RNG : public Botan::RandomNumberGenerator
   {
   public:
      bool is_seeded() const override { return !buf.empty(); }

      uint8_t random()
         {
         if(!is_seeded())
            throw Test_Error("Fixed output RNG ran out of bytes, test bug?");

         uint8_t out = buf.front();
         buf.pop_front();
         return out;
         }

      size_t reseed_with_sources(Botan::Entropy_Sources&,
                                 size_t,
                                 std::chrono::milliseconds) { return 0; }

      void randomize(uint8_t out[], size_t len) override
         {
         for(size_t j = 0; j != len; j++)
            out[j] = random();
         }

      void add_entropy(const uint8_t b[], size_t s) override
         {
         buf.insert(buf.end(), b, b + s);
         }

      std::string name() const override { return "Fixed_Output_RNG"; }

      void clear() throw() override {}

      Fixed_Output_RNG(const std::vector<uint8_t>& in)
         {
         buf.insert(buf.end(), in.begin(), in.end());
         }

      Fixed_Output_RNG(const std::string& in_str)
         {
         std::vector<uint8_t> in = Botan::hex_decode(in_str);
         buf.insert(buf.end(), in.begin(), in.end());
         }

      Fixed_Output_RNG() {}
   protected:
      size_t remaining() const { return buf.size(); }
   private:
      std::deque<uint8_t> buf;
   };

}

#endif
