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

#include <qtlzip.h>

#define _FILE_OFFSET_BITS 64

#include <algorithm>
#include <cerrno>
#include <climits>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <fcntl.h>
#include <stdint.h>

#include "lzip.h"
#include "encoder_base.h"
#include "encoder.h"
#include "fast_encoder.h"

QT_BEGIN_NAMESPACE

//////////////////////////////////////////////////////////////////////////////

unsigned char  lzipmagic [ 4 ] = { 0x4C, 0x5A, 0x49, 0x50 } ; // LZIP
LZipLib::CRC32 lcrc32 ;

//////////////////////////////////////////////////////////////////////////////

QtLZip:: QtLZip     (void)
       : lzipPacket (NULL)
{
}

QtLZip::~QtLZip (void)
{
  CleanUp ( ) ;
}

bool QtLZip::isLZip (QByteArray & header)
{
  return false ;
}

bool QtLZip::IsCorrect(int returnCode)
{
  return false ;
}

bool QtLZip::IsEnd(int returnCode)
{
  return ( returnCode > 0 ) ;
}

bool QtLZip::IsFault(int returnCode)
{
  return false ;
}

void QtLZip::CleanUp(void)
{
  if ( NULL == lzipPacket ) return ;
}

void QtLZip::CompressHeader(QByteArray & Compressed)
{
}

void QtLZip::CompressTail(unsigned int checksum,QByteArray & Compressed)
{
}

int QtLZip::DecompressHeader(const QByteArray & Source)
{
  return 0 ;
}

int QtLZip::BeginCompress(QVariantList arguments)
{
  return 0 ;
}

int QtLZip::BeginCompress(int level,int method)
{
  QVariantList args             ;
  args << level                 ;
  args << method                ;
  return BeginCompress ( args ) ;
}

int QtLZip::doCompress(const QByteArray & Source,QByteArray & Compressed)
{
  return 0 ;
}

int QtLZip::doSection(QByteArray & Source,QByteArray & Compressed)
{
  return 0 ;
}

int QtLZip::CompressDone(QByteArray & Compressed)
{
  return 0 ;
}

int QtLZip::BeginDecompress(void)
{
  return 0 ;
}

int QtLZip::doDecompress(const QByteArray & Source,QByteArray & Decompressed)
{
  return 0 ;
}

int QtLZip::undoSection(QByteArray & Source,QByteArray & Decompressed)
{
  return 0 ;
}

int QtLZip::DecompressDone(void)
{
  return 0 ;
}

bool QtLZip::IsTail(QByteArray & header)
{
  if ( NULL == lzipPacket    ) return false                     ;
  if ( header . size ( ) < 8 ) return false                     ;
  unsigned char * footer  = (unsigned char *) header . data ( ) ;
  bool            correct = true                                ;
  return correct                                                ;
}

//////////////////////////////////////////////////////////////////////////////

bool ToLZip(const QByteArray & data,QByteArray & lzip,int level,int method)
{
  if ( data . size ( ) <= 0 ) return false ;
  //////////////////////////////////////////
  QtLZip       L                           ;
  int          r                           ;
  QVariantList v                           ;
  v << level                               ;
  v << method                              ;
  r = L . BeginCompress ( v )              ;
  if ( L . IsCorrect ( r ) )               {
    L . doCompress   ( data , lzip )       ;
    L . CompressDone (        lzip )       ;
  }                                        ;
  //////////////////////////////////////////
  return ( lzip . size ( ) > 0 )           ;
}

//////////////////////////////////////////////////////////////////////////////

bool FromLZip(const QByteArray & lzip,QByteArray & data)
{
  if ( lzip . size ( ) <= 0 ) return false ;
  //////////////////////////////////////////
  QtLZip L                                 ;
  int   r                                  ;
  r = L . BeginDecompress ( )              ;
  if ( L . IsCorrect ( r ) )               {
    L . doDecompress   ( lzip , data )     ;
    L . DecompressDone (             )     ;
  }                                        ;
  //////////////////////////////////////////
  return ( data . size ( ) > 0 )           ;
}

bool SaveLZip (QString filename,QByteArray & data,int level,int method)
{
  if ( data . size ( ) <= 0 ) return false                         ;
  QByteArray lzip                                                  ;
  if ( level < 0 ) level = 9                                       ;
  if ( ! ToLZip ( data , lzip , level , method ) ) return false    ;
  if ( lzip . size ( ) <= 0                      ) return false    ;
  QFile F ( filename )                                             ;
  if ( ! F . open ( QIODevice::WriteOnly | QIODevice::Truncate ) ) {
    return false                                                   ;
  }                                                                ;
  F . write ( lzip )                                               ;
  F . close (      )                                               ;
  return true                                                      ;
}

bool LoadLZip (QString filename,QByteArray & data)
{
  QFile F ( filename )                                   ;
  if ( ! F . open ( QIODevice::ReadOnly ) ) return false ;
  QByteArray lzip                                        ;
  lzip = F . readAll ( )                                 ;
  F . close          ( )                                 ;
  if ( lzip . size ( ) <= 0 ) return false               ;
  if ( ! FromLZip ( lzip , data ) ) return false         ;
  return ( data . size ( ) > 0 )                         ;
}

bool FileToLZip (QString filename,QString lzip,int level,int method)
{
  QFile F ( filename )                                   ;
  if ( ! F . open ( QIODevice::ReadOnly ) ) return false ;
  QByteArray data                                        ;
  data = F . readAll ( )                                 ;
  F . close ( )                                          ;
  if ( data . size ( ) <= 0 ) return false               ;
  return SaveLZip ( lzip , data , level , method )       ;
}

bool LZipToFile (QString lzip,QString filename)
{
  QByteArray data                                        ;
  if ( ! LoadLZip ( lzip , data ) ) return false         ;
  if ( data . size ( ) <=0        ) return false         ;
  QFile F ( filename )                                   ;
  if ( ! F . open ( QIODevice::WriteOnly                 |
                    QIODevice::Truncate ) ) return false ;
  F . write ( data )                                     ;
  F . close (      )                                     ;
  return true                                            ;
}

QT_END_NAMESPACE
