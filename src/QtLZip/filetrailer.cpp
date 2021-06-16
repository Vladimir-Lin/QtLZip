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

FileTrailer:: FileTrailer(void)
{
}

FileTrailer::~FileTrailer(void)
{
}

int FileTrailer::size(const int version)
{
  return ( ( version >= 1 ) ? 20 : 12 );
}

unsigned FileTrailer::data_crc(void) const
{
  unsigned tmp = 0               ;
  for ( int i = 3; i >= 0; --i ) {
    tmp <<= 8                    ;
    tmp  += data [ i ]           ;
  }                              ;
  return   tmp                   ;
}

void FileTrailer::data_crc(unsigned crc)
{
  for ( int i = 0 ; i <= 3 ; ++i ) {
    data [ i ] = (quint8) crc      ;
    crc      >>= 8                 ;
  }                                ;
}

unsigned long long FileTrailer::data_size(void) const
{
  unsigned long long tmp = 0      ;
  for ( int i = 11; i >= 4; --i ) {
    tmp <<= 8                     ;
    tmp  += data [ i ]            ;
  }                               ;
  return             tmp          ;
}

void FileTrailer::data_size(unsigned long long sz)
{
  for ( int i = 4 ; i <= 11 ; ++i ) {
    data [ i ] = (quint8) sz        ;
    sz       >>= 8                  ;
  }                                 ;
}

unsigned long long FileTrailer::member_size(void) const
{
  unsigned long long tmp = 0         ;
  for ( int i = 19 ; i >= 12 ; --i ) {
    tmp <<= 8                        ;
    tmp  += data [ i ]               ;
  }                                  ;
  return             tmp             ;
}

void FileTrailer::member_size(unsigned long long sz)
{
  for ( int i = 12 ; i <= 19 ; ++i ) {
    data [ i ] = (quint8) sz         ;
    sz       >>= 8                   ;
  }                                  ;
}

//////////////////////////////////////////////////////////////////////////////

END_LZIP_NAMESPACE
QT_END_NAMESPACE
