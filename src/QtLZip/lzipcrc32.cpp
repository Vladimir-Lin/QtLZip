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

CRC32:: CRC32(void)
{
  Configure ( ) ;
}

CRC32::~CRC32(void)
{
}

void CRC32::Configure(void)
{
  for   ( quint32 n = 0 ; n < 256 ; ++n )  {
    quint32 c = n                          ;
    for ( int     k = 0 ; k < 8   ; ++k )  {
      if ( c & 1 )                         {
        c   = LZIP_CRC32_BASE ^ ( c >> 1 ) ;
      } else                               {
        c >>= 1                            ;
      }                                    ;
    }                                      ;
    ////////////////////////////////////////
    data [ n ] = c                         ;
  }                                        ;
}

quint32 CRC32::operator [ ] (quint8 index) const
{
  return data [ index ] ;
}

#define UPDATECRC(crc,byte) crc = data [ ( crc ^ byte ) & 0xFF ] ^ ( crc >> 8 )

void CRC32::update(quint32 & crc,const quint8 byte) const
{
  UPDATECRC ( crc , byte ) ;
}

void CRC32::update                  (
             quint32 &       crc    ,
       const quint8  * const buffer ,
       const int             size   ) const
{
  for (int i = 0 ; i < size ; ++i )  {
    UPDATECRC ( crc , buffer [ i ] ) ;
  }                                  ;
}

//////////////////////////////////////////////////////////////////////////////

END_LZIP_NAMESPACE
QT_END_NAMESPACE
