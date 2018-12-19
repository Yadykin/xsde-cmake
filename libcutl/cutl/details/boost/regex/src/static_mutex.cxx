/*
 *
 * Copyright (c) 2004
 * John Maddock
 *
 * Use, modification and distribution are subject to the 
 * Boost Software License, Version 1.0. (See accompanying file 
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
 
 /*
  *   LOCATION:    see http://www.boost.org for most recent version.
  *   FILE         static_mutex.cpp
  *   VERSION      see <cutl/details/boost/version.hpp>
  *   DESCRIPTION: Declares static_mutex lock type.
  */

#define BOOST_REGEX_SOURCE
#include <cutl/details/boost/config.hpp>

#ifdef BOOST_HAS_THREADS

#include <cutl/details/boost/regex/pending/static_mutex.hpp>

#if defined(BOOST_HAS_WINTHREADS)
#ifndef NOMINMAX
#  define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cutl/details/boost/static_assert.hpp>
#endif


namespace cutl_details_boost{

#if defined(BOOST_HAS_PTHREADS) && defined(PTHREAD_MUTEX_INITIALIZER)

scoped_static_mutex_lock::scoped_static_mutex_lock(static_mutex& m, bool lk)
: m_mutex(m), m_have_lock(false)
{
   if(lk)
      lock();
}

scoped_static_mutex_lock::~scoped_static_mutex_lock()
{
   if(m_have_lock)
      unlock();
}

void scoped_static_mutex_lock::lock()
{
   if(0 == m_have_lock)
   {
      pthread_mutex_lock(&(m_mutex.m_mutex));
      m_have_lock = true;
   }
}

void scoped_static_mutex_lock::unlock()
{
   if(m_have_lock)
   {
      pthread_mutex_unlock(&(m_mutex.m_mutex));
      m_have_lock = false;
   }
}

#elif defined(BOOST_HAS_WINTHREADS)

BOOST_STATIC_ASSERT(sizeof(LONG) == sizeof(cutl_details_boost::int32_t));

scoped_static_mutex_lock::scoped_static_mutex_lock(static_mutex& m, bool lk)
: m_mutex(m), m_have_lock(false)
{
   if(lk)
      lock();
}

scoped_static_mutex_lock::~scoped_static_mutex_lock()
{
   if(m_have_lock)
      unlock();
}

void scoped_static_mutex_lock::lock()
{
   if(0 == m_have_lock)
   {
#if !defined(InterlockedCompareExchangePointer)
      while(0 != InterlockedCompareExchange(reinterpret_cast<void**>((cutl_details_boost::uint_least16_t*)&(m_mutex.m_mutex)), (void*)1, 0))
#else
      while(0 != InterlockedCompareExchange(reinterpret_cast<LONG*>(&(m_mutex.m_mutex)), 1, 0))
#endif
      {
         Sleep(0);
      }
      m_have_lock = true;
   }
}

void scoped_static_mutex_lock::unlock()
{
   if(m_have_lock)
   {
#if !defined(InterlockedCompareExchangePointer)
      InterlockedExchange((LONG*)&(m_mutex.m_mutex), 0);
#else
      InterlockedExchange(reinterpret_cast<LONG*>(&(m_mutex.m_mutex)), 0);
#endif
      m_have_lock = false;
   }
}

#else
//
// Portable version of a static mutex based on Boost.Thread library:
//
#include <stdlib.h>
#include <cutl/details/boost/assert.hpp>

cutl_details_boost::recursive_mutex* static_mutex::m_pmutex = 0;
cutl_details_boost::once_flag static_mutex::m_once = BOOST_ONCE_INIT;

extern "C" BOOST_REGEX_DECL void cutl_details_boost_regex_free_static_mutex()
{
   delete static_mutex::m_pmutex;
   static_mutex::m_pmutex = 0;
}

void static_mutex::init()
{
   m_pmutex = new cutl_details_boost::recursive_mutex();
   int r = atexit(cutl_details_boost_regex_free_static_mutex);
   BOOST_ASSERT(0 == r);
}

scoped_static_mutex_lock::scoped_static_mutex_lock(static_mutex& , bool lk)
: m_plock(0), m_have_lock(false)
{
   if(lk)
      lock();
}

scoped_static_mutex_lock::~scoped_static_mutex_lock()
{
   if(m_have_lock)
      unlock();
   delete m_plock;
}

void scoped_static_mutex_lock::lock()
{
   if(0 == m_have_lock)
   {
       cutl_details_boost::call_once(static_mutex::m_once,&static_mutex::init);
      if(0 == m_plock)
         m_plock = new cutl_details_boost::recursive_mutex::scoped_lock(*static_mutex::m_pmutex, cutl_details_boost::defer_lock);
      m_plock->lock();
      m_have_lock = true;
   }
}

void scoped_static_mutex_lock::unlock()
{
   if(m_have_lock)
   {
      m_plock->unlock();
      m_have_lock = false;
   }
}

#endif

}

#endif // BOOST_HAS_THREADS
