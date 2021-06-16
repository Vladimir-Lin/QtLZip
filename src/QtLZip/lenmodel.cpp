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

LenModel:: LenModel(void)
{
}

LenModel::~LenModel(void)
{
}

void LenModel::reset(void)
{
  choice1             . reset (                              ) ;
  choice2             . reset (                              ) ;
  bm_low  [ 0 ] [ 0 ] . reset ( pos_states * len_low_symbols ) ;
  bm_mid  [ 0 ] [ 0 ] . reset ( pos_states * len_mid_symbols ) ;
  bm_high [ 0       ] . reset ( len_high_symbols             ) ;
}

//////////////////////////////////////////////////////////////////////////////

END_LZIP_NAMESPACE
QT_END_NAMESPACE
