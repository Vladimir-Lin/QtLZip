/****************************************************************************
 *
 * Copyright (C) 2001~2016 Neutrino International Inc.
 *
 * Author   : Brian Lin ( Foxman , Vladimir Lin , Vladimir Forest )
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

#ifndef QT_LZIP_H
#define QT_LZIP_H

#include <QtCore>
#ifndef QT_STATIC
#include <QtScript>
#endif

QT_BEGIN_NAMESPACE

#ifndef QT_STATIC
#  if defined(QT_BUILD_QTLZIP_LIB)
#    define Q_LZIP_EXPORT Q_DECL_EXPORT
#  else
#    define Q_LZIP_EXPORT Q_DECL_IMPORT
#  endif
#else
#    define Q_LZIP_EXPORT
#endif

#define QT_LZIP_LIB          1

#define BEGIN_LZIP_NAMESPACE namespace LZipLib {
#define END_LZIP_NAMESPACE   }

class Q_LZIP_EXPORT QtLZip         ;
#ifndef QT_STATIC
class Q_LZIP_EXPORT ScriptableLZip ;
#endif

#define LZIP_CRC32_BASE 0xEDB88320U

enum                                                         {
  min_dictionary_bits  = 12                                  ,
  min_dictionary_size  =  1 << min_dictionary_bits           , // >= modeled_distances
  max_dictionary_bits  = 29                                  ,
  max_dictionary_size  =  1 << max_dictionary_bits           ,

  literal_context_bits =  3                                  ,
  literal_context_size = 1 << literal_context_bits           ,

  pos_state_bits       =  2                                  ,
  pos_states           =  1 << pos_state_bits                ,
  pos_state_mask       = pos_states - 1                      ,

  len_states           =  4                                  ,
  dis_slot_bits        =  6                                  ,
  dis_slot_size        =  1 << dis_slot_bits                 ,

  start_dis_model      =  4                                  ,
  end_dis_model        = 14                                  ,
  modeled_distances    =  1 << ( end_dis_model / 2 )         , // 128
  dis_align_bits       =  4                                  ,
  dis_align_size       =  1 << dis_align_bits                ,

  len_low_bits         =  3                                  ,
  len_mid_bits         =  3                                  ,
  len_high_bits        =  8                                  ,
  len_low_symbols      =  1 << len_low_bits                  ,
  len_mid_symbols      =  1 << len_mid_bits                  ,
  len_high_symbols     =  1 << len_high_bits                 ,
  max_len_symbols      = len_low_symbols                     +
                         len_mid_symbols                     +
                         len_high_symbols                    ,

  min_match_len        =  2                                  , // must be 2
  max_match_len        = min_match_len + max_len_symbols - 1 , // 273
  min_match_len_limit  =  5                                  ,

  bit_model_move_bits  =  5                                  ,
  bit_model_total_bits = 11                                  ,
  bit_model_total      =  1 << bit_model_total_bits          ,

  price_shift_bits     =  6                                  ,
  price_step_bits      =  2                                  ,
  price_step           =  1 << price_step_bits

}                                                            ;

#ifdef QT_BUILD_QTLZIP_LIB

inline int get_len_state(const int len)
{
  return std::min ( len - min_match_len , len_states - 1 ) ;
}

inline int get_lit_state(const quint8 prev_byte)
{
  return ( prev_byte >> ( 8 - literal_context_bits ) ) ;
}

inline int real_bits(unsigned value)
{
  int bits = 0        ;
  while ( value > 0 ) {
    value >>= 1       ;
    ++bits            ;
  }                   ;
  return bits         ;
}

#endif

BEGIN_LZIP_NAMESPACE

class Q_LZIP_EXPORT CRC32
{
  public:

    explicit     CRC32        (void) ;
    virtual     ~CRC32        (void) ;

    quint32      operator [ ] (quint8 index) const ;

    virtual void update       (quint32 & crc,const quint8 byte) const ;
    virtual void update       (quint32 & crc,const quint8 * const buffer,const int size) const ;

  protected:

    quint32 data [ 256 ] ; // Table of CRCs of all 8-bit messages.

    virtual void Configure    (void) ;

  private:

} ;

class Q_LZIP_EXPORT State
{
  public:

    enum { states = 12 } ;

    explicit     State         (void) ;
    virtual     ~State         (void) ;

    virtual int  operator ( )  (void) const ;
    virtual bool is_char       (void) const ;

    virtual void set_char      (void) ;
    virtual void set_char1     (void) ;
    virtual void set_char2     (void) ;
    virtual void set_match     (void) ;
    virtual void set_rep       (void) ;
    virtual void set_short_rep (void) ;

  protected:

    int state ;

  private:

} ;

struct Q_LZIP_EXPORT BitModel
{
  public:

    int probability ;

    explicit BitModel (void) ;
    virtual ~BitModel (void) ;

    virtual void reset (void) ;
    virtual void reset (const int size) ;

  protected:

  private:

} ;

struct Q_LZIP_EXPORT LenModel
{
  public:

    BitModel choice1                                    ;
    BitModel choice2                                    ;
    BitModel bm_low  [ pos_states ] [ len_low_symbols ] ;
    BitModel bm_mid  [ pos_states ] [ len_mid_symbols ] ;
    BitModel bm_high [ len_high_symbols               ] ;

    explicit     LenModel (void) ;
    virtual     ~LenModel (void) ;

    virtual void reset    (void) ;

  protected:

  private:

} ;

struct Q_LZIP_EXPORT FileHeader
{
  public:

    enum { size = 6 } ;

    quint8 data [ size ] ; // 0-3 magic bytes
                           //   4 version
                           //   5 coded_dict_size

    explicit             FileHeader      (void) ;
    virtual             ~FileHeader      (void) ;

    virtual void         set_magic       (void) ;
    virtual bool         verify_magic    (void) const ;
    virtual quint8       version         (void) const ;
    virtual bool         verify_version  (void) const ;
    virtual unsigned int dictionary_size (void) const ;
    virtual bool         dictionary_size (const unsigned int sz) ;

  protected:

  private:

} ;

struct Q_LZIP_EXPORT FileTrailer
{
  public:

    quint8 data [ 20 ] ; //  0-3  CRC32 of the uncompressed data
                         //  4-11 size of the uncompressed data
                         // 12-19 member size including header and trailer

    explicit                   FileTrailer (void) ;
    virtual                   ~FileTrailer (void) ;

    static  int                size        (const int version = 1) ;
    virtual unsigned           data_crc    (void) const ;
    virtual void               data_crc    (unsigned crc) ;
    virtual unsigned long long data_size   (void) const ;
    virtual void               data_size   (unsigned long long sz) ;
    virtual unsigned long long member_size (void) const ;
    virtual void               member_size (unsigned long long sz) ;

  protected:

  private:

} ;

class Q_LZIP_EXPORT RangeDecoder
{
  public:

    enum { buffer_size = 16384 } ;

    unsigned long long partial_member_pos ;
    quint8 *           buffer             ; // input buffer
    int                pos                ; // current pos in buffer
    int                stream_pos         ; // when reached, a new block must be read
    quint32            code               ;
    quint32            range              ;
    const int          infd               ; // input file descriptor
    bool               at_stream_end      ;

    explicit           RangeDecoder          (void) ;
    explicit           RangeDecoder          (const int ifd) ;
    virtual           ~RangeDecoder          (void) ;

    bool               code_is_zero          (void) const ;
    bool               finished              (void) ;
    unsigned long long member_position       (void) const ;
    void               reset_member_position (void) ;
    quint8             get_byte              (void) ;
    int                read_data             (quint8 * outbuf,const int size) ;
    void               load                  (void) ;
    void               normalize             (void) ;
    int                decode                (const int num_bits) ;
    int                decode_bit            (BitModel & bm    ) ;
    int                decode_tree3          (BitModel   bm [ ]) ;
    int                decode_tree6          (BitModel   bm [ ]) ;
    int                decode_tree8          (BitModel   bm [ ]) ;
    int                decode_tree_reversed  (BitModel   bm [ ], const int num_bits) ;
    int                decode_tree_reversed4 (BitModel   bm [ ]) ;
    int                decode_matched        (BitModel   bm [ ],int match_byte) ;
    int                decode_len            (LenModel & lm,const int pos_state) ;

  protected:

    bool               read_block            (void) ;
    int                readblock             (const int fd     ,
                                              quint8  * buf    ,
                                              const int size ) ;

  private:

} ;

class Decoder
{
  public:

    explicit           Decoder (const FileHeader   & header ,
                                      RangeDecoder & rde    ,
                                const int            ofd  ) ;
    virtual           ~Decoder (void) ;

    unsigned           crc           (void) const ;
    unsigned long long data_position (void) const ;

    int decode_member (void) ;

  protected:

    unsigned long long partial_data_pos ;
    RangeDecoder     & rdec             ;
    const unsigned     dictionary_size  ;
    const int          buffer_size      ;
    quint8           * buffer           ; // output buffer
    int                pos              ; // current pos in buffer
    int                stream_pos       ; // first byte not yet written to file
    quint32            crc_             ;
    const int          outfd            ; // output file descriptor
    const int          member_version   ;

    bool verify_trailer(void) const ;
    void flush_data();
    quint8 peek_prev() const ;

    quint8 peek( const int distance ) const ;
    void put_byte( const quint8 b ) ;
    void copy_block( const int distance, int len ) ;

    int readblock( const int fd, quint8 * buf, const int size ) ;
    int writeblock( const int fd, const quint8 * buf, const int size ) ;

  private:

} ;

END_LZIP_NAMESPACE

class Q_LZIP_EXPORT QtLZip
{
  public:

    explicit     QtLZip           (void) ;
    virtual     ~QtLZip           (void) ;

    virtual bool isLZip           (QByteArray & header) ;

    virtual void CleanUp          (void) ;

    virtual bool IsCorrect        (int returnCode) ;
    virtual bool IsEnd            (int returnCode) ;
    virtual bool IsFault          (int returnCode) ;

    // Compression functions

    virtual int  BeginCompress    (int level = 9,int method = 0) ;
    virtual int  BeginCompress    (QVariantList arguments = QVariantList() ) ;
    virtual int  doCompress       (const QByteArray & Source      ,
                                         QByteArray & Compressed) ;
    virtual int  doSection        (      QByteArray & Source      ,
                                         QByteArray & Compressed) ;
    virtual int  CompressDone     (QByteArray & Compressed) ;

    // Decompression functions

    virtual int  BeginDecompress  (void) ;
    virtual int  doDecompress     (const QByteArray & Source        ,
                                         QByteArray & Decompressed) ;
    virtual int  undoSection      (      QByteArray & Source        ,
                                         QByteArray & Decompressed) ;
    virtual int  DecompressDone   (void) ;

    virtual bool IsTail           (QByteArray & header) ;

  protected:

    void * lzipPacket ;

    virtual void CompressHeader   (QByteArray & Compressed) ;
    virtual void CompressTail     (unsigned int checksum,QByteArray & Compressed) ;
    virtual int  DecompressHeader (const QByteArray & Source) ;

  private:

} ;

#ifndef QT_STATIC

class Q_LZIP_EXPORT ScriptableLZip : public QObject
                                   , public QScriptable
                                   , public QtLZip
{
  Q_OBJECT
  public:

    static QScriptValue Attachment     (QScriptContext * context,QScriptEngine * engine) ;

    explicit            ScriptableLZip (QObject * parent) ;
    virtual            ~ScriptableLZip (void) ;

  protected:

  private:

  public slots:

    virtual bool        ToLZip         (QString file,QString lzip,int level = 9,int method = 0) ;
    virtual bool        ToFile         (QString lzip,QString file) ;

  protected slots:

  private slots:

  signals:

} ;

#endif

Q_LZIP_EXPORT bool ToLZip     (const QByteArray & data,QByteArray & lzip,int level = 9,int method = 0) ;
Q_LZIP_EXPORT bool FromLZip   (const QByteArray & lzip,QByteArray & data) ;
Q_LZIP_EXPORT bool SaveLZip   (QString filename,QByteArray & data,int level = 9,int method = 0) ;
Q_LZIP_EXPORT bool LoadLZip   (QString filename,QByteArray & data) ;
Q_LZIP_EXPORT bool FileToLZip (QString file,QString lzip,int level = 9,int method = 0) ;
Q_LZIP_EXPORT bool LZipToFile (QString lzip,QString file) ;

QT_END_NAMESPACE

#endif
