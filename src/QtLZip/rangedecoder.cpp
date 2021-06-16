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

BEGIN_LZIP_NAMESPACE

//////////////////////////////////////////////////////////////////////////////

RangeDecoder:: RangeDecoder       ( void                       )
             : partial_member_pos ( 0                          )
             , buffer             ( new quint8 [ buffer_size ] )
             , pos                ( 0                          )
             , stream_pos         ( 0                          )
             , code               ( 0                          )
             , range              ( 0xFFFFFFFFU                )
             , infd               ( 0                          )
             , at_stream_end      ( false                      )
{
}

RangeDecoder:: RangeDecoder       ( const int ifd              )
             : partial_member_pos ( 0                          )
             , buffer             ( new quint8 [ buffer_size ] )
             , pos                ( 0                          )
             , stream_pos         ( 0                          )
             , code               ( 0                          )
             , range              ( 0xFFFFFFFFU                )
             , infd               ( ifd                        )
             , at_stream_end      ( false                      )
{
}

RangeDecoder::~RangeDecoder(void)
{
  if ( NULL != buffer ) {
    delete [ ] buffer   ;
  }                     ;
  buffer = NULL         ;
}

bool RangeDecoder::code_is_zero(void) const
{
  return ( code == 0 ) ;
}

bool RangeDecoder::finished(void)
{
  return pos >= stream_pos && !read_block() ;
}

unsigned long long RangeDecoder::member_position(void) const
{
  return partial_member_pos + pos ;
}

void RangeDecoder::reset_member_position(void)
{
  partial_member_pos = -pos ;
}

quint8 RangeDecoder::get_byte(void)
{
  if( finished() ) return 0xAA; // make code != 0
  return buffer[pos++];
}

int RangeDecoder::read_data(quint8 * outbuf,const int size)
{
  int rest = size;
  while( rest > 0 && !finished() )
    {
    const int rd = std::min( rest, stream_pos - pos );
    std::memcpy( outbuf + size - rest, buffer + pos, rd );
    pos += rd;
    rest -= rd;
    }
  return size - rest;
}

void RangeDecoder::load(void)
{
  code = 0;
  for( int i = 0; i < 5; ++i ) code = (code << 8) | get_byte();
  range = 0xFFFFFFFFU;
  code &= range;		// make sure that first byte is discarded
}

void RangeDecoder::normalize(void)
{
  if( range <= 0x00FFFFFFU )
    { range <<= 8; code = (code << 8) | get_byte(); }
}

int RangeDecoder::decode(const int num_bits)
{
  int symbol = 0;
  for( int i = num_bits; i > 0; --i )
    {
    normalize();
    range >>= 1;
//      symbol <<= 1;
//      if( code >= range ) { code -= range; symbol |= 1; }
    const quint32 mask = 0U - (code < range);
    code -= range;
    code += range & mask;
    symbol = (symbol << 1) + (mask + 1);
    }
  return symbol;
}

int RangeDecoder::decode_bit(BitModel & bm)
{
  normalize();
  const quint32 bound = ( range >> bit_model_total_bits ) * bm.probability;
  if( code < bound )
    {
    range = bound;
    bm.probability += (bit_model_total - bm.probability) >> bit_model_move_bits;
    return 0;
    }
  else
    {
    range -= bound;
    code -= bound;
    bm.probability -= bm.probability >> bit_model_move_bits;
    return 1;
    }
}

int RangeDecoder::decode_tree3(BitModel bm[])
{
  int symbol = 1;
  symbol = ( symbol << 1 ) | decode_bit( bm[symbol] );
  symbol = ( symbol << 1 ) | decode_bit( bm[symbol] );
  symbol = ( symbol << 1 ) | decode_bit( bm[symbol] );
  return symbol & 7;
}

int RangeDecoder::decode_tree6(BitModel bm[])
{
  int symbol = 1;
  symbol = ( symbol << 1 ) | decode_bit( bm[symbol] );
  symbol = ( symbol << 1 ) | decode_bit( bm[symbol] );
  symbol = ( symbol << 1 ) | decode_bit( bm[symbol] );
  symbol = ( symbol << 1 ) | decode_bit( bm[symbol] );
  symbol = ( symbol << 1 ) | decode_bit( bm[symbol] );
  symbol = ( symbol << 1 ) | decode_bit( bm[symbol] );
  return symbol & 0x3F;
}

int RangeDecoder::decode_tree8(BitModel bm[] )
{
  int symbol = 1;
  while( symbol < 0x100 )
    symbol = ( symbol << 1 ) | decode_bit( bm[symbol] );
  return symbol & 0xFF;
}

int RangeDecoder::decode_tree_reversed(BitModel bm[], const int num_bits )
{
  int model = 1;
  int symbol = 0;
  for( int i = 0; i < num_bits; ++i )
    {
    const bool bit = decode_bit( bm[model] );
    model <<= 1;
    if( bit ) { ++model; symbol |= (1 << i); }
    }
  return symbol;
}

int RangeDecoder::decode_tree_reversed4(BitModel bm[] )
{
  int model = 1;
  int symbol = decode_bit( bm[model] );
  model = (model << 1) + symbol;
  int bit = decode_bit( bm[model] );
  model = (model << 1) + bit; symbol |= (bit << 1);
  bit = decode_bit( bm[model] );
  model = (model << 1) + bit; symbol |= (bit << 2);
  if( decode_bit( bm[model] ) ) symbol |= 8;
  return symbol;
}

int RangeDecoder::decode_matched(BitModel bm[], int match_byte )
{
  BitModel * const bm1 = bm + 0x100;
  int symbol = 1;
  while( symbol < 0x100 )
    {
    match_byte <<= 1;
    const int match_bit = match_byte & 0x100;
    const int bit = decode_bit( bm1[match_bit+symbol] );
    symbol = ( symbol << 1 ) | bit;
    if( match_bit != bit << 8 )
      {
      while( symbol < 0x100 )
        symbol = ( symbol << 1 ) | decode_bit( bm[symbol] );
      break;
      }
    }
  return symbol & 0xFF;
}

int RangeDecoder::decode_len(LenModel & lm,const int pos_state)
{
  if ( 0 == decode_bit ( lm . choice1 ) )                                  {
    return decode_tree3 ( lm . bm_low [ pos_state ] )                      ;
  }                                                                        ;
  if ( 0 == decode_bit ( lm . choice2 ) )                                  {
    return len_low_symbols + decode_tree3 ( lm . bm_mid [ pos_state ]    ) ;
  }                                                                        ;
  return len_low_symbols + len_mid_symbols + decode_tree8 ( lm . bm_high ) ;
}


//////////////////////////////////////////////////////////////////////////////

bool RangeDecoder::read_block(void)
{
  if ( !at_stream_end ) {
    stream_pos = readblock( infd, buffer, buffer_size );
    if( stream_pos != buffer_size && errno ) {
#pragma message("drop throw Error")
//        throw Error( "Read error" );
    } ;
    at_stream_end = ( stream_pos < buffer_size );
    partial_member_pos += pos;
    pos = 0;
  }
  return pos < stream_pos;
}

int RangeDecoder::readblock(const int fd,quint8 * buf, const int size)
{
#pragma message("readblock is about to be replaced")
  int sz = 0;
#ifdef XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
  errno = 0;
  while( sz < size ) {
    const int n = read( fd, buf + sz, size - sz );
    if( n > 0 ) sz += n;
    else if( n == 0 ) break ; // EOF
    else if( errno != EINTR ) break;
    errno = 0;
  }
#endif
  return sz;
}

//////////////////////////////////////////////////////////////////////////////

END_LZIP_NAMESPACE
QT_END_NAMESPACE
