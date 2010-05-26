#include "dmzEntityPluginWheels.h"
#include <dmzObjectAttributeMasks.h>
#include <dmzObjectConsts.h>
#include <dmzObjectModule.h>
#include <dmzRuntimeConfig.h>
#include <dmzRuntimeConfigToTypesBase.h>
#include <dmzRuntimePluginFactoryLinkSymbol.h>
#include <dmzRuntimePluginInfo.h>
#include <dmzRuntimeObjectType.h>
#include <dmzTypesMatrix.h>
#include <dmzTypesVector.h>

dmz::EntityPluginWheels::EntityPluginWheels (const PluginInfo &Info, Config &local) :
      Plugin (Info),
      TimeSlice (Info),
      ObjectObserverUtil (Info, local),
      _log (Info),
      _defs (Info),
      _defaultAttr (0) {

   _init (local);
}


dmz::EntityPluginWheels::~EntityPluginWheels () {

   _wheelTable.empty ();
   _objTable.empty ();
}


// Plugin Interface
void
dmz::EntityPluginWheels::update_plugin_state (
      const PluginStateEnum State,
      const UInt32 Level) {

   if (State == PluginStateInit) {

   }
   else if (State == PluginStateStart) {

   }
   else if (State == PluginStateStop) {

   }
   else if (State == PluginStateShutdown) {

   }
}


void
dmz::EntityPluginWheels::discover_plugin (
      const PluginDiscoverEnum Mode,
      const Plugin *PluginPtr) {

   if (Mode == PluginDiscoverAdd) {

   }
   else if (Mode == PluginDiscoverRemove) {

   }
}


// Time Slice Interface
void
dmz::EntityPluginWheels::update_time_slice (const Float64 DeltaTime) {

   ObjectModule *module = get_object_module ();

   if (module) {

      HashTableHandleIterator it;
      ObjectStruct *os (0);

      while (_objTable.get_next (it, os)) {

         Matrix ori;
         Vector vel;

         module->lookup_velocity (os->Object, _defaultAttr, vel);
         module->lookup_orientation (os->Object, _defaultAttr, ori);

         Float64 speed = vel.magnitude ();

         if (!is_zero64 (speed)) {

            Vector dir (0.0, 0.0, -1.0);
            ori.transform_vector (dir);

            if (dir.get_angle (vel) > HalfPi64) { speed = -speed; }

            const Float64 Distance = speed * DeltaTime;

            WheelStruct *wheel (os->wheels);

            while (wheel) {

               Float64 value (0.0);

               module->lookup_scalar (os->Object, wheel->Attr, value);

               value -= (Distance * wheel->InvertRadius * wheel->Mod);
               if (value > Pi64) { value -= TwoPi64; }
               else if (value < -Pi64) { value += TwoPi64; }

               module->store_scalar (os->Object, wheel->Attr, value);

               wheel = wheel->next;
            }
         }
      }
   }
}


// Object Observer Interface
void
dmz::EntityPluginWheels::create_object (
      const UUID &Identity,
      const Handle ObjectHandle,
      const ObjectType &Type,
      const ObjectLocalityEnum Locality) {

   WheelStruct *ws = _lookup_wheels_def (Type);

   if (ws) {

      ObjectStruct *os = new ObjectStruct (ObjectHandle, ws);

      if (os && !_objTable.store (ObjectHandle, os)) { delete os; os = 0; }
   }
}


void
dmz::EntityPluginWheels::destroy_object (
      const UUID &Identity,
      const Handle ObjectHandle) {

   ObjectStruct *os = _objTable.remove (ObjectHandle);

   if (os) { delete os; os = 0; }
}


dmz::EntityPluginWheels::WheelStruct *
dmz::EntityPluginWheels::_lookup_wheels_def (const ObjectType &Type) {

   WheelStruct *result (0);

   ObjectType current (Type);

   while (!result && current) {

      result = _wheelTable.lookup (current.get_handle ());

      if (!result) { result = _create_wheels_def (current); }

      current.become_parent ();
   }

   return result;
}


dmz::EntityPluginWheels::WheelStruct *
dmz::EntityPluginWheels::_create_wheels_def (const ObjectType &Type) {

   WheelStruct *result (0);

   Config list;

   if (Type.get_config ().lookup_all_config ("wheels.attribute", list)) {

      ConfigIterator it;
      Config attr;

      const Float64 DefaultRadius = config_to_float64 (
         "wheels.radius",
         Type.get_config (),
         1.0);

      while (list.get_next_config (it, attr)) {

         const Handle ScalarAttr = _defs.create_named_handle (
            config_to_string ("name", attr));

         const Float64 Radius = config_to_float64 ("radius", attr, DefaultRadius);
         const Float64 Mod = config_to_float64 ("modifier", attr, 1.0);

         if (Radius > 0.0) {

            WheelStruct *ws = new WheelStruct (ScalarAttr, 1.0 / Radius, Mod);

            if (ws) {

               ws->next = result;
               result = ws;
            }
         }
         else {

            _log.error << "Radius of wheel: "
               << _defs.lookup_named_handle_name (ScalarAttr)
               << " of type: " << Type.get_name ()
               << " is less than or equal to zero: " << Radius << endl;
         }
      }
   }

   return result;
}


void
dmz::EntityPluginWheels::_init (Config &local) {

   _defaultAttr = _defs.create_named_handle (ObjectAttributeDefaultName);

   activate_default_object_attribute (ObjectCreateMask | ObjectDestroyMask);
}


extern "C" {

DMZ_PLUGIN_FACTORY_LINK_SYMBOL dmz::Plugin *
create_dmzEntityPluginWheels (
      const dmz::PluginInfo &Info,
      dmz::Config &local,
      dmz::Config &global) {

   return new dmz::EntityPluginWheels (Info, local);
}

};
