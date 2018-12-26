// -*- C++ -*-
//
// This file was generated by CLI, a command line interface
// compiler for C++.
//

#ifndef OPTIONS_HXX
#define OPTIONS_HXX

// Begin prologue.
//
//
// End prologue.

#include <deque>
#include <iosfwd>
#include <string>
#include <cstddef>
#include <exception>

#ifndef CLI_POTENTIALLY_UNUSED
#  if defined(_MSC_VER) || defined(__xlC__)
#    define CLI_POTENTIALLY_UNUSED(x) (void*)&x
#  else
#    define CLI_POTENTIALLY_UNUSED(x) (void)x
#  endif
#endif

namespace cli
{
  class usage_para
  {
    public:
    enum value
    {
      none,
      text,
      option
    };

    usage_para (value);

    operator value () const 
    {
      return v_;
    }

    private:
    value v_;
  };

  class unknown_mode
  {
    public:
    enum value
    {
      skip,
      stop,
      fail
    };

    unknown_mode (value);

    operator value () const 
    {
      return v_;
    }

    private:
    value v_;
  };

  // Exceptions.
  //

  class exception: public std::exception
  {
    public:
    virtual void
    print (::std::wostream&) const = 0;
  };

  ::std::wostream&
  operator<< (::std::wostream&, const exception&);

  class unknown_option: public exception
  {
    public:
    virtual
    ~unknown_option () throw ();

    unknown_option (const std::string& option);

    const std::string&
    option () const;

    virtual void
    print (::std::wostream&) const;

    virtual const char*
    what () const throw ();

    private:
    std::string option_;
  };

  class unknown_argument: public exception
  {
    public:
    virtual
    ~unknown_argument () throw ();

    unknown_argument (const std::string& argument);

    const std::string&
    argument () const;

    virtual void
    print (::std::wostream&) const;

    virtual const char*
    what () const throw ();

    private:
    std::string argument_;
  };

  class missing_value: public exception
  {
    public:
    virtual
    ~missing_value () throw ();

    missing_value (const std::string& option);

    const std::string&
    option () const;

    virtual void
    print (::std::wostream&) const;

    virtual const char*
    what () const throw ();

    private:
    std::string option_;
  };

  class invalid_value: public exception
  {
    public:
    virtual
    ~invalid_value () throw ();

    invalid_value (const std::string& option,
                   const std::string& value,
                   const std::string& message = std::string ());

    const std::string&
    option () const;

    const std::string&
    value () const;

    const std::string&
    message () const;

    virtual void
    print (::std::wostream&) const;

    virtual const char*
    what () const throw ();

    private:
    std::string option_;
    std::string value_;
    std::string message_;
  };

  class eos_reached: public exception
  {
    public:
    virtual void
    print (::std::wostream&) const;

    virtual const char*
    what () const throw ();
  };

  class file_io_failure: public exception
  {
    public:
    virtual
    ~file_io_failure () throw ();

    file_io_failure (const std::string& file);

    const std::string&
    file () const;

    virtual void
    print (::std::wostream&) const;

    virtual const char*
    what () const throw ();

    private:
    std::string file_;
  };

  class unmatched_quote: public exception
  {
    public:
    virtual
    ~unmatched_quote () throw ();

    unmatched_quote (const std::string& argument);

    const std::string&
    argument () const;

    virtual void
    print (::std::wostream&) const;

    virtual const char*
    what () const throw ();

    private:
    std::string argument_;
  };

  // Command line argument scanner interface.
  //
  // The values returned by next() are guaranteed to be valid
  // for the two previous arguments up until a call to a third
  // peek() or next().
  //
  class scanner
  {
    public:
    virtual
    ~scanner ();

    virtual bool
    more () = 0;

    virtual const char*
    peek () = 0;

    virtual const char*
    next () = 0;

    virtual void
    skip () = 0;
  };

  class argv_scanner: public scanner
  {
    public:
    argv_scanner (int& argc, char** argv, bool erase = false);
    argv_scanner (int start, int& argc, char** argv, bool erase = false);

    int
    end () const;

    virtual bool
    more ();

    virtual const char*
    peek ();

    virtual const char*
    next ();

    virtual void
    skip ();

    private:
    int i_;
    int& argc_;
    char** argv_;
    bool erase_;
  };

  class argv_file_scanner: public argv_scanner
  {
    public:
    argv_file_scanner (int& argc,
                       char** argv,
                       const std::string& option,
                       bool erase = false);

    argv_file_scanner (int start,
                       int& argc,
                       char** argv,
                       const std::string& option,
                       bool erase = false);

    struct option_info
    {
      // If search_func is not NULL, it is called, with the arg
      // value as the second argument, to locate the options file.
      // If it returns an empty string, then the file is ignored.
      //
      const char* option;
      std::string (*search_func) (const char*, void* arg);
      void* arg;
    };

    argv_file_scanner (int& argc,
                        char** argv,
                        const option_info* options,
                        std::size_t options_count,
                        bool erase = false);

    argv_file_scanner (int start,
                       int& argc,
                       char** argv,
                       const option_info* options,
                       std::size_t options_count,
                       bool erase = false);

    virtual bool
    more ();

    virtual const char*
    peek ();

    virtual const char*
    next ();

    virtual void
    skip ();

    private:
    const option_info*
    find (const char*) const;

    void
    load (const std::string& file);

    typedef argv_scanner base;

    const std::string option_;
    option_info option_info_;
    const option_info* options_;
    std::size_t options_count_;

    std::deque<std::string> args_;

    // Circular buffer of two arguments.
    //
    std::string hold_[2];
    std::size_t i_;

    bool skip_;
  };

  template <typename X>
  struct parser;
}

#include <cstddef>

#include <types.hxx>

class help_options
{
  public:
  help_options ();

  help_options (int& argc,
                char** argv,
                bool erase = false,
                ::cli::unknown_mode option = ::cli::unknown_mode::fail,
                ::cli::unknown_mode argument = ::cli::unknown_mode::stop);

  help_options (int start,
                int& argc,
                char** argv,
                bool erase = false,
                ::cli::unknown_mode option = ::cli::unknown_mode::fail,
                ::cli::unknown_mode argument = ::cli::unknown_mode::stop);

  help_options (int& argc,
                char** argv,
                int& end,
                bool erase = false,
                ::cli::unknown_mode option = ::cli::unknown_mode::fail,
                ::cli::unknown_mode argument = ::cli::unknown_mode::stop);

  help_options (int start,
                int& argc,
                char** argv,
                int& end,
                bool erase = false,
                ::cli::unknown_mode option = ::cli::unknown_mode::fail,
                ::cli::unknown_mode argument = ::cli::unknown_mode::stop);

  help_options (::cli::scanner&,
                ::cli::unknown_mode option = ::cli::unknown_mode::fail,
                ::cli::unknown_mode argument = ::cli::unknown_mode::stop);

  // Option accessors and modifiers.
  //
  const bool&
  help () const;

  bool&
  help ();

  void
  help (const bool&);

  const bool&
  version () const;

  bool&
  version ();

  void
  version (const bool&);

  const bool&
  proprietary_license () const;

  bool&
  proprietary_license ();

  void
  proprietary_license (const bool&);

  // Print usage information.
  //
  static ::cli::usage_para
  print_usage (::std::wostream&,
               ::cli::usage_para = ::cli::usage_para::none);

  // Implementation details.
  //
  protected:
  bool
  _parse (const char*, ::cli::scanner&);

  private:
  bool
  _parse (::cli::scanner&,
          ::cli::unknown_mode option,
          ::cli::unknown_mode argument);

  public:
  bool help_;
  bool version_;
  bool proprietary_license_;
};

class options
{
  public:
  // Option accessors and modifiers.
  //
  const NarrowStrings&
  disable_warning () const;

  NarrowStrings&
  disable_warning ();

  void
  disable_warning (const NarrowStrings&);

  const std::string&
  options_file () const;

  std::string&
  options_file ();

  void
  options_file (const std::string&);

  const bool&
  show_sloc () const;

  bool&
  show_sloc ();

  void
  show_sloc (const bool&);

  const std::size_t&
  sloc_limit () const;

  std::size_t&
  sloc_limit ();

  void
  sloc_limit (const std::size_t&);

  const bool&
  proprietary_license () const;

  bool&
  proprietary_license ();

  void
  proprietary_license (const bool&);

  const bool&
  preserve_anonymous () const;

  bool&
  preserve_anonymous ();

  void
  preserve_anonymous (const bool&);

  const bool&
  show_anonymous () const;

  bool&
  show_anonymous ();

  void
  show_anonymous (const bool&);

  const NarrowStrings&
  anonymous_regex () const;

  NarrowStrings&
  anonymous_regex ();

  void
  anonymous_regex (const NarrowStrings&);

  const bool&
  anonymous_regex_trace () const;

  bool&
  anonymous_regex_trace ();

  void
  anonymous_regex_trace (const bool&);

  const bool&
  morph_anonymous () const;

  bool&
  morph_anonymous ();

  void
  morph_anonymous (const bool&);

  const NarrowStrings&
  location_map () const;

  NarrowStrings&
  location_map ();

  void
  location_map (const NarrowStrings&);

  const NarrowStrings&
  location_regex () const;

  NarrowStrings&
  location_regex ();

  void
  location_regex (const NarrowStrings&);

  const bool&
  location_regex_trace () const;

  bool&
  location_regex_trace ();

  void
  location_regex_trace (const bool&);

  const bool&
  file_per_type () const;

  bool&
  file_per_type ();

  void
  file_per_type (const bool&);

  const NarrowStrings&
  type_file_regex () const;

  NarrowStrings&
  type_file_regex ();

  void
  type_file_regex (const NarrowStrings&);

  const bool&
  type_file_regex_trace () const;

  bool&
  type_file_regex_trace ();

  void
  type_file_regex_trace (const bool&);

  const NarrowStrings&
  schema_file_regex () const;

  NarrowStrings&
  schema_file_regex ();

  void
  schema_file_regex (const NarrowStrings&);

  const bool&
  schema_file_regex_trace () const;

  bool&
  schema_file_regex_trace ();

  void
  schema_file_regex_trace (const bool&);

  const bool&
  fat_type_file () const;

  bool&
  fat_type_file ();

  void
  fat_type_file (const bool&);

  const NarrowString&
  file_list () const;

  NarrowString&
  file_list ();

  void
  file_list (const NarrowString&);

  const NarrowString&
  file_list_prologue () const;

  NarrowString&
  file_list_prologue ();

  void
  file_list_prologue (const NarrowString&);

  const NarrowString&
  file_list_epilogue () const;

  NarrowString&
  file_list_epilogue ();

  void
  file_list_epilogue (const NarrowString&);

  const NarrowString&
  file_list_delim () const;

  NarrowString&
  file_list_delim ();

  void
  file_list_delim (const NarrowString&);

  const bool&
  disable_multi_import () const;

  bool&
  disable_multi_import ();

  void
  disable_multi_import (const bool&);

  const bool&
  disable_full_check () const;

  bool&
  disable_full_check ();

  void
  disable_full_check (const bool&);

  // Print usage information.
  //
  static ::cli::usage_para
  print_usage (::std::wostream&,
               ::cli::usage_para = ::cli::usage_para::none);

  // Implementation details.
  //
  protected:
  options ();

  bool
  _parse (const char*, ::cli::scanner&);

  public:
  NarrowStrings disable_warning_;
  std::string options_file_;
  bool show_sloc_;
  std::size_t sloc_limit_;
  bool proprietary_license_;
  bool preserve_anonymous_;
  bool show_anonymous_;
  NarrowStrings anonymous_regex_;
  bool anonymous_regex_trace_;
  bool morph_anonymous_;
  NarrowStrings location_map_;
  NarrowStrings location_regex_;
  bool location_regex_trace_;
  bool file_per_type_;
  NarrowStrings type_file_regex_;
  bool type_file_regex_trace_;
  NarrowStrings schema_file_regex_;
  bool schema_file_regex_trace_;
  bool fat_type_file_;
  NarrowString file_list_;
  NarrowString file_list_prologue_;
  NarrowString file_list_epilogue_;
  NarrowString file_list_delim_;
  bool disable_multi_import_;
  bool disable_full_check_;
};

#include <options.ixx>

// Begin epilogue.
//
//
// End epilogue.

#endif // OPTIONS_HXX