#ifndef event_HPP
#define event_HPP 1 
//#ifdef TO_ROOT
#include "TObject.h"
#include "TClass.h"
struct adc : public TObject{
  unsigned short m_adc[1024];
  ClassDef(adc,1)
};
struct VL_CRtest_BgoRaw : public TObject{
  float evt_time;
  unsigned int id;
  std::vector<unsigned int> channels_id;
  std::vector<adc> channels_adc;
  ClassDef(VL_CRtest_BgoRaw,1)
};

//#else
//#warning "..."
//#endif
#endif
