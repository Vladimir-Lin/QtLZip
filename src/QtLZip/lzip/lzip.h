struct Error
  {
  const char * const msg;
  explicit Error( const char * const s ) : msg( s ) {}
  };

class Pretty_print
  {
  std::string name_;
  const char * const stdin_name;
  unsigned longest_name;
  mutable bool first_post;

public:
  explicit Pretty_print( const std::vector< std::string > & filenames )
    : stdin_name( "(stdin)" ), longest_name( 0 ), first_post( false )
    {
    const unsigned stdin_name_len = std::strlen( stdin_name );
    for( unsigned i = 0; i < filenames.size(); ++i )
      {
      const std::string & s = filenames[i];
      const unsigned len = ( s == "-" ) ? stdin_name_len : s.size();
      if( len > longest_name ) longest_name = len;
      }
    if( longest_name == 0 ) longest_name = stdin_name_len;
    }

  void set_name( const std::string & filename )
    {
    if( filename.size() && filename != "-" ) name_ = filename;
    else name_ = stdin_name;
    first_post = true;
    }

  void reset() const { if( name_.size() ) first_post = true; }
  const char * name() const { return name_.c_str(); }
  void operator()( const char * const msg = 0 ) const;
  };

// defined in decoder.cc
int readblock( const int fd, uint8_t * const buf, const int size );
int writeblock( const int fd, const uint8_t * const buf, const int size );

// defined in main.cc
extern int verbosity;
void show_error( const char * const msg, const int errcode = 0,
                 const bool help = false );
