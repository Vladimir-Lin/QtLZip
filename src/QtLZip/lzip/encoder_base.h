class Dis_slots
  {
  uint8_t data[1<<10];

public:
  void init()
    {
    for( int slot = 0; slot < 4; ++slot ) data[slot] = slot;
    for( int i = 4, size = 2, slot = 4; slot < 20; slot += 2 )
      {
      std::memset( &data[i], slot, size );
      std::memset( &data[i+size], slot + 1, size );
      size <<= 1;
      i += size;
      }
    }

  uint8_t operator[]( const int dis ) const { return data[dis]; }
  };

extern Dis_slots dis_slots;

inline uint8_t get_slot( const unsigned dis )
  {
  if( dis < (1 << 10) ) return dis_slots[dis];
  if( dis < (1 << 19) ) return dis_slots[dis>> 9] + 18;
  if( dis < (1 << 28) ) return dis_slots[dis>>18] + 36;
  return dis_slots[dis>>27] + 54;
  }


class Prob_prices
  {
  short data[bit_model_total >> price_step_bits];

public:
  void init()
    {
    for( int i = 0; i < bit_model_total >> price_step_bits; ++i )
      {
      unsigned val = ( i * price_step ) + ( price_step / 2 );
      int bits = 0;				// base 2 logarithm of val
      for( int j = 0; j < price_shift_bits; ++j )
        {
        val = val * val;
        bits <<= 1;
        while( val >= 1 << 16 ) { val >>= 1; ++bits; }
        }
      bits += 15;				// remaining bits in val
      data[i] = ( bit_model_total_bits << price_shift_bits ) - bits;
      }
    }

  int operator[]( const int probability ) const
    { return data[probability >> price_step_bits]; }
  };

extern Prob_prices prob_prices;


inline int price0( const LZipLib::BitModel bm )
  { return prob_prices[bm.probability]; }

inline int price1( const LZipLib::BitModel bm )
  { return prob_prices[bit_model_total - bm.probability]; }

inline int price_bit( const LZipLib::BitModel bm, const int bit )
  { if( bit ) return price1( bm ); else return price0( bm ); }


inline int price_symbol3( const LZipLib::BitModel bm[], int symbol )
  {
  int bit = symbol & 1;
  symbol |= 8; symbol >>= 1;
  int price = price_bit( bm[symbol], bit );
  bit = symbol & 1; symbol >>= 1; price += price_bit( bm[symbol], bit );
  return price + price_bit( bm[1], symbol & 1 );
  }


inline int price_symbol6( const LZipLib::BitModel bm[], int symbol )
  {
  int bit = symbol & 1;
  symbol |= 64; symbol >>= 1;
  int price = price_bit( bm[symbol], bit );
  bit = symbol & 1; symbol >>= 1; price += price_bit( bm[symbol], bit );
  bit = symbol & 1; symbol >>= 1; price += price_bit( bm[symbol], bit );
  bit = symbol & 1; symbol >>= 1; price += price_bit( bm[symbol], bit );
  bit = symbol & 1; symbol >>= 1; price += price_bit( bm[symbol], bit );
  return price + price_bit( bm[1], symbol & 1 );
  }


inline int price_symbol8( const LZipLib::BitModel bm[], int symbol )
  {
  int bit = symbol & 1;
  symbol |= 0x100; symbol >>= 1;
  int price = price_bit( bm[symbol], bit );
  bit = symbol & 1; symbol >>= 1; price += price_bit( bm[symbol], bit );
  bit = symbol & 1; symbol >>= 1; price += price_bit( bm[symbol], bit );
  bit = symbol & 1; symbol >>= 1; price += price_bit( bm[symbol], bit );
  bit = symbol & 1; symbol >>= 1; price += price_bit( bm[symbol], bit );
  bit = symbol & 1; symbol >>= 1; price += price_bit( bm[symbol], bit );
  bit = symbol & 1; symbol >>= 1; price += price_bit( bm[symbol], bit );
  return price + price_bit( bm[1], symbol & 1 );
  }


inline int price_symbol_reversed( const LZipLib::BitModel bm[], int symbol,
                                  const int num_bits )
  {
  int price = 0;
  int model = 1;
  for( int i = num_bits; i > 0; --i )
    {
    const int bit = symbol & 1;
    price += price_bit( bm[model], bit );
    model = ( model << 1 ) | bit;
    symbol >>= 1;
    }
  return price;
  }


inline int price_matched( const LZipLib::BitModel bm[], int symbol, int match_byte )
  {
  int price = 0;
  int mask = 0x100;
  symbol |= mask;

  do {
    match_byte <<= 1;
    const int match_bit = match_byte & mask;
    symbol <<= 1;
    const int bit = symbol & 0x100;
    price += price_bit( bm[match_bit+(symbol>>9)+mask], bit );
    mask &= ~(match_byte ^ symbol);	// if( match_bit != bit ) mask = 0;
    }
  while( symbol < 0x10000 );
  return price;
  }


class Matchfinder_base
  {
  bool read_block();
  void normalize_pos();

  Matchfinder_base( const Matchfinder_base & );	// declared as private
  void operator=( const Matchfinder_base & );	// declared as private

protected:
  unsigned long long partial_data_pos;
  uint8_t * buffer;		// input buffer
  int32_t * prev_positions;	// 1 + last seen position of key. else 0
  int32_t * pos_array;		// may be tree or chain
  const int before_size;	// bytes to keep in buffer before dictionary
  int buffer_size;
  int dictionary_size;		// bytes to keep in buffer before pos
  int pos;			// current pos in buffer
  int cyclic_pos;		// cycles through [0, dictionary_size]
  int stream_pos;		// first byte not yet read from file
  int pos_limit;		// when reached, a new block must be read
  int key4_mask;
  int num_prev_positions;	// size of prev_positions
  int pos_array_size;
  const int infd;		// input file descriptor
  bool at_stream_end;		// stream_pos shows real end of file

  Matchfinder_base( const int before, const int dict_size,
                    const int after_size, const int dict_factor,
                    const int num_prev_positions23,
                    const int pos_array_factor, const int ifd );

  ~Matchfinder_base()
    { delete[] prev_positions; std::free( buffer ); }

public:
  uint8_t peek( const int distance ) const { return buffer[pos-distance]; }
  int available_bytes() const { return stream_pos - pos; }
  unsigned long long data_position() const { return partial_data_pos + pos; }
  bool data_finished() const { return at_stream_end && pos >= stream_pos; }
  const uint8_t * ptr_to_current_pos() const { return buffer + pos; }

  int true_match_len( const int index, const int distance, int len_limit ) const
    {
    const uint8_t * const data = buffer + pos + index;
    int i = 0;
    if( index + len_limit > available_bytes() )
      len_limit = available_bytes() - index;
    while( i < len_limit && data[i-distance] == data[i] ) ++i;
    return i;
    }

  void move_pos()
    {
    if( ++cyclic_pos > dictionary_size ) cyclic_pos = 0;
    if( ++pos >= pos_limit ) normalize_pos();
    }

  void reset();
  };


class Range_encoder
  {
  enum { buffer_size = 65536 };
  uint64_t low;
  unsigned long long partial_member_pos;
  uint8_t * const buffer;	// output buffer
  int pos;			// current pos in buffer
  uint32_t range;
  unsigned ff_count;
  const int outfd;		// output file descriptor
  uint8_t cache;
  LZipLib::FileHeader header;

  void shift_low()
    {
    const bool carry = ( low > 0xFFFFFFFFU );
    if( carry || low < 0xFF000000U )
      {
      put_byte( cache + carry );
      for( ; ff_count > 0; --ff_count ) put_byte( 0xFF + carry );
      cache = low >> 24;
      }
    else ++ff_count;
    low = ( low & 0x00FFFFFFU ) << 8;
    }

  Range_encoder( const Range_encoder & );	// declared as private
  void operator=( const Range_encoder & );	// declared as private

public:
  void reset()
    {
    low = 0;
    partial_member_pos = 0;
    pos = 0;
    range = 0xFFFFFFFFU;
    ff_count = 0;
    cache = 0;
    for( int i = 0; i < LZipLib::FileHeader::size; ++i )
      put_byte( header.data[i] );
    }

  Range_encoder( const unsigned dictionary_size, const int ofd )
    :
    buffer( new uint8_t[buffer_size] ),
    outfd( ofd )
    {
    header.set_magic();
    header.dictionary_size( dictionary_size );
    reset();
    }

  ~Range_encoder() { delete[] buffer; }

  unsigned long long member_position() const
    { return partial_member_pos + pos + ff_count; }

  void flush() { for( int i = 0; i < 5; ++i ) shift_low(); }
  void flush_data();

  void put_byte( const uint8_t b )
    {
    buffer[pos] = b;
    if( ++pos >= buffer_size ) flush_data();
    }

  void encode( const int symbol, const int num_bits )
    {
    for( int i = num_bits - 1; i >= 0; --i )
      {
      range >>= 1;
      if( (symbol >> i) & 1 ) low += range;
      if( range <= 0x00FFFFFFU ) { range <<= 8; shift_low(); }
      }
    }

  void encode_bit( LZipLib::BitModel & bm, const int bit )
    {
    const uint32_t bound = ( range >> bit_model_total_bits ) * bm.probability;
    if( !bit )
      {
      range = bound;
      bm.probability += (bit_model_total - bm.probability) >> bit_model_move_bits;
      }
    else
      {
      low += bound;
      range -= bound;
      bm.probability -= bm.probability >> bit_model_move_bits;
      }
    if( range <= 0x00FFFFFFU ) { range <<= 8; shift_low(); }
    }

  void encode_tree3( LZipLib::BitModel bm[], const int symbol )
    {
    int model = 1;
    int bit = ( symbol >> 2 ) & 1;
    encode_bit( bm[model], bit ); model = ( model << 1 ) | bit;
    bit = ( symbol >> 1 ) & 1;
    encode_bit( bm[model], bit ); model = ( model << 1 ) | bit;
    encode_bit( bm[model], symbol & 1 );
    }

  void encode_tree6( LZipLib::BitModel bm[], const int symbol )
    {
    int model = 1;
    int bit = ( symbol >> 5 ) & 1;
    encode_bit( bm[model], bit ); model = ( model << 1 ) | bit;
    bit = ( symbol >> 4 ) & 1;
    encode_bit( bm[model], bit ); model = ( model << 1 ) | bit;
    bit = ( symbol >> 3 ) & 1;
    encode_bit( bm[model], bit ); model = ( model << 1 ) | bit;
    bit = ( symbol >> 2 ) & 1;
    encode_bit( bm[model], bit ); model = ( model << 1 ) | bit;
    bit = ( symbol >> 1 ) & 1;
    encode_bit( bm[model], bit ); model = ( model << 1 ) | bit;
    encode_bit( bm[model], symbol & 1 );
    }

  void encode_tree8( LZipLib::BitModel bm[], const int symbol )
    {
    int model = 1;
    int mask = ( 1 << 7 );
    do {
      const int bit = ( symbol & mask );
      encode_bit( bm[model], bit );
      model <<= 1; if( bit ) ++model;
      }
    while( mask >>= 1 );
    }

  void encode_tree_reversed( LZipLib::BitModel bm[], int symbol, const int num_bits )
    {
    int model = 1;
    for( int i = num_bits; i > 0; --i )
      {
      const int bit = symbol & 1;
      encode_bit( bm[model], bit );
      model = ( model << 1 ) | bit;
      symbol >>= 1;
      }
    }

  void encode_matched( LZipLib::BitModel bm[], int symbol, int match_byte )
    {
    int mask = 0x100;
    symbol |= mask;

    do {
      match_byte <<= 1;
      const int match_bit = match_byte & mask;
      symbol <<= 1;
      const int bit = symbol & 0x100;
      encode_bit( bm[match_bit+(symbol>>9)+mask], bit );
      mask &= ~(match_byte ^ symbol);	// if( match_bit != bit ) mask = 0;
      }
    while( symbol < 0x10000 );
    }

  void encode_len( LZipLib::LenModel & lm, int symbol, const int pos_state )
    {
    symbol -= min_match_len;
    bool bit = ( symbol >= len_low_symbols );
    encode_bit( lm.choice1, bit );
    if( !bit )
      encode_tree3( lm.bm_low[pos_state], symbol );
    else
      {
      bit = ( symbol >= len_low_symbols + len_mid_symbols );
      encode_bit( lm.choice2, bit );
      if( !bit )
        encode_tree3( lm.bm_mid[pos_state], symbol - len_low_symbols );
      else
        encode_tree8( lm.bm_high, symbol - len_low_symbols - len_mid_symbols );
      }
    }
  };


class LZ_encoder_base : public Matchfinder_base
  {
protected:
  enum { max_marker_size = 16,
         num_rep_distances = 4 };	// must be 4

  uint32_t crc_;

  LZipLib::BitModel bm_literal[literal_context_size][0x300];
  LZipLib::BitModel bm_match[LZipLib::State::states][pos_states];
  LZipLib::BitModel bm_rep[LZipLib::State::states];
  LZipLib::BitModel bm_rep0[LZipLib::State::states];
  LZipLib::BitModel bm_rep1[LZipLib::State::states];
  LZipLib::BitModel bm_rep2[LZipLib::State::states];
  LZipLib::BitModel bm_len[LZipLib::State::states][pos_states];
  LZipLib::BitModel bm_dis_slot[len_states][dis_slot_size];
  LZipLib::BitModel bm_dis[modeled_distances-end_dis_model];
  LZipLib::BitModel bm_align[dis_align_size];
  LZipLib::LenModel match_len_model;
  LZipLib::LenModel rep_len_model;
  Range_encoder renc;

  LZ_encoder_base( const int before, const int dict_size, const int after_size,
                   const int dict_factor, const int num_prev_positions23,
                   const int pos_array_factor, const int ifd, const int outfd )
    :
    Matchfinder_base( before, dict_size, after_size, dict_factor,
                      num_prev_positions23, pos_array_factor, ifd ),
    crc_( 0xFFFFFFFFU ),
    renc( dictionary_size, outfd )
    {}

  unsigned crc() const { return crc_ ^ 0xFFFFFFFFU; }

  int price_literal( const uint8_t prev_byte, const uint8_t symbol ) const
    { return price_symbol8( bm_literal[get_lit_state(prev_byte)], symbol ); }

  int price_matched( const uint8_t prev_byte, const uint8_t symbol,
                     const uint8_t match_byte ) const
    { return ::price_matched( bm_literal[get_lit_state(prev_byte)], symbol,
                              match_byte ); }

  void encode_literal( const uint8_t prev_byte, const uint8_t symbol )
    { renc.encode_tree8( bm_literal[get_lit_state(prev_byte)], symbol ); }

  void encode_matched( const uint8_t prev_byte, const uint8_t symbol,
                       const uint8_t match_byte )
    { renc.encode_matched( bm_literal[get_lit_state(prev_byte)], symbol,
                           match_byte ); }

  void encode_pair( const unsigned dis, const int len, const int pos_state )
    {
    renc.encode_len( match_len_model, len, pos_state );
    const int dis_slot = get_slot( dis );
    renc.encode_tree6( bm_dis_slot[get_len_state(len)], dis_slot );

    if( dis_slot >= start_dis_model )
      {
      const int direct_bits = ( dis_slot >> 1 ) - 1;
      const unsigned base = ( 2 | ( dis_slot & 1 ) ) << direct_bits;
      const unsigned direct_dis = dis - base;

      if( dis_slot < end_dis_model )
        renc.encode_tree_reversed( bm_dis + base - dis_slot - 1, direct_dis,
                                   direct_bits );
      else
        {
        renc.encode( direct_dis >> dis_align_bits, direct_bits - dis_align_bits );
        renc.encode_tree_reversed( bm_align, direct_dis, dis_align_bits );
        }
      }
    }

  void full_flush( const LZipLib::State state );

public:
  virtual ~LZ_encoder_base() {}

  unsigned long long member_position() const { return renc.member_position(); }
  virtual void reset();

  virtual bool encode_member( const unsigned long long member_size ) = 0;
  };
