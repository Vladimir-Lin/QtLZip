/****************************************************************************
 *
 * Copyright (C) 2016 Neutrino International Inc.
 *
 * Author   : Brian Lin ( Vladimir Lin , Vladimir Forest )
 * E-mail   : lin.foxman@gmail.com
 *          : lin.vladimir@gmail.com
 *          : wolfram_lin@yahoo.com
 *          : wolfram_lin@sina.com
 *          : wolfram_lin@163.com
 * Skype    : wolfram_lin
 * WeChat   : 153-0271-7160
 * WhatsApp : 153-0271-7160
 * QQ       : lin.vladimir@gmail.com (2107437784)
 * URL      : http://qtlzip.sourceforge.net/
 *
 * QtLZip acts as an interface between Qt and LZip.
 * Please keep QtLZip as simple as possible.
 *
 * Copyright 2001 ~ 2016
 *
 ****************************************************************************/

#include "qtlzip.h"

QT_BEGIN_NAMESPACE

extern LZipLib::CRC32 lcrc32 ;

BEGIN_LZIP_NAMESPACE

//////////////////////////////////////////////////////////////////////////////

Decoder:: Decoder          ( const FileHeader   & header           ,
                                   RangeDecoder & rde              ,
                             const int            ofd              )
        : partial_data_pos ( 0                                     )
        , rdec             ( rde                                   )
        , dictionary_size  ( header . dictionary_size ( )          )
        , buffer_size      ( std::max ( 65536U , dictionary_size ) )
        , buffer           ( new quint8 [ buffer_size ]            )
        , pos              ( 0                                     )
        , stream_pos       ( 0                                     )
        , crc_             ( 0xFFFFFFFFU                           )
        , outfd            ( ofd                                   )
        , member_version   ( header . version ( )                  )
{
  buffer [ buffer_size - 1 ] = 0 ;
}

Decoder::~Decoder(void)
{
  if ( NULL != buffer ) {
    delete [ ] buffer   ;
  }                     ;
  buffer = NULL         ;
}

unsigned int Decoder::crc(void) const
{
  return crc_ ^ 0xFFFFFFFFU ;
}

unsigned long long Decoder::data_position(void) const
{
  return partial_data_pos + pos ;
}

quint8 Decoder::peek_prev(void) const
{
  const int i = ( ( pos > 0 ) ? pos : buffer_size ) - 1 ;
  return buffer [ i ]                                   ;
}

quint8 Decoder::peek(const int distance) const
{
  int i = pos - distance - 1;
  if( i < 0 ) i += buffer_size;
  return buffer[i];
}

void Decoder::put_byte(const quint8 b)
{
  buffer[pos] = b;
  if( ++pos >= buffer_size ) flush_data();
}

void Decoder::copy_block(const int distance, int len)
{
  int i = pos - distance - 1;
  if( i < 0 ) i += buffer_size;
  if( len < buffer_size - std::max( pos, i ) && len <= std::abs( pos - i ) ) {
    ::memcpy( buffer + pos, buffer + i, len );
    pos += len;
  } else {
    for( ; len > 0; --len ) {
    buffer[pos] = buffer[i];
    if( ++pos >= buffer_size ) flush_data();
    if( ++i >= buffer_size ) i = 0;
    }
  } ;
}

int Decoder::readblock( const int fd, quint8 * buf, const int size )
{
#pragma message("readblock is about to be replaced")
  int sz = 0;
#ifdef XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
  errno = 0;
  while( sz < size )
    {
    const int n = read( fd, buf + sz, size - sz );
    if( n > 0 ) sz += n;
    else if( n == 0 ) break;
    else if( errno != EINTR ) break;
    errno = 0;
    }
#endif
  return sz;
}

int Decoder::writeblock( const int fd, const quint8 * buf, const int size )
{
#pragma message("writeblock is about to be replaced")
  int sz = 0;
#ifdef XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
  errno = 0;
  while( sz < size )
    {
    const int n = write( fd, buf + sz, size - sz );
    if( n > 0 ) sz += n;
    else if( n < 0 && errno != EINTR ) break;
    errno = 0;
    }
#endif
  return sz;
}

void Decoder::flush_data()
{
  if( pos > stream_pos )
    {
    const int size = pos - stream_pos;
    lcrc32.update( crc_, buffer + stream_pos, size );
    if( outfd >= 0 && writeblock( outfd, buffer + stream_pos, size ) != size ) {
#pragma message("drop throw Error")
//      throw Error( "Write error" );
    } ;
    if( pos >= buffer_size ) { partial_data_pos += pos; pos = 0; }
    stream_pos = pos;
    }
}

bool Decoder::verify_trailer(void) const
{
  FileTrailer trailer;
  const int trailer_size = FileTrailer::size( member_version );
  const unsigned long long member_size = rdec.member_position() + trailer_size;
  bool error = false;

  int size = rdec.read_data( trailer.data, trailer_size );
  if( size < trailer_size )
    {
    error = true;
//    if( verbosity >= 0 )
//      {
//      pp();
//      std::fprintf( stderr, "Trailer truncated at trailer position %d;"
//                            " some checks may fail.\n", size );
//      }
    while( size < trailer_size ) trailer.data[size++] = 0;
    }

  if( member_version == 0 ) trailer.member_size( member_size );

  if( !rdec.code_is_zero() )
    {
    error = true;
//    pp( "Range decoder final code is not zero." );
    }
  if( trailer.data_crc() != crc() )
    {
    error = true;
//    if( verbosity >= 0 )
//      {
//      pp();
//      std::fprintf( stderr, "CRC mismatch; trailer says %08X, data CRC is %08X\n",
//                    trailer.data_crc(), crc() );
//      }
    }
  if( trailer.data_size() != data_position() )
    {
    error = true;
//    if( verbosity >= 0 )
//      {
//      pp();
//      std::fprintf( stderr, "Data size mismatch; trailer says %llu, data size is %llu (0x%llX)\n",
//                    trailer.data_size(), data_position(), data_position() );
//      }
    }
  if( trailer.member_size() != member_size )
    {
    error = true;
//    if( verbosity >= 0 )
//      {
//      pp();
//      std::fprintf( stderr, "Member size mismatch; trailer says %llu, member size is %llu (0x%llX)\n",
//                    trailer.member_size(), member_size, member_size );
//      }
    }
//  if( !error && verbosity >= 2 && data_position() > 0 && member_size > 0 )
//    std::fprintf( stderr, "%6.3f:1, %6.3f bits/byte, %5.2f%% saved.  ",
//                  (double)data_position() / member_size,
//                  ( 8.0 * member_size ) / data_position(),
//                  100.0 * ( 1.0 - ( (double)member_size / data_position() ) ) );
//  if( !error && verbosity >= 4 )
//    std::fprintf( stderr, "data CRC %08X, data size %9llu, member size %8llu.  ",
//                  trailer.data_crc(), trailer.data_size(), trailer.member_size() );
  return !error;
  }


/* Return value: 0 = OK, 1 = decoder error, 2 = unexpected EOF,
                 3 = trailer error, 4 = unknown marker found. */
int Decoder::decode_member(void)
  {
  BitModel bm_literal[literal_context_size][0x300];
  BitModel bm_match[State::states][pos_states];
  BitModel bm_rep[State::states];
  BitModel bm_rep0[State::states];
  BitModel bm_rep1[State::states];
  BitModel bm_rep2[State::states];
  BitModel bm_len[State::states][pos_states];
  BitModel bm_dis_slot[len_states][dis_slot_size];
  BitModel bm_dis[modeled_distances-end_dis_model];
  BitModel bm_align[dis_align_size];
  LenModel match_len_model;
  LenModel rep_len_model;
  unsigned rep0 = 0;		// rep[0-3] latest four distances
  unsigned rep1 = 0;		// used for efficient coding of
  unsigned rep2 = 0;		// repeated distances
  unsigned rep3 = 0;
  State state;

  rdec.load();
  while( !rdec.finished() )
    {
    const int pos_state = data_position() & pos_state_mask;
    if( rdec.decode_bit( bm_match[state()][pos_state] ) == 0 )	// 1st bit
      {
      const quint8 prev_byte = peek_prev();
      if( state.is_char() )
        {
        state.set_char1();
        put_byte( rdec.decode_tree8( bm_literal[get_lit_state(prev_byte)] ) );
        }
      else
        {
        state.set_char2();
        put_byte( rdec.decode_matched( bm_literal[get_lit_state(prev_byte)],
                                       peek( rep0 ) ) );
        }
      }
    else					// match or repeated match
      {
      int len;
      if( rdec.decode_bit( bm_rep[state()] ) != 0 )		// 2nd bit
        {
        if( rdec.decode_bit( bm_rep0[state()] ) != 0 )		// 3rd bit
          {
          unsigned distance;
          if( rdec.decode_bit( bm_rep1[state()] ) == 0 )	// 4th bit
            distance = rep1;
          else
            {
            if( rdec.decode_bit( bm_rep2[state()] ) == 0 )	// 5th bit
              distance = rep2;
            else
              { distance = rep3; rep3 = rep2; }
            rep2 = rep1;
            }
          rep1 = rep0;
          rep0 = distance;
          }
        else
          {
          if( rdec.decode_bit( bm_len[state()][pos_state] ) == 0 ) // 4th bit
            { state.set_short_rep(); put_byte( peek( rep0 ) ); continue; }
          }
        state.set_rep();
        len = min_match_len + rdec.decode_len( rep_len_model, pos_state );
        }
      else					// match
        {
        const unsigned rep0_saved = rep0;
        len = min_match_len + rdec.decode_len( match_len_model, pos_state );
        const int dis_slot = rdec.decode_tree6( bm_dis_slot[get_len_state(len)] );
        if( dis_slot < start_dis_model ) rep0 = dis_slot;
        else
          {
          const int direct_bits = ( dis_slot >> 1 ) - 1;
          rep0 = ( 2 | ( dis_slot & 1 ) ) << direct_bits;
          if( dis_slot < end_dis_model )
            rep0 += rdec.decode_tree_reversed( bm_dis + rep0 - dis_slot - 1,
                                               direct_bits );
          else
            {
            rep0 += rdec.decode( direct_bits - dis_align_bits ) << dis_align_bits;
            rep0 += rdec.decode_tree_reversed4( bm_align );
            if( rep0 == 0xFFFFFFFFU )		// marker found
              {
              rep0 = rep0_saved;
              rdec.normalize();
              flush_data();
              if( len == min_match_len )	// End Of Stream marker
                {
                if( verify_trailer( ) ) return 0; else return 3;
                }
              if( len == min_match_len + 1 )	// Sync Flush marker
                {
                rdec.load(); continue;
                }
//              if( verbosity >= 0 )
//                {
//                pp();
//                std::fprintf( stderr, "Unsupported marker code '%d'\n", len );
//                }
              return 4;
              }
            }
          }
        rep3 = rep2; rep2 = rep1; rep1 = rep0_saved;
        state.set_match();
        if( rep0 >= dictionary_size || rep0 >= data_position() )
          { flush_data(); return 1; }
        }
      copy_block( rep0, len );
      }
    }
  flush_data();
  return 2;
  }

//////////////////////////////////////////////////////////////////////////////

END_LZIP_NAMESPACE
QT_END_NAMESPACE
