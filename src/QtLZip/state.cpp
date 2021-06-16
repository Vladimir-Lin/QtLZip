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

State:: State (void)
      : state (0   )
{
}

State::~State(void)
{
}

int State::operator ( ) (void) const
{
  return state ;
}

bool State::is_char(void) const
{
  return state < 7 ;
}

void State::set_char(void)
{
  static const int next [ states ] = { 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 4, 5 } ;
  state = next          [ state  ]                                          ;
}

void State::set_char1(void)
{
  state -= ( state < 4 ) ? state : 3 ;
}

void State::set_char2(void)
{
  state -= ( state < 10 ) ? 3 : 6 ;
}

void State::set_match(void)
{
  state = ( state < 7 ) ? 7 : 10 ;
}

void State::set_rep(void)
{
  state = ( state < 7 ) ? 8 : 11 ;
}

void State::set_short_rep(void)
{
  state = ( state < 7 ) ? 9 : 11 ;
}

//////////////////////////////////////////////////////////////////////////////

END_LZIP_NAMESPACE
QT_END_NAMESPACE
