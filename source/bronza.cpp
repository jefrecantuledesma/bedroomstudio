/*
 code is Public Domain and written by Peter Semiletov, 2023
*/

#include "lv2/core/lv2.h"


#include <cmath>
#include <stdint.h>
#include <stdlib.h>
#include <iostream>

#include "dsp.h"
#include "fx-resofilter.h"

#define BRONZA_URI "https://github.com/psemiletov/bronza"

typedef enum { MTL_INPUT = 0, MTL_OUTPUT = 1, MTL_LEVEL = 2, MTL_INTENSITY = 3, MTL_WEIGHT = 4} PortIndex;



class CBronza
{
public:

  int samplerate;

  //CResoFilter hp_pre;


  CResoFilter lp;
  CResoFilter hp;
  CResoFilter bp;

  const float* level;
  const float* intensity;
  const float* weight;
  const float* reso;
  const float* warmth;

  const float* input;
  float *output;

  CBronza();
};


CBronza::CBronza()
{
  hp.mode = FILTER_MODE_HIGHPASS;
  lp.reset();

  bp.mode = FILTER_MODE_BANDPASS;
  bp.reset();

  hp.reset();
  hp.set_cutoff (0.50);
}


static void
activate(LV2_Handle instance)
{}


static LV2_Handle
instantiate(const LV2_Descriptor*     descriptor,
            double                    rate,
            const char*               bundle_path,
            const LV2_Feature* const* features)
{
  init_db();
  CBronza *instance = new CBronza;
  instance->samplerate = rate;


  //instance->lp.set_cutoff ((float) 3000.f / rate);

  return (LV2_Handle)instance;
}


static void
connect_port(LV2_Handle instance, uint32_t port, void* data)
{
  CBronza *inst = (CBronza*)instance;

  switch ((PortIndex)port)
         {
          case MTL_INPUT:
                          inst->input = (const float*)data;
                          break;

          case MTL_OUTPUT:
                          inst->output = (float*)data;
                          break;

          case MTL_LEVEL:
                          inst->level = (const float*)data;
                          break;

          case MTL_INTENSITY:
                          inst->intensity = (const float*)data;
                          break;

          case MTL_WEIGHT:
                          inst->weight = (const float*)data;
                          break;

        }
}


static void
run(LV2_Handle instance, uint32_t n_samples)
{
  CBronza *inst = (CBronza*)instance;

  const float* const input  = inst->input;
  float* const       output = inst->output;

  for (uint32_t pos = 0; pos < n_samples; pos++)
      {

       float f = input[pos];
       inst->hp.set_cutoff (0.50f - *(inst->weight));

       f = inst->hp.process (f);
       f = fuzz (f, db2lin (*(inst->level)), *(inst->intensity));

       output[pos] = f;
   }
}


static void deactivate (LV2_Handle instance)
{
}


static void cleanup(LV2_Handle instance)
{
  delete (CBronza*)instance;
}


static const void* extension_data (const char* uri)
{
  return NULL;
}


static const LV2_Descriptor descriptor = {BRONZA_URI,
                                          instantiate,
                                          connect_port,
                                          activate,
                                          run,
                                          deactivate,
                                          cleanup,
                                          extension_data};


LV2_SYMBOL_EXPORT
const LV2_Descriptor* lv2_descriptor (uint32_t index)
{
  return index == 0 ? &descriptor : NULL;
}
