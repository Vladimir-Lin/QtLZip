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

extern unsigned char lzipmagic [ 4 ] ;

inline int bits_real(unsigned value)
{
  int bits = 0        ;
  while ( value > 0 ) {
    value >>= 1       ;
    ++bits            ;
  }                   ;
  return bits         ;
}

BEGIN_LZIP_NAMESPACE

//////////////////////////////////////////////////////////////////////////////

FileHeader:: FileHeader(void)
{
}

FileHeader::~FileHeader(void)
{
}

void FileHeader::set_magic(void)
{
  ::memcpy ( data , lzipmagic , 4 ) ;
  data [ 4 ] = 1                    ;
}

bool FileHeader::verify_magic(void) const
{
  return ( 0 == memcmp( data, lzipmagic, 4 ) ) ;
}

quint8 FileHeader::version(void) const
{
  return data [ 4 ] ;
}

bool FileHeader::verify_version(void) const
{
  return ( data [ 4 ] <= 1 ) ;
}

unsigned int FileHeader::dictionary_size(void) const
{
  unsigned int sz = ( 1 << ( data [ 5 ] & 0x1F ) )  ;
  if ( sz > min_dictionary_size )                   {
    sz -= ( sz / 16 ) * ( ( data [ 5 ] >> 5 ) & 7 ) ;
  }                                                 ;
  return sz                                         ;
}

bool FileHeader::dictionary_size(const unsigned int sz)
{
  if ( ( sz >= min_dictionary_size ) && ( sz <= max_dictionary_size ) ) {
    data [ 5 ] = bits_real ( sz - 1 )                                   ;
    if ( sz > min_dictionary_size )                                     {
      const unsigned base_size = 1 << data [ 5 ]                        ;
      const unsigned fraction  = base_size / 16                         ;
      for ( int i = 7 ; i >= 1 ; --i )                                  {
        if ( base_size - ( i * fraction ) >= sz )                       {
          data [ 5 ] |= ( i << 5 )                                      ;
          break                                                         ;
        }                                                               ;
      }                                                                 ;
    }                                                                   ;
    return true                                                         ;
  }                                                                     ;
  return   false                                                        ;
}

//////////////////////////////////////////////////////////////////////////////

END_LZIP_NAMESPACE
QT_END_NAMESPACE
