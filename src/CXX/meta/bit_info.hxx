#ifndef meta__bit_info_hxx
#define meta__bit_info_hxx

/**
 *  \brief  Static table of information on bits in bytes
 *
 *  Useful for various bit-related operations.
 *  Generated using templates on compile-time.
 *
 *  \author  Vaclav Krpec  <vencik@razdva.cz>
 *  \date    2014/06/06
 *
 *  Legal notices
 *
 *  Copyright 2014 Vaclav Krpec
 *
 *  CTX Fryer is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "static_array.hxx"

#include <cstdint>


namespace meta {

namespace impl {

/** Get set bit count */
template <size_t B> class set_cnt {
    public: enum { value = set_cnt<B / 2>::value + (B & 0x1) };
};
template <> class set_cnt<0> { public: enum { value = 0 }; };

/** Get least significant set bit offset */
template <size_t B, int O = 0> class ls1b_off {
    public:
        enum { value = B & 0x1 ? O : ls1b_off<B / 2, O + 1>::value };
};
template <int O> class ls1b_off<0, O> {
    public: enum { value = 8 };
};

/** Get most significant set bit offset */
template <size_t B, int O = 7> class ms1b_off {
    public:
        enum { value = B & 0x80 ? O : ms1b_off<B * 2, O - 1>::value };
};
template <int O> class ms1b_off<0, O> {
    public: enum { value = 8 };
};

};  // end of namespace impl


/** Bit info table */
class bit_info {
    private:

    /**
     *  \brief  Packed record (for byte \c b)
     *
     *  Record structure:
     *  1/ bits 0x000f : set bits count in \c b
     *  2/ bits 0x00f0 : offset of least significant set bit in \c b
     *  3/ bits 0x0f00 : offset of most  significant set bit in \c b
     *  4/ bits 0xf000 : unused so far
     *
     *  Should you need to add an entry, use bits in an unused area
     *  or create more space by increasing the record size.
     */
    typedef uint16_t rec_t;

    /** Info geneartor */
    template <size_t B>
    class gen_info {
        public:

        /** Record packing */
        enum { value =
            impl::set_cnt<B>::value  << 0 |
            impl::ls1b_off<B>::value << 4 |
            impl::ms1b_off<B>::value << 8
        };

    };  // end of template class gen_info

    /** Info table */
    static base<rec_t>::static_array<256, gen_info> s_impl;

    /** Does not yield */
    bit_info() {}

    /**
     *  \brief  Unpack \c len bits from \c rec starting at \c off offset
     *
     *  \param  rec  Packed records
     *  \param  off  Bit offset
     *  \param  len  Length (in bits)
     *
     *  \return The bits (base-aligned)
     */
    inline static unsigned unpack(const rec_t & rec, int off, int len) {
        rec_t mask = 0;

        mask = ~mask; mask >>= 8 * sizeof(rec_t) - len;

        return ((mask << off) & rec) >> off;
    }

    public:

    /** Get set bits count */
    inline static unsigned set_cnt(unsigned char byte) {
        return unpack(s_impl[byte], 0, 4);
    }

    /** Get set bits count */
    inline static unsigned set_cnt(uint16_t word) {
        return
            set_cnt((unsigned char)(word >> 8)) +
            set_cnt((unsigned char)(word));
    }

    /** Get clear bits count */
    inline static unsigned clear_cnt(unsigned char byte) {
        return set_cnt((unsigned char)(~byte));
    }

    /** Get clear bits count */
    inline static unsigned clear_cnt(uint16_t word) {
        return set_cnt((uint16_t)(~word));
    }

    /**
     *  \brief  Get least-significant set bit offset
     *
     *  Note that the position is 0-based.
     *  Return value of 8 means that there are no set bits
     *  (i.e. \c byte == 0).
     */
    inline static unsigned ls1b_off(unsigned char byte) {
        return unpack(s_impl[byte], 4, 4);
    }

    /**
     *  \brief  Get least-significant set bit offset
     *
     *  Note that the position is 0-based.
     *  Return value of 16 means that there are no set bits
     *  (i.e. \c word == 0).
     */
    inline static unsigned ls1b_off(uint16_t word) {
        unsigned off = ls1b_off((unsigned char)word);

        if (off < 8) return off;

        return ls1b_off((unsigned char)(word >> 8)) + 8;
    }

    /**
     *  \brief  Get most-significant set bit offset
     *
     *  Note that the position is 0-based.
     *  Return value of 8 means that there are no set bits
     *  (i.e. \c byte == 0).
     */
    inline static unsigned ms1b_off(unsigned char byte) {
        return unpack(s_impl[byte], 8, 4);
    }

    /**
     *  \brief  Get most-significant set bit offset
     *
     *  Note that the position is 0-based.
     *  Return value of 16 means that there are no set bits
     *  (i.e. \c byte == 0).
     */
    inline static unsigned ms1b_off(uint16_t word) {
        unsigned off = ms1b_off((unsigned char)(word >> 8));

        if (off < 8) return off + 8;

        off = ms1b_off((unsigned char)word);

        return off < 8 ? off : 16;
    }

};  // end of class bit_info

// Bit info table implementation
base<bit_info::rec_t>::static_array<256, bit_info::gen_info> bit_info::s_impl;

}  // end of namespace meta

#endif  // end of #ifndef meta__bit_info_hxx
