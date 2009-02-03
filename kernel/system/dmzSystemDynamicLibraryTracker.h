#ifndef DMZ_SYSTEM_DYNAMIC_LIBRARY_TRACKER_DOT_H
#define DMZ_SYSTEM_DYNAMIC_LIBRARY_TRACKER_DOT_H

#include <dmzTypesBase.h>
#include <dmzTypesHashTableString.h>
#include <dmzSystem.h>
#include <dmzSystemSpinLock.h>
#include <dmzSystemStream.h>

namespace {

static dmz::Boolean DumpLoaded = dmz::string_to_boolean (dmz::get_env ("DMZ_DUMP_DYLD"));

static dmz::HashTableString list;

static dmz::SpinLock lock;

static inline void add_dyld (const dmz::String &Name) {

   if (DumpLoaded) {

      lock.lock ();
      list.store (Name, (void *)1);
      lock.unlock ();
   }
}

static inline void dump_dyld (dmz::Stream &out) {

   if (DumpLoaded) {

      dmz::HashTableStringIterator it;

      lock.lock ();

      void *ptr = list.get_next (it);
      while (ptr) {

         out << it.get_hash_key () << dmz::endl;
         ptr = list.get_next (it);
      }

      lock.unlock ();
   }
   else { out << "DMZ_DUMP_DYLD environment variable not set." << dmz::endl; }
}

};

#endif // DMZ_SYSTEM_DYNAMIC_LIBRARY_TRACKER_DOT_H
