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
 * URL      : http://qtucl.sourceforge.net/
 *
 * QtLZip acts as an interface between Qt and LZip library.
 * Please keep QtLZip as simple as possible.
 *
 * Copyright 2001 ~ 2016
 *
 ****************************************************************************/

#include <qtlzip.h>

QT_BEGIN_NAMESPACE

#ifndef QT_STATIC

ScriptableLZip:: ScriptableLZip ( QObject * parent )
               : QObject        (           parent )
               , QScriptable    (                  )
               , QtLZip         (                  )
{
}

ScriptableLZip::~ScriptableLZip (void)
{
}

bool ScriptableLZip::ToLZip(QString file,QString lzip,int level,int method)
{
  return FileToLZip ( file , lzip , level , method ) ;
}

bool ScriptableLZip::ToFile(QString lzip,QString file)
{
  return LZipToFile ( lzip , file ) ;
}

QScriptValue ScriptableLZip::Attachment(QScriptContext * context,QScriptEngine * engine)
{
  ScriptableLZip * lzip = new ScriptableLZip ( engine ) ;
  return engine -> newQObject                ( lzip   ) ;
}

#endif

QT_END_NAMESPACE
