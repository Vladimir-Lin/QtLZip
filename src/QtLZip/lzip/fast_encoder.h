class FLZ_encoder : public LZ_encoder_base
  {
  int key4;			// key made from latest 4 bytes

  void reset_key4()
    {
    key4 = 0;
    for( int i = 0; i < 3 && i < available_bytes(); ++i )
      key4 = ( key4 << 4 ) ^ buffer[i];
    }

  int longest_match_len( int * const distance );

  void update_and_move( int n )
    {
    while( --n >= 0 )
      {
      if( available_bytes() >= 4 )
        {
        key4 = ( ( key4 << 4 ) ^ buffer[pos+3] ) & key4_mask;
        const int newpos = prev_positions[key4];
        prev_positions[key4] = pos + 1;
        pos_array[cyclic_pos] = newpos;
        }
      move_pos();
      }
    }

  enum { before = 0,
         dict_size = 65536,
         // bytes to keep in buffer after pos
         after_size = max_match_len,
         dict_factor = 16,
         num_prev_positions23 = 0,
         pos_array_factor = 1 };

public:
  FLZ_encoder( const int ifd, const int outfd )
    :
    LZ_encoder_base( before, dict_size, after_size, dict_factor,
                     num_prev_positions23, pos_array_factor, ifd, outfd )
    {}

  bool encode_member( const unsigned long long member_size );
  };
