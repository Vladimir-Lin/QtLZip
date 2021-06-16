#define _FILE_OFFSET_BITS 64

#include <algorithm>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <stdint.h>

#ifdef WIN32
#include <io.h>
#include <fcntl.h>
#endif

#include "qtlzip.h"
#include "lzip.h"
#include "encoder_base.h"


/* Returns the number of bytes really read.
   If (returned value < size) and (errno == 0), means EOF was reached.
*/
int readblock( const int fd, uint8_t * const buf, const int size )
  {
  int sz = 0;
  errno = 0;
  while( sz < size )
    {
    const int n = read( fd, buf + sz, size - sz );
    if( n > 0 ) sz += n;
    else if( n == 0 ) break;				// EOF
    else if( errno != EINTR ) break;
    errno = 0;
    }
  return sz;
  }


/* Returns the number of bytes really written.
   If (returned value < size), it is always an error.
*/
int writeblock( const int fd, const uint8_t * const buf, const int size )
  {
  int sz = 0;
  errno = 0;
  while( sz < size )
    {
    const int n = write( fd, buf + sz, size - sz );
    if( n > 0 ) sz += n;
    else if( n < 0 && errno != EINTR ) break;
    errno = 0;
    }
  return sz;
  }

Dis_slots dis_slots;
Prob_prices prob_prices;


bool Matchfinder_base::read_block()
  {
  if( !at_stream_end && stream_pos < buffer_size )
    {
    const int size = buffer_size - stream_pos;
    const int rd = readblock( infd, buffer + stream_pos, size );
    stream_pos += rd;
    if( rd != size && errno ) throw Error( "Read error" );
    if( rd < size ) { at_stream_end = true; pos_limit = buffer_size; }
    }
  return pos < stream_pos;
  }


void Matchfinder_base::normalize_pos()
{
  if ( pos > stream_pos ) return ;
  if( !at_stream_end )
    {
    const int offset = pos - dictionary_size - before_size;
    const int size = stream_pos - offset;
    std::memmove( buffer, buffer + offset, size );
    partial_data_pos += offset;
    pos -= offset;
    stream_pos -= offset;
    for( int i = 0; i < num_prev_positions; ++i )
      prev_positions[i] -= std::min( prev_positions[i], offset );
    for( int i = 0; i < pos_array_size; ++i )
      pos_array[i] -= std::min( pos_array[i], offset );
    read_block();
    }
}

Matchfinder_base::Matchfinder_base( const int before, const int dict_size,
                    const int after_size, const int dict_factor,
                    const int num_prev_positions23,
                    const int pos_array_factor, const int ifd )
  :
  partial_data_pos( 0 ),
  before_size( before ),
  pos( 0 ),
  cyclic_pos( 0 ),
  stream_pos( 0 ),
  infd( ifd ),
  at_stream_end( false )
  {
  const int buffer_size_limit =
    ( dict_factor * dict_size ) + before_size + after_size;
  buffer_size = std::max( 65536, dict_size );
  buffer = (uint8_t *)std::malloc( buffer_size );
  if( !buffer ) throw std::bad_alloc();
  if( read_block() && !at_stream_end && buffer_size < buffer_size_limit )
    {
    buffer_size = buffer_size_limit;
    uint8_t * const tmp = (uint8_t *)std::realloc( buffer, buffer_size );
    if( !tmp ) { std::free( buffer ); throw std::bad_alloc(); }
    buffer = tmp;
    read_block();
    }
  if( at_stream_end && stream_pos < dict_size )
    dictionary_size = std::max( (int)min_dictionary_size, stream_pos );
  else
    dictionary_size = dict_size;
  pos_limit = buffer_size;
  if( !at_stream_end ) pos_limit -= after_size;
  unsigned size = 1 << std::max( 16, real_bits( dictionary_size - 1 ) - 2 );
  if( dictionary_size > 1 << 26 )		// 64 MiB
    size >>= 1;
  key4_mask = size - 1;
  size += num_prev_positions23;

  num_prev_positions = size;
  pos_array_size = pos_array_factor * ( dictionary_size + 1 );
  size += pos_array_size;
  prev_positions = new( std::nothrow ) int32_t[size];
  if( !prev_positions ) { std::free( buffer ); throw std::bad_alloc(); }
  pos_array = prev_positions + num_prev_positions;
  for( int i = 0; i < num_prev_positions; ++i ) prev_positions[i] = 0;
  }


void Matchfinder_base::reset()
  {
  if( stream_pos > pos )
    std::memmove( buffer, buffer + pos, stream_pos - pos );
  partial_data_pos = 0;
  stream_pos -= pos;
  pos = 0;
  cyclic_pos = 0;
  for( int i = 0; i < num_prev_positions; ++i ) prev_positions[i] = 0;
  read_block();
  }


void Range_encoder::flush_data()
  {
  if( pos > 0 )
    {
    if( outfd >= 0 && writeblock( outfd, buffer, pos ) != pos )
      throw Error( "Write error" );
    partial_member_pos += pos;
    pos = 0;
    }
  }


     // End Of Stream mark => (dis == 0xFFFFFFFFU, len == min_match_len)
void LZ_encoder_base::full_flush( const LZipLib::State state )
  {
  const int pos_state = data_position() & pos_state_mask;
  renc.encode_bit( bm_match[state()][pos_state], 1 );
  renc.encode_bit( bm_rep[state()], 0 );
  encode_pair( 0xFFFFFFFFU, min_match_len, pos_state );
  renc.flush();
  LZipLib::FileTrailer trailer;
  trailer.data_crc( crc() );
  trailer.data_size( data_position() );
  trailer.member_size( renc.member_position() + LZipLib::FileTrailer::size() );
  for( int i = 0; i < LZipLib::FileTrailer::size(); ++i )
    renc.put_byte( trailer.data[i] );
  renc.flush_data();
  }


void LZ_encoder_base::reset()
  {
  Matchfinder_base::reset();
  crc_ = 0xFFFFFFFFU;
  bm_literal[0][0].reset( ( literal_context_size ) * 0x300 );
  bm_match[0][0].reset( LZipLib::State::states * pos_states );
  bm_rep[0].reset( LZipLib::State::states );
  bm_rep0[0].reset( LZipLib::State::states );
  bm_rep1[0].reset( LZipLib::State::states );
  bm_rep2[0].reset( LZipLib::State::states );
  bm_len[0][0].reset( LZipLib::State::states * pos_states );
  bm_dis_slot[0][0].reset( len_states * dis_slot_size );
  bm_dis[0].reset( modeled_distances - end_dis_model );
  bm_align[0].reset( dis_align_size );
  match_len_model.reset();
  rep_len_model.reset();
  renc.reset();
  }
